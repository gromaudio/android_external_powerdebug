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

extern void find_parents_for_clock(char *clkname, int complete);
extern int display_print_line(int window, int line, char *str,
			      int bold, void *data);

extern int display_refresh_pad(int window);
extern int display_reset_cursor(int window);
extern void *display_get_row_data(int window);

extern int clock_toggle_expanded(void);
extern int regulator_display(void);
extern void print_sensor_header(void);
extern void print_clock_header(void);
extern void print_regulator_header(void);

