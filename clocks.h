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

#include <sys/stat.h>
#include <sys/vfs.h>
#include <errno.h>
#include <sys/stat.h>
#include <linux/magic.h>

extern int maxy;
extern int dump;

#define MAX_LINES 120

struct clock_info {
	char name[NAME_MAX];
	int flags;
	int rate;
	int usecount;
	int num_children;
	int last_child;
	int expanded;
	int level;
	struct clock_info *parent;
	struct clock_info **children;
} *clocks_info;

char debugfs_mntpoint[1024];
char clock_lines[MAX_LINES][128];
int  clock_line_no;
int  old_clock_line_no;

char *likely_mpoints[] = {
	"/sys/kernel/debug",
	"/debug",
	NULL
};

void add_clock_details_recur(struct clock_info *clk, int hrow, int selected);
void destroy_clocks_info(void);
void destroy_clocks_info_recur(struct clock_info *clock);
void collapse_all_subclocks(struct clock_info *clock);
void dump_all_parents(char *clkarg);
struct clock_info *find_clock(struct clock_info *clk, char *clkarg);
