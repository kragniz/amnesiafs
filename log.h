// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_LOG_H
#define AMNESIAFS_LOG_H

void amnesiafs_msg(const char *level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#endif