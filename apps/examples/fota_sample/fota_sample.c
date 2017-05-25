/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
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
 * Included Files
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <tinyara/config.h>
#ifdef CONFIG_TASH
#include <apps/shell/tash.h>
#endif
#include <apps/system/fota_hal.h>

#include <sys/socket.h>
//#include "wifi.h"

/****************************************************************************
 * Struct
 ****************************************************************************/
char  m_packageuri[100] = "http://10.113.65.37/sample_download.txt";

/****************************************************************************
 * Public Functions
 ****************************************************************************/
//void startingFirmwareUpdate(fotahal_handle_t handle);
//int data_download(fotahal_handle_t handle, char *filename, char *serverIP, uint16_t port);
//int parsing_header(char *buf, char *value);

/****************************************************************************
 * Name: fota_sample
 *
 * Description:
 *      fota sample application
 ****************************************************************************/
struct boot_dev_s {
	uint32_t offset;
	uint32_t size;
	uint8_t *buffer;
	uint32_t base_address;
};

//static struct wifi_ap_info_s *_ap_info;

enum TC_RETURN_CODE
{
	TC_RET_ERROR_NONE          = 0,
	TC_RET_WIFI_ARGUMENT_FAIL  = -1,    /* --> return */
	TC_RET_WIFI_CONNECT_FAIL   = -2,    /* --> wifi_destory */
	TC_RET_WIFI_START_FAIL     = -3,    /* --> wifi_destory */
	TC_RET_OCSERVER_INIT_FAIL  = -4,    /* --> wifi_stop, wifi_destory */
	TC_RET_OCSERVER_START_FAIL = -5     /* --> ocserver_destory, wifi_stop, wifi_destory */
};

#define O_RDONLY    (1 << 0)	/* Open for read access (only) */
#define O_RDOK      O_RDONLY	/* Read access is permitted (non-standard) */
#define O_WRONLY    (1 << 1)	/* Open for write access (only) */
#define O_WROK      O_WRONLY	/* Write access is permitted (non-standard) */
#define O_RDWR      (O_RDOK|O_WROK)	/* Open for both read & write access */

#define OTA_SIZE 0x002E0000
#define HTTP_LEN_MSG "Content-Length:"
/*
#define _WIFI_DEFAULT_CHANNEL 11

static int _wifi_start(struct wifi_api_s *wifi)
{
	int ret = -1;

	if (!wifi)
	{
		//_ERROR_LOG("error has occurred while getting WIFI APIs");
		return TC_RET_WIFI_ARGUMENT_FAIL;
	}

	ret = wifi->connect(_ap_info);
	if(0 != ret)
	{
		//_ERROR_LOG("error has occurred while connect wifi(%d)", ret);
		return TC_RET_WIFI_CONNECT_FAIL;
	}

	// Step 2. DHCP Server Start
	ret = wifi->start();
	if(0 != ret)
	{
		//_ERROR_LOG("error has occurred while start the dhcp(%d)", ret);
		return TC_RET_WIFI_START_FAIL;
	}

	return TC_RET_ERROR_NONE;

}
*/

int parsing_header(char *buf, char *value)
{
	int i;
	int pos_len;
	int pos_end = 0;
	char str[15];

	snprintf(str, sizeof(str), HTTP_LEN_MSG);

	for (i = 0; i < 512; i++) {
		if (!strncmp(buf + i, "Content-Length:", 15))
			break;
	}
	pos_len = i + 16;

	{
		for (i = pos_len + 1; i < 512; i++)
			if (buf[i] == '\n' && buf[i - 1] == '\r') {
				pos_end = i - 1;
				break;
			}
	}

	for (i = pos_len; i < 512 - pos_end; i++) {
		value[i - pos_len] = buf[i];
		if (i - pos_len < 0) {
			return -1;
		}
	}
	value[i - pos_len] = '\0';

	for (i = pos_end; i < 512 - pos_end; i++) {
		if (buf[i] == '\n' && buf[i - 1] == '\r' && buf[i - 2] == '\n'
				&& buf[i - 3] == '\r') {
			return i + 1;
		}
	}

	printf("Can not find end of header\n");

	return 0;
}

