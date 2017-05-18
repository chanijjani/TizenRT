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

	char data[12] = { 0xDE, 0xAD, 0xBE, 0xEF,  // our data to write
					   0xDE, 0xAD, 0xBE, 0xEF,
					   0xDE, 0xAD, 0xBE, 0xEF};
	ret = fotahal_write(fotahal_handle, data, sizeof(data));
	if (ret != FOTAHAL_RETURN_SUCCESS) {
		printf("%s : fotahal_write error. %d\n", __func__, ret);
		goto write_error;
	}
	printf("[FOTA_WRITE]\n");
	//*
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
