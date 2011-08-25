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

struct gpio_info {
	bool expanded;
	int active_low;
	int value;
	int direction;
	int edge;
} *gpios_info;

static struct tree *gpio_tree = NULL;

static struct gpio_info *gpio_alloc(void)
{
	struct gpio_info *gi;

	gi = malloc(sizeof(*gi));
	if (gi)
		memset(gi, 0, sizeof(*gi));

	return gi;
}

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

static int gpio_filter_cb(const char *name)
{
	/* let's ignore some directories in order to avoid to be
	 * pulled inside the sysfs circular symlinks mess/hell
	 * (choose the word which fit better)
	 */
	if (!strcmp(name, "device"))
		return 1;

	if (!strcmp(name, "subsystem"))
		return 1;

	if (!strcmp(name, "driver"))
		return 1;

        /* we want to ignore the gpio chips */
	if (strstr(name, "chip"))
		return 1;

        /* we are not interested by the power value */
	if (!strcmp(name, "power"))
		return 1;

	return 0;
}

static inline int read_gpio_cb(struct tree *t, void *data)
{
	struct gpio_info *gpio = t->private;

	file_read_value(t->path, "active_low", "%d", &gpio->active_low);
	file_read_value(t->path, "value", "%d", &gpio->value);
	file_read_value(t->path, "edge", "%d", &gpio->edge);
	file_read_value(t->path, "direction", "%d", &gpio->direction);

	return 0;
}

static int read_gpio_info(struct tree *tree)
{
	return tree_for_each(tree, read_gpio_cb, NULL);
}

static int fill_gpio_cb(struct tree *t, void *data)
{
	struct gpio_info *gpio;

	gpio = gpio_alloc();
	if (!gpio)
		return -1;
	t->private = gpio;

        /* we skip the root node but we set it expanded for its children */
	if (!t->parent) {
		gpio->expanded = true;
		return 0;
	}

	return read_gpio_cb(t, data);

}

static int fill_gpio_tree(void)
{
	return tree_for_each(gpio_tree, fill_gpio_cb, NULL);
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
	gpio_tree = tree_load(SYSFS_GPIO, gpio_filter_cb, false);
	if (!gpio_tree)
		return -1;

	if (fill_gpio_tree())
		return -1;

	return display_register(GPIO, &gpio_ops);
}