int data_download(fotahal_handle_t handle, char *filename, char *serverIP, uint16_t port)
{
	int totalDownloadedByte = 0;
	int socket_fd;
	int ret;
	int original_content_len;
	int content_len;
	int content_pos;
	int readbyte;
	int readbyte_sum;
	int i, pos;
	int first_size, first_remain;
	int remain, data_num;
	int crc_len;
	char msg[64] = { 0, };
	char size[32] = { 0, };
	char first_data[512] = { 0, };
	struct sockaddr_in server;
	char pBinData[4096];

	//pBinData = (uint8_t *) malloc(4096);

	snprintf(msg, sizeof(msg), "GET %s HTTP/1.0 \n\n", filename);

	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd < 0) {
		printf("ERROR: socket failed: %d\n", errno);
	}

	server.sin_addr.s_addr = inet_addr(serverIP);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	printf("Try to Connect OTA Server\n");
	ret = connect(socket_fd, (struct sockaddr *) &server,
			sizeof(struct sockaddr_in));

	if (ret < 0) {
		printf("ERROR: connect failed: %d errno: %d\n", ret, errno);
		close(socket_fd);
		return -1;
	} else {
		printf("Connect Success\n");
	}
	printf("Request OTA Binary\n");

	send(socket_fd, msg, sizeof(msg), 0);
	readbyte = recv(socket_fd, first_data, 512, 0);

	content_pos = parsing_header(first_data, size);

	if (content_pos == 0) {
		return -1;
	}

	original_content_len = content_len = atoi(size);
	crc_len = content_len;

	printf("Binary Size = %d\n", content_len);
	first_size = 512 - content_pos;

	first_remain = 4096 - first_size;

	for (pos = 0; pos < first_size; pos++)
		pBinData[pos] = first_data[content_pos + pos];

	//totalDownloadedByte += first_size;
	printf("first_size : %d\n", first_size);

	printf("first_remain : %d\n\n", first_remain);

	readbyte_sum = first_size;
	while (readbyte_sum < 4096) {
		readbyte = recv(socket_fd, &pBinData[readbyte_sum], 4096 - readbyte_sum,
				MSG_WAITALL);
		readbyte_sum += readbyte;
	}

	// first 4k byte write
	fotahal_write(handle, pBinData, 0x1000);

	totalDownloadedByte += readbyte_sum;

	printf("Download OTA File\n");
	printf(".");

	content_len -= 4096;

	data_num = content_len / 4096;
	remain = content_len % 4096;

	int k = 0;
	printf("- %d : - %d\n", k, readbyte_sum);
	for (i = 1; i < data_num + 1; i++) {
		readbyte_sum = 0;
		readbyte = 0;

		while (readbyte_sum < 4096) {
			readbyte = recv(socket_fd, &pBinData[readbyte_sum],
					4096 - readbyte_sum, MSG_WAITALL);
			readbyte_sum += readbyte;
			printf(".");
		}
		totalDownloadedByte += readbyte_sum;

		fotahal_write(handle, pBinData, 0x1000);

		printf("%d : - %d /", ++k, readbyte_sum);
		if (i % 5 == 0)
			printf("\n");
	}

	readbyte = recv(socket_fd, pBinData, remain, MSG_WAITALL);

	totalDownloadedByte += readbyte;

	fotahal_write(handle, pBinData, 0x1000);

#if 1
	printf("\n");
	for (i = readbyte - 10; i < readbyte; i++) {
		printf("%x ", pBinData[i]);
	}

	printf("\n\n lastReadByte : %d\n", readbyte);
	printf("\n\n totalDownloadedByte : %d\n\n\n\n", totalDownloadedByte);

#endif

	printf(".\n");

	close(socket_fd);
	free(pBinData);

	if (totalDownloadedByte != original_content_len) {
		return -1;
	}

	//crc_download(crc_len);

	return 0;
}

