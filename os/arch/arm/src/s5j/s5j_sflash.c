/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics Co., LTD. All Rights Reserved.
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
 * arch/arm/src/s5j/s5j_sflash.c
 *
 *   Copyright (C) 2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name tinyara nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <tinyara/config.h>

#include <debug.h>
#include <errno.h>
#include <tinyara/irq.h>
#include <tinyara/progmem.h>

#include "up_arch.h"
#include "cache.h"
#include "chip.h"

#include "s5j_vclk.h"
#include "s5j_gpio.h"

#include "tinyara/spi/spi.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define rSF_CON			(SFI_BASE + 0x0004)
#define rERASE_ADDRESS		(SFI_BASE + 0x0010)
#define rCOMMAND1		(SFI_BASE + 0x001C)
#define rCOMMAND2		(SFI_BASE + 0x0020)
#define rCOMMAND3		(SFI_BASE + 0x0024)
#define rCOMMAND4		(SFI_BASE + 0x0028)
#define rCOMMAND5		(SFI_BASE + 0x002C)
#define rADDR_CMD		(SFI_BASE + 0x0059)
#define rSE			(SFI_BASE + 0x005E)
#define rFLASH_IO_MODE		(SFI_BASE + 0x0074)
#define rFLASH_PERF_MODE	(SFI_BASE + 0x0078)
#define rRDID			(SFI_BASE + 0x00AC)
#define rBE			(SFI_BASE + 0x00BE)
#define rCE			(SFI_BASE + 0x00CE)
#define rRDSR			(SFI_BASE + 0x00DC)
#define rWRDI			(SFI_BASE + 0x00DD)
#define rWRSR			(SFI_BASE + 0x00DE)
#define rWREN			(SFI_BASE + 0x00EE)

/* Command definitions */

enum s5j_sflash_command_e {
	SFLASH_WRITE_STATUS_REGISTER = 0x01, /* WRSR                   */
	SFLASH_WRITE = 0x02, SFLASH_READ = 0x03, SFLASH_WRITE_DISABLE = 0x04, /* WRDI                   */
	SFLASH_READ_STATUS_REGISTER = 0x05, /* RDSR                   */
	SFLASH_WRITE_ENABLE = 0x06, /* WREN                   */
	SFLASH_SECTOR_ERASE = 0x20, /* SE                     */
	SFLASH_READ_STATUS_REGISTER2 = 0x35, /* RDSR-2 - Winbond only  */
	SFLASH_BLOCK_ERASE_MID = 0x52, /* SE                     */
	SFLASH_BLOCK_ERASE_LARGE = 0xD8, /* SE                     */
	SFLASH_READ_ID1 = 0x90, /* data size varies       */
	SFLASH_READ_ID2 = 0xAB, /* data size varies       */
	SFLASH_READ_JEDEC_ID = 0x9F, /* RDID                   */
	SFLASH_CHIP_ERASE1 = 0x60, /* CE                     */
	SFLASH_CHIP_ERASE2 = 0xC7, /* CE                     */
	SFLASH_DUMMY = 0xA5, /* Dummy */
};

FAR struct s5j_sflash_dev_s *sflashdev;
//FAR struct spi_dev_s *spidev;

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static void s5j_sflash_set_gpio(void) {
	int gpio_sf_clk;
	int gpio_sf_cs, gpio_sf_si, gpio_sf_so, gpio_sf_wp, gpio_sf_hld;

	gpio_sf_clk = s5j_gpio(GPP1, 0);
	gpio_sf_cs = s5j_gpio(GPP1, 1);
	gpio_sf_si = s5j_gpio(GPP1, 2);
	gpio_sf_so = s5j_gpio(GPP1, 3);
	gpio_sf_wp = s5j_gpio(GPP1, 4);
	gpio_sf_hld = s5j_gpio(GPP1, 5);

	gpio_cfg_pin(gpio_sf_clk, GPIO_FUNC(2));
	gpio_cfg_pin(gpio_sf_cs, GPIO_FUNC(2));
	gpio_cfg_pin(gpio_sf_si, GPIO_FUNC(2));
	gpio_cfg_pin(gpio_sf_so, GPIO_FUNC(2));
	gpio_cfg_pin(gpio_sf_wp, GPIO_FUNC(2));
	gpio_cfg_pin(gpio_sf_hld, GPIO_FUNC(2));

	gpio_set_pull(gpio_sf_clk, GPIO_PULL_UP);
	gpio_set_pull(gpio_sf_cs, GPIO_PULL_UP);
	gpio_set_pull(gpio_sf_si, GPIO_PULL_UP);
	gpio_set_pull(gpio_sf_so, GPIO_PULL_UP);
	gpio_set_pull(gpio_sf_wp, GPIO_PULL_UP);
	gpio_set_pull(gpio_sf_hld, GPIO_PULL_UP);
}

