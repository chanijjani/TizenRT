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

#define MAX_IN_AUDIO_DEV_NUM 2
#define MAX_OUT_AUDIO_DEV_NUM 2

/**
 * @brief Result types of Audio Manager APIs such as FAIL, SUCCESS, or INVALID ARGS
 */
typedef enum {
	AUDIO_MANAGER_FAIL = -1,
	AUDIO_MANAGER_SUCCESS,
	AUDIO_MANAGER_INVALID_PARAMS,
}
audio_manager_result_e;

typedef enum {
	AUDIO_DEVICE_UNIDENTIFIED = -1,
	AUDIO_DEVICE_IDENTIFIED,
	AUDIO_DEVICE_RESERVED
} audio_device_status_e;

typedef enum {
	AUDIO_VOLUME_LOW = 30,
	AUDIO_VOLUME_MEDIUM = 80,
	AUDIO_VOLUME_HIGH = 100
} audio_volume_e;

typedef struct {
	unsigned int volume;
} audio_config_s;

typedef struct {
//  int audio_dev_id;
	char *dev_name;
	audio_device_status_e status;
	audio_config_s config;
//  audio_dev_type_e direction;
} audio_dev_info_s;

audio_manager_result_e audio_manager_init(void);
int get_avail_audio_card_id(void);
int get_avail_audio_device_id(void);
audio_manager_result_e set_audio_volume(audio_volume_e volume);

#if defined(__cplusplus)
}								/* extern "C" */
#endif
/** @} */// end of TinyAlsa group
#endif
