/******************************************************************************
	appender.cpp: description
	Copyright (C) 2012 Wang Bin <wbsecg1@gmail.com>
	
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


#include "appender.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eztime.h"
#include "list.h"

typedef struct {
	appender handle;
	struct list_head list;
} appender_node;

//appender_node console_appender_node = {console_appender, {}};
//appender_node file_appender_node = {file_appender, {}};

#ifndef PATH_MAX
#define PATH_MAX 256
#endif
typedef struct {
	char first;
	FILE *file;
	char name[PATH_MAX];
	LogOpenMode mode; //LogOpenMode
	struct list_head list;
} logfile_node;

LIST_HEAD(appenders_head);
LIST_HEAD(logfiles_head);

#define SIZE_LOGFILENAME 19 //yyyyMMddhhmmss.log
static char default_logfile[SIZE_LOGFILENAME];
static char last_default_logfile[SIZE_LOGFILENAME];
void ezlog_registerAppender(appender handle)
{
	appender_node *node = (appender_node*)malloc(sizeof(appender_node));
	node->handle = handle;
	list_add_tail(&(node->list), &appenders_head);

	if (handle == file_appender) {
		eztime t;
		getTime(&t);
		memset(default_logfile, 0, SIZE_LOGFILENAME);
		sprintf(default_logfile, "%d%02d%02d%02d%02d%02d.log", t.year, t.month, t.day
				, t.hour, t.min, t.sec);
		ezlog_add_logfile(default_logfile, New | OPEN_ON_WRITE);
		strcpy(last_default_logfile, default_logfile);
	}
}

void ezlog_unregisterAppender(appender handle)
{
	struct list_head *pos = &appenders_head;
	list_for_each(pos, &appenders_head) {
		appender_node* node = list_entry(pos, appender_node, list);
		if (node->handle == handle) {
			list_del(&node->list);
			free(node);
			node = 0;
			break;
		}
	}
}

void ezlog_unregisterAllAppenders()
{
	struct list_head *pos = &appenders_head;
	list_for_each(pos, &appenders_head) {
		appender_node* node = list_entry(pos, appender_node, list);
		//FIXME: seg fault!
		//list_del(&node->list);
		free(node);
		node = 0;
	}
}

FILE* __open_logfile(const char *path, int mode, logfile_node* node)
{
	//First time is the same as mode. Then will will append the msg if OPEN_ON_WRITE
	FILE *file = 0;
	if(!strcmp(path, "stdout")) {
		file = stdout;
	} else if(!strcmp(path, "stderr")) {
		file = stderr;
	} else {
		const char* m = "ab";
		if ((mode & New) == New && node->first) {
			m = "wb";
		}

		file = fopen(path, m);
		if(!file) {
			//perror("open log file failed!"); //wince does not support
		} else {
			node->first = 0;
		}
	}
	return file;
}

void ezlog_add_logfile(const char *path, int mode)
{
	//ezscoped_lock lock(mutex);
	ezlog_remove_logfile(last_default_logfile);
	logfile_node *node = (logfile_node*)malloc(sizeof(logfile_node));
	node->first = 1;
	if (!IS_OPEN_ON_WRITE(mode)) {
		FILE *file = __open_logfile(path, mode, node);
		node->file = file;
	}
	strcpy(node->name, path);
	node->mode = (LogOpenMode)mode;
	//printf("%s: %p\n", node->name, node->file);
	list_add_tail(&(node->list), &logfiles_head);
}

//convert to abs path?
void ezlog_remove_logfile(const char *path)
{
	struct list_head *pos = &logfiles_head;
	list_for_each(pos, &logfiles_head) { //list_for_each_entry
		logfile_node* node = list_entry(pos, logfile_node, list);
		if (strcmp(node->name, path) == 0) {
			if (!IS_OPEN_ON_WRITE(node->mode))
				fclose(node->file);
			list_del(&(node->list));
			break;
		}
	}
}


void console_appender(const char *msg)
{
	fprintf(stdout, "%s\n", msg);
	fflush(stdout);  //condition?
}

void file_appender(const char *msg)
{
	struct list_head *pos = &logfiles_head;
	list_for_each(pos, &logfiles_head) { //list_for_each_entry
		logfile_node* node = list_entry(pos, logfile_node, list);
		if (IS_OPEN_ON_WRITE(node->mode)) {
			FILE *file = __open_logfile(node->name, node->mode, node);
			fprintf(file, "%s\n", msg);
			fclose(file);
		} else {
			fprintf(node->file, "%s\n", msg);
			fflush(node->file);  //condition?
		}
	}
}


//for internal use
void __log_to_appenders(const char* msg)
{
	struct list_head *pos = &appenders_head;
	list_for_each(pos, &appenders_head) { //list_for_each_entry
		appender_node* node = list_entry(pos, appender_node, list);
		node->handle(msg);
	}
}