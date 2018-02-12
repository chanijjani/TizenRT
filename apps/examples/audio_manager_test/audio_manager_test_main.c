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

#define AUDIO_DEFAULT_RATE 16000
#define AUDIO_RECORD_DURATION 3	//3sec

record_result_t audio_record_pcm(int fd);
void clean_all_data(struct pcm *g_pcm, int fd, char *buffer);

/****************************************************************************
 * audio_manager_test_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int audio_manager_test_main(int argc, char **argv)
#endif
{
	int fd = -1;

	if (audio_manager_init() != AUDIO_MANAGER_INITIALIZED) {
		printf("Error: audio_manager_init()\n");
		return 0;
	} else {
		printf("Audio Manager Init Okay..\n");
	}

	// Record voice to a file 'mnt/record'
	if (media_record_init() != RECORD_OK) {
		printf("Error: media_record_init()\n");
		return 0;
	} else {
		printf("Media Record Init Okay..\n");
	}

	fd = open("/mnt/record", O_TRUNC | O_CREAT | O_RDWR);
	if (fd < 0) {
		printf("Cannot create file!\n");
		return 0;
	}

	if (media_record_set_config(2, 16000, 0, MEDIA_FORMAT_PCM) != RECORD_OK) {
		printf("Error: media_record_set_config()\n");
		return 0;
	}

	if (audio_record_pcm(fd) != RECORD_OK) {
		printf("Error: media_record()\n");
		return 0;
	}

	// Play the recorded file.
	if (media_play_init() != PLAY_OK) {
		printf("Error: media_play_init()\n");
		return 0;
	} else {
		printf("MediaPlay Init Okay..\n");
	}

	if (media_play(fd, MEDIA_FORMAT_WAV) != MEDIA_OK) {
		printf("Error: media_play\n");
		return 0;
	}

	return 0;
}

record_result_t audio_record_pcm(int fd)
{
	char input_str[16];
//	int ret;
	char *str;
	char *buffer;
	unsigned int bytes_per_frame;
	int frames_read;
	int remain;
	int ret;
	struct pcm *g_pcm;
	int audio_card_id = get_avail_audio_card_id();
	int audio_device_id = get_avail_audio_device_id();
	g_pcm = pcm_open(audio_card_id, audio_device_id, PCM_IN, NULL);

	buffer = malloc(pcm_frames_to_bytes(g_pcm, pcm_get_buffer_size(g_pcm)));

	bytes_per_frame = pcm_frames_to_bytes(g_pcm, 1);
	frames_read = 0;
	remain = AUDIO_DEFAULT_RATE * AUDIO_RECORD_DURATION;

	printf("Sample will be recorded for 3s, press any key to start(Total frame: %d)\n", remain);
	fflush(stdout);
	str = gets(input_str);

	while (remain > 0) {
		frames_read = pcm_readi(g_pcm, buffer, pcm_get_buffer_size(g_pcm));
		if (frames_read <= 0) {
			break;
		}
		remain -= frames_read;
		ret = write(fd, buffer, bytes_per_frame * frames_read);
		//printf("[audio_record_pcm()] ret = %d\n", ret);
	}

	printf("Record done.\n");
	sleep(2);

	clean_all_data(g_pcm, fd, buffer);

	return RECORD_OK;
}

void clean_all_data(struct pcm *g_pcm, int fd, char *buffer)
{
	if (fd > 0) {
		close(fd);
	}

	if (g_pcm) {
		pcm_close(g_pcm);
		g_pcm = NULL;
	}

	if (buffer) {
		free(buffer);
	}
}
