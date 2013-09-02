/*
 * FB driver for the ST 7565 LCD
 *
 * Copyright (C) 2013 Karol Poczesny
 *
 * This driver based on fbtft drivers solution created by Noralf Tronnes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include "fbtft.h"

#define DRVNAME		"fb_st7565"
#define WIDTH		132
#define HEIGHT		64
#define TXBUFLEN   1024 //128 x 8
#define DEFAULT_GAMMA	"10"

#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF

#define CMD_SET_DISP_START_LINE 0x40
#define CMD_SET_PAGE 0xB0

#define CMD_SET_COLUMN_UPPER 0x10
#define CMD_SET_COLUMN_LOWER 0x00

#define CMD_SET_ADC_NORMAL 0xA0
#define CMD_SET_ADC_REVERSE 0xA1

#define CMD_SET_DISP_NORMAL 0xA6
#define CMD_SET_DISP_REVERSE 0xA7

#define CMD_SET_ALLPTS_NORMAL 0xA4
#define CMD_SET_ALLPTS_ON 0xA5
#define CMD_SET_BIAS_9 0xA2
#define CMD_SET_BIAS_7 0xA3

#define CMD_RMW 0xE0
#define CMD_RMW_CLEAR 0xEE
#define CMD_INTERNAL_RESET 0xE2
#define CMD_SET_COM_NORMAL 0xC0
#define CMD_SET_COM_REVERSE 0xC8
#define CMD_SET_POWER_CONTROL 0x28
#define CMD_SET_RESISTOR_RATIO 0x20
#define CMD_SET_VOLUME_FIRST 0x81
#define CMD_SET_VOLUME_SECOND 0
#define CMD_SET_STATIC_OFF 0xAC
#define CMD_SET_STATIC_ON 0xAD
#define CMD_SET_STATIC_REG 0x0
#define CMD_SET_BOOSTER_FIRST 0xF8
#define CMD_SET_BOOSTER_234 0
#define CMD_SET_BOOSTER_5 1
#define CMD_SET_BOOSTER_6 3
#define CMD_NOP 0xE3
#define CMD_TEST 0xF0

void write_data_command(struct fbtft_par *par, unsigned dc, u32 val)
{
	int ret;

	if (par->gpio.dc != -1)
		gpio_set_value(par->gpio.dc, dc);

	*par->buf = (u8)val;

	ret = par->fbtftops.write(par, par->buf, 1);
}


static int init_display(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	par->fbtftops.reset(par);

	mdelay(550);

	gpio_set_value(par->gpio.dc, 0);
	  // LCD bias select
	write_reg(par,CMD_SET_BIAS_7);
	  // ADC select
	write_reg(par,CMD_SET_ADC_NORMAL);
	  // SHL select
	write_reg(par,CMD_SET_COM_NORMAL);
	  // Initial display line
	write_reg(par,CMD_SET_DISP_START_LINE);

	  // turn on voltage converter (VC=1, VR=0, VF=0)
	write_reg(par,CMD_SET_POWER_CONTROL | 0x4);
	  // wait for 50% rising
	mdelay(5);

	  // turn on voltage regulator (VC=1, VR=1, VF=0)
	write_reg(par,CMD_SET_POWER_CONTROL | 0x6);
	  // wait >=50ms
	mdelay(5);
	  // turn on voltage follower (VC=1, VR=1, VF=1)
	write_reg(par,CMD_SET_POWER_CONTROL | 0x7);
	  // wait
	mdelay(10);

	  // set lcd operating voltage (regulator resistor, ref voltage resistor)
	write_reg(par,CMD_SET_RESISTOR_RATIO | 0x6);

	write_reg(par,CMD_DISPLAY_ON);
	write_reg(par,CMD_SET_ALLPTS_NORMAL);
	mdelay(30);

	write_reg(par,CMD_SET_VOLUME_FIRST);
	write_reg(par,CMD_SET_VOLUME_SECOND | (0x03 & 0x3f));

	//clear screen
	char p,c;
	for(p = 0; p < 8; p++) {
		write_data_command(par,0,CMD_SET_PAGE | p);

	    for(c = 0; c < 132; c++) {
	    	write_data_command(par,0, CMD_SET_COLUMN_LOWER | (c & 0xf));
	    	write_data_command(par,0, CMD_SET_COLUMN_UPPER | ((c >> 4) & 0xf));
	    	write_data_command(par,1, 0x00);
	    }
	  }

	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	fbtft_par_dbg(DEBUG_SET_ADDR_WIN, par,
		"%s(xs=%d, ys=%d, xe=%d, ye=%d)\n", __func__, xs, ys, xe, ye);

	// TODO : implement set_addr_win

}

static int set_var(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	// TODO : implement additional functions like rotate settings

	return 0;
}

static int write_vmem(struct fbtft_par *par, size_t offset, size_t len)
{
	u16 *vmem16 = (u16 *)par->info->screen_base;
	u8 *buf = par->txbuf.buf;
	u8 *p_buf = par->txbuf.buf;
	int x, y, i;
	int ret = 0;
	char p, c;

	fbtft_par_dbg(DEBUG_WRITE_VMEM, par, "%s()\n", __func__);

	for (y=0;y<8;y++) {
	for (x=0;x<132;x++) {
			*buf = 0x00;
			for (i=0;i<8;i++) {
				*buf |= (vmem16[(y*8+i)*132+x] ? 1 : 0) << i;
			}
			buf++;
		}
	}

	for(p = 0; p < 8; p++) {
		write_data_command(par,0 ,CMD_SET_PAGE | p);
		write_data_command(par,0 ,CMD_SET_COLUMN_LOWER | ((1) & 0xf));
		write_data_command(par,0 ,CMD_SET_COLUMN_UPPER | (((1) >> 4) & 0x0F));
		write_data_command(par,0 ,CMD_RMW);
	    for(c = 0; c < 132; c++) {
			write_data_command(par,1, *p_buf);
	    	p_buf++;
	    }
	  }

	return ret;
}

static int set_gamma(struct fbtft_par *par, unsigned long *curves)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);
	// TODO : gamma can be used to control contrast
	return 0;
}

static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.txbuflen = TXBUFLEN,
	.gamma_num = 1,
	.gamma_len = 1,
	.gamma = DEFAULT_GAMMA,   // TODO : gamma can be used to control contrast
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		.set_var = set_var,
		.write_vmem = write_vmem,
		.set_gamma = set_gamma,
	},
	.backlight = 1,
};

FBTFT_REGISTER_DRIVER(DRVNAME, &display);

MODULE_ALIAS("spi:" DRVNAME);

MODULE_DESCRIPTION("FB driver for the ST7565 LCD Controller");
MODULE_AUTHOR("Karol Poczesny");
MODULE_LICENSE("GPL");
