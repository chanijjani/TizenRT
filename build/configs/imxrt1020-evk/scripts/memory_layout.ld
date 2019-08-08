/* This file is auto-generated based on values from .config file */

MEMORY
{
  kflash (rx)  : ORIGIN = 0x60000000, LENGTH = 2048K
  uflash (rx)  : ORIGIN = 0x60200000, LENGTH = 256K
  ksram  (rwx)  : ORIGIN = 0x20200000, LENGTH = 192K
  usram  (rwx)  : ORIGIN = 0x20230000, LENGTH = 64K
}
