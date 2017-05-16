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
#include <tinyara/config.h>
#ifdef CONFIG_TASH
#include <apps/shell/tash.h>
#endif
#include <apps/system/fota_hal.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

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

#define O_RDONLY    (1 << 0)	/* Open for read access (only) */
#define O_RDOK      O_RDONLY	/* Read access is permitted (non-standard) */
#define O_WRONLY    (1 << 1)	/* Open for write access (only) */
#define O_WROK      O_WRONLY	/* Write access is permitted (non-standard) */
#define O_RDWR      (O_RDOK|O_WROK)	/* Open for both read & write access */

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int fota_sample(int argc, char *argv[])
#endif
{
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
	/*
	printf(" before set partition\n", next_part_id);

	if (fotahal_set_partition(fotahal_handle, next_part_id)
			!= FOTAHAL_RETURN_SUCCESS) {
		printf("%s : fotahal_set_partition error\n", __func__);
		goto part_error;
	}
	printf(" current running partition is  [ OTA%d ]\n", next_part_id);
	*/
	char buffer[] = "dummy";
	ret = fotahal_write(fotahal_handle, buffer, sizeof(buffer));
	if (ret != FOTAHAL_RETURN_SUCCESS) {
		printf("%s : fotahal_write error. %d\n", __func__, ret);
		goto write_error;
	}
	printf("[FOTA_WRITE] %s\n", buffer);
	//*
	char temp[] = "yummy";
	if (fotahal_read(fotahal_handle, temp, sizeof(temp))
			!= FOTAHAL_RETURN_SUCCESS) {
		printf("%s : fotahal_read error\n", __func__);
		goto write_error;
	}
	printf("[FOTA_READ] %s\n", temp);
	//*/
	/*
	 int fota_fd;
	 int ota_size;
	 FAR struct boot_dev_s *dev;
	 struct inode *inode;
	 char path[20];
	 static char fota_driver_path[20] = "/dev/fota";
	 uint8_t *pBinData, *pBinData2;
	 // open fota driver
	 fota_fd = open(fota_driver_path, O_RDWR);
	 if (fota_fd < 0) {
	 dbg("%s: fota driver open failed\n", __func__);
	 return -1;
	 }

	 snprintf(path, sizeof(path), "/dev/ota");

	 ret = open_blockdriver(path, 0, &inode);

	 if (ret < 0) {
	 printf("Failed to open %s\n", path);
	 }
	 //printf("Open Sucess!! %s\n", path);

	 dev = (FAR struct boot_dev_s *) inode->i_private;

	 printf("Erase OTA Flash region \n");

	 ota_size = 0x002E0000;

	 // ota binary size / 4K
	 for (i = 0; i < (ota_size / 0x1000); i++) {
	 dev->offset = i;
	 dev->size = 0x1000;
	 inode->u.i_bops->ioctl(inode, 0, 0x20);
	 printf(".");
	 }
	 printf("\n");

	 pBinData = (uint8_t *) malloc(4096);
	 pBinData2 = (uint8_t *) malloc(4096);

	 for (i = 0; i < 4096; ++i) {
	 pBinData[i] = i % 255;
	 }

	 dev->offset = 0;
	 dev->size = 0x1000;
	 dev->buffer = pBinData;
	 inode->u.i_bops->ioctl(inode, 1, 0);

	 dev->offset = 0;
	 dev->size = 0x1000;
	 dev->buffer = pBinData2;

	 inode->u.i_bops->ioctl(inode, 2, 0);

	 for (i = 0; i < 255; ++i) {
	 printf("DATA_i = %d\n", pBinData[i]);
	 printf("DATA2_i = %d\n", pBinData2[i]);
	 }

	 //data_download(inode);

	 printf("OTA finish\n");

	 (void) close_blockdriver(inode);

	 free(pBinData);
	 free(pBinData2);
	 */
	write_error: part_error: fotahal_close(fotahal_handle);
	open_error:

	return ret;
}

/****************************************************************************
 * Name: fota_sample_app_install
 *
 * Description:
 *  Install fota_sample command to TASH
 ****************************************************************************/
void fota_sample_app_install(void) {
	tash_cmd_install("fota_update", fota_sample, TASH_EXECMD_ASYNC);
}
