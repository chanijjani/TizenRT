#!/usr/bin/env bash
###########################################################################
#
# Copyright 2019 Samsung Electronics All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the License.
#
###########################################################################

set -e

THIS_PATH=`test -d ${0%/*} && cd ${0%/*}; pwd`

# When location of this script is changed, only TOP_PATH should be changed together!!!
TOP_PATH=${THIS_PATH}/../../..

OS_PATH=${TOP_PATH}/os
OUTBIN_PATH=${TOP_PATH}/build/output/bin

CONFIG=${OS_PATH}/.config
if [ ! -f ${CONFIG} ]; then
        echo "No .config file"
        exit 1
fi

source ${CONFIG}
source ${THIS_PATH}/imxrt1020-evk_get_filename.sh

MAKE_FLASHFILE() {
	if [[ "${CONFIG_APP_BINARY_SEPARATION}" == "y" ]]; then
		# Extract partition sizes form CONFIG_FLASH_PART_SIZE
		OIFS=$IFS
		IFS=","
		NAMES=($CONFIG_FLASH_PART_NAME)
		SIZES=($CONFIG_FLASH_PART_SIZE)

		# Set the flash partition sizes.
		for i in "${!NAMES[@]}"; do
			if [ "${NAMES[$i]}" = "kernel" ]; then
				let OS_PART=${SIZES[$i]}*1024
				echo "OS_PART="$OS_PART
			elif [ "${NAMES[$i]}" = "app" ]; then
				let USER_PART=${SIZES[$i]}*1024
				echo "USER_PART="$USER_PART
			elif [ "${NAMES[$i]}" = "micom" ]; then
				let APP1_PART=${SIZES[$i]}*1024
				echo "APP1_PART="$APP1_PART
			elif [ "${NAMES[$i]}" = "wifi" ]; then
				let APP2_PART=${SIZES[$i]}*1024
				echo "APP2_PART="$APP2_PART
			fi
		done

		IFS=$OIFS

		# Fetch the binary file sizes

		KERN_SIZE=$(stat -c%s "$KERN_IMG")
		USER_SIZE=$(stat -c%s "$USER_IMG")
		APP1_SIZE1=$(stat -c%s "$APP1_IMG1")
		APP1_SIZE2=$(stat -c%s "$APP1_IMG2")
		APP2_SIZE1=$(stat -c%s "$APP2_IMG1")
		APP2_SIZE2=$(stat -c%s "$APP2_IMG2")

		# Check the size of partition and file
		if [ ${OS_PART} -lt ${KERN_SIZE} -o ${USER_PART} -lt ${USER_SIZE} -o ${APP1_PART} -lt ${APP1_SIZE1} -o ${APP2_PART} -lt ${APP2_SIZE1} ]; then
			echo "FAIL to make \"${FLASH_IMG##*/}\"!! Partition size is smaller than binary. Please resize it using menuconfig."
			exit 1
		fi

		# Calculate padding sizes. Padding is required to adjust the binary size with
		# the flash partition size.

		let kern_padding="$OS_PART - $KERN_SIZE"
		let user_padding="$USER_PART - $USER_SIZE"
		let app1_padding1="$APP1_PART - $APP1_SIZE1"
		let app1_padding2="$APP1_PART - $APP1_SIZE2"
		let app2_padding1="$APP2_PART - $APP2_SIZE1"
		let app2_padding2="$APP2_PART - $APP2_SIZE2"

		# Create a temporary flash img file by concatenation of all binaries and padding

		(
		head -c $KERN_SIZE $KERN_IMG ;
		head -c $kern_padding /dev/zero ;
		head -c $USER_SIZE $USER_IMG ;
		head -c $user_padding /dev/zero ;
		head -c $APP1_SIZE1 $APP1_IMG1 ;
		head -c $app1_padding1 /dev/zero ;
		head -c $APP1_SIZE2 $APP1_IMG2 ;
		head -c $app1_padding2 /dev/zero ;
		head -c $APP2_SIZE1 $APP2_IMG1 ;
		head -c $app2_padding1 /dev/zero ;
		head -c $APP2_SIZE2 $APP2_IMG2 ;
		head -c $app2_padding2 /dev/zero ;
		) > ${FLASH_IMG}
	elif [[ "${CONFIG_BUILD_PROTECTED}" == "y" ]]; then

		# Extract partition sizes form CONFIG_FLASH_PART_SIZE
		OIFS=$IFS
		IFS=","
		NAMES=($CONFIG_FLASH_PART_NAME)
		SIZES=($CONFIG_FLASH_PART_SIZE)

		# Set the flash partition sizes.
		for i in "${!NAMES[@]}"; do
			if [ "${NAMES[$i]}" = "kernel" ]; then
				let OS_PART=${SIZES[$i]}*1024
			elif [ "${NAMES[$i]}" = "app" ]; then
				let USER_PART=${SIZES[$i]}*1024
			fi
		done

		IFS=$OIFS

		# Fetch the binary file sizes

		KERN_SIZE=$(stat -c%s "$KERN_IMG")
		USER_SIZE=$(stat -c%s "$USER_IMG")

		# Check the size of partition and file
		if [ ${OS_PART} -lt ${KERN_SIZE} -o ${USER_PART} -lt ${USER_SIZE} ]; then
			echo "FAIL to make \"${FLASH_IMG##*/}\"!! Partition size is smaller than binary. Please resize it using menuconfig."
			exit 1
		fi

		# Calculate padding sizes. Padding is required to adjust the binary size with
		# the flash partition size.

		let kern_padding="$OS_PART - $KERN_SIZE"
		let user_padding="$USER_PART - $USER_SIZE"

		# Create a temporary flash img file by concatenation of all binaries and padding

		(
		head -c $KERN_SIZE $KERN_IMG ;
		head -c $kern_padding /dev/zero ;
		head -c $USER_SIZE $USER_IMG ;
		head -c $user_padding /dev/zero ;
		) > $FLASH_IMG
	fi
}

GET_FILENAME #imxrt1020-evk_get_filename.sh
MAKE_FLASHFILE
