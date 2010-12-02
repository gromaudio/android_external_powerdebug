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

#include "powerdebug.h"
#include "sensor.h"

char *get_num(char *fname, char *sensor)
{
	char tmpstr[NAME_MAX];
	char *str;

	strcpy(tmpstr, (fname+strlen(sensor)));

	str = strrchr(tmpstr, '_');
	str[0] = '\0';

	str = strdup(tmpstr);
	return str;
}
	

void get_sensor_info(char *path, char *fname, char *sensor, int verbose)
{
	FILE *filep;
	char filename[PATH_MAX];
	char **items = NULL, **suffix = NULL;
	char *item, result[NAME_MAX], *num;
	int ret, count = 0;

	(void)verbose; // get rid of warning

	sprintf(filename, "%s/%s", path, fname);

	if(!strcmp(sensor, "in")) {
		items = (char **)items_in;
		suffix = (char **)suffix_in;
	}

	if(!strcmp(sensor, "temp")) {
		items = (char **)items_temp;
		suffix = (char **)suffix_temp;
	}

	if(!strcmp(sensor, "fan")) {
		items = (char **)items_fan;
		suffix = (char **)suffix_fan;
	}
	if(!strcmp(sensor, "pwm")) {
		items = (char **)items_pwm;
		suffix = (char **)suffix_pwm;
	}


	if (!items || !suffix)
		return;

	item = strrchr(fname, '_');
	if(!item)
		return;

	if(item)
		item++;

	if(item > (fname + strlen(fname)))
		return;

	num = get_num(fname, sensor);
	filep = fopen(filename, "r");

	if(!filep) 
		goto exit;
	ret = fscanf(filep, "%s", result);
	fclose(filep);

	if(ret != 1)
		goto exit;

	while(strcmp(items[count], "")) {
		if(!strcmp(items[count], item))
			printf("\'temp\' %s sensor %s\t\t%d%s\n",
				num, items[count], atoi(result)/1000,
				suffix[count]);
		count++;
	}
exit:
	free(num);
	return;
}

int read_and_print_sensor_info(int verbose)
{
	DIR *dir, *subdir;
	int len, found = 0;
	char filename[PATH_MAX], devpath[PATH_MAX];
	char device[PATH_MAX];
	struct dirent *item, *subitem;

	sprintf(filename, "%s", "/sys/class/hwmon");
	dir = opendir(filename);
	if (!dir)
		return errno;

	while ((item = readdir(dir))) {
		if (item->d_name[0] == '.')  /* skip the hidden files */
			continue;

		found = 1;

		sprintf(filename, "/sys/class/hwmon/%s", item->d_name);
		sprintf(devpath, "%s/device", filename);

		len = readlink(devpath, device, PATH_MAX - 1);

		if (len < 0)
			strcpy(devpath, filename);
		else
			device[len] = '\0';

		subdir = opendir(devpath);

		printf("\nSensor Information for %s :\n", item->d_name);
		fflush(stdin);

		while ((subitem = readdir(subdir))) {
			if (subitem->d_name[0] == '.') /* skip hidden files */
				continue;

			if(!strncmp(subitem->d_name, "in", 2))
				get_sensor_info(devpath, subitem->d_name, "in",
						verbose);
			else if (!strncmp(subitem->d_name, "temp", 4))
				get_sensor_info(devpath, subitem->d_name,
						"temp", verbose);
			else if (!strncmp(subitem->d_name, "fan", 4))
				get_sensor_info(devpath, subitem->d_name,
						"fan", verbose);
			else if (!strncmp(subitem->d_name, "pwm", 4))
				get_sensor_info(devpath, subitem->d_name,
						"pwm", verbose);

		}

		closedir(subdir);
	}
	closedir(dir);

	if(!found && verbose) {
		printf("Could not find sensor information!");
		printf(" Looks like /sys/class/hwmon is empty.\n");
	}

	return 0;
}
