/******************************************************************************
	ezlog: A tiny log for C/C++
	Copyright (C) 2011 Wang Bin <wbsecg1@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
******************************************************************************/

#include "ezlog.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ezthread.h"
#include "eztime.h"
#include "layout.h"

typedef struct {
	const char* level;
	const char* file;
	const char* func;
	int line;
	const eztime* t;
	unsigned long tid;
	long pid;
	const char* msg;
} ezlog_info;

extern void __format_msg(char* result_msg, ezlog_info* info);
extern void __log_to_appenders(const char* msg);

//__func__
//strftime()

/*!
 * \fn int ezlog_version()
 * \brief Get ezlog's version
 *
 */
int ezlog_version()
{
	return LIB_VERSION;
}

const char* ezlog_version_string()
{
	return version_string;
}

void ezlog_init_default()
{
	printf("ezlog version: %s\n", ezlog_version_string());
	ezlog_init_layout(layout_format[DEFAULT_LAYOUT]);
	ezlog_registerAppender(console_appender);
}



void _ezlog_print(const char* level, const char* file, const int line, const char* func, const char* fmt, ...)
{
	char msg[512];
	va_list args;
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);
	//r += sprintf(msg + r, "\n");

	eztime t;
	getTime(&t);
	ezlog_info info; //static
	info.level = level;
	info.file = file;
	info.func = func;
	info.line = line;
	info.t = &t;
	info.pid = pid();
	info.tid = threadId();
	info.msg = msg;

	static char result_msg[1024];
	memset(result_msg, 0, sizeof(result_msg));
	__format_msg(result_msg, &info);
	__log_to_appenders(result_msg);
}


void ezlog_fini() {
	ezlog_unregisterAllAppenders();
}