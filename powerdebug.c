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

#include <getopt.h>
#include "powerdebug.h"

int numregulators;
int dump;
int highlighted_row;
int selectedwindow = -1;
double ticktime = 10.0;  /* in seconds */

char *win_names[TOTAL_FEATURE_WINS] = {
	"Regulators",
	"Clocks",
	"Sensors" };

int init_regulator_ds(void)
{
	DIR *regdir;
	struct dirent *item;

	regdir = opendir("/sys/class/regulator");
	if (!regdir)
		return(1);
	while((item = readdir(regdir))) {
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
	while((item = readdir(regdir))) {
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
		while((ritem = readdir(dir))) {
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


int main(int argc, char **argv)
{
	int c, i;
	int firsttime[TOTAL_FEATURE_WINS];
	int enter_hit = 0, verbose = 0, findparent_ncurses = 0;
	int regulators = 0, sensors = 0, clocks = 0, findparent = 0;
	char clkarg[64], clkname_str[64];

	for (i = 0; i < TOTAL_FEATURE_WINS; i++)
		firsttime[i] = 1;

	/*
	 * Options:
	 * -r, --regulator      : regulator
	 * -s, --sensor	 : sensors
	 * -c, --clock	  : clocks
	 * -p, --findparents    : clockname whose parents have to be found
	 * -t, --time		: ticktime
	 * -d, --dump		: dump
	 * -v, --verbose	: verbose
	 * -V, --version	: version
	 * -h, --help		: help
	 * no option / default : show usage!
	 */

	while (1) {
		int optindex = 0;
		static struct option long_options[] = {
			{"regulator", 0, 0, 'r'},
			{"sensor", 0, 0, 's'},
			{"clock", 0, 0, 'c'},
			{"findparents", 0, 0, 'p'},
			{"time", 0, 0, 't'},
			{"dump", 0, 0, 'd'},
			{"verbose", 0, 0, 'v'},
			{"version", 0, 0, 'V'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "rscp:t:dvVh", long_options, &optindex);
		if (c == -1)
			break;

		switch (c) {
		case 'r':
			regulators = 1;
			selectedwindow = REGULATOR;
			break;
		case 's':
			sensors = 1;
			selectedwindow = SENSOR;
			break;
		case 'c':
			clocks = 1;
			selectedwindow = CLOCK;
			break;
		case 'p':
			findparent = 1;
			strcpy(clkarg, optarg);
			break;
		case 't':
			ticktime = strtod(optarg, NULL);
			break;
		case 'd':
			dump = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'V':
			version();
			break;
		case 'h':
			usage(argv);
			break;
		case '?':
			fprintf (stderr, "%s: Unknown option %c'.\n",
				 argv[0], optopt);
			exit(1);
		default:
			usage(argv);
			break;
		}
	}

	if (!dump && (regulators || clocks || sensors)) {
		fprintf(stderr, "Option supported only in dump mode (-d)\n");
		usage(argv);
	}

	if (findparent && (!clocks || !dump)) {
		fprintf(stderr, "-p option passed without -c and -d."
			" Exiting...\n");
		usage(argv);
	}

	if (!dump)
		selectedwindow = REGULATOR;

	init_regulator_ds();

	while(1) {
		int key = 0;
		struct timeval tval;
		fd_set readfds;

		if (!dump) {
			if(firsttime[0])
				init_curses();
			create_windows();
			show_header();
		}

	
		if (selectedwindow == REGULATOR) {
			read_regulator_info();
			if (!dump) {
				create_selectedwindow();
				show_regulator_info(verbose);
			}
			else
				print_regulator_info(verbose);
		}

		if (selectedwindow == CLOCK) {
			int ret = 0;
			if (firsttime[CLOCK]) {
				ret = init_clock_details();
				if (!ret)
					firsttime[CLOCK] = 0;
				strcpy(clkname_str, "");
			}
			if (!ret && !dump) {
				int hrow;

				create_selectedwindow();
				if (!findparent_ncurses) {
					hrow = read_and_print_clock_info(
								verbose,
								highlighted_row,
								enter_hit);
					highlighted_row = hrow;
					enter_hit = 0;
				} else
					find_parents_for_clock(clkname_str,
								enter_hit);
			}
			if (!ret && dump) {
				if (findparent)
					read_and_dump_clock_info_one(clkarg);
				else
					read_and_dump_clock_info(verbose);
			}
		}

		if (selectedwindow == SENSOR) {
			if (!dump) {
				create_selectedwindow();
				print_sensor_header();
			} else
				read_and_print_sensor_info(verbose);
		}

		if (dump)
			break;

		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		tval.tv_sec = ticktime;
		tval.tv_usec = (ticktime - tval.tv_sec) * 1000000;

		key = select(1, &readfds, NULL, NULL, &tval);

		if (key)  {
			char keychar;
			int keystroke = getch();
			int oldselectedwin = selectedwindow;

			if (keystroke == EOF)
				exit(0);

			if (keystroke == KEY_RIGHT || keystroke == 9)
				selectedwindow++;

			if (keystroke == KEY_LEFT || keystroke == 353)
				selectedwindow--;

			if (selectedwindow >= TOTAL_FEATURE_WINS)
				selectedwindow = 0;

			if (selectedwindow < 0)
				selectedwindow = TOTAL_FEATURE_WINS - 1;

			if (selectedwindow == CLOCK) {
				if (keystroke == KEY_DOWN)
					highlighted_row++;
				if (keystroke == KEY_UP && highlighted_row > 0)
					highlighted_row--;
				if (keystroke == 47)
					findparent_ncurses = 1;

				if ((keystroke == 27 || oldselectedwin !=
					selectedwindow) && findparent_ncurses) {
					findparent_ncurses = 0;
					clkname_str[0] = '\0';
				}

				if (findparent_ncurses && keystroke != 13) {
					int len = strlen(clkname_str);
					char str[2];

					if (keystroke == 263) {
						if (len > 0)
							len--;

						clkname_str[len] = '\0';
					} else {
						if (strlen(clkname_str) ||
							keystroke != '/') {
						str[0] = keystroke;
						str[1] = '\0';
						if (len < 63)
							strcat(clkname_str,
								str);
						}
					}
				}
			}

			keychar = toupper(keystroke);
//#define DEBUG
#ifdef DEBUG
			killall_windows(1); fini_curses();
			printf("key entered %d:%c\n", keystroke, keychar);
			exit(1);
#endif

			if (keystroke == 13)
				enter_hit = 1;

			if (keychar == 'Q' && !findparent_ncurses)
				exit(0);
			if (keychar == 'R')
				ticktime = 3;
		}
	}
	exit(0);
}
