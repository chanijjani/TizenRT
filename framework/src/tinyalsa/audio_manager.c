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

audio_manager_result_e audio_manager_init(void)
{
	struct dirent *dir_entry;
	DIR	*dir_info = opendir("/dev/audio/");
	if (!dir_info) {
		return AUDIO_MANAGER_FAIL;
	}

	dir_entry = readdir(dir_info);
	while (dir_entry) {
		printf("%s\n", dir_entry->d_name);
		dir_entry = readdir(dir_info);
	}

	closedir(dir_info);

	return AUDIO_MANAGER_INITIALIZED;
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


