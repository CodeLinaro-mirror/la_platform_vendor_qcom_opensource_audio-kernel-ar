// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2013-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/export.h>
#include <linux/ion.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#ifndef CONFIG_SPF_CORE
#include <ipc/apr.h>
#endif
#include <dsp/msm_audio_ion.h>
#include <linux/msm_audio.h>
#include <soc/qcom/secure_buffer.h>

// MDF subsystems VMID
#define VMID_SSC_Q6  (0x5)
#define VMID_ADSP_Q6 (0x6)
#define VMID_MSS_MBA (0xf)

#define MSM_AUDIO_ION_PROBED (1 << 0)

#define MSM_AUDIO_ION_PHYS_ADDR(alloc_data) \
	alloc_data->table->sgl->dma_address

#define MSM_AUDIO_SMMU_SID_OFFSET 32

#define TZ_PIL_PROTECT_MEM_SUBSYS_ID 0x0C
#define TZ_PIL_CLEAR_PROTECT_MEM_SUBSYS_ID 0x0D

#define MINOR_NUMBER_COUNT 1
struct msm_audio_ion_private {
	bool smmu_enabled;
	struct device *cb_dev;
	u8 device_status;
	struct list_head alloc_list;
	struct mutex list_mutex;
	u64 smmu_sid_bits;
	u32 smmu_version;
	bool is_non_hypervisor;
	char *driver_name;
	/*char dev related data */
	dev_t ion_major;
	struct class *ion_class;
	struct device *chardev;
	struct cdev cdev;
};

struct msm_audio_alloc_data {
	size_t len;
	void *vaddr;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attach;
	struct sg_table *table;
	struct list_head list;
};

static struct msm_audio_ion_private msm_audio_ion_data = {0,};
static struct msm_audio_ion_private msm_audio_ion_data_cma = {0,};

struct msm_audio_ion_fd_list_private {
	struct mutex list_mutex;
	/*list to store fd, phy. addr and handle data */
	struct list_head fd_list;
};

static struct msm_audio_ion_fd_list_private msm_audio_ion_fd_list = {0,};
static bool msm_audio_ion_fd_list_init = false;

enum hyp_assign_type {
	HYP_UNASSIGN,
	HYP_ASSIGN_CMA,
	HYP_ASSIGN_MDF,
};

struct msm_audio_fd_data {
	int fd;
	size_t pa_len;
	void *handle;
	dma_addr_t paddr;
	struct device *dev;
	struct list_head list;
	int hyp_assign_type;
};

static void msm_audio_ion_add_allocation(
	struct msm_audio_ion_private *msm_audio_ion_data,
	struct msm_audio_alloc_data *alloc_data)
{
	/*
	 * Since these APIs can be invoked by multiple
	 * clients, there is need to make sure the list
	 * of allocations is always protected
	 */
	mutex_lock(&(msm_audio_ion_data->list_mutex));
	list_add_tail(&(alloc_data->list),
		      &(msm_audio_ion_data->alloc_list));
	mutex_unlock(&(msm_audio_ion_data->list_mutex));
}

