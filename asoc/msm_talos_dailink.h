/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <sound/soc.h>



SND_SOC_DAILINK_DEFS(proxy_tx,
        DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
        DAILINK_COMP_ARRAY(COMP_DUMMY()),
        DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(proxy_rx,
        DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
        DAILINK_COMP_ARRAY(COMP_DUMMY()),
        DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(usb_audio_rx,
        DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
        DAILINK_COMP_ARRAY(COMP_DUMMY()),
        DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(usb_audio_tx,
        DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
        DAILINK_COMP_ARRAY(COMP_DUMMY()),
        DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(pcm_dummy_tx0,
        DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
        DAILINK_COMP_ARRAY(COMP_DUMMY()),
        DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));


SND_SOC_DAILINK_DEFS(pcm_dummy_rx0,
        DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
        DAILINK_COMP_ARRAY(COMP_DUMMY()),
        DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(vi_feedback,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "wsa_macro_vifeedback")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(wsa_cdcdma0_capture_stub,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "wsa_macro_vifeedback")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

/* BE dai-links */
SND_SOC_DAILINK_DEFS(afe_pcm_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.224")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(afe_pcm_rx1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.226")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(afe_pcm_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.225")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(incall_record_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.32772")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(incall_record_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.32771")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(voice_playback_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.32773")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(voice2_playback_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.32770")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(pri_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36864")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(pri_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36865")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36880")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36881")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36896")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36897")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36912")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36913")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quin_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36928")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quin_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36929")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36944")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36946")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36948")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36950")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36952")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36954")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36956")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36958")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36945")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36947")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36949")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36951")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36953")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36955")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36957")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36959")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36960")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36962")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36964")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36966")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36968")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36970")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36972")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36974")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36961")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36963")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36965")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36967")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36969")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36971")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36973")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sep_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36975")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36976")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36978")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36980")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36982")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36984")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36986")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36988")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36990")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36977")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36979")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36981")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36983")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36985")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36987")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36989")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif0_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36991")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36992")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36994")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36996")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36998")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37000")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37002")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37004")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37006")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36993")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36995")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36997")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36999")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37001")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37003")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37005")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif1_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37007")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37008")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37010")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37012")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37014")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37016")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37018")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37020")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37022")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_0,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37009")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37011")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37013")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37015")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37017")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_5,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37019")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_6,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37021")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(hsif2_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.37023")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_0_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16384")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_rx1"),
		COMP_CODEC("tasha_codec", "tasha_mix_rx1"),
		COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_0_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16385")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_tx1"),
		COMP_CODEC("tasha_codec", "tasha_tx1"),
			   COMP_CODEC("wsa-codec1", "wsa_rx1"),
			   COMP_CODEC("wsa-codec2", "wsa_rx2"),
		COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_1_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16386")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_rx1"),
		COMP_CODEC("tasha_codec", "tasha_mix_rx1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_1_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16387")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_tx3"),
		COMP_CODEC("tasha_codec", "tasha_tx3")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_2_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16388")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_rx2")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_3_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16390")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_rx1"),
		COMP_CODEC("tasha_codec", "tasha_mix_rx1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_3_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16391")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_tx1"),
		COMP_CODEC("tasha_codec", "tasha_tx1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_4_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16392")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_rx1"),
		COMP_CODEC("tasha_codec", "tasha_mix_rx1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_5_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.16394")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_rx3"),
		COMP_CODEC("tasha_codec", "tasha_rx3")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(slimbus_5_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_mad1"),
		COMP_CODEC("tasha_codec", "tasha_mad1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(slimbus_6_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_rx4"),
		COMP_CODEC("tasha_codec", "tasha_rx4")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(slimbus_tx_vi,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("tavil_codec", "tavil_vifeedback")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(wsa_dma_tx0_vi,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "wsa_macro_vifeedback")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(slimbus_7_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("btfmslim_slave",
			"btfm_bt_sco_a2dp_slim_rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(slimbus_7_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("btfmslim_slave",
			"btfm_bt_sco_slim_tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(slimbus_8_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("btfmslim_slave",
			"btfm_bt_sco_slim_tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(display_port,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC(
			"soc:qcom,msm-ext-disp:qcom,msm-ext-disp-audio-codec-rx",
			"msm_dp_audio_codec_rx_dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(display_port1,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC(
			"soc:qcom,msm-ext-disp:qcom,msm-ext-disp-audio-codec-rx",
			"msm_dp_audio_codec_rx1_dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(pri_mi2s_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(pri_mi2s_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(sec_mi2s_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(sec_mi2s_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(tert_mi2s_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy-dai")));

SND_SOC_DAILINK_DEFS(tert_mi2s_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(quat_mi2s_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(quat_mi2s_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(quin_mi2s_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(quin_mi2s_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(sen_mi2s_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(sen_mi2s_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(auxpcm_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(auxpcm_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(sec_auxpcm_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(sec_auxpcm_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(tert_auxpcm_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(tert_auxpcm_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(quat_auxpcm_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(quat_auxpcm_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(quin_auxpcm_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-auxpcm.5")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quin_auxpcm_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-auxpcm.5")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_auxpcm_rx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-auxpcm.6")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sen_auxpcm_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(wsa_dma_rx0,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "wsa_macro_rx1"),
			   COMP_CODEC("wsa-codec.1", "wsa_rx1"),
			   COMP_CODEC("wsa-codec.2", "wsa_rx2"),
			   COMP_CODEC("wsa-codec.3", "wsa_rx3"),
			   COMP_CODEC("wsa-codec.4", "wsa_rx4")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(wsa_dma_rx1,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "wsa_macro_rx_mix"),
			   COMP_CODEC("wsa-codec.1", "wsa_rx1"),
			   COMP_CODEC("wsa-codec.2", "wsa_rx2"),
			   COMP_CODEC("wsa-codec.3", "wsa_rx3"),
			   COMP_CODEC("wsa-codec.4", "wsa_rx4")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(wsa_dma_tx1,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "wsa_macro_echo"),
			   COMP_CODEC("wsa-codec.1", "wsa_rx1"),
			   COMP_CODEC("wsa-codec.2", "wsa_rx2"),
                           COMP_CODEC("wsa-codec.3", "wsa_rx3"),
                           COMP_CODEC("wsa-codec.4", "wsa_rx4")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_rx0,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "rx_macro_rx1"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_rx1,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "rx_macro_rx2"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_rx2,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "rx_macro_rx3"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_rx3,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "rx_macro_rx4"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_tx3,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "tx_macro_tx1"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_tx4,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "tx_macro_tx2"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_rx5,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "rx_macro_rx5"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(rx_dma_rx6,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "rx_macro_rx6"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(tx_dma_tx3,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "tx_macro_tx1"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(tx_dma_tx4,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "tx_macro_tx2"),
			   COMP_CODEC("wcd938x_codec", "wcd938x_cdc"),
			   COMP_CODEC("wcd937x_codec", "wcd937x_cdc"),
			   COMP_CODEC("swr-dmic.01", "swr_dmic_tx0"),
			   COMP_CODEC("swr-dmic.02", "swr_dmic_tx1"),
			   COMP_CODEC("swr-dmic.03", "swr_dmic_tx2"),
			   COMP_CODEC("swr-dmic.04", "swr_dmic_tx3")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(va_dma_tx0,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "va_macro_tx1")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(va_dma_tx1,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "va_macro_tx2")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(va_dma_tx2,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("bolero_codec", "va_macro_tx3")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(afe_loopback_tx,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-dev.24577")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));


SND_SOC_DAILINK_DEFS(pri_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36866")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(pri_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36868")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(pri_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36870")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(pri_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36867")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(pri_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36869")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(pri_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36871")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36882")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36884")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36886")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36894")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36883")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36885")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(sec_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36887")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36898")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36900")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36902")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_rx_4,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36904")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36899")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36901")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36903")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(tert_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36911")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_rx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36914")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_rx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36916")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_rx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36918")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36926")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_tx_1,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36915")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_tx_2,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36917")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_tx_3,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36919")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quat_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36927")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quin_tdm_rx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("msm-dai-q6-tdm.36942")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(quin_tdm_tx_7,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("msm-pcm-routing")));

SND_SOC_DAILINK_DEFS(ext_display_port,
	DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
	DAILINK_COMP_ARRAY(COMP_CODEC("msm-ext-disp-audio-codec-rx",
			"msm_dp_audio_codec_rx_dai")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(tert_tdm_rx_0_dummy,
    DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
    DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
    DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(tert_tdm_tx_0_dummy,
    DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
    DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
    DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(lpass_be_auxpcm_rx_dummy,
    DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
    DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
    DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(lpass_be_auxpcm_tx_dummy,
    DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
    DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
    DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(primary_tdm_rx_0_dummy,
    DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
    DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-rx")),
    DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(primary_tdm_tx_0_dummy,
    DAILINK_COMP_ARRAY(COMP_CPU("snd-soc-dummy-dai")),
    DAILINK_COMP_ARRAY(COMP_CODEC("msm-stub-codec.1", "msm-stub-tx")),
    DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));
