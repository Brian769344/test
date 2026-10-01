/* SPDX-License-Identifier: GPL-2.0 */
/*  Himax Android Driver Sample Code for OFR platform
 *
 *  Copyright (C) 2026 Himax Corporation.
 *
 *  This software is licensed under the terms of the GNU General Public
 *  License version 2,  as published by the Free Software Foundation,  and
 *  may be copied,  distributed,  and modified under those terms.
 *
 *  This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef himax_OFR_OFRPLATFORM_H
#define himax_OFR_OFRPLATFORM_H

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include "himax_ic_core.h"

#define himax_OFR_SPI_FIFO_POLLING
#define himax_OFR_BUS_RETRY_TIMES 3U


#define himax_OFR_common_NAME "himax_ofr"
#define INPUT_DEV_NAME "himax-touchscreen"

#define D_OFR(x...) pr_info("[HXTP][OFR][DEBUG] " x)
#define I_OFR(x...) pr_info("[HXTP][OFR] " x)
#define W_OFR(x...) pr_warn("[HXTP][OFR][WARNING] " x)
#define E_OFR(x...) pr_err("[HXTP][OFR][ERROR] " x)

extern int himax_OFR_OFR_bus_read(uint8_t command, uint8_t *data, uint32_t length,
			  uint8_t toRetry);
extern int himax_OFR_OFR_bus_write(uint8_t command, uint8_t *data, uint32_t length,
			   uint8_t toRetry);

extern int himax_OFR_OFR_SID_bus_read(uint8_t command, uint8_t *data, uint32_t length,
			  uint8_t toRetry);
extern int himax_OFR_OFR_SID_bus_write(uint8_t command, uint8_t *data, uint32_t length,
			   uint8_t toRetry);

#endif
