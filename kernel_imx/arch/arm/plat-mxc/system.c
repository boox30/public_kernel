/*
 * Copyright (C) 1999 ARM Limited
 * Copyright (C) 2000 Deep Blue Solutions Ltd
 * Copyright (C) 2006-2011 Freescale Semiconductor, Inc.
 * Copyright 2008 Juergen Beisert, kernel@pengutronix.de
 * Copyright 2009 Ilya Yanok, Emcraft Systems Ltd, yanok@emcraft.com
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/delay.h>

#include <asm/mach-types.h>
#include <mach/iomux-mx53.h>
#include <mach/hardware.h>
#include <mach/common.h>
#include <asm/proc-fns.h>
#include <asm/system.h>
#include <mach/gpio.h>

static void __iomem *wdog_base;
extern int dvfs_core_is_active;
extern void stop_dvfs(void);
#define MX53_WDA_GPIO 9
/*
 * Reset the system. It is called by machine_restart().
 */
void arch_reset(char mode, const char *cmd)
{
	unsigned int wcr_enable;

#ifdef CONFIG_ARCH_MXC91231
	if (cpu_is_mxc91231()) {
		mxc91231_arch_reset(mode, cmd);
		return;
	}
#endif

#ifdef CONFIG_ARCH_MX51
	/* Workaround to reset NFC_CONFIG3 register
	 * due to the chip warm reset does not reset it
	 */
	 if (cpu_is_mx51() || cpu_is_mx53())
		__raw_writel(0x20600, IO_ADDRESS(NFC_BASE_ADDR) + 0x28);
#endif

#ifdef CONFIG_ARCH_MX5
	/* Stop DVFS-CORE before reboot. */
	if (dvfs_core_is_active)
		stop_dvfs();
#endif

	if (cpu_is_mx1()) {
		wcr_enable = (1 << 0);
	} else {
		struct clk *clk;

		clk = clk_get_sys("imx-wdt.0", NULL);
		if (!IS_ERR(clk))
			clk_enable(clk);
		wcr_enable = (1 << 2);
	}

	/* Assert SRS signal */
	__raw_writew(wcr_enable, wdog_base);

	/* wait for reset to assert... */
	mdelay(500);

	printk(KERN_ERR "Watchdog reset failed to assert reset\n");

	/* delay to allow the serial port to show the message */
	mdelay(50);

	/* we'll take a jump through zero as a poor second */
	cpu_reset(0);
}

void mxc_arch_reset_init(void __iomem *base)
{
	wdog_base = base;
}
