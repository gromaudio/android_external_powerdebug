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

#include <sys/stat.h>
#include <sys/vfs.h>
#include <linux/magic.h>

#define MAX_LINES 80

struct clock_info {
	char name[NAME_MAX];
	int flags;
	int rate;
	int usecount;
	int num_children;
	int last_child;
	int expanded;
	struct clock_info *parent;
	struct clock_info **children;
} *clocks_info;

char debugfs_mntpoint[1024];
char clock_lines[MAX_LINES][128];
int  clock_line_no;

char *likely_mpoints[] = {
	"/sys/kernel/debug",
	"/debug",
	NULL
};

void add_clock_details_recur(struct clock_info *clk);
void destroy_clocks_info(void);
void destroy_clocks_info_recur(struct clock_info *clock);