void startingFirmwareUpdate(fotahal_handle_t handle)
{
	printf("startingFirmwareUpdate URI : %s\n", m_packageuri);
	//OCStackResult result = OC_STACK_ERROR;
	//result = OCNotifyAllObservers(gFirmwareInstance.handle, OC_NA_QOS);

	int i;
	//int ret;
	int ota_size;

	printf("Erase OTA Flash region \n");

	ota_size = OTA_SIZE;

	/* ota binary size / 4K */
	for (i = 0; i < (ota_size / 0x1000); i++) {
		//inode->u.i_bops->ioctl(inode, OTA_CMD_ERASE, 0x20);
		printf(".");
	}
	printf("\n");

	char filename[100];
	char hostname[80] = "http://www.seilevel.com:80/wp-content/uploads/DFDExample.png";
	uint16_t port = 80;

	//netlib_parsehttpurl(m_packageuri, &port, hostname, 20, filename, 100);
	//printf("New hostname='%s' filename='%s' port : %u\n", hostname, filename,	port);

	int retryCount = 0;
	while (data_download(handle, filename, hostname, port) == -1) {
		if (retryCount > 1)
			break;

		printf("\n\n network error\n Retry to Download tryCount : %d\n",
				++retryCount);
		printf(".\n");
	}

	printf("OTA finish\n");
	printf(
			"Firmware Update Done.....\nIt will be rebooted........Bye Bye.....\n");
	sleep(1);
	//board_reset(1);

	return;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int fota_sample(int argc, char *argv[])
#endif
{
	/*
	_ap_info = wifi_set_ap_info("Gorani", SLSI_SEC_MODE_WPA2_MIXED, "roadkill1", _WIFI_DEFAULT_CHANNEL);
	struct wifi_api_s *wifi = NULL;
	wifi = wifi_create(SLSI_WIFI_STATION_IF, NULL, NULL);
	int ret = _wifi_start(wifi);

	//sleep(5000);

	switch (ret)
	{
	case TC_RET_ERROR_NONE :
		printf("connected to wifi AP!");
		break;
	case TC_RET_WIFI_ARGUMENT_FAIL :    // --> return
		printf("error has occurred while get WIFI APIs");
		return -1;
	case TC_RET_WIFI_CONNECT_FAIL :    // --> wifi_destory
	case TC_RET_WIFI_START_FAIL :    // --> wifi_destory
		printf("error has occurred while start or connect WIFI");
		return -1;
	default:
		return -1;
	}
	*/
	//int i;
	int ret = ERROR;
	uint32_t part_id;
	uint32_t next_part_id;
	fotahal_handle_t fotahal_handle;

	printf("***********************************************************\n");
	printf(" fota update is in progress !!!\n");

	/* open fota hal, get the handle to work with it */
	if ((fotahal_handle = fotahal_open()) == NULL) {
		printf("%s : fotahal_open error\n", __func__);
		goto open_error;
	}

	/* Get the current running partition */
	part_id = fotahal_get_partition(fotahal_handle);
	if (part_id == -1) {
		printf("%s : fotahal_get_partition error\n", __func__);
		goto part_error;
	}
	printf(" current running partition is  [ OTA%d ]\n", part_id);

	for (next_part_id = FOTA_PARTITION_OTA0;
			(next_part_id < FOTA_PARTITION_MAX) && next_part_id == part_id;
			next_part_id++)
		;

	if (next_part_id >= FOTA_PARTITION_MAX) {
		printf("%s : No free partition left\n", __func__);
		goto part_error;
	}
	/**
	char data[12] = { 0xDE, 0xAD, 0xBE, 0xEF,  // our data to write
					   0xDE, 0xAD, 0xBE, 0xEF,
					   0xDE, 0xAD, 0xBE, 0xEF};
	ret = fotahal_write(fotahal_handle, data, sizeof(data));
	if (ret != FOTAHAL_RETURN_SUCCESS) {
		printf("%s : fotahal_write error. %d\n", __func__, ret);
		goto write_error;
	}
	printf("[FOTA_WRITE]\n");

	char read_buf[12] = {0x00};
	if (fotahal_read(fotahal_handle, read_buf, sizeof(read_buf))
			!= FOTAHAL_RETURN_SUCCESS) {
		printf("%s : fotahal_read error\n", __func__);
		goto write_error;
	}
	printf("[FOTA_READ]\n");
	int i = 0;
	for (i = 0; i < 12; i++)
		printf("buf[%d] = 0x%02x\n", i, (unsigned int) read_buf[i]);
	**/
	startingFirmwareUpdate(fotahal_handle);

	//write_error:
	part_error: fotahal_close(fotahal_handle);
	open_error:

	return ret;
}

/****************************************************************************
 * Name: fota_sample_app_install
 *
 * Description:
 *  Install fota_sample command to TASH
 ****************************************************************************/
void fota_sample_app_install(void)
{
	tash_cmd_install("fota_update", fota_sample, TASH_EXECMD_ASYNC);
}
