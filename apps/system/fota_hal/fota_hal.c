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
#include <fcntl.h>
#include <tinyara/config.h>
#include <tinyara/fs/ioctl.h>
#ifdef CONFIG_TASH
#include <apps/shell/tash.h>
#endif
#include <apps/system/fota_hal.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Type
 ****************************************************************************/
struct priv_fotahal_handle_s {
	uint32_t *priv;
};

typedef struct priv_fotahal_handle_s priv_fotahal_handle_t;

/****************************************************************************
 * Private Data
 ****************************************************************************/
static uint32_t g_fota_fd;
static bool g_partition_set = false;
static bool g_binary_set = false;
static char fota_driver_path[20] = "/dev/mtdblock8";
static priv_fotahal_handle_t g_priv_handle;

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static int verify_fotahal_handle(fotahal_handle_t handle) {
	priv_fotahal_handle_t *priv_handle = (priv_fotahal_handle_t *) handle;

	if (priv_handle && priv_handle->priv) {
		return OK;
	}

	return ERROR;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * Name: fotahal_open
 *
 * Description:
 *   open fotahal layer
 ****************************************************************************/
fotahal_handle_t fotahal_open(void) {
	int fota_fd;

	//* open fota driver
	fota_fd = open(fota_driver_path, O_RDWR);
	if (fota_fd < 0) {
		dbg("%s: fota driver open failed\n", __func__);
		return NULL;
	}
	//*/
	/*
	fota_fd = fs_initiate("/dev/fota", "fota_mtd");
	if (fota_fd < 0) {
		dbg("fs_initiate error\n");
	}
	//*/
	g_fota_fd = fota_fd;
	g_priv_handle.priv = &g_fota_fd;

	g_partition_set = false;
	g_binary_set = false;

	dbg("[fota_hal.c] fotahal_open success, g_fota_fd = %d\n", g_fota_fd);
	return (fotahal_handle_t) &g_priv_handle;
}

/****************************************************************************
 * Name: fotahal_get_partition
 *
 * Description:
 *   Get current running partition id
 ****************************************************************************/
fota_partition_id_t fotahal_get_partition(fotahal_handle_t handle) {
	int ret;
	if (verify_fotahal_handle(handle) != OK) {
		return FOTAHAL_RETURN_WRONGHANDLE;
	}

	ret = ioctl(g_fota_fd, FOTA_GET_PART, 0);
	if (ret == -1) {
		dbg("%s: fota driver ioctl failed\n", __func__);
		return ret;
	}

	return (fota_partition_id_t) ret;
}

/****************************************************************************
 * Name: fotahal_set_partition
 *
 * Description:
 *   Set a new partition for fota write
 ****************************************************************************/
fotahal_return_t fotahal_set_partition(fotahal_handle_t handle,
		fota_partition_id_t part_id) {
	int ret;
	if (verify_fotahal_handle(handle) != OK) {
		return FOTAHAL_RETURN_WRONGHANDLE;
	}

	ret = ioctl(g_fota_fd, FOTA_SET_PART, (unsigned long) part_id);
	if (ret == -1) {
		dbg("%s: fota driver ioctl failed\n", __func__);
		return FOTAHAL_RETURN_PART_NOTSET;
	}

	g_partition_set = true;
	return FOTAHAL_RETURN_SUCCESS;
}

/****************************************************************************
 * Name: fotahal_set_binary
 *
 * Description:
 *   Set a binary type for fota write
 ****************************************************************************/
fotahal_return_t fotahal_set_binary(fotahal_handle_t handle, uint32_t bin_id) {
	int ret;
	if (verify_fotahal_handle(handle) != OK) {
		return FOTAHAL_RETURN_WRONGHANDLE;
	}

	ret = ioctl(g_fota_fd, FOTA_SET_BIN, (unsigned long) bin_id);
	if (ret == -1) {
		dbg("%s: fota driver ioctl failed\n", __func__);
		return FOTAHAL_RETURN_BIN_NOTSET;
	}

	g_binary_set = true;
	return FOTAHAL_RETURN_SUCCESS;
}

/****************************************************************************
 * Name: fotahal_write
 *
 * Description:
 *   write binary chunck in buffer to fota partition
 ****************************************************************************/
fotahal_return_t fotahal_write(fotahal_handle_t handle, const char *buffer,
		uint32_t bin_size) {
	int ret;

	if (verify_fotahal_handle(handle) != OK) {
		return FOTAHAL_RETURN_WRONGHANDLE;
	}
	/*
	 if (g_partition_set == false) {
	 return FOTAHAL_RETURN_PART_NOTSET;
	 }

	 if (g_binary_set == false) {
	 return FOTAHAL_RETURN_BIN_NOTSET;
	 }
	 */
	ret = write(g_fota_fd, buffer, bin_size);
	dbg("[Write Done] fd: %d, target: %d, ret: %d\n", g_fota_fd, bin_size, ret);
	if (ret != bin_size) {
		dbg("%s: fota driver write failed\n", __func__);
		return FOTAHAL_RETURN_WRITEFAIL;
	}

	return FOTAHAL_RETURN_SUCCESS;
}
//*
fotahal_return_t fotahal_read(fotahal_handle_t handle, char *buffer,
		uint32_t bin_size) {
	int ret;
	ret = read(g_fota_fd, buffer, bin_size);
	dbg("[Read Done] fd: %d, target: %d, ret: %d\n", g_fota_fd, bin_size, ret);
	if (ret != bin_size) {
		dbg("%s: fota driver read failed: %d\n", __func__, bin_size);
		return FOTAHAL_RETURN_WRITEFAIL;
	}

	return FOTAHAL_RETURN_SUCCESS;
}
//*/
/****************************************************************************
 * Name: fotahal_update_bootparam
 *
 * Description:
 *   Update boot param
 ****************************************************************************/
fotahal_return_t fotahal_update_bootparam(fotahal_handle_t handle) {
	int ret;
	if (verify_fotahal_handle(handle) != OK) {
		return FOTAHAL_RETURN_WRONGHANDLE;
	}

	ret = ioctl(g_fota_fd, FOTA_SET_PARAM, 0);
	if (ret == -1) {
		dbg("%s: fota driver ioctl failed\n", __func__);
		return FOTAHAL_RETURN_PARAM_NOTSET;
	}

	return FOTAHAL_RETURN_SUCCESS;
}

/****************************************************************************
 * Name: fotahal_close
 *
 * Description:
 *   close fotahal
 ****************************************************************************/
fotahal_return_t fotahal_close(fotahal_handle_t handle) {
	priv_fotahal_handle_t *priv_handle = (priv_fotahal_handle_t *) handle;

	if (verify_fotahal_handle(handle) != OK) {
		return FOTAHAL_RETURN_WRONGHANDLE;
	}

	close(g_fota_fd);

	g_partition_set = false;
	g_binary_set = false;
	priv_handle->priv = NULL;
	return FOTAHAL_RETURN_SUCCESS;
}
