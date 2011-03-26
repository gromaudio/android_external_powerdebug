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

#include "regulator.h"

#define SYSFS_REGULATOR "/sys/class/regulator"

struct regulator_info *regulator_init(int *nr_regulators)
{
	DIR *regdir;
	struct dirent *item;

	*nr_regulators = 0;

	regdir = opendir(SYSFS_REGULATOR);
	if (!regdir) {
		fprintf(stderr, "failed to open '%s': %m\n", SYSFS_REGULATOR);
		return NULL;
	}

	while ((item = readdir(regdir))) {

		if (!strcmp(item->d_name, "."))
			continue;

		if (!strcmp(item->d_name, ".."))
			continue;

		(*nr_regulators)++;
	}

	closedir(regdir);

	return malloc(*nr_regulators * sizeof(struct regulator_info));
}

static void print_string_val(char *name, char *val)
{
	printf("\t%s=%s", name, val);
	if (!strchr(val, '\n'))
		printf("\n");
}

void regulator_print_info(struct regulator_info *reg_info, int verbose)
{
	int i;

	printf("\nRegulator Information:\n");
	printf("*********************\n\n");

	for (i = 0; i < numregulators; i++) {
		printf("Regulator %d:\n", i + 1);
		print_string_val("name", reg_info[i].name);
		if (strcmp(reg_info[i].status, ""))
			print_string_val("status", reg_info[i].status);
		if (strcmp(reg_info[i].state, ""))
			print_string_val("state", reg_info[i].state);

		if (!verbose)
			continue;

		if (strcmp(reg_info[i].type, ""))
			print_string_val("type", reg_info[i].type);
		if (strcmp(reg_info[i].opmode, ""))
			print_string_val("opmode", reg_info[i].opmode);

		if (reg_info[i].microvolts)
			printf("\tmicrovolts=%d\n",
				reg_info[i].microvolts);
		if (reg_info[i].min_microvolts)
			printf("\tmin_microvolts=%d\n",
				reg_info[i].min_microvolts);
		if (reg_info[i].max_microvolts)
			printf("\tmax_microvolts=%d\n",
				reg_info[i].max_microvolts);

		if (reg_info[i].microamps)
			printf("\tmicroamps=%d\n",
				reg_info[i].microamps);
		if (reg_info[i].min_microamps)
			printf("\tmin_microamps=%d\n",
				reg_info[i].min_microamps);
		if (reg_info[i].max_microamps)
			printf("\tmax_microamps=%d\n",
				reg_info[i].max_microamps);
		if (reg_info[i].requested_microamps)
			printf("\trequested_microamps=%d\n",
				reg_info[i].requested_microamps);

		if (reg_info[i].num_users)
			printf("\tnum_users=%d\n",
				reg_info[i].num_users);
		printf("\n");
	}

	if (!numregulators && verbose) {
		printf("Could not find regulator information!");
		printf(" Looks like /sys/class/regulator is empty.\n\n");
	}

	printf("\n\n");
}

static void read_info_from_dirent(struct regulator_info *reg_info,
				  struct dirent *ritem, char *str, int idx)
{
	if (!strcmp(ritem->d_name, "name"))
		strcpy(reg_info[idx].name, str);
	if (!strcmp(ritem->d_name, "state"))
		strcpy(reg_info[idx].state, str);
	if (!strcmp(ritem->d_name, "status"))
		strcpy(reg_info[idx].status, str);

	if (!strcmp(ritem->d_name, "type"))
		strcpy(reg_info[idx].type, str);
	if (!strcmp(ritem->d_name, "opmode"))
		strcpy(reg_info[idx].opmode, str);

	if (!strcmp(ritem->d_name, "microvolts"))
		reg_info[idx].microvolts = atoi(str);
	if (!strcmp(ritem->d_name, "min_microvolts"))
		reg_info[idx].min_microvolts = atoi(str);
	if (!strcmp(ritem->d_name, "max_microvolts"))
		reg_info[idx].max_microvolts = atoi(str);

	if (!strcmp(ritem->d_name, "microamps"))
		reg_info[idx].microamps = atoi(str);
	if (!strcmp(ritem->d_name, "min_microamps"))
		reg_info[idx].min_microamps = atoi(str);
	if (!strcmp(ritem->d_name, "max_microamps"))
		reg_info[idx].max_microamps = atoi(str);
	if (!strcmp(ritem->d_name, "requested_microamps"))
		reg_info[idx].requested_microamps = atoi(str);

	if (!strcmp(ritem->d_name, "num_users"))
		reg_info[idx].num_users = atoi(str);
}

int regulator_read_info(void)
{
	FILE *file = NULL;
	DIR *regdir, *dir;
	int len, count = 0, ret = 0;
	char line[1024], filename[1024], *fptr;
	struct dirent *item, *ritem;

	regdir = opendir("/sys/class/regulator");
	if (!regdir)
		return(1);
	while ((item = readdir(regdir))) {
		if (strlen(item->d_name) < 3)
			continue;

		if (strncmp(item->d_name, "regulator", 9))
			continue;

		len = sprintf(filename, "/sys/class/regulator/%s",
			item->d_name);

		dir = opendir(filename);
		if (!dir)
			continue;
		count++;

		if (count > numregulators) {
			ret = 1;
			goto exit;
		}

		strcpy(regulators_info[count-1].name, item->d_name);
		while ((ritem = readdir(dir))) {
			if (strlen(ritem->d_name) < 3)
				continue;

			sprintf(filename + len, "/%s", ritem->d_name);
			file = fopen(filename, "r");
			if (!file)
				continue;
			memset(line, 0, 1024);
			fptr = fgets(line, 1024, file);
			fclose(file);
			if (!fptr)
				continue;

			read_info_from_dirent(regulators_info, ritem,
					      fptr, count - 1);
		}
	exit:
		closedir(dir);
		if (ret)
			break;
	}
	closedir(regdir);

	return ret;
}
