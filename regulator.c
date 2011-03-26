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

int regulator_init(void)
{
	DIR *regdir;
	struct dirent *item;

	regdir = opendir("/sys/class/regulator");
	if (!regdir)
		return(1);
	while ((item = readdir(regdir))) {
		if (strncmp(item->d_name, "regulator", 9))
			continue;

		numregulators++;
	}
	closedir(regdir);

	regulators_info = (struct regulator_info *)malloc(numregulators*
							sizeof(struct regulator_info));
	if (!regulators_info) {
		fprintf(stderr, "init_regulator_ds: Not enough memory to "
			"read information for %d regulators!\n", numregulators);
		return(1);
	}

	return(0);
}

void print_string_val(char *name, char *val)
{
	printf("\t%s=%s", name, val);
	if (!strchr(val, '\n'))
		printf("\n");
}

void print_regulator_info(int verbose)
{
	int i;

	printf("\nRegulator Information:\n");
	printf("*********************\n\n");

	for (i = 0; i < numregulators; i++) {
		printf("Regulator %d:\n", i + 1);
		print_string_val("name", regulators_info[i].name);
		if (strcmp(regulators_info[i].status, ""))
			print_string_val("status", regulators_info[i].status);
		if (strcmp(regulators_info[i].state, ""))
			print_string_val("state", regulators_info[i].state);

		if (!verbose)
			continue;

		if (strcmp(regulators_info[i].type, ""))
			print_string_val("type", regulators_info[i].type);
		if (strcmp(regulators_info[i].opmode, ""))
			print_string_val("opmode", regulators_info[i].opmode);

		if (regulators_info[i].microvolts)
			printf("\tmicrovolts=%d\n",
				regulators_info[i].microvolts);
		if (regulators_info[i].min_microvolts)
			printf("\tmin_microvolts=%d\n",
				regulators_info[i].min_microvolts);
		if (regulators_info[i].max_microvolts)
			printf("\tmax_microvolts=%d\n",
				regulators_info[i].max_microvolts);

		if (regulators_info[i].microamps)
			printf("\tmicroamps=%d\n",
				regulators_info[i].microamps);
		if (regulators_info[i].min_microamps)
			printf("\tmin_microamps=%d\n",
				regulators_info[i].min_microamps);
		if (regulators_info[i].max_microamps)
			printf("\tmax_microamps=%d\n",
				regulators_info[i].max_microamps);
		if (regulators_info[i].requested_microamps)
			printf("\trequested_microamps=%d\n",
				regulators_info[i].requested_microamps);

		if (regulators_info[i].num_users)
			printf("\tnum_users=%d\n",
				regulators_info[i].num_users);
		printf("\n");
	}

	if (!numregulators && verbose) {
		printf("Could not find regulator information!");
		printf(" Looks like /sys/class/regulator is empty.\n\n");
	}

	printf("\n\n");
}

void read_info_from_dirent(struct dirent *ritem, char *str, int idx)
{
	if (!strcmp(ritem->d_name, "name"))
		strcpy(regulators_info[idx].name, str);
	if (!strcmp(ritem->d_name, "state"))
		strcpy(regulators_info[idx].state, str);
	if (!strcmp(ritem->d_name, "status"))
		strcpy(regulators_info[idx].status, str);

	if (!strcmp(ritem->d_name, "type"))
		strcpy(regulators_info[idx].type, str);
	if (!strcmp(ritem->d_name, "opmode"))
		strcpy(regulators_info[idx].opmode, str);

	if (!strcmp(ritem->d_name, "microvolts"))
		regulators_info[idx].microvolts = atoi(str);
	if (!strcmp(ritem->d_name, "min_microvolts"))
		regulators_info[idx].min_microvolts = atoi(str);
	if (!strcmp(ritem->d_name, "max_microvolts"))
		regulators_info[idx].max_microvolts = atoi(str);

	if (!strcmp(ritem->d_name, "microamps"))
		regulators_info[idx].microamps = atoi(str);
	if (!strcmp(ritem->d_name, "min_microamps"))
		regulators_info[idx].min_microamps = atoi(str);
	if (!strcmp(ritem->d_name, "max_microamps"))
		regulators_info[idx].max_microamps = atoi(str);
	if (!strcmp(ritem->d_name, "requested_microamps"))
		regulators_info[idx].requested_microamps = atoi(str);

	if (!strcmp(ritem->d_name, "num_users"))
		regulators_info[idx].num_users = atoi(str);
}

int read_regulator_info(void)
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
			read_info_from_dirent(ritem, fptr, count - 1);
		}
	exit:
		closedir(dir);
		if (ret)
			break;
	}
	closedir(regdir);

	return ret;
}