void s5j_sflash_init(void);
//static int s5j_sflash_write_enable(struct s5j_sflash_dev_s *dev);

/******************************************************************************
 * Private Data
 *****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/
static void s5j_sflash_disable_wp(void) {
	unsigned int sfcon;

	/* someone has been disabled wp, we should wait until it's released */
	do {
		sfcon = getreg32(rSF_CON);
	} while (sfcon & (1 << 31));

	sfcon = getreg32(rSF_CON) | (1 << 31);
	putreg32(sfcon, rSF_CON);
}

static void s5j_sflash_enable_wp(void) {
	unsigned int sfcon;

	sfcon = getreg32(rSF_CON) & ~(1 << 31);
	putreg32(sfcon, rSF_CON);
}

static uint8_t s5j_sflash_read_status(void) {
	return getreg8(rRDSR);
}

#ifdef CONFIG_MTD_PROGMEM
size_t up_progmem_getaddress(size_t page) {
	return S5J_FLASH_PADDR + up_progmem_pagesize(page) * page;
}

ssize_t up_progmem_getpage(size_t addr) {
	return (addr - S5J_FLASH_PADDR) / up_progmem_pagesize(0);
}

ssize_t up_progmem_erasepage(size_t page) {
	size_t addr;
	irqstate_t irqs;

	if (page >= up_progmem_npages())
		return -EFAULT;

	addr = up_progmem_getaddress(page);

	/* Disable IRQs while erasing sector */
	irqs = irqsave();

	s5j_sflash_disable_wp();

	/* Set sector address and then send erase command */
	putreg32(addr - S5J_FLASH_PADDR, rERASE_ADDRESS);
	putreg8(0xff, rSE);

	/* Wait for the completion */
	while (s5j_sflash_read_status() & 0x1)
		;

	s5j_sflash_enable_wp();

	/* Invalidate cache */
	arch_invalidate_dcache(addr, addr + up_progmem_pagesize(page));

	/* Restore IRQs */
	irqrestore(irqs);

	return up_progmem_pagesize(page);
}

ssize_t up_progmem_ispageerased(size_t page) {
	size_t addr;
	size_t count;
	size_t bwritten;

	if (page >= up_progmem_npages())
		return -EFAULT;

	bwritten = 0;
	addr = up_progmem_getaddress(page);
	for (count = up_progmem_pagesize(page); count; count--) {
		if (getreg32(addr) != 0xffffff)
			bwritten++;
		addr += sizeof(int);
	}

	return bwritten * sizeof(int);
}

ssize_t up_progmem_write(size_t addr, const void *buf, size_t count) {
	int page;
	int pagesize;
	size_t remain = count;

	page = up_progmem_getpage(addr + count);
	if (page < 0)
		return -EINVAL;

	pagesize = up_progmem_pagesize(page);

	while (remain) {
		int tmp = remain;
		irqstate_t irqs;

		if (tmp > pagesize)
			tmp = pagesize;

		/* Disable IRQs */
		irqs = irqsave();

		s5j_sflash_disable_wp();

		/* Load and write data */
		memcpy((void *) addr, buf, tmp);

		/* Flush cache */
		arch_flush_dcache(addr, addr + tmp);

		s5j_sflash_enable_wp();

		/* Restore IRQs */
		irqrestore(irqs);

		buf += tmp;
		addr += tmp;
		remain -= tmp;
	}

	return count;
}
#endif /* CONFIG_MTD_PROGMEM */


/******************************************************************************
 * Name: s5j_sflash_erase
 *
 * Description:
 *   erase flash
 *
 *****************************************************************************/
#define Outp32(addr, data)	(*(volatile u32 *)(addr) = (data))
#define Outp8(addr, data)  (*(volatile u8 *)(addr) = (data))
int s5j_sflash_erase(struct s5j_sflash_dev_s *dev, uint8_t cmd, uint32_t addr) {
	//int ret;
	//uint32_t val;
	int size = 0;

#ifdef S5J_SFLASH_USE_DIRECT_RW
	/* debug code */

	if (dev == NULL) {
		dev = sflashdev;
	}
#endif
	/*
	do {
		ret = s5j_sflash_write_enable(dev);
		if (0 != ret) {
			usleep(100);
		}
	} while (0 != ret);
	*/
	switch (cmd) {
	/*
	case SFLASH_CHIP_ERASE1:
		val = getreg32(rSF_CON) | (1 << 31) | (1 << 15);
		putreg32(val, rSF_CON);
		addr = 0;
		Outp8(rCE, 0x1);
		break;
	case SFLASH_CHIP_ERASE2:
		addr = 0;
		Outp8(rCE, 0x1);
		break;
		*/
	case SFLASH_SECTOR_ERASE:
		Outp32(rERASE_ADDRESS, addr);
		Outp8(rSE, 0x1);
		size = 4096; /* Sub-sector size */
		break;
		/*
	case SFLASH_BLOCK_ERASE_LARGE:
		size = 65536; // block size
		SetBits(rCOMMAND2, 16, 0xFF, 0xD8);
		// SetBits(rCOMMAND2, 16, 0xFF, COMMAND_ERASE_32KB);

		Outp32(rERASE_ADDRESS, addr);
		Outp8(rBE, 0x1);

		break;
		*/
	default:
		break;
	}

	while ( getreg8(rRDSR) & 0x1)
		;

	if (cmd != SFLASH_CHIP_ERASE1) {
		arch_invalidate_dcache(addr + 0x04000000, (addr + 0x04000000 + size));
	}

	return 0;
}

