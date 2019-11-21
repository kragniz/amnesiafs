// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/printk.h>

void amnesiafs_msg(const char *level, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	printk("%samnesiafs: %pV\n", level, &vaf);
	va_end(args);
}
