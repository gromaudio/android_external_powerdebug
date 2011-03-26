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

extern int maxy;

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

extern int clock_init(void);
