// SPDX-License-Identifier: GPL-2.0
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

#include "himax_OFR_platform.h"
#include "himax_common.h"

static uint8_t *gBuffer;
static int g_GPIO_4;
int himax_OFR_bus_read(uint8_t command, uint8_t *data, uint32_t length,
		   uint8_t toRetry);
int himax_OFR_bus_write(uint8_t command, uint8_t *data, uint32_t length,
		    uint8_t toRetry);

int himax_OFR_SID_bus_read(uint8_t command, uint8_t *data, uint32_t length,
		   uint8_t toRetry);
int himax_OFR_SID_bus_write(uint8_t command, uint8_t *data, uint32_t length,
		    uint8_t toRetry);

static ssize_t himax_OFR_spi_sync(struct himax_OFR_ts_data *ts,
			      struct spi_message *message)
{
	int status;

	status = spi_sync(ts->spi, message);

	if (status == 0) {
		status = message->status;
		if (status == 0) {
			status = message->actual_length;
		}
	}
	return status;
}

static int himax_OFR_spi_read(uint8_t *command, uint8_t command_len, uint8_t *data,
			  uint32_t length, uint8_t toRetry)
{
	struct spi_message message;
	struct spi_transfer xfer[2];
	uint8_t retry;
	int error;

	spi_message_init(&message);
	(void)memset(xfer, 0, sizeof(xfer));

	xfer[0].tx_buf = command;
	xfer[0].len = command_len;
	spi_message_add_tail(&xfer[0], &message);

	xfer[1].rx_buf = data;
	xfer[1].len = length;
	spi_message_add_tail(&xfer[1], &message);

	for (retry = 0; retry < toRetry; retry++) {
		error = spi_sync(private_OFR_ts->spi, &message);
		if (unlikely(error) != 0) {
			E_OFR("SPI read error: %d\n", error);
		} else {
			break;
		}
	}

	if (retry == toRetry) {
		E_OFR("%s: SPI read error retry over %d\n", __func__, toRetry);
		return -EIO;
	}

	return 0;
}

