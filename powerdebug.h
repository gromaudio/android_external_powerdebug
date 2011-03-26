/*******************************************************************************
 * Copyright (C) 2010, Linaro Limited.
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

#define VERSION "0.4.1"

#define TOTAL_FEATURE_WINS 3  /* Regulator, Clock and Sensor (for now) */
enum {CLOCK, REGULATOR, SENSOR};
enum {CLOCK_SELECTED = 1, REFRESH_WINDOW};

extern int numregulators;

extern void read_and_dump_clock_info(int verbose);
extern void read_and_dump_clock_info_one(char *clk, bool dump);
extern void read_clock_info(char *clkpath);
extern struct clock_info *read_clock_info_recur(char *clkpath, int level,
						struct clock_info *parent);
extern void dump_clock_info(struct clock_info *clk, int level, int bmp);
extern void insert_children(struct clock_info **parent, struct clock_info *clk);
extern void find_parents_for_clock(char *clkname, int complete, bool dump);
extern int  read_and_print_clock_info(int verbose, int hrow, int selected);
extern void print_clock_info(int verbose, int hrow, int selected);
extern void print_string_val(char *name, char *val);
extern int  init_clock_details(bool dump, int selectedwindow);
extern void print_clock_header(void);
extern void print_one_clock(int line, char *str, int bold, int highlight);
extern char *debugfs_locate_mpoint(void);

extern void get_sensor_info(char *path, char *name, char *sensor, int verbose);
extern int  read_and_print_sensor_info(int verbose);
extern void print_sensor_header(void);

extern void init_curses(void);
extern void fini_curses(void);
extern void killall_windows(int all);
extern void show_header(int selectedwindow);
extern void create_windows(int selectedwindow);
extern void create_selectedwindow(int selectedwindow);

struct regulator_info;
extern void show_regulator_info(struct regulator_info *reg_info, int verbose);
