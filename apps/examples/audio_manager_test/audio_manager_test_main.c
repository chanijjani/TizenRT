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

#include <sys/socket.h>

#define AUDIO_TEST_FILE "/mnt/pcm"
#define AUDIO_DEFAULT_RATE 16000
#define AUDIO_RECORD_DURATION 3	//3sec

record_result_t audio_record_pcm(int fd);
void clean_all_data(int fd);

static void audio_pcm_readi_p(void);
void audio_pcm_writei_p(void);

//struct pcm *g_pcm;
//char *buffer;

/****************************************************************************
 * audio_manager_test_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int audio_manager_test_main(int argc, char **argv)
#endif
{
//	int fd = -1;

	if (audio_manager_init() != AUDIO_MANAGER_SUCCESS) {
		printf("Error: audio_manager_init()\n");
		return 0;
	} else {
		printf("Audio Manager Init Okay..\n");
	}

	audio_pcm_writei_p();

	return 0;
}


void audio_pcm_writei_p(void)
{
	int fd;
	int ret;

	char *buffer;
	int num_read;
	int total_frames;
	unsigned int size;

	/* use default config here */
	struct pcm *g_pcm = pcm_open(0, 0, PCM_OUT, NULL);

	set_audio_volume(100);

	size = pcm_frames_to_bytes(g_pcm, pcm_get_buffer_size(g_pcm));
	buffer = malloc(size);

	fd = open("/mnt/record2", O_RDONLY);

	printf("Playback start!!\n");

	total_frames = 0;
	do {
		num_read = read(fd, buffer, size);
		if (num_read > 0) {
			total_frames += num_read;
			ret = pcm_writei(g_pcm, buffer, pcm_bytes_to_frames(g_pcm, num_read));
		}
	} while (num_read > 0);

	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}
	if (fd > 0) {
		close(fd);
	}

	printf("Playback done!!\n");
}
