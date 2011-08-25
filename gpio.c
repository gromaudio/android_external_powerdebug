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
 *     Daniel Lezcano <daniel.lezcano@linaro.org> (IBM Corporation)
 *       - initial API and implementation
 *******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#endif
#include <mntent.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "powerdebug.h"
#include "display.h"
#include "tree.h"
#include "utils.h"

#define SYSFS_GPIO "/sys/class/gpio"

static struct tree *gpio_tree = NULL;

static int gpio_display(bool refresh)
{
	return 0;
}

static int gpio_select(void)
{
	return 0;
}

static int gpio_find(const char *name)
{
	return 0;
}

static int gpio_selectf(void)
{
	return 0;
}

static struct display_ops gpio_ops = {
	.display = gpio_display,
	.select  = gpio_select,
	.find    = gpio_find,
	.selectf = gpio_selectf,
};

static inline int read_gpio_cb(struct tree *t, void *data)
{
	return 0;
}

static int read_gpio_info(struct tree *tree)
{
	return 0;
}

static int fill_gpio_cb(struct tree *t, void *data)
{
	return 0;
}

static int fill_gpio_tree(void)
{
	return 0;
}

int gpio_dump(void)
{
	return 0;
}

/*
 * Initialize the gpio framework
 */
int gpio_init(void)
{
	gpio_tree = tree_load(SYSFS_GPIO, NULL, false);
	if (!gpio_tree)
		return -1;

	if (fill_gpio_tree())
		return -1;

	return display_register(GPIO, &gpio_ops);
}
