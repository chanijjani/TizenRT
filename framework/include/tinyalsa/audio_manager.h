/****************************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/**
 * @defgroup TinyAlsa TinyAlsa
 * @ingroup TinyAlsa
 * @brief Provides APIs for Audio Manager
 * @{
 */

/**
 * @file audio_manager.h
 * @brief All macros, structures and functions to manager audio devices.
 */

#ifndef __AUDIO_MANAGER_H
#define __AUDIO_MANAGER_H

#include <sys/time.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Result types of Audio Manager APIs such as FAIL, SUCCESS, or INVALID ARGS
 */
typedef enum {
	AUDIO_MANAGER_FAIL = -1,
	AUDIO_MANAGER_SUCCESS,
	AUDIO_MANAGER_INVALID_ARGS,
	AUDIO_MANAGER_INITIALIZED,
	AUDIO_MANAGER_BUSY,
} audio_manager_result_e;

typedef struct {
	int audio_dev_id;
	char *dev_name;
//	audio_dev_type_e direction;
} audio_dev_info_s;

//const struct audio_dev_info_s input_audio_devices[MAX_IN_AUDIO_DEV_NUM];
//const struct audio_dev_info_s output_audio_devices[MAX_OUT_AUDIO_DEV_NUM];

audio_manager_result_e audio_manager_init(void);
int get_avail_audio_card_id(void);
int get_avail_audio_device_id(void);

#if defined(__cplusplus)
}								/* extern "C" */
#endif
/** @} */// end of TinyAlsa group
#endif
