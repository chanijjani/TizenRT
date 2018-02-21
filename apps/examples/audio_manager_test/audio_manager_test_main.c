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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <apps/shell/tash.h>
#include <fcntl.h>

#include <media/media_player.h>
#include <media/media_recorder.h>
#include <media/media_utils.h>
#include <tinyalsa/audio_manager.h>

void audio_play(audio_volume_e volume);

/****************************************************************************
 * audio_manager_test_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int audio_manager_test_main(int argc, char **argv)
#endif
{
	if (audio_manager_init() != AUDIO_MANAGER_SUCCESS) {
		printf("Error: audio_manager_init()\n");
		return 0;
	} else {
		printf("Audio Manager Init Okay..\n");
	}

	audio_play(AUDIO_VOLUME_LOW);

	audio_play(AUDIO_VOLUME_MEDIUM);

	audio_play(AUDIO_VOLUME_HIGH);

	return 0;
}

void audio_play(audio_volume_e volume)
{
	char *buffer;
	int fd;
	int num_read;
	int total_frames = 0;
	unsigned int size;
#ifdef CONFIG_ARCH_BOARD_ARTIK053
	const char file_path[] = "/mnt/pcm";
#elif CONFIG_ARCH_BOARD_ARTIK055S
	const char file_path[] = "/mnt/record2";
#endif

	/* use default config here */
	struct pcm *g_pcm = pcm_open(0, 0, PCM_OUT, NULL);
	if (!g_pcm) {
		printf("[audio_manager_init()] pcm_open() is failed\n");
		return;
	}

	set_audio_volume(volume);

	size = pcm_frames_to_bytes(g_pcm, pcm_get_buffer_size(g_pcm));
	if (size <= 0) {
		printf("[audio_manager_init()] Invalid frame size: %d\n", size);
		return;
	}
	buffer = malloc(size);

	fd = open(file_path, O_RDONLY);
	if (fd < 0) {
		printf("[audio_manager_init()] open() is failed: %s\n", file_path);
		return;
	}

	printf("Start Playing!!\n");
	do {
		num_read = read(fd, buffer, size);
		if (num_read > 0) {
			total_frames += num_read;
			pcm_writei(g_pcm, buffer, pcm_bytes_to_frames(g_pcm, num_read));
		}
	} while (num_read > 0);
	printf("Finish Playing!!\n");

	if (g_pcm) {
		pcm_close(g_pcm);
		g_pcm = NULL;
	}
	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}
	if (fd > 0) {
		close(fd);
	}
}
