/*******************************************************************************
 * Copyright (C) 2010, Linaro
 * Copyright (C) 2010, IBM Corporation
 *
 * This file is part of PowerDebug.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Amit Arora <amit.arora@linaro.org> (IBM Corporation)
 *       - initial API and implementation
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>
#include <errno.h>
#include <ncurses.h>

#define VERSION "1.0"

#define VALUE_MAX 16

struct regulator_info {
	char name[NAME_MAX];
	char state[VALUE_MAX];
	char status[VALUE_MAX];
	char type[VALUE_MAX];
	char opmode[VALUE_MAX];
	int microvolts;
	int min_microvolts;
	int max_microvolts;
	int microamps;
	int min_microamps;
	int max_microamps;
	int requested_microamps;
	int num_users;
} *regulators_info;

extern int numregulators;
extern int dump;

extern void usage(char **argv);
extern void version(void);
extern void print_regulator_info(int verbose);
extern void get_sensor_info(char *path, char *name, char *sensor, int verbose);
extern void print_string_val(char *name, char *val);

#define PT_COLOR_DEFAULT    1
#define PT_COLOR_HEADER_BAR 2
#define PT_COLOR_ERROR      3
#define PT_COLOR_RED        4
#define PT_COLOR_YELLOW     5
#define PT_COLOR_GREEN      6
#define PT_COLOR_BRIGHT     7
#define PT_COLOR_BLUE       8



extern void init_curses(void);
extern void fini_curses(void);
extern void killall_windows(void);
extern void show_header(void);
extern void create_windows(void);
extern void show_regulator_info(int verbose);