static void *msm_audio_ion_map_kernel(struct dma_buf *dma_buf)
{
	int rc = 0;
	void *addr = NULL;
	struct msm_audio_alloc_data *alloc_data = NULL;

	rc = dma_buf_begin_cpu_access(dma_buf, DMA_BIDIRECTIONAL);
	if (rc) {
		pr_err("%s: kmap dma_buf_begin_cpu_access fail\n", __func__);
		goto exit;
	}

	addr = dma_buf_vmap(dma_buf);
	if (!addr) {
		pr_err("%s: kernel mapping of dma_buf failed\n",
		       __func__);
		goto exit;
	}

	/*
	 * TBD: remove the below section once new API
	 * for mapping kernel virtual address is available.
	 */
	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_entry(alloc_data, &(msm_audio_ion_data.alloc_list),
			    list) {
		if (alloc_data->dma_buf == dma_buf) {
			alloc_data->vaddr = addr;
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

exit:
	return addr;
}

static int msm_audio_dma_buf_map(struct dma_buf *dma_buf,
				 dma_addr_t *addr, size_t *len, bool is_iova,
				 bool cma_mem)
{

	struct msm_audio_alloc_data *alloc_data = NULL;
	struct device *cb_dev;
	unsigned long ionflag = 0;
	int rc = 0;
	void *vaddr = NULL;

	if (cma_mem)
		cb_dev = msm_audio_ion_data_cma.cb_dev;
	else
		cb_dev = msm_audio_ion_data.cb_dev;

	/* Data required per buffer mapping */
	alloc_data = kzalloc(sizeof(*alloc_data), GFP_KERNEL);
	if (!alloc_data)
		return -ENOMEM;

	alloc_data->dma_buf = dma_buf;
	alloc_data->len = dma_buf->size;
	*len = dma_buf->size;

	/* Attach the dma_buf to context bank device */
	alloc_data->attach = dma_buf_attach(alloc_data->dma_buf,
					    cb_dev);
	if (IS_ERR(alloc_data->attach)) {
		rc = PTR_ERR(alloc_data->attach);
		dev_err(cb_dev,
			"%s: Fail to attach dma_buf to CB, rc = %d\n",
			__func__, rc);
		goto free_alloc_data;
	}

	/* For uncached buffers, avoid cache maintanance */
	rc = dma_buf_get_flags(alloc_data->dma_buf, &ionflag);
	if (rc) {
		dev_err(cb_dev, "%s: dma_buf_get_flags failed: %d\n",
			__func__, rc);
		goto detach_dma_buf;
	}

	if (!(ionflag & ION_FLAG_CACHED))
		alloc_data->attach->dma_map_attrs |= DMA_ATTR_SKIP_CPU_SYNC;

	/*
	 * Get the scatter-gather list.
	 * There is no info as this is a write buffer or
	 * read buffer, hence the request is bi-directional
	 * to accommodate both read and write mappings.
	 */
	alloc_data->table = dma_buf_map_attachment(alloc_data->attach,
				DMA_BIDIRECTIONAL);
	if (IS_ERR(alloc_data->table)) {
		rc = PTR_ERR(alloc_data->table);
		dev_err(cb_dev,
			"%s: Fail to map attachment, rc = %d\n",
			__func__, rc);
		goto detach_dma_buf;
	}

	/* physical address from mapping */
	if (!is_iova) {
		*addr = sg_phys(alloc_data->table->sgl);
		vaddr = msm_audio_ion_map_kernel((void *)dma_buf);
		if (IS_ERR_OR_NULL(vaddr)) {
			pr_err("%s: ION memory mapping for AUDIO failed\n",
				__func__);
			rc = -ENOMEM;
			goto detach_dma_buf;
		}
		alloc_data->vaddr = vaddr;
	} else {
		*addr = MSM_AUDIO_ION_PHYS_ADDR(alloc_data);
	}

	msm_audio_ion_add_allocation(&msm_audio_ion_data,
				     alloc_data);
	return rc;

detach_dma_buf:
	dma_buf_detach(alloc_data->dma_buf,
		       alloc_data->attach);
free_alloc_data:
	kfree(alloc_data);
	alloc_data = NULL;

	return rc;
}

static int msm_audio_dma_buf_unmap(struct dma_buf *dma_buf, bool cma_mem)
{
	int rc = 0;
	struct msm_audio_alloc_data *alloc_data = NULL;
	struct list_head *ptr, *next;
	struct device *cb_dev;
	bool found = false;

	if (cma_mem)
		cb_dev = msm_audio_ion_data_cma.cb_dev;
	else
		cb_dev = msm_audio_ion_data.cb_dev;
	/*
	 * Though list_for_each_safe is delete safe, lock
	 * should be explicitly acquired to avoid race condition
	 * on adding elements to the list.
	 */
	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_safe(ptr, next,
			    &(msm_audio_ion_data.alloc_list)) {

		alloc_data = list_entry(ptr, struct msm_audio_alloc_data,
					list);

		if (alloc_data->dma_buf == dma_buf) {
			found = true;
			dma_buf_unmap_attachment(alloc_data->attach,
						 alloc_data->table,
						 DMA_BIDIRECTIONAL);

			dma_buf_detach(alloc_data->dma_buf,
				       alloc_data->attach);

			dma_buf_put(alloc_data->dma_buf);

			list_del(&(alloc_data->list));
			kfree(alloc_data);
			alloc_data = NULL;
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

	if (!found) {
		dev_err(cb_dev,
			"%s: cannot find allocation, dma_buf %pK",
			__func__, dma_buf);
		rc = -EINVAL;
	}

	return rc;
}

static int msm_audio_ion_get_phys(struct dma_buf *dma_buf,
				  dma_addr_t *addr, size_t *len, bool is_iova)
{
	int rc = 0;

	rc = msm_audio_dma_buf_map(dma_buf, addr, len, is_iova, false);
	if (rc) {
		pr_err("%s: failed to map DMA buf, err = %d\n",
			__func__, rc);
		goto err;
	}
	if (msm_audio_ion_data.smmu_enabled && is_iova) {
		/* Append the SMMU SID information to the IOVA address */
		*addr |= msm_audio_ion_data.smmu_sid_bits;
	}

	pr_debug("phys=%pK, len=%zd, rc=%d\n", &(*addr), *len, rc);
err:
	return rc;
}

int spf_msm_audio_ion_get_smmu_info(struct device **cb_dev,
		u64 *smmu_sid)
{
	if (!cb_dev || !smmu_sid) {
		pr_err("%s: Invalid params\n",
			__func__);
		return -EINVAL;
	}

	if (!msm_audio_ion_data.cb_dev ||
		!msm_audio_ion_data.smmu_sid_bits) {
		pr_err("%s: Params not initialized\n",
			__func__);
		return -EINVAL;
	}

	*cb_dev = msm_audio_ion_data.cb_dev;
	*smmu_sid = msm_audio_ion_data.smmu_sid_bits;

	return 0;
}

int msm_audio_set_hyp_assign(int fd, int hyp_assign_type)
{
	struct msm_audio_fd_data *msm_audio_fd_data = NULL;
	int status = -EINVAL;
	pr_debug("%s, fd %d\n", __func__, fd);
	mutex_lock(&(msm_audio_ion_fd_list.list_mutex));
	list_for_each_entry(msm_audio_fd_data,
			&msm_audio_ion_fd_list.fd_list, list) {
		if (msm_audio_fd_data->fd == fd) {
			status = 0;
			pr_debug("%s Found fd %d\n", __func__, fd);
			msm_audio_fd_data->hyp_assign_type = hyp_assign_type;
			mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
			return status;
		}
	}
	mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
	return status;
}

static int msm_audio_ion_unmap_kernel(struct dma_buf *dma_buf)
{
	int rc = 0;
	void *vaddr = NULL;
	struct msm_audio_alloc_data *alloc_data = NULL;
	struct device *cb_dev = msm_audio_ion_data.cb_dev;

	/*
	 * TBD: remove the below section once new API
	 * for unmapping kernel virtual address is available.
	 */
	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_entry(alloc_data, &(msm_audio_ion_data.alloc_list),
			    list) {
		if (alloc_data->dma_buf == dma_buf) {
			vaddr = alloc_data->vaddr;
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

	if (!vaddr) {
		dev_err(cb_dev,
			"%s: cannot find allocation for dma_buf %pK",
			__func__, dma_buf);
		rc = -EINVAL;
		goto err;
	}

	dma_buf_vunmap(dma_buf, vaddr);

	rc = dma_buf_end_cpu_access(dma_buf, DMA_BIDIRECTIONAL);
	if (rc) {
		dev_err(cb_dev, "%s: kmap dma_buf_end_cpu_access fail\n",
			__func__);
		goto err;
	}

err:
	return rc;
}

static int msm_audio_ion_map_buf(struct dma_buf *dma_buf, dma_addr_t *paddr,
				 size_t *plen, void **vaddr)
{
	int rc = 0;
	bool is_iova = true;

	if (!dma_buf || !paddr || !vaddr || !plen) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_audio_ion_get_phys(dma_buf, paddr, plen, is_iova);
	if (rc) {
		pr_err("%s: ION Get Physical for AUDIO failed, rc = %d\n",
				__func__, rc);
		dma_buf_put(dma_buf);
		goto err;
	}

	*vaddr = msm_audio_ion_map_kernel(dma_buf);
	if (IS_ERR_OR_NULL(*vaddr)) {
		pr_err("%s: ION memory mapping for AUDIO failed\n", __func__);
		rc = -ENOMEM;
		msm_audio_dma_buf_unmap(dma_buf, false);
		goto err;
	}

err:
	return rc;
}

static u32 msm_audio_ion_get_smmu_sid_mode32(void)
{
	if (msm_audio_ion_data.smmu_enabled)
		return upper_32_bits(msm_audio_ion_data.smmu_sid_bits);
	else
		return 0;
}

/**
 * spf_msm_audio_ion_alloc -
 *        Allocs ION memory for given client name
 *
 * @dma_buf: dma_buf for the ION memory
 * @bufsz: buffer size
 * @paddr: Physical address to be assigned with allocated region
 * @plen: length of allocated region to be assigned
 * vaddr: virtual address to be assigned
 *
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_alloc(struct dma_buf **dma_buf, size_t bufsz,

			dma_addr_t *paddr, size_t *plen, void **vaddr)
{
	int rc = -EINVAL;
	unsigned long err_ion_ptr = 0;

	if (!(msm_audio_ion_data.device_status & MSM_AUDIO_ION_PROBED)) {
		pr_debug("%s:probe is not done, deferred\n", __func__);
		return -EPROBE_DEFER;
	}
	if (!dma_buf || !paddr || !vaddr || !bufsz || !plen) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	if (msm_audio_ion_data.smmu_enabled == true) {
		pr_debug("%s: system heap is used\n", __func__);
		*dma_buf = ion_alloc(bufsz, ION_HEAP(ION_SYSTEM_HEAP_ID), 0);
	} else {
		pr_debug("%s: audio heap is used\n", __func__);
		*dma_buf = ion_alloc(bufsz, ION_HEAP(ION_AUDIO_HEAP_ID), 0);
	}
	if (IS_ERR_OR_NULL((void *)(*dma_buf))) {
		if (IS_ERR((void *)(*dma_buf)))
			err_ion_ptr = PTR_ERR((int *)(*dma_buf));
		pr_err("%s: ION alloc fail err ptr=%ld, smmu_enabled=%d\n",
		       __func__, err_ion_ptr, msm_audio_ion_data.smmu_enabled);
		rc = -ENOMEM;
		goto err;
	}

	rc = msm_audio_ion_map_buf(*dma_buf, paddr, plen, vaddr);
	if (rc) {
		pr_err("%s: failed to map ION buf, rc = %d\n", __func__, rc);
		goto err;
	}
	pr_debug("%s: mapped address = %pK, size=%zd\n", __func__,
		*vaddr, bufsz);

	memset(*vaddr, 0, bufsz);

// Cleanp dmabuf if error?
err:
	return rc;
}
EXPORT_SYMBOL(spf_msm_audio_ion_alloc);

int msm_audio_ion_phys_free(void *handle,
			   dma_addr_t *paddr,
			   size_t *pa_len,
			   u8 assign_type,
			   int id,
			   int key)
{
	handle = NULL;
	return 0;
}
EXPORT_SYMBOL(msm_audio_ion_phys_free);

int msm_audio_ion_phys_assign(void **handle, int fd,
		dma_addr_t *paddr, size_t *pa_len, u8 assign_type, int id)
{
	*handle = NULL;
	return 0;
}
EXPORT_SYMBOL(msm_audio_ion_phys_assign);

bool msm_audio_is_hypervisor_supported(void)
{
	return !(msm_audio_ion_data.is_non_hypervisor);
}
EXPORT_SYMBOL(msm_audio_is_hypervisor_supported);
/**
 * spf_msm_audio_ion_dma_map -
 *        Memory maps for a given DMA buffer
 *
 * @phys_addr: Physical address of DMA buffer to be mapped
 * @iova_base: IOVA address of memory mapped DMA buffer
 * @size: buffer size
 * @dir: DMA direction
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_dma_map(dma_addr_t *phys_addr, dma_addr_t *iova_base,
			u32 size, enum dma_data_direction dir)
{
	dma_addr_t iova;
	struct device *cb_dev = msm_audio_ion_data.cb_dev;

	if (!phys_addr || !iova_base || !size)
		return -EINVAL;

	iova = dma_map_resource(cb_dev, *phys_addr, size,
				dir, 0);
	if (dma_mapping_error(cb_dev, iova)) {
		pr_err("%s: dma_mapping_error\n", __func__);
		return -EIO;
	}
	pr_debug("%s: dma_mapping_success iova:0x%lx\n", __func__,
			 (unsigned long)iova);
	if (msm_audio_ion_data.smmu_enabled)
		/* Append the SMMU SID information to the IOVA address */
		iova |= msm_audio_ion_data.smmu_sid_bits;

	*iova_base = iova;

	return 0;
}
EXPORT_SYMBOL(spf_msm_audio_ion_dma_map);

void msm_audio_fd_list_debug(void)
{
	struct msm_audio_fd_data *msm_audio_fd_data = NULL;

	list_for_each_entry(msm_audio_fd_data,
			&msm_audio_ion_fd_list.fd_list, list) {
		pr_debug("%s fd %d handle %pK phy. addr %pK\n", __func__,
			msm_audio_fd_data->fd, msm_audio_fd_data->handle,
			(void *)msm_audio_fd_data->paddr);
	}
}

void msm_audio_update_fd_list(struct msm_audio_fd_data *msm_audio_fd_data)
{
	struct msm_audio_fd_data *msm_audio_fd_data1 = NULL;

	mutex_lock(&(msm_audio_ion_fd_list.list_mutex));
	list_for_each_entry(msm_audio_fd_data1,
			&msm_audio_ion_fd_list.fd_list, list) {
		if (msm_audio_fd_data1->fd == msm_audio_fd_data->fd) {
			pr_err("%s fd already present, not updating the list",
				__func__);
			mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
			return;
		}
	}
	list_add_tail(&msm_audio_fd_data->list, &msm_audio_ion_fd_list.fd_list);
	mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
}

void msm_audio_delete_fd_entry(void *handle)
{
	struct msm_audio_fd_data *msm_audio_fd_data = NULL;
	struct list_head *ptr, *next;

	mutex_lock(&(msm_audio_ion_fd_list.list_mutex));
	list_for_each_safe(ptr, next,
			&msm_audio_ion_fd_list.fd_list) {
		msm_audio_fd_data = list_entry(ptr, struct msm_audio_fd_data,
					list);
		if (msm_audio_fd_data->handle == handle) {
			pr_debug("%s deleting handle %pK entry from list\n",
				__func__, handle);
			list_del(&(msm_audio_fd_data->list));
			kfree(msm_audio_fd_data);
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
}

int msm_audio_get_phy_addr(int fd, dma_addr_t *paddr, size_t *pa_len)
{
	struct msm_audio_fd_data *msm_audio_fd_data = NULL;
	int status = -EINVAL;

	if (!paddr) {
		pr_err("%s Invalid paddr param status %d\n", __func__, status);
		return status;
	}
	pr_debug("%s, fd %d\n", __func__, fd);
	mutex_lock(&(msm_audio_ion_fd_list.list_mutex));
	list_for_each_entry(msm_audio_fd_data,
			&msm_audio_ion_fd_list.fd_list, list) {
		if (msm_audio_fd_data->fd == fd) {
			*paddr = msm_audio_fd_data->paddr;
			*pa_len = msm_audio_fd_data->pa_len;
			status = 0;
			pr_debug("%s Found fd %d paddr %pK pa_len %d\n",
				__func__, fd, paddr, pa_len);
			mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
			return status;
		}
	}
	mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
	return status;
}
EXPORT_SYMBOL(msm_audio_get_phy_addr);

void msm_audio_get_handle(int fd, void **handle)
{
	struct msm_audio_fd_data *msm_audio_fd_data = NULL;

	pr_debug("%s fd %d\n", __func__, fd);
	mutex_lock(&(msm_audio_ion_fd_list.list_mutex));
	list_for_each_entry(msm_audio_fd_data,
			&msm_audio_ion_fd_list.fd_list, list) {
		if (msm_audio_fd_data->fd == fd) {
			*handle = (struct dma_buf *)msm_audio_fd_data->handle;
			pr_debug("%s handle %pK\n", __func__, *handle);
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_fd_list.list_mutex));
}

/**
 * spf_msm_audio_ion_import-
 *        Import ION buffer with given file descriptor
 *
 * @dma_buf: dma_buf for the ION memory
 * @fd: file descriptor for the ION memory
 * @ionflag: flags associated with ION buffer
 * @bufsz: buffer size
 * @paddr: Physical address to be assigned with allocated region
 * @plen: length of allocated region to be assigned
 * @vaddr: virtual address to be assigned
 *
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_import(struct dma_buf **dma_buf, int fd,
			unsigned long *ionflag, size_t bufsz,
			dma_addr_t *paddr, size_t *plen, void **vaddr)
{
	int rc = 0;

	if (!(msm_audio_ion_data.device_status & MSM_AUDIO_ION_PROBED)) {
		pr_debug("%s: probe is not done, deferred\n", __func__);
		return -EPROBE_DEFER;
	}

	if (!dma_buf || !paddr || !vaddr || !plen) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	/* bufsz should be 0 and fd shouldn't be 0 as of now */
	*dma_buf = dma_buf_get(fd);
	pr_debug("%s: dma_buf =%pK, fd=%d\n", __func__, *dma_buf, fd);
	if (IS_ERR_OR_NULL((void *)(*dma_buf))) {
		pr_err("%s: dma_buf_get failed\n", __func__);
		rc = -EINVAL;
		goto err;
	}

	if (ionflag != NULL) {
		rc = dma_buf_get_flags(*dma_buf, ionflag);
		if (rc) {
			pr_err("%s: could not get flags for the dma_buf\n",
				__func__);
			goto err_ion_flag;
		}
	}

	rc = msm_audio_ion_map_buf(*dma_buf, paddr, plen, vaddr);
	if (rc) {
		pr_err("%s: failed to map ION buf, rc = %d\n", __func__, rc);
		goto err;
	}
	pr_debug("%s: mapped address = %pK, size=%zd\n", __func__,
		*vaddr, bufsz);

	return 0;

err_ion_flag:
	dma_buf_put(*dma_buf);
err:
	*dma_buf = NULL;
	return rc;
}
EXPORT_SYMBOL(spf_msm_audio_ion_import);

/**
 * spf_msm_audio_ion_import_cma-
 *        Import ION buffer with given file descriptor
 *
 * @dma_buf: dma_buf for the ION memory
 * @fd: file descriptor for the ION memory
 * @ionflag: flags associated with ION buffer
 * @bufsz: buffer size
 * @paddr: Physical address to be assigned with allocated region
 * @plen: length of allocated region to be assigned
 * vaddr: virtual address to be assigned
 *
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_import_cma(struct dma_buf **dma_buf, int fd,
			unsigned long *ionflag, size_t bufsz,
			dma_addr_t *paddr, size_t *plen, void **vaddr)
{
	int rc = 0;

	if (!(msm_audio_ion_data.device_status & MSM_AUDIO_ION_PROBED)) {
		pr_debug("%s: probe is not done, deferred\n", __func__);
		return -EPROBE_DEFER;
	}

	if (!dma_buf || !paddr || !vaddr || !plen ||
	    !msm_audio_ion_data_cma.cb_dev) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	/* bufsz should be 0 and fd shouldn't be 0 as of now */
	*dma_buf = dma_buf_get(fd);
	pr_debug("%s: dma_buf =%pK, fd=%d\n", __func__, *dma_buf, fd);
	if (IS_ERR_OR_NULL((void *)(*dma_buf))) {
		pr_err("%s: dma_buf_get failed\n", __func__);
		rc = -EINVAL;
		goto err;
	}

	if (ionflag != NULL) {
		rc = dma_buf_get_flags(*dma_buf, ionflag);
		if (rc) {
			pr_err("%s: could not get flags for the dma_buf\n",
				__func__);
			goto err_ion_flag;
		}
	}

	msm_audio_dma_buf_map(*dma_buf, paddr, plen, true, true);

	return 0;

err_ion_flag:
	dma_buf_put(*dma_buf);
err:
	*dma_buf = NULL;
	return rc;
}
EXPORT_SYMBOL(spf_msm_audio_ion_import_cma);

/**
 * spf_msm_audio_ion_free -
 *        fress ION memory for given client and handle
 *
 * @dma_buf: dma_buf for the ION memory
 *
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_free(struct dma_buf *dma_buf)
{
	int ret = 0;

	if (!dma_buf) {
		pr_err("%s: dma_buf invalid\n", __func__);
		return -EINVAL;
	}

	ret = msm_audio_ion_unmap_kernel(dma_buf);
	if (ret)
		return ret;

	msm_audio_dma_buf_unmap(dma_buf, false);

	return 0;
}
EXPORT_SYMBOL(spf_msm_audio_ion_free);

/**
 * msm_audio_ion_crash_handler -
 *        handles cleanup after userspace crashes.
 *
 * To be called from machine driver.
 */
void msm_audio_ion_crash_handler(void)
{
	struct msm_audio_fd_data *msm_audio_fd_data = NULL;
	struct list_head *ptr, *next;
	void *handle = NULL;

	pr_debug("Inside %s\n", __func__);
	list_for_each_entry(msm_audio_fd_data,
		&msm_audio_ion_fd_list.fd_list, list) {
		handle = msm_audio_fd_data->handle;
		// TODO need to add cma memory free process here once hyp_unassign issue fixed.
		spf_msm_audio_ion_free(handle);
	}
	list_for_each_safe(ptr, next,
		&msm_audio_ion_fd_list.fd_list) {
		msm_audio_fd_data = list_entry(ptr, struct msm_audio_fd_data,
						list);
		list_del(&(msm_audio_fd_data->list));
		kfree(msm_audio_fd_data);
	}
}
EXPORT_SYMBOL(msm_audio_ion_crash_handler);

/**
 * spf_msm_audio_ion_free_cma -
 *        fress ION memory for given client and handle
 *
 * @dma_buf: dma_buf for the ION memory
 *
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_free_cma(struct dma_buf *dma_buf)
{
	if (!dma_buf) {
		pr_err("%s: dma_buf invalid\n", __func__);
		return -EINVAL;
	}

	msm_audio_dma_buf_unmap(dma_buf, true);

	return 0;
}
EXPORT_SYMBOL(spf_msm_audio_ion_free_cma);

/**
 * spf_msm_audio_ion_mmap -
 *       Audio ION memory map
 *
 * @abuff: audio buf pointer
 * @vma: virtual mem area
 *
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_mmap(struct audio_buffer *abuff,
		       struct vm_area_struct *vma)
{
	struct msm_audio_alloc_data *alloc_data = NULL;
	struct sg_table *table;
	unsigned long addr = vma->vm_start;
	unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
	struct scatterlist *sg;
	unsigned int i;
	struct page *page;
	int ret = 0;
	bool found = false;
	struct device *cb_dev = msm_audio_ion_data.cb_dev;

	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_entry(alloc_data, &(msm_audio_ion_data.alloc_list),
			    list) {
		if (alloc_data->dma_buf == abuff->dma_buf) {
			found = true;
			table = alloc_data->table;
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

	if (!found) {
		dev_err(cb_dev,
			"%s: cannot find allocation, dma_buf %pK",
			__func__, abuff->dma_buf);
		return -EINVAL;
	}
	/* uncached */
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	/* We need to check if a page is associated with this sg list because:
	 * If the allocation came from a carveout we currently don't have
	 * pages associated with carved out memory. This might change in the
	 * future and we can remove this check and the else statement.
	 */
	page = sg_page(table->sgl);
	if (page) {
		pr_debug("%s: page is NOT null\n", __func__);
		for_each_sg(table->sgl, sg, table->nents, i) {
			unsigned long remainder = vma->vm_end - addr;
			unsigned long len = sg->length;

			page = sg_page(sg);

			if (offset >= len) {
				offset -= len;
				continue;
			} else if (offset) {
				page += offset / PAGE_SIZE;
				len -= offset;
				offset = 0;
			}
			len = min(len, remainder);
			pr_debug("vma=%pK, addr=%x len=%ld vm_start=%x vm_end=%x vm_page_prot=%lu\n",
				vma, (unsigned int)addr, len,
				(unsigned int)vma->vm_start,
				(unsigned int)vma->vm_end,
				(unsigned long)pgprot_val(vma->vm_page_prot));
			remap_pfn_range(vma, addr, page_to_pfn(page), len,
					vma->vm_page_prot);
			addr += len;
			if (addr >= vma->vm_end)
				return 0;
		}
	} else {
		pr_debug("%s: page is NULL\n", __func__);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(spf_msm_audio_ion_mmap);

/**
 * spf_msm_audio_ion_cache_operations-
 *       Cache operations on cached Audio ION buffers
 *
 * @abuff: audio buf pointer
 * @cache_op: cache operation to be performed
 *
 * Returns 0 on success or error on failure
 */
int spf_msm_audio_ion_cache_operations(struct audio_buffer *abuff, int cache_op)
{
	unsigned long ionflag = 0;
	int rc = 0;

	if (!abuff) {
		pr_err("%s: Invalid params: %pK\n", __func__, abuff);
		return -EINVAL;
	}
	rc = dma_buf_get_flags(abuff->dma_buf, &ionflag);
	if (rc) {
		pr_err("%s: dma_buf_get_flags failed: %d\n", __func__, rc);
		goto cache_op_failed;
	}

	/* Has to be CACHED */
	if (ionflag & ION_FLAG_CACHED) {
		/* MSM_AUDIO_ION_INV_CACHES or MSM_AUDIO_ION_CLEAN_CACHES */
		switch (cache_op) {
		case MSM_AUDIO_ION_INV_CACHES:
		case MSM_AUDIO_ION_CLEAN_CACHES:
			dma_buf_begin_cpu_access(abuff->dma_buf,
						  DMA_BIDIRECTIONAL);
			dma_buf_end_cpu_access(abuff->dma_buf,
						  DMA_BIDIRECTIONAL);
			break;
		default:
			pr_err("%s: Invalid cache operation %d\n",
			       __func__, cache_op);
		}
	} else {
		pr_err("%s: Cache ops called on uncached buffer: %pK\n",
			__func__, abuff->dma_buf);
		rc = -EINVAL;
	}

cache_op_failed:
	return rc;
}
EXPORT_SYMBOL(spf_msm_audio_ion_cache_operations);

/**
 * spf_msm_audio_populate_upper_32_bits -
 *        retrieve upper 32bits of 64bit address
 *
 * @pa: 64bit physical address
 *
 */
u32 spf_msm_audio_populate_upper_32_bits(dma_addr_t pa)
{
	if (sizeof(dma_addr_t) == sizeof(u32))
		return msm_audio_ion_get_smmu_sid_mode32();
	else
		return upper_32_bits(pa);
}
EXPORT_SYMBOL(spf_msm_audio_populate_upper_32_bits);

static int msm_audio_ion_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct msm_audio_ion_private *ion_data = container_of(inode->i_cdev,
						struct msm_audio_ion_private,
						cdev);
	struct device *dev = ion_data->chardev;

	pr_debug("Inside %s\n", __func__);
	get_device(dev);
	return ret;
}

static int msm_audio_ion_release(struct inode *inode, struct file *file)
{
	struct msm_audio_ion_private *ion_data = container_of(inode->i_cdev,
						struct msm_audio_ion_private,
						cdev);
	struct device *dev = ion_data->chardev;

	pr_debug("Inside %s\n", __func__);
	put_device(dev);
	return 0;
}

static long msm_audio_ion_ioctl(struct file *file, unsigned int ioctl_num,
				unsigned long __user ioctl_param)
{
	void *mem_handle;
	dma_addr_t paddr;
	size_t pa_len = 0;
	void *vaddr;
	int ret = 0;
	struct msm_audio_fd_data *msm_audio_fd_data = NULL;

	int dest_perms_map[2] = {PERM_READ | PERM_WRITE, PERM_READ | PERM_WRITE};
	int source_vm_map[1] = {VMID_HLOS};
	int dest_vm_map[3] = {VMID_LPASS, VMID_ADSP_HEAP, VMID_HLOS};
	int dest_perms_unmap[1] = {PERM_READ | PERM_WRITE | PERM_EXEC};
	int source_vm_unmap[3] = {VMID_LPASS, VMID_ADSP_HEAP, VMID_HLOS};
	int dest_vm_unmap[1] = {VMID_HLOS};

	int mdf_dest_perms_map[3] = {
		[0 ... 2] = PERM_READ | PERM_WRITE | PERM_EXEC,
	};
	int mdf_dest_vm_map[3] = {VMID_SSC_Q6, VMID_ADSP_Q6, VMID_MSS_MBA};
	int mdf_source_vm_unmap[3] = {VMID_SSC_Q6, VMID_ADSP_Q6, VMID_MSS_MBA};

	struct msm_audio_ion_private *ion_data =
			container_of(file->f_inode->i_cdev, struct msm_audio_ion_private, cdev);

	pr_debug("%s ioctl num %u\n", __func__, ioctl_num);
	switch (ioctl_num) {
	case IOCTL_MAP_PHYS_ADDR:
		msm_audio_fd_data = kzalloc((sizeof(struct msm_audio_fd_data)),
					GFP_KERNEL);
		if (!msm_audio_fd_data) {
			ret = -ENOMEM;
			pr_err("%s : Out of memory ret %d\n", __func__, ret);
			return ret;
		}
		if (ion_data == &msm_audio_ion_data) {
			ret = spf_msm_audio_ion_import((struct dma_buf **)&mem_handle, (int)ioctl_param,
						NULL, 0, &paddr, &pa_len, &vaddr);
		} else {
			ret = spf_msm_audio_ion_import_cma((struct dma_buf **)&mem_handle, (int)ioctl_param,
						NULL, 0, &paddr, &pa_len, &vaddr);
		}
		if (ret < 0) {
			pr_err("%s Memory map Failed %d\n", __func__, ret);
			kfree(msm_audio_fd_data);
			return ret;
		}
		msm_audio_fd_data->fd = (int)ioctl_param;
		msm_audio_fd_data->handle = mem_handle;
		msm_audio_fd_data->paddr = paddr;
		msm_audio_fd_data->pa_len = pa_len;
		msm_audio_fd_data->hyp_assign_type = HYP_UNASSIGN;
		msm_audio_fd_data->dev = ion_data->cb_dev;
		msm_audio_update_fd_list(msm_audio_fd_data);
		break;
	case IOCTL_UNMAP_PHYS_ADDR:
		msm_audio_get_handle((int)ioctl_param, &mem_handle);
		if (ion_data == &msm_audio_ion_data) {
			ret = spf_msm_audio_ion_free(mem_handle);
		} else {
			ret = spf_msm_audio_ion_free_cma(mem_handle);
		}
		if (ret < 0) {
			pr_err("%s Ion free failed %d\n", __func__, ret);
			return ret;
		}
		msm_audio_delete_fd_entry(mem_handle);
		break;
	case IOCTL_MAP_HYP_ASSIGN:
	    ret = msm_audio_get_phy_addr((int)ioctl_param, &paddr, &pa_len);
		if (ret < 0) {
			pr_err("%s get phys addr failed %d\n", __func__, ret);
			return ret;
		}
		ret = hyp_assign_phys(paddr, pa_len, source_vm_map, 1,
		                      dest_vm_map, dest_perms_map, 2);
		if (ret < 0) {
			pr_err("%s: hyp_assign_phys failed result = %d addr = 0x%pK size = %d\n",
					__func__, ret, paddr, pa_len);
			return ret;
		}
		msm_audio_set_hyp_assign((int)ioctl_param, HYP_ASSIGN_CMA);
		pr_debug("%s: hyp_assign_phys success\n", __func__);
	    break;
	case IOCTL_UNMAP_HYP_ASSIGN:
	    ret = msm_audio_get_phy_addr((int)ioctl_param, &paddr, &pa_len);
		if (ret < 0) {
			pr_err("%s get phys addr failed %d\n", __func__, ret);
			return ret;
		}
		ret = hyp_assign_phys(paddr, pa_len, source_vm_unmap, 2,
		                      dest_vm_unmap, dest_perms_unmap, 1);
		if (ret < 0) {
			pr_err("%s: hyp_assign_phys failed result = %d addr = 0x%pK size = %d\n",
					__func__, ret, paddr, pa_len);
			return ret;
		}
		msm_audio_set_hyp_assign((int)ioctl_param, HYP_UNASSIGN);
		pr_debug("%s: hyp_assign_phys success\n", __func__);
	    break;
	case IOCTL_MAP_HYP_ASSIGN_V2:
	    ret = msm_audio_get_phy_addr((int)ioctl_param, &paddr, &pa_len);
		if (ret < 0) {
			pr_err("%s get phys addr failed %d\n", __func__, ret);
			return ret;
		}
		ret = hyp_assign_phys(paddr, pa_len, source_vm_map, 1,
		                      mdf_dest_vm_map, mdf_dest_perms_map, 3);
		if (ret < 0) {
			pr_err("%s: hyp_assign_phys for mdf failed result = %d addr = 0x%pK size = %d\n",
					__func__, ret, paddr, pa_len);
			return ret;
		}
		msm_audio_set_hyp_assign((int)ioctl_param, HYP_ASSIGN_MDF);
		pr_debug("%s: hyp_assign_phys for mdf success\n", __func__);
	    break;
	case IOCTL_UNMAP_HYP_ASSIGN_V2:
	    ret = msm_audio_get_phy_addr((int)ioctl_param, &paddr, &pa_len);
		if (ret < 0) {
			pr_err("%s get phys addr failed %d\n", __func__, ret);
			return ret;
		}
		ret = hyp_assign_phys(paddr, pa_len, mdf_source_vm_unmap, 3,
		                      dest_vm_unmap, dest_perms_unmap, 1);
		if (ret < 0) {
			pr_err("%s: hyp_assign_phys failed result = %d addr = 0x%pK size = %d\n",
					__func__, ret, paddr, pa_len);
			return ret;
		}
		msm_audio_set_hyp_assign((int)ioctl_param, HYP_UNASSIGN);
		pr_debug("%s: hyp_unassign_phys mdf success\n", __func__);
	    break;
	default:
		pr_err("%s Entered default. Invalid ioctl num %u",
			__func__, ioctl_num);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int msm_audio_smmu_init(struct device *dev)
{
	INIT_LIST_HEAD(&msm_audio_ion_data.alloc_list);
	mutex_init(&(msm_audio_ion_data.list_mutex));

	return 0;
}

static int msm_audio_cma_init(struct device *dev)
{
	INIT_LIST_HEAD(&msm_audio_ion_data_cma.alloc_list);
	mutex_init(&(msm_audio_ion_data_cma.list_mutex));

	return 0;
}

static const struct of_device_id msm_audio_ion_dt_match[] = {
	{ .compatible = "qcom,msm-audio-ion" },
	{ .compatible = "qcom,msm-audio-ion-cma"},
	{ }
};
MODULE_DEVICE_TABLE(of, msm_audio_ion_dt_match);

static const struct file_operations msm_audio_ion_fops = {
	.owner = THIS_MODULE,
	.open = msm_audio_ion_open,
	.release = msm_audio_ion_release,
	.unlocked_ioctl = msm_audio_ion_ioctl,
};

static int msm_audio_ion_reg_chrdev(struct msm_audio_ion_private *ion_data)
{
	int ret = 0;

	ret = alloc_chrdev_region(&ion_data->ion_major, 0,
				MINOR_NUMBER_COUNT, ion_data->driver_name);
	if (ret < 0) {
		pr_err("%s alloc_chr_dev_region failed ret : %d\n",
			__func__, ret);
		return ret;
	}
	pr_debug("%s major number %d", __func__, MAJOR(ion_data->ion_major));
	ion_data->ion_class = class_create(THIS_MODULE,
					ion_data->driver_name);
	if (IS_ERR(ion_data->ion_class)) {
		ret = PTR_ERR(ion_data->ion_class);
		pr_err("%s class create failed. ret : %d", __func__, ret);
		goto err_class;
	}
	ion_data->chardev = device_create(ion_data->ion_class, NULL,
				ion_data->ion_major, NULL,
				ion_data->driver_name);
	if (IS_ERR(ion_data->chardev)) {
		ret = PTR_ERR(ion_data->chardev);
		pr_err("%s device create failed ret : %d\n", __func__, ret);
		goto err_device;
	}
	cdev_init(&ion_data->cdev, &msm_audio_ion_fops);
	ret = cdev_add(&ion_data->cdev, ion_data->ion_major, 1);
	if (ret) {
		pr_err("%s cdev add failed, ret : %d\n", __func__, ret);
		goto err_cdev;
	}
	return ret;

err_cdev:
	device_destroy(ion_data->ion_class, ion_data->ion_major);
err_device:
	class_destroy(ion_data->ion_class);
err_class:
	unregister_chrdev_region(0, MINOR_NUMBER_COUNT);
	return ret;
}

static int msm_audio_ion_unreg_chrdev(struct msm_audio_ion_private *ion_data)
{
	cdev_del(&ion_data->cdev);
	device_destroy(ion_data->ion_class, ion_data->ion_major);
	class_destroy(ion_data->ion_class);
	unregister_chrdev_region(0, MINOR_NUMBER_COUNT);
	return 0;
}
static int msm_audio_ion_probe(struct platform_device *pdev)
{
	int rc = 0;
	u64 smmu_sid = 0;
	u64 smmu_sid_mask = 0;
	const char *msm_audio_ion_dt = "qcom,smmu-enabled";
	const char *msm_audio_ion_non_hyp = "qcom,non-hyp-assign";
	const char *msm_audio_ion_smmu = "qcom,smmu-version";
	const char *msm_audio_ion_smmu_sid_mask = "qcom,smmu-sid-mask";
	bool smmu_enabled;
	bool is_non_hypervisor_en;
	struct device *dev = &pdev->dev;
	struct of_phandle_args iommuspec;
	struct msm_audio_ion_private *ion_data = NULL;
#ifndef CONFIG_SPF_CORE
	enum apr_subsys_state q6_state;
#endif

	dev_err(dev, "%s: msm_audio_ion_probe\n", __func__);
	if (dev->of_node == NULL) {
		dev_err(dev,
			"%s: device tree is not found\n",
			__func__);
		msm_audio_ion_data.smmu_enabled = 0;
		return 0;
	}

	smmu_enabled = of_property_read_bool(dev->of_node,
					     msm_audio_ion_dt);
	if (smmu_enabled)
		ion_data = &msm_audio_ion_data;
	else
		ion_data = &msm_audio_ion_data_cma;
	ion_data->smmu_enabled = smmu_enabled;

	is_non_hypervisor_en = of_property_read_bool(dev->of_node,
					     msm_audio_ion_non_hyp);
	ion_data->is_non_hypervisor = is_non_hypervisor_en;



#ifndef CONFIG_SPF_CORE
	q6_state = apr_get_q6_state();
	if (q6_state == APR_SUBSYS_DOWN) {
		dev_info(dev,
			"defering %s, adsp_state %d\n",
			__func__, q6_state);
		return -EPROBE_DEFER;
	}
#endif
	dev_dbg(dev, "%s: adsp is ready\n", __func__);

	if (smmu_enabled) {
		ion_data->driver_name = "msm_audio_ion";
		rc = of_property_read_u32(dev->of_node,
					msm_audio_ion_smmu,
					&ion_data->smmu_version);
		if (rc) {
			dev_err(dev,
				"%s: qcom,smmu_version missing in DT node\n",
				__func__);
			return rc;
		}
		dev_dbg(dev, "%s: SMMU is Enabled. SMMU version is (%d)",
			__func__, ion_data->smmu_version);

		/* Get SMMU SID information from Devicetree */
		rc = of_property_read_u64(dev->of_node,
					msm_audio_ion_smmu_sid_mask,
					&smmu_sid_mask);
		if (rc) {
			dev_err(dev,
				"%s: qcom,smmu-sid-mask missing in DT node, using default\n",
				__func__);
			smmu_sid_mask = 0xFFFFFFFFFFFFFFFF;
		}

		rc = of_parse_phandle_with_args(dev->of_node, "iommus",
						"#iommu-cells", 0, &iommuspec);
		if (rc)
			dev_err(dev, "%s: could not get smmu SID, ret = %d\n",
				__func__, rc);
		else
			smmu_sid = (iommuspec.args[0] & smmu_sid_mask);

		ion_data->smmu_sid_bits =
			smmu_sid << MSM_AUDIO_SMMU_SID_OFFSET;

		if (ion_data->smmu_version == 0x2) {
			rc = msm_audio_smmu_init(dev);
		} else {
			dev_err(dev, "%s: smmu version invalid %d\n",
				__func__, ion_data->smmu_version);
			rc = -EINVAL;
		}
		if (rc)
			dev_err(dev, "%s: smmu init failed, err = %d\n",
				__func__, rc);
	} else {
		ion_data->driver_name = "msm_audio_ion_cma";
		msm_audio_cma_init(dev);
	}
	ion_data->cb_dev = dev;

	if (!rc)
		ion_data->device_status |= MSM_AUDIO_ION_PROBED;

	if (!msm_audio_ion_fd_list_init) {
		INIT_LIST_HEAD(&msm_audio_ion_fd_list.fd_list);
		mutex_init(&(msm_audio_ion_fd_list.list_mutex));
		msm_audio_ion_fd_list_init = true;
	}
	dev_set_drvdata(dev, ion_data);
	rc = msm_audio_ion_reg_chrdev(ion_data);
	if (rc) {
		pr_err("%s register char dev failed, rc : %d", __func__, rc);
		return rc;
	}
	return rc;
}

static int msm_audio_ion_remove(struct platform_device *pdev)
{
	struct device *audio_cb_dev;

	audio_cb_dev = msm_audio_ion_data.cb_dev;

	msm_audio_ion_data.smmu_enabled = 0;
	msm_audio_ion_data.device_status = 0;
	msm_audio_ion_unreg_chrdev(&msm_audio_ion_data);
	return 0;
}

static struct platform_driver msm_audio_ion_driver = {
	.driver = {
		.name = "msm-audio-ion",
		.owner = THIS_MODULE,
		.of_match_table = msm_audio_ion_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = msm_audio_ion_probe,
	.remove = msm_audio_ion_remove,
};

int __init msm_audio_ion_init(void)
{
	pr_debug("%s: msm_audio_ion_init called \n",__func__);
	return platform_driver_register(&msm_audio_ion_driver);
}

void msm_audio_ion_exit(void)
{
	platform_driver_unregister(&msm_audio_ion_driver);
}

module_init(msm_audio_ion_init);
module_exit(msm_audio_ion_exit);
MODULE_DESCRIPTION("MSM Audio ION module");
MODULE_LICENSE("GPL v2");
