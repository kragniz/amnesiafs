// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_LOG_H
#define AMNESIAFS_LOG_H

void amnesiafs_msg(const char *level, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

void amnesiafs_msg_line_number(const char *level, const char *fname, int lineno,
			       const char *fxname, const char *fmt, ...)
	__attribute__((format(printf, 5, 6)));

#define amnesiafs_err(fmt, ...) amnesiafs_msg(KERN_ERR, fmt, ##__VA_ARGS__)

#define amnesiafs_info(fmt, ...) amnesiafs_msg(KERN_INFO, fmt, ##__VA_ARGS__)

#if defined(DEBUG)
#define amnesiafs_debug(fmt, ...)                                              \
	amnesiafs_msg_line_number(KERN_DEBUG, __FILE__, __LINE__, __func__,    \
				  fmt, ##__VA_ARGS__)
#else
#define amnesiafs_debug(fmt, ...) no_printk(KERN_DEBUG fmt, ##__VA_ARGS__)
#endif

#endif