static int himax_OFR_spi_write(uint8_t *buf, uint32_t length)
{
	struct spi_transfer t = {
		.tx_buf = buf,
		.len = length,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	return himax_OFR_spi_sync(private_OFR_ts, &m);
}

int himax_OFR_SID_bus_read(uint8_t command, uint8_t *data, uint32_t length,
		   uint8_t toRetry)
{
	int result = 0;
	uint8_t spi_format_buf[4];
	uint8_t MID = (private_OFR_ts->spi_id + 1U);
	uint8_t SID = 0xC0U;

	mutex_lock(&(private_OFR_ts->rw_lock));
	spi_format_buf[0] = MID;
	spi_format_buf[1] = SID;
	spi_format_buf[2] = command;
	spi_format_buf[3] = 0x00;

	result = himax_OFR_spi_read(&spi_format_buf[0], 4, data, length, toRetry);
	mutex_unlock(&(private_OFR_ts->rw_lock));

	return result;
}
EXPORT_SYMBOL(himax_OFR_SID_bus_read);

int himax_OFR_SID_bus_write(uint8_t command, uint8_t *data, uint32_t length,
		    uint8_t toRetry)
{
	uint8_t *spi_format_buf = gBuffer;
	unsigned int i = 0;
	int result = 0;
	uint8_t MID = private_OFR_ts->spi_id;
	uint8_t SID = 0xC0U;

	UNUSED(toRetry);

	mutex_lock(&(private_OFR_ts->rw_lock));
	spi_format_buf[0] = MID;
	spi_format_buf[1] = SID;
	spi_format_buf[2] = command;

	for (i = 0; i < length; i++) {
		spi_format_buf[i + 3U] = data[i];
	}

	result = himax_OFR_spi_write(spi_format_buf, length + 3U);
	mutex_unlock(&(private_OFR_ts->rw_lock));

	return result;
}
EXPORT_SYMBOL(himax_OFR_SID_bus_write);


int himax_OFR_bus_read(uint8_t command, uint8_t *data, uint32_t length,
		   uint8_t toRetry)
{
	int result = 0;
	uint8_t spi_format_buf[3];
	uint8_t read_id = (private_OFR_ts->spi_id + 1U);

	mutex_lock(&(private_OFR_ts->rw_lock));
	spi_format_buf[0] = read_id;
	spi_format_buf[1] = command;
	spi_format_buf[2] = 0x00;

	result = himax_OFR_spi_read(&spi_format_buf[0], 3, data, length, toRetry);
	mutex_unlock(&(private_OFR_ts->rw_lock));

	return result;
}
EXPORT_SYMBOL(himax_OFR_bus_read);

int himax_OFR_bus_write(uint8_t command, uint8_t *data, uint32_t length,
		    uint8_t toRetry)
{
	uint8_t *spi_format_buf = gBuffer;
	unsigned int i = 0;
	int result = 0;
	uint8_t write_id = private_OFR_ts->spi_id;

	UNUSED(toRetry);

	mutex_lock(&(private_OFR_ts->rw_lock));
	spi_format_buf[0] = write_id;
	spi_format_buf[1] = command;

	for (i = 0; i < length; i++) {
		spi_format_buf[i + 2U] = data[i];
	}

	result = himax_OFR_spi_write(spi_format_buf, length + 2U);
	mutex_unlock(&(private_OFR_ts->rw_lock));

	return result;
}
EXPORT_SYMBOL(himax_OFR_bus_write);

static int himax_OFR_flash_write_burst_lenth(const uint8_t *reg_byte,
					     const uint8_t *write_data,
					     uint32_t length)
{
	uint8_t data_byte[FLASH_RW_MAX_LEN] = { 0U };
	int ret = 0;
	uint32_t i = 0U;

	if ((write_data == NULL) || (reg_byte == NULL)
		|| ((length + ADDR_LEN_4) > FLASH_RW_MAX_LEN)) {
		E("%s: write_data or reg_byte or length is invalid !\n", __func__);
		return -EINVAL;
	}
	/* assign addr 4bytes */
	for (i = 0; i < ADDR_LEN_4; i++) {
		data_byte[i] = reg_byte[i];
	}
	/* assign data n bytes */
	for (i = 0; i < length; i++) {
		data_byte[ADDR_LEN_4 + i] = write_data[i];
	}
	if (private_OFR_ts->SID_Protocol == true) {
		ret = himax_OFR_SID_bus_write(addr_AHB_address_byte_0, data_byte,
			      length + ADDR_LEN_4, himax_OFR_BUS_RETRY_TIMES);
	} else {
		ret = himax_OFR_bus_write(addr_AHB_address_byte_0, data_byte,
			      length + ADDR_LEN_4, himax_OFR_BUS_RETRY_TIMES);
	}
	if (ret < 0) {
		E("%s: xfer fail!\n", __func__);
		return I2C_FAIL;
	}

	return NO_ERR;
}
static void hx_OFR_burst_mode_enable(void)
{
	uint8_t tmp_data[DATA_LEN_4];
	uint8_t auto_add_4_byte = 0x01U;
	int ret;

#if defined(HIMAX_I2C_PLATFORM)
	tmp_data[0] = ((uint8_t)para_AHB_INC4 | auto_add_4_byte);
#else
	tmp_data[0] = (0x12U | auto_add_4_byte);
#endif

	if (private_OFR_ts->SID_Protocol == true) {
		ret = himax_OFR_SID_bus_write(addr_AHB_INC4, tmp_data, 1,
			      himax_OFR_BUS_RETRY_TIMES);
	} else {
		ret = himax_OFR_bus_write(addr_AHB_INC4, tmp_data, 1,
			      himax_OFR_BUS_RETRY_TIMES);
	}
	if (ret < 0) {
		E("%s: bus access fail!\n", __func__);
		return;
	}
}

void himax_OFR_register_write(uint32_t write_addr, uint32_t write_length,
			     uint8_t *write_data)
{
	uint32_t address = 0;
	uint8_t tmp_addr[DATA_LEN_4] = { 0U };
	uint8_t *tmp_data;
	uint8_t total_write_times = 0;
	uint32_t max_bus_size = MAX_I2C_TRANS_SZ;
	uint32_t total_size_temp = 0;
	uint32_t i = 0;
	int ret = 0;

	total_size_temp = write_length;

	himax_parse_assign_cmd(write_addr, tmp_addr, sizeof(tmp_addr));

	if ((total_size_temp % max_bus_size) == 0U) {
		total_write_times = (uint8_t) (total_size_temp / max_bus_size);
	} else {
		total_write_times = (uint8_t) (total_size_temp / max_bus_size) + 1U;
	}

	if (write_length > DATA_LEN_4) {
		hx_OFR_burst_mode_enable();
	}

	for (i = 0; i < (total_write_times); i++) {

		if (total_size_temp >= max_bus_size) {
			tmp_data = &write_data[i * max_bus_size];

			ret = himax_OFR_flash_write_burst_lenth(
				tmp_addr, tmp_data, max_bus_size);
			if (ret < 0) {
				I("%s: bus access fail!\n", __func__);
				return;
			}
			total_size_temp = total_size_temp - max_bus_size;
		} else {
			tmp_data = &write_data[i * max_bus_size];
			/* I("last total_size_temp=%d\n",
			 *	total_size_temp % max_bus_size);
			 */
			ret = himax_OFR_flash_write_burst_lenth(
				tmp_addr, tmp_data, total_size_temp);
			if (ret < 0) {
				I("%s: bus access fail!\n", __func__);
				return;
			}
		}

		address = ((i + 1U) * max_bus_size);
		tmp_addr[0] = (uint8_t)(write_addr & 0xFFU)
					+ (uint8_t)((address) & 0xFFU);
		tmp_addr[1] = (uint8_t)((write_addr >> 8U) & 0xFFU)
					+ (uint8_t)((address >> 8U) & 0xFFU);

		if (tmp_addr[0] < (uint8_t)(write_addr & 0xFFU)) {
			tmp_addr[1] += 1U;
		}

		udelay(100);
	}

}

void himax_OFR_register_read(uint32_t read_addr, uint32_t read_length,
			    uint8_t *read_data)
{
	uint8_t tmp_data[DATA_LEN_4] = { 0U };
	int ret = 0;

	if (read_length > FLASH_RW_MAX_LEN) {
		E_OFR("%s: read len over %d!\n", __func__, FLASH_RW_MAX_LEN);
		return;
	}

	himax_parse_assign_cmd(read_addr, tmp_data, sizeof(tmp_data));

	ret = himax_OFR_bus_write(addr_AHB_address_byte_0, tmp_data, DATA_LEN_4,
				himax_OFR_BUS_RETRY_TIMES);
	if (ret < 0) {
		if (g_hx_chip_inited == true) {
			E_OFR("%s: bus access fail!\n", __func__);
		}
		return;
	}

	tmp_data[0] = (uint8_t)para_AHB_access_direction_read;

	ret = himax_OFR_bus_write(addr_AHB_access_direction, tmp_data, 1U,
				himax_OFR_BUS_RETRY_TIMES);
	if (ret < 0) {
		E_OFR("%s: bus access fail!\n", __func__);
		return;
	}

	ret = himax_OFR_bus_read(addr_AHB_rdata_byte_0, read_data, read_length,
				himax_OFR_BUS_RETRY_TIMES);
	if (ret < 0) {
		E_OFR("%s: bus access fail!\n", __func__);
		return;
	}
	
}

void himax_mcu_OFR(uint8_t update_option)
{
	uint8_t tmp_data[DATA_LEN_4] = { 0 };

	himax_OFR_register_read(0x90088070U, DATA_LEN_4, tmp_data);

	I("%s:busy zone = %02X%02X%02X%02X\n",
		__func__, tmp_data[3], tmp_data[2], tmp_data[1], tmp_data[0]);

	if (tmp_data[0] == HX_PART_A) { /*A:busy; B:idle */
		if (update_option == HX_PART_A) {
			E("%s: rejected: zone is currently busy\n", __func__);
		} else if (update_option == HX_PART_B) {
			I("%s: update idle zone\n", __func__);
			himax_mcu_OFR_program_FW(HX_PART_B);
		} else {
			E("%s: Unknonw behavior\n", __func__);
		}
	} else if (tmp_data[0] == HX_PART_B) { /*A:idle; B:busy */
		if (update_option == HX_PART_B) {
			E("%s: rejected: zone is currently busy\n", __func__);
		} else if (update_option == HX_PART_A) {
			I("%s: Update idle zone\n", __func__);
			himax_mcu_OFR_program_FW(HX_PART_A);
		} else {
			E("%s: Unknonw behavior\n", __func__);
		}
	} else {
		E("%s: Unknonw behavior\n", __func__);
	}

}
#define WIP_PRT_LOG "%s: retry:%d, bf[0]=0x%02X, bf[1]=0x%02X,bf[2]=0x%02X, bf[3]=0x%02X\n"
bool himax_OFR_wait_wip(int Timing)
{
	uint8_t tmp_data[DATA_LEN_4];
	int retry_cnt = 0;

	himax_parse_assign_cmd(data_spi200_trans_fmt, tmp_data,
			       sizeof(tmp_data));
	himax_OFR_register_write(addr_spi200_trans_fmt, DATA_LEN_4, tmp_data);
	tmp_data[0] = 0x01;

	do {
		himax_parse_assign_cmd(data_spi200_trans_ctrl_1, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_trans_ctrl, DATA_LEN_4,
					 tmp_data);

		himax_parse_assign_cmd(data_spi200_cmd_1, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_cmd, DATA_LEN_4, tmp_data);

		himax_OFR_register_read(addr_spi200_data, DATA_LEN_4, tmp_data);

		if ((tmp_data[0] & 0x01U) == 0x00U) {
			return true;
		}

		retry_cnt++;

		if ((tmp_data[0] != 0x00U) || (tmp_data[1] != 0x00U) ||
		    (tmp_data[2] != 0x00U) || (tmp_data[3] != 0x00U)) {
			I(WIP_PRT_LOG, __func__, retry_cnt, tmp_data[0],
			  tmp_data[1], tmp_data[2], tmp_data[3]);
		}

		if (retry_cnt > 100) {
			E("%s: Wait wip error!\n", __func__);
			return false;
		}

		msleep(Timing);
	} while ((tmp_data[0] & 0x01U) == 0x01U);

	return true;
}
void himax_OFR_sector_erase(uint32_t start_addr, uint32_t length)
{
	uint32_t tmp_addr_32 = 0;
	uint8_t data[DATA_LEN_4] = { 0 };
	uint32_t page_prog_start = start_addr;
	uint32_t sector_size = 0x1000;

	/*=====================================
	 *SPI Transfer Format : 0x8000_0010 ==> 0x0002_0780
	 *=====================================
	 */
	data[3] = 0x00;
	data[2] = 0x02;
	data[1] = 0x07;
	data[0] = 0x80;

	tmp_addr_32 = 0x80000010U;
	himax_OFR_register_write(tmp_addr_32, DATA_LEN_4, data);

	while (page_prog_start < (start_addr + length)) {
		/*=====================================
		 *Write Enable : 1. 0x8000_0020 ==> 0x4700_0000 [control]
		 *			 2. 0x8000_0024 ==> 0x0000_0006 [WREN]
		 *=====================================
		 */
		data[3] = 0x47;
		data[2] = 0x00;
		data[1] = 0x00;
		data[0] = 0x00;
		tmp_addr_32 = 0x80000020U;
		himax_OFR_register_write(tmp_addr_32, DATA_LEN_4, data);
		data[3] = 0x00;
		data[2] = 0x00;
		data[1] = 0x00;
		data[0] = 0x06;
		tmp_addr_32 = 0x80000024U;
		himax_OFR_register_write(tmp_addr_32, DATA_LEN_4, data);

		/*=====================================
		 *Sector Erase
		 *Command : 0x8000_0028 ==> 0x0000_0000 [SPI addr]
		 *				0x8000_0020 ==> 0x6700_0000 [control]
		 *				0x8000_0024 ==> 0x0000_0020 [SE]
		 *=====================================
		 */

		data[3] = (uint8_t)(page_prog_start >> 24);
		data[2] = (uint8_t)(page_prog_start >> 16);
		data[1] = (uint8_t)(page_prog_start >> 8);
		data[0] = (uint8_t)page_prog_start;

		tmp_addr_32 = 0x80000028U;
		himax_OFR_register_write(tmp_addr_32, DATA_LEN_4, data);
		data[3] = 0x67;
		data[2] = 0x00;
		data[1] = 0x00;
		data[0] = 0x00;
		tmp_addr_32 = 0x80000020U;
		himax_OFR_register_write(tmp_addr_32, DATA_LEN_4, data);
		data[3] = 0x00;
		data[2] = 0x00;
		data[1] = 0x00;
		data[0] = 0x20;
		tmp_addr_32 = 0x80000024U;
		himax_OFR_register_write(tmp_addr_32, DATA_LEN_4, data);

		if (!himax_OFR_wait_wip(100)) {
			E("%s: Fail:\n", __func__);
		}
		I("%s:page_prog_start = 0x%8X,\n", __func__, page_prog_start);
		page_prog_start += sector_size;
	}


	I("%s:exit\n", __func__);
}
void himax_OFR_block_erase(uint32_t start_addr, uint32_t length)
{
	uint32_t page_prog_start = start_addr;
	uint32_t block_size = 0x10000;//64KB
	uint8_t tmp_data[DATA_LEN_4] = { 0 };

	himax_parse_assign_cmd(data_spi200_trans_fmt, tmp_data,
			       sizeof(tmp_data));
	himax_OFR_register_write(addr_spi200_trans_fmt, DATA_LEN_4, tmp_data);

	while (page_prog_start < (start_addr + length)) {
		himax_parse_assign_cmd(data_spi200_trans_ctrl_2, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_trans_ctrl, DATA_LEN_4,
					 tmp_data);

		himax_parse_assign_cmd(data_spi200_cmd_2, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_cmd, DATA_LEN_4, tmp_data);

		tmp_data[3] = (uint8_t)((page_prog_start >> 24U) & 0xFFU);
		tmp_data[2] = (uint8_t)((page_prog_start >> 16U) & 0xFFU);
		tmp_data[1] = (uint8_t)((page_prog_start >> 8U) & 0xFFU);
		tmp_data[0] = (uint8_t)(page_prog_start & 0xFFU);
		himax_OFR_register_write(addr_spi200_addr, DATA_LEN_4,
					 tmp_data);

		himax_parse_assign_cmd(data_spi200_trans_ctrl_3, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_trans_ctrl, DATA_LEN_4,
					 tmp_data);

		himax_parse_assign_cmd(data_spi200_cmd_4, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_cmd, DATA_LEN_4, tmp_data);

		msleep(100);

		if (!himax_OFR_wait_wip(100)) {
			E("%s:Erase Fail\n", __func__);
			return;
		}
		page_prog_start += block_size;
	}

	I("%s:exit\n", __func__);
}
static void hx_OFR_burst_mode_disable(void)
{
	uint8_t tmp_data[DATA_LEN_4];
	int ret;

#if defined(HIMAX_I2C_PLATFORM)
	tmp_data[0] = (uint8_t)para_AHB_INC4;
#else
	tmp_data[0] = 0x12U;
#endif

	if (private_OFR_ts->SID_Protocol == true) {
		ret = himax_OFR_SID_bus_write(addr_AHB_INC4, tmp_data, 1,
			      himax_OFR_BUS_RETRY_TIMES);
	} else {
		ret = himax_OFR_bus_write(addr_AHB_INC4, tmp_data, 1,
			    himax_OFR_BUS_RETRY_TIMES);
	}
	if (ret < 0) {
		E("%s: bus access fail!\n", __func__);
		return;
	}
}

static bool hx_OFR_flash_programming(const u8 *FW_content, unsigned int start_addr,
					  unsigned int length)
{
	unsigned int page_prog_start = 0;
	uint8_t tmp_data[DATA_LEN_4] = { 0 };
	uint8_t buring_data[FLASH_RW_MAX_LEN] = { 0 }; /* Read for flash data, 128K*/
	uint8_t Original_speed[DATA_LEN_4] = { 0 };
	bool ret_data = true;
	uint16_t index = 0;

	I("%s: range [0x%08X ~ 0x%08X]\n",
		__func__, start_addr, start_addr + length - 1U);

	/* ===Get Flash Speed===*/
	himax_OFR_register_read(addr_spi200_flash_speed, DATA_LEN_4,
				Original_speed);

	/* ===Set Flash Speed===*/
	himax_parse_assign_cmd(data_set_flash_speed, tmp_data,
			       sizeof(tmp_data));
	himax_OFR_register_write(addr_spi200_flash_speed, DATA_LEN_4, tmp_data);

	hx_OFR_burst_mode_disable();

	/* ===SPI TX-FIFO Reset===*/
	himax_parse_assign_cmd(data_spi200_txfifo_rst, tmp_data,
			       sizeof(tmp_data));
	himax_OFR_register_write(addr_spi200_fifo_rst, DATA_LEN_4, tmp_data);

	/* ===SPI Format===*/
	himax_parse_assign_cmd(data_spi200_trans_fmt, tmp_data,
			       sizeof(tmp_data));
	himax_OFR_register_write(addr_spi200_trans_fmt, DATA_LEN_4, tmp_data);

	page_prog_start = start_addr;
	while (page_prog_start < (start_addr + length)) {
		/* ===Flash Write Enable ===*/
		himax_parse_assign_cmd(data_spi200_trans_ctrl_2, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_trans_ctrl, DATA_LEN_4,
					 tmp_data);

		himax_parse_assign_cmd(data_spi200_cmd_2, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_cmd, DATA_LEN_4, tmp_data);

		/* ===WEL Write Control ===*/
		himax_parse_assign_cmd(data_spi200_trans_ctrl_6, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_trans_ctrl, DATA_LEN_4,
					 tmp_data);

		himax_parse_assign_cmd(data_spi200_cmd_1, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_cmd, DATA_LEN_4, tmp_data);

		himax_OFR_register_read(addr_spi200_data, DATA_LEN_4, tmp_data);
		/* === Check WEL Fail ===*/
		if (((tmp_data[0] & 0x02U) >> 1U) == 0U) {
			I("%s:SPI 0x8000002c = %d, Check WEL Fail\n", __func__, tmp_data[0]);
			ret_data = false;
		}

		/*Set 256 Bytes Page Write*/
		himax_parse_assign_cmd(data_spi200_trans_ctrl_4, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_trans_ctrl, DATA_LEN_4,
					 tmp_data);

		(void)memset(tmp_data, 0x00, sizeof(tmp_data));
		tmp_data[3] = (uint8_t)(page_prog_start >> 24U);
		tmp_data[2] = (uint8_t)(page_prog_start >> 16U);
		tmp_data[1] = (uint8_t)(page_prog_start >> 8U);
		tmp_data[0] = (uint8_t)page_prog_start;
		himax_OFR_register_write(addr_spi200_addr, DATA_LEN_4,
					 tmp_data);

		(void)memset(buring_data, 0x00, sizeof(buring_data));
		himax_parse_assign_cmd(addr_spi200_data, buring_data,
				       ADDR_LEN_4);
		for (index = 0; index < 16U; index++) {
			buring_data[ADDR_LEN_4 + index] =
				FW_content[page_prog_start - start_addr + index];
		}
		if (private_OFR_ts->SID_Protocol == true) {
			if (himax_OFR_SID_bus_write(addr_AHB_address_byte_0, buring_data,
						(ADDR_LEN_4 + 16U),
						himax_OFR_BUS_RETRY_TIMES) < 0) {
				E("%s: bus access fail!\n", __func__);
				ret_data = false;
			}
		} else {
			if (himax_OFR_bus_write(addr_AHB_address_byte_0, buring_data,
						(ADDR_LEN_4 + 16U),
						himax_OFR_BUS_RETRY_TIMES) < 0) {
				E("%s: bus access fail!\n", __func__);
				ret_data = false;
			}
		}
		/*Write Command: PP*/
		himax_parse_assign_cmd(data_spi200_cmd_6, tmp_data,
				       sizeof(tmp_data));
		himax_OFR_register_write(addr_spi200_cmd, DATA_LEN_4, tmp_data);

		for (index = 0; index < 240U; index++) {
			buring_data[ADDR_LEN_4 + index] =
				FW_content[page_prog_start - start_addr + 16U + index];
		}
		if (private_OFR_ts->SID_Protocol == true) {
			if (himax_OFR_SID_bus_write(addr_AHB_address_byte_0, buring_data,
						(ADDR_LEN_4 + 240U),
						himax_OFR_BUS_RETRY_TIMES) < 0) {
				E("%s: bus access fail!\n", __func__);
				ret_data = false;
			}
		} else {
			if (himax_OFR_bus_write(addr_AHB_address_byte_0, buring_data,
						(ADDR_LEN_4 + 240U),
						himax_OFR_BUS_RETRY_TIMES) < 0) {
				E("%s: bus access fail!\n", __func__);
				ret_data = false;
			}
		}

		if (!himax_OFR_wait_wip(1)) {
			E("%s:Flash_Programming Fail\n", __func__);
			ret_data = false;
		}
		if (ret_data == false) {
			break;
		}
		page_prog_start += FLASH_RW_MAX_LEN;
	}
	/* ===Set Flash Speed===*/
	himax_OFR_register_write(addr_spi200_flash_speed, DATA_LEN_4,
				 Original_speed);
	return ret_data;
}
void himax_OFR_read_FW_status(void)
{
	uint8_t len = 0;
	uint8_t i = 0;
	uint8_t data[DATA_LEN_4] = { 0 };

	len = (uint8_t)(sizeof(dbg_reg_ary) / sizeof(uint32_t));

	for (i = 0; i < len; i++) {
		himax_OFR_register_read(dbg_reg_ary[i], DATA_LEN_4, data);

		I("reg[0-3] : 0x%08X = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
		  dbg_reg_ary[i], data[0], data[1], data[2], data[3]);
	}
}
uint32_t himax_OFR_check_CRC(uint32_t start_addr, unsigned int reload_length)
{
	uint32_t result = 0;
	uint8_t tmp_data[DATA_LEN_4] = { 0 };
	uint8_t i_counter = 0;
	unsigned int length = reload_length / DATA_LEN_4;

	tmp_data[0] = 0xA5;

	/* Disable retry wrapper to avoid I2C CLK low issue */
	himax_OFR_register_write(addr_retry_wrapper_clr_pw, 4, tmp_data);

	himax_parse_assign_cmd(start_addr, tmp_data, sizeof(tmp_data));

	himax_OFR_register_write(addr_reload_addr_from, DATA_LEN_4,
					tmp_data);

	tmp_data[3] = 0x00;
	tmp_data[2] = 0x99;
	tmp_data[1] = (uint8_t)((length >> 8U) & 0xFFU);
	tmp_data[0] = (uint8_t)(length & 0xFFU);

	himax_OFR_register_write(addr_reload_addr_cmd_beat, DATA_LEN_4,
					tmp_data);

	himax_OFR_register_read(addr_reload_status, DATA_LEN_4,
						tmp_data);

	if (tmp_data[1] != 0x99U) {
		E("%s: Reload status cmd fail and out of retry count!\n", __func__);
		return HW_CRC_FAIL;
	}

	i_counter = 0;

	do {

		himax_OFR_register_read(addr_reload_status, DATA_LEN_4,
							tmp_data);

		if ((tmp_data[0] & 0x01U) != 0x01U) {
			himax_OFR_register_read(addr_reload_crc32_result,
						      DATA_LEN_4, tmp_data);
			result = ((uint32_t)tmp_data[3] << 24);
			result += ((uint32_t)tmp_data[2] << 16);
			result += ((uint32_t)tmp_data[1] << 8);
			result += (uint32_t)tmp_data[0];
			I("%s:Check CRC result=0x%08X, Check length=0x%X\n",
				__func__, result, reload_length);
			goto END;
		} else {
			usleep_range(1000, 1100);
			if (i_counter >= 100U) {
				I("%s:CRC Wait loop timeout\n", __func__);
				himax_OFR_read_FW_status();
				return HW_CRC_FAIL;
			}
		}
		i_counter += 1U;
	} while (i_counter < 100U);
END:
	return result;
}

void himax_OFR_disable_flash_protected_mode(void)
{
	uint8_t data[DATA_LEN_4] = { 0 };
	uint8_t loop_count = 0;

	/*Disable Write Protect*/
	himax_parse_assign_cmd(data_WP_disable_HX83195, data, sizeof(data));
	himax_OFR_register_write(addr_WP_pin_HX83195, DATA_LEN_4, data);

	/*Disable Block Protect*/
	himax_parse_assign_cmd(data_BP_lock_cmd_1, data, sizeof(data));
	himax_OFR_register_write(addr_BP_lock_cmd_10, DATA_LEN_4, data);

	himax_parse_assign_cmd(data_BP_lock_cmd_2, data, sizeof(data));
	himax_OFR_register_write(addr_BP_lock_cmd_20, DATA_LEN_4, data);

	himax_parse_assign_cmd(data_BP_lock_cmd_3, data, sizeof(data));
	himax_OFR_register_write(addr_BP_lock_cmd_24, DATA_LEN_4, data);

	himax_parse_assign_cmd(data_BP_lock_cmd_4, data, sizeof(data));
	himax_OFR_register_write(addr_BP_lock_cmd_20, DATA_LEN_4, data);

	himax_parse_assign_cmd(data_BP_lock_cmd_5, data, sizeof(data));
	himax_OFR_register_write(addr_BP_lock_cmd_2C, DATA_LEN_4, data);

	himax_parse_assign_cmd(data_BP_lock_cmd_6, data, sizeof(data));
	himax_OFR_register_write(addr_BP_lock_cmd_24, DATA_LEN_4, data);

	/*Check Block Protect */
	himax_parse_assign_cmd(data_BP_check_cmd_1, data, sizeof(data));
	himax_OFR_register_write(addr_BP_lock_cmd_20, DATA_LEN_4, data);

	do {
		himax_parse_assign_cmd(data_BP_check_cmd_2, data, sizeof(data));
		himax_OFR_register_write(addr_BP_lock_cmd_24, DATA_LEN_4, data);

		himax_OFR_register_read(addr_BP_lock_cmd_2C, DATA_LEN_4, data);
		loop_count++;
		if (loop_count == 30U) {
			W("%s: time out of loop_count: %d.\n",
			  __func__, loop_count);
			  break;
		}
		usleep_range(1000, 1100);
	} while ((data[0] & 0x03U) != 0x00U);

	if (data[0] != 0x00U) {
		W("%s: Fail. value :0x%02X, loop_count: %d\n",
		  __func__, data[0], loop_count);
	} else {
		I("%s: Finish. loop_count %d.\n",
			__func__, loop_count);
	}
}

void himax_mcu_OFR_program_FW(uint8_t part)
{
    const struct firmware *ofr_fw = NULL;
    uint32_t CRC_result = 0;
    int ret = 0;
	uint8_t retry = 0U;
    uint32_t addr_1st, size_1st, off_1st;
    uint32_t addr_2nd, size_2nd, off_2nd;
    uint32_t addr_3rd, size_3rd, off_3rd;

    if (request_firmware(&ofr_fw, OFR_FWNAME, private_OFR_ts->dev) < 0) {
		E("fail to request_firmware fwname: %s \n", OFR_FWNAME);
        return;
	}

    if (part == HX_PART_A) {      
        addr_1st = 0x3F000U; size_1st = HX4K;    off_1st = 0x3F000U;
        addr_2nd = 0x20000U; size_2nd = HX124K;  off_2nd = 0x20000U;
		
		if (ofr_fw->size == FW_SIZE_512k) {
			addr_3rd = 0x60000U; size_3rd = HX128K;  off_3rd = 0x60000U;
		} else if (ofr_fw->size == FW_SIZE_384k) {
			addr_3rd = 0x50000U; size_3rd = HX64K;   off_3rd = 0x50000U;
		} else {
			E("%s: Undefined bin_size: %zu\n", __func__, ofr_fw->size);
			goto END;
		}
    } 
    else if (part == HX_PART_B) {           
        addr_1st = 0x1F000U; size_1st = HX4K;    off_1st = 0x1F000U;
        addr_2nd = 0x00000U; size_2nd = HX124K;  off_2nd = 0x00000U;
		
		if (ofr_fw->size == FW_SIZE_512k) {
			addr_3rd = 0x40000U; size_3rd = HX128K;  off_3rd = 0x40000U;
		} else if (ofr_fw->size == FW_SIZE_384k) {
			addr_3rd = 0x40000U; size_3rd = HX64K;   off_3rd = 0x40000U;
		} else {
			E("%s: Undefined bin_size: %zu\n", __func__, ofr_fw->size);
			goto END;
		}
    }
    else {
        return; // Invalid input
    }

    himax_OFR_disable_flash_protected_mode();

    // -------------------------------------------------
    // Step 1: Erase 1st section first (do not program it now)
    // -------------------------------------------------
    himax_OFR_sector_erase(addr_1st, size_1st);

    // -------------------------------------------------
    // Step 2: Program 2nd + 3rd sections as a pair
    //         (Erase together → Program together → CRC together)
    // -------------------------------------------------
    for (retry = 0U; retry < 3U; retry++) {

        himax_OFR_block_erase(addr_2nd, size_2nd);
        himax_OFR_block_erase(addr_3rd, size_3rd);

        if (!hx_OFR_flash_programming(ofr_fw->data + off_2nd, addr_2nd, size_2nd)) {
            continue;
		}
        if (!hx_OFR_flash_programming(ofr_fw->data + off_3rd, addr_3rd, size_3rd)) {
            continue;
		}
        CRC_result = himax_OFR_check_CRC(addr_2nd, size_2nd - 4U);
        if (CRC_result != 0U) {
			continue;
		}

        if (size_3rd >= 4U) {
            CRC_result = himax_OFR_check_CRC(addr_3rd, size_3rd - 4U);
            if (CRC_result != 0U) {
                continue;
            }
        } else {
            continue;
        }

        ret = 0;
        break;
    }

    if (ret != 0) {
        goto END;
	}

    // -------------------------------------------------
    // Step 3: Program 1st section at the end
    // -------------------------------------------------
    for (retry = 0U; retry < 3U; retry++) {

        if (!hx_OFR_flash_programming(ofr_fw->data + off_1st, addr_1st, size_1st)) {
            continue;
		}

        CRC_result = himax_OFR_check_CRC(addr_1st, size_1st - 4U);

        if (CRC_result == 0U) { 
            ret = 0; 
            break; 
        }

        himax_OFR_sector_erase(addr_1st, size_1st);
    }

END:
    release_firmware(ofr_fw);
}

void himax_OFR_init_psl(void)
{
	uint8_t data[DATA_LEN_4] = { 0 };

	himax_parse_assign_cmd(data_clear, data, sizeof(data));
	himax_OFR_register_write(addr_psl, DATA_LEN_4, data);
	I("%s: power saving level reset OK!\n", __func__);
}

bool himax_mcu_OFR_fw_update(uint8_t update_option)
{
	bool ret = true;

	I("%s: enter\n", __func__);

	if(!himax_mcu_OFR_enter_checking()) {
		E("%s: OFR enter checking fail\n", __func__);
		ret = false;
		return ret;
	}
	himax_OFR_init_psl();

	himax_OFR_disable_flash_protected_mode();

	himax_mcu_OFR(update_option);

	/* himax_mcu_OFR_change_partition(update_option);*/

	I("%s: exit\n", __func__);

	return ret;
}

bool himax_mcu_OFR_enter_checking(void)
{
	uint8_t tmp_data[DATA_LEN_4] = { 0 };
	bool ret = false;
    uint8_t i;
	int status = 0;

	I("%s: enter\n", __func__);

	tmp_data[0] = 0x11U;
	if (private_OFR_ts->SID_Protocol == true) {
		status = himax_OFR_SID_bus_write(addr_AHB_address_byte_0, tmp_data, 1,
			    himax_OFR_BUS_RETRY_TIMES);
	} else {
		status = himax_OFR_bus_write(addr_AHB_address_byte_0, tmp_data, 1,
				himax_OFR_BUS_RETRY_TIMES);
	}
	if (status < 0) {
		E_OFR("%s: bus access fail!\n", __func__);
		return ret;
	}

	if (private_OFR_ts->SID_Protocol == true) {
		status = himax_OFR_SID_bus_read(addr_AHB_address_byte_0, tmp_data, 1,
				himax_OFR_BUS_RETRY_TIMES);
	} else {
		status = himax_OFR_bus_read(addr_AHB_address_byte_0, tmp_data, 1,
				himax_OFR_BUS_RETRY_TIMES);
	}
	if (status < 0) {
		E_OFR("%s: bus access fail!\n", __func__);
		return ret;
	}
	I("%s: tmp_data[0] = 0x02%X\n", __func__, tmp_data[0]);

	if (tmp_data[0] == 0x11U) {
		himax_OFR_register_read(addr_cs_central_state, DATA_LEN_4, tmp_data);

		if (tmp_data[0] == 0x05U) {
			for (i = 0; i < 20U; i++) {
				himax_OFR_register_read(addr_OFR_command, DATA_LEN_4, tmp_data);
				if ((tmp_data[0] == 0xFFU) && (tmp_data[1] == 0xFFU) &&
					(tmp_data[2] == 0xFFU) && (tmp_data[3] == 0xFFU)) {
					ret = true;
					break;
				} 
				msleep(50);
			}
			if (ret) {
				ret = false;
				tmp_data[0] = 0x5AU;
				tmp_data[1] = 0x00U;
				tmp_data[2] = 0x00U;
				tmp_data[3] = 0x00U;
				himax_OFR_register_write(addr_OFR_command, DATA_LEN_4, tmp_data);

				for (i = 0; i < 20U; i++) {
					himax_OFR_register_read(addr_OFR_command, DATA_LEN_4, tmp_data);
					if (tmp_data[0] == 0xA5U) {
						ret = true;
						break;
					}
					msleep(50);
				}		
				if (!ret) {	
					E("%s: Polling %08X fail \n", __func__, addr_OFR_command);
				}
			} else {
				E("%s: Check %08X fail \n", __func__, addr_OFR_command);
			}
		} else {
			E("%s: 0x900000A8 =0x%02X\n", __func__, tmp_data[0]);
		}
	} else {
		E("%s: Check Interface Fail\n", __func__);
	}

	return ret;
}


void himax_OFR_config_reload_enable(void)
{
	uint8_t data[DATA_LEN_4] = { 0 };

	/*clear config reload done*/
	himax_parse_assign_cmd(data_clear, data,
		sizeof(data));
	himax_OFR_register_write(addr_chk_fw_reload2,
		DATA_LEN_4, data);

	/*reload enable*/
	himax_parse_assign_cmd(data_fw_define_flash_reload_en, data,
		sizeof(data));
	himax_OFR_register_write(addr_fw_define_flash_reload,
		DATA_LEN_4, data);

	I("%s: setting OK!\n", __func__);
}

void himax_mcu_OFR_change_partition(uint8_t option)
{
	uint8_t tmp_data[DATA_LEN_4] = { 0 };

	I("%s: enter\n", __func__);
	if (option == HX_PART_A) {
		gpio_direction_output(g_GPIO_4, 1);
		I("%s: Change Partition to A\n", __func__);
	} else {
		gpio_direction_output(g_GPIO_4, 0);
		I("%s: Change Partition to B\n", __func__);
	}

	himax_OFR_register_write(addr_OFR_command, DATA_LEN_4, tmp_data);

	himax_OFR_config_reload_enable();

	himax_mcu_tp_reset();

	I("%s: exit\n", __func__);
}

static bool hx_OFR_read_and_check_id(uint32_t *id_check) {

	bool ret_data = false;
	uint8_t index = 0U;
	uint8_t tmp_data[DATA_LEN_4] = { 0 };
	uint8_t product_data[DATA_LEN_4] = { 0 };
	
	const uint32_t valid_ids[] = {0x83195AU, 0x83195BU, 0x83194AU, 0x83194BU};


    himax_OFR_register_read(addr_icid_addr, DATA_LEN_4, tmp_data);
    himax_OFR_register_read(addr_product_id, DATA_LEN_4, product_data);
    
    *id_check = 0U;
    *id_check |= ((uint32_t) tmp_data[3] << 16U);
    *id_check |= ((uint32_t) tmp_data[2] << 8U);
    *id_check |= ((uint32_t) product_data[1]);

	for (index = 0; index < DATA_LEN_4; index++) {
		if (*id_check == valid_ids[index]) {
			ret_data = true;
			break;
		}
	}

    return ret_data;
}

static bool hx_OFR_chip_detect(void)
{
	bool ret = true;
	uint8_t attempts = 0U;
	uint32_t id_check = 0U;

	for (attempts = 0; attempts < 5U; attempts++) {
		private_OFR_ts->SID_Protocol = false;
 		if (!hx_OFR_read_and_check_id(&id_check)) {
            W_OFR("Using SID protocol and try again! id_check = HX%06X\n", id_check);
            private_OFR_ts->SID_Protocol = true;
            
            if (!hx_OFR_read_and_check_id(&id_check)) {
                E_OFR("SID protocol still fails! id_check = HX%06X\n", id_check);
                ret = false;
            } else {
				break; /*Exit on SID_Protocol = true */
			}
        } else {
			break; /*Exit on SID_Protocol = false*/
		}
	}

	return ret;
}

int himax_OFR_chip_common_probe(struct spi_device *hx_spi)
{ 
	struct himax_OFR_ts_data *ts;
	int ret = 0;
	struct device_node *dt = hx_spi->dev.of_node;

	if ((hx_spi->master->flags & SPI_MASTER_HALF_DUPLEX) != 0) {
		dev_err(&hx_spi->dev, "%s: Full duplex not supported by host\n",
			__func__);
		return -EIO;
	}

	gBuffer = kzalloc(sizeof(uint8_t) * HX_MAX_WRITE_SZ, GFP_KERNEL);
	if (gBuffer == NULL) {
		E_OFR("%s: allocate gBuffer failed\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_gbuffer_failed;
	}

	ts = kzalloc(sizeof(struct himax_OFR_ts_data), GFP_KERNEL);
	if (ts == NULL) {
		E_OFR("%s: allocate himax_OFR_ts_data failed\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}


	private_OFR_ts = ts;
	private_OFR_ts->spi_id = 0xA0U;
	hx_spi->bits_per_word = 8;
	hx_spi->mode = SPI_MODE_0;
	hx_spi->chip_select = 0;

	ts->spi = hx_spi;
	mutex_init(&ts->rw_lock);
	ts->dev = &hx_spi->dev;
	dev_set_drvdata(&hx_spi->dev, ts);
	spi_set_drvdata(hx_spi, ts);

	g_GPIO_4 = of_get_named_gpio(dt, "himax,GPIO_4", 0);

	if (!gpio_is_valid(g_GPIO_4)) {
		E_OFR(" DT:g_GPIO_4 value is not valid\n");
	}

	if (!hx_OFR_chip_detect()) {
		E_OFR("hx_OFR_chip_detect failed\n");
		ret = -ENOMEM;
		goto err_common_init_failed;
	}
	return ret;

err_common_init_failed:
	kfree(ts);
	ts = NULL;
err_alloc_data_failed:
	kfree(gBuffer);
	gBuffer = NULL;
err_alloc_gbuffer_failed:

	return ret;
}

#if !defined(KERNEL_VER_6_01)
int himax_OFR_chip_common_remove(struct spi_device *hx_spi)
#else
void himax_OFR_chip_common_remove(struct spi_device *hx_spi)
#endif
{
    struct himax_OFR_ts_data *ts = spi_get_drvdata(hx_spi);

	ts->spi = NULL;
    /* spin_unlock_irq(&ts->spi_lock); */
    spi_set_drvdata(hx_spi, NULL);
    kfree(gBuffer);
    gBuffer = NULL;
    I_OFR("%s: completed.\n", __func__);

#if !defined(KERNEL_VER_6_01)
    return 0;
#endif
}

#if defined(CONFIG_OF)
static const struct of_device_id himax_OFR_match_table[] = {
	{ .compatible = "himax,hx_ofr_common" },
	{},
};
#else
#define himax_OFR_match_table NULL
#endif

static struct spi_driver himax_OFR_common_driver = {
	.driver = {
		.name =		himax_OFR_common_NAME,
		.owner =	THIS_MODULE,
		.of_match_table = himax_OFR_match_table,
	},
	.probe =	himax_OFR_chip_common_probe,
	.remove =	himax_OFR_chip_common_remove,
};

int __init himax_OFR_common_init(void)
{
	I_OFR("Himax common touch panel driver init\n");

	spi_register_driver(&himax_OFR_common_driver);

	return 0;
}

void __exit himax_OFR_common_exit(void)
{
	spi_unregister_driver(&himax_OFR_common_driver);
}

module_init(himax_OFR_common_init);
module_exit(himax_OFR_common_exit);

MODULE_DESCRIPTION("himax_OFR_common driver");
MODULE_LICENSE("GPL");
