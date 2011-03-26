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

#define VALUE_MAX 16

int numregulators;

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
};

extern struct regulator_info *regulator_init(int *nr_regulators);
extern int regulator_read_info(struct regulator_info *reg_info, int nr_reg);
extern void regulator_print_info(struct regulator_info *reg_info,
				 int nr_reg, int verbose);
