#!/bin/bash
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

SRCDIR=./src
mkdir -p $SRCDIR/smartfs
SMARTFS_TMPDIR=$SRCDIR/smartfs
BASE_DIR=$(pwd)/../..
SMARTFSDIR=$BASE_DIR/os/fs/smartfs
APPNAME=nxfuse

echo "==========Copying Smartfs files====================="
cp $SMARTFSDIR/smartfs.h $SMARTFS_TMPDIR/
cp $SMARTFSDIR/smartfs_utils.c $SMARTFS_TMPDIR/
cp $SMARTFSDIR/smartfs_smart.c $SMARTFS_TMPDIR/
cp $SMARTFSDIR/../driver/mtd/smart.c $SMARTFS_TMPDIR/
echo "Copying Done"

echo "============Executing Make command====================="
make

#Waiting for make to complete

rm -rf $SMARTFS_TMPDIR/*

echo -e "Generated nxfuse \nDone"
