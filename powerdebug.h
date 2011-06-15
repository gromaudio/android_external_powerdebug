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

extern int read_and_dump_clock_info(char *clk);
extern void find_parents_for_clock(char *clkname, int complete);
extern int read_and_print_clock_info(void);
extern int print_clock_info(int hrow, int selected);
extern void print_string_val(char *name, char *val);
extern void print_clock_header(void);

extern int display_print_line(int window, int line, char *str,
			      int bold, void *data);

extern int display_refresh_pad(int window);
extern int display_reset_cursor(int window);
extern int display_next_line(int window);
extern int display_prev_line(int window);
extern void *display_get_row_data(int window);

extern int clock_toggle_expanded(void);
extern int display_clock_select(int window, int line);
extern int display_clock_unselect(int window, int line, bool bold);

extern void get_sensor_info(char *path, char *name, char *sensor, int verbose);
extern int  read_and_print_sensor_info(int verbose);
extern void print_sensor_header(void);

extern void killall_windows(int all);
extern void show_header(int selectedwindow);
extern void create_windows(int selectedwindow);
extern void create_selectedwindow(int selectedwindow);

struct regulator_info;
extern void show_regulator_info(struct regulator_info *reg_info,
				int nr_reg, int verbose);