/******************************************************************************
 * Name: s5j_sflash_write
 *
 * Description:
 *  Write data.
 *
 *****************************************************************************/
#define MIN(a,b) (a < b ? a : b)

int s5j_sflash_write(struct s5j_sflash_dev_s *dev, uint32_t addr, uint8_t *buf,
		uint32_t size) {
	int32_t i;
	int32_t readsize;
	uint32_t offset;

	i = 0;

	/* write is directly called from nshlib, so may allow it */

#ifdef S5J_SFLASH_USE_DIRECT_RW
	/* debug code */

	if (dev == NULL)
	{
		dev = sflashdev;
	}
#endif

	while (size != 0) {
		/* first align */

		offset = addr % 4096;
		readsize = MIN(4096 - offset, size);

		memcpy((void *) (addr + 0x04000000), (void *) (buf + i), readsize);
		arch_flush_dcache(addr + 0x04000000, (addr + 0x04000000 + readsize));
		addr += readsize;
		i += readsize;
		size -= readsize;

		/* check write in progress */
		while ( getreg8(rRDSR) & 0x1)
			;
	}

	return 0;
}

/******************************************************************************
 * Name: s5j_sflash_read
 *
 * Description:
 *   Read data from flash.
 *
 *****************************************************************************/

int s5j_sflash_read(struct s5j_sflash_dev_s *dev,
    uint32_t addr, uint8_t *buf, uint32_t size)
{
  int32_t i;
  int32_t readsize;
  uint32_t offset;

  i = 0;

#ifdef S5J_SFLASH_USE_DIRECT_RW
  /* debug code */

  if (dev == NULL)
    {
      dev = sflashdev;
    }
#endif

  fvdbg("addr 0x%x, size %d\n", addr, size);
  while (size != 0)
    {
      /* first align */

      offset = addr % 4096;
      readsize = MIN(4096 - offset, size);

      memcpy((void *)(buf+i), (void *)(addr+0x04000000), readsize);

      addr += readsize;
      i += readsize;
      size -= readsize;
    }

  fvdbg("buf[%d]\n", *buf);
  return OK;
}

/**
 * @brief	initialize FLASH for QUAD IO in 80Mhz
 * @param	void
 * @return	void
 * @note
 */
void s5j_sflash_init(void) {
	/* Set mix i/o to be FLASH signal, CLK/CS/SI/SO/WP/HOLD */
	dbg("[s5j_sflash.c] s5j_sflash_init entered\n");
	s5j_sflash_set_gpio();

	putreg32(0x8010001A, rSF_CON); /* disable WP */
	putreg32(0x8, rFLASH_PERF_MODE); /* FLASH_PERF_MODE */
	putreg32(0x4, rFLASH_IO_MODE); /* QUAD */

	/* Check FLASH has Quad Enabled */
	while (!(s5j_sflash_read_status() & (0x1 << 6)))
		; lldbg("FLASH Quad Enabled\n");

	putreg32(0x0010001A, rSF_CON); /* Enable WP */

	/* Set FLASH clk 80Mhz for Max performance */
	cal_clk_setrate(d1_serialflash, 80000000);
}

/*******************************************************************************
 * Name: up_spiflashinitialize
 *
 * Description:
 *   Initialize the selected SPI port
 *
 * Returned Value:
 *   Valid SPI device structure reference on success; a NULL on failure
 *
 ******************************************************************************/

FAR struct spi_dev_s *up_spiflashinitialize(void) {
	static bool sflashdev_initialized = 0;
	/* Lets have single instance */
	dbg("[s5j_sflash.c] up_spiflashinitialize\n");
	if (sflashdev_initialized == 0) {
		//if (!(sflashdev = kmm_malloc(sizeof(struct s5j_sflash_dev_s)))) {
		//	return NULL;
		//}

		//s5j_sflash_init();
		sflashdev_initialized = 1;
	}
	return (FAR struct spi_dev_s *) sflashdev;
}
