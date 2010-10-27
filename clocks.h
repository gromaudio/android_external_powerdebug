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

struct clock_info {
        char name[NAME_MAX];
        int flags;
        int rate;
        int usecount;
        int num_children;
        int last_child;
        struct clock_info *parent;
        struct clock_info **children;
} *clocks_info;

char debugfs_mntpoint[1024];

char *likely_mpoints[] = {
        "/sys/kernel/debug",
        "/debug",
        NULL
};
