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
/****************************************************************************
** Developed by: Chanhee Lee <ch2102.lee@samsung.com>
****************************************************************************/

#include <tinyara/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>

#define __force
#define __bitwise
#define __user

#include <tinyara/audio/audio.h>
#include <tinyalsa/tinyalsa.h>
#include <tinyalsa/audio_manager.h>
#include <tinyara/audio/pcm.h>

audio_dev_info_s input_audio_devices[MAX_IN_AUDIO_DEV_NUM];
audio_dev_info_s output_audio_devices[MAX_OUT_AUDIO_DEV_NUM];

static int out_dev_fd = -1;		/** TODO: fd needs to be decided at run-time when multiple output devices exist later. **/

audio_manager_result_e audio_manager_init(void)
{
	unsigned int card_id, device_id;
	char type;
	struct dirent *dir_entry;
	DIR *dir_info;
	int i;

	for (i = 0; i < MAX_IN_AUDIO_DEV_NUM; i++) {
		input_audio_devices[i].status = AUDIO_DEVICE_UNIDENTIFIED;
	}
	for (i = 0; i < MAX_OUT_AUDIO_DEV_NUM; i++) {
		output_audio_devices[i].status = AUDIO_DEVICE_UNIDENTIFIED;
	}

	if (!(dir_info = opendir("/dev/audio/"))) {
		return AUDIO_MANAGER_FAIL;
	}

	dir_entry = readdir(dir_info);
	while (dir_entry) {
		printf("%s\n", dir_entry->d_name);
		// TODO: Add cases for various drivers. Currently, identify 'pcm' drivers only.
		if ((dir_entry->d_name[0] != 'p')
			|| (dir_entry->d_name[1] != 'c')
			|| (dir_entry->d_name[2] != 'm') || (sscanf(&dir_entry->d_name[3], "C%uD%u%c", &card_id, &device_id, &type) != 3)) {
			closedir(dir_info);
			return AUDIO_MANAGER_INVALID_PARAMS;
		}

		if (type == 'c' && device_id < MAX_IN_AUDIO_DEV_NUM && input_audio_devices[device_id].status == AUDIO_DEVICE_UNIDENTIFIED) {
			input_audio_devices[device_id].status = AUDIO_DEVICE_IDENTIFIED;
		} else if (type == 'p' && device_id < MAX_OUT_AUDIO_DEV_NUM && output_audio_devices[device_id].status == AUDIO_DEVICE_UNIDENTIFIED) {
			output_audio_devices[device_id].status = AUDIO_DEVICE_IDENTIFIED;
		} else {
			return AUDIO_MANAGER_INVALID_PARAMS;
		}

		dir_entry = readdir(dir_info);
	}
	closedir(dir_info);

	// Pre-open fd to handle volume changes..
	out_dev_fd = open("/dev/audio/pcmC0D0c", O_WRONLY);
	if (out_dev_fd < 0) {
		printf("[audio_manager.c] file open failed in audio_manager_init(), fd = %d\n", out_dev_fd);
		return -1;
	}

	return AUDIO_MANAGER_SUCCESS;
}

int get_avail_audio_card_id(void)
{
	printf("[audio_manager] get_avail_audio_card_id() is called\n");
	return 0;
}

int get_avail_audio_device_id(void)
{
	printf("[audio_manager] get_avail_audio_device_id() is called\n");
	return 0;
}

audio_manager_result_e set_audio_volume(audio_volume_e volume)
{
	struct audio_caps_desc_s cap_desc;
	audio_manager_result_e ret = AUDIO_MANAGER_SUCCESS;

	if (!(volume == AUDIO_VOLUME_LOW || volume == AUDIO_VOLUME_MEDIUM || volume == AUDIO_VOLUME_HIGH)) {
		return AUDIO_MANAGER_INVALID_PARAMS;
	}

	cap_desc.caps.ac_len = sizeof(struct audio_caps_s);
	cap_desc.caps.ac_type = AUDIO_TYPE_FEATURE;
	cap_desc.caps.ac_format.hw = AUDIO_FU_VOLUME;
	cap_desc.caps.ac_controls.hw[0] = volume;

	ret = ioctl(out_dev_fd, AUDIOIOC_SETVOLUME, (unsigned long)&cap_desc);
	if (ret < 0) {
		ret = AUDIO_MANAGER_FAIL;
		printf("AUDIOIOC_SETVOLUME ioctl failed, ret = %d\n", ret);
	}
	return ret;
}
