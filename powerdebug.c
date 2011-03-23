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

#include <getopt.h>
#include "powerdebug.h"

int dump;
int highlighted_row;
int selectedwindow = -1;

char *win_names[TOTAL_FEATURE_WINS] = {
	"Clocks",
	"Regulators",
	"Sensors"
};

void usage(void)
{
	printf("Usage: powerdebug [OPTIONS]\n");
	printf("\n");
	printf("powerdebug -d [ -r ] [ -s ] [ -c [ -p <clock-name> ] ] "
		"[ -v ]\n");
	printf("powerdebug [ -r | -s | -c ]\n");
	printf("  -r, --regulator 	Show regulator information\n");
	printf("  -s, --sensor		Show sensor information\n");
	printf("  -c, --clock		Show clock information\n");
	printf("  -p, --findparents	Show all parents for a particular"
		" clock\n");
	printf("  -t, --time		Set ticktime in seconds (eg. 10.0)\n");
	printf("  -d, --dump		Dump information once (no refresh)\n");
	printf("  -v, --verbose		Verbose mode (use with -r and/or"
		" -s)\n");
	printf("  -V, --version		Show Version\n");
	printf("  -h, --help 		Help\n");
}

void version()
{
	printf("powerdebug version %s\n", VERSION);
}

/*
 * Options:
 * -r, --regulator      : regulator
 * -s, --sensor	 	: sensors
 * -c, --clock	  	: clocks
 * -p, --findparents    : clockname whose parents have to be found
 * -t, --time		: ticktime
 * -d, --dump		: dump
 * -v, --verbose	: verbose
 * -V, --version	: version
 * -h, --help		: help
 * no option / default : show usage!
 */

static struct option long_options[] = {
	{ "regulator", 0, 0, 'r' },
	{ "sensor", 0, 0, 's' },
	{ "clock",  0, 0, 'c' },
	{ "findparents", 1, 0, 'p' },
	{ "time", 1, 0, 't' },
	{ "dump", 0, 0, 'd' },
	{ "verbose", 0, 0, 'v' },
	{ "version", 0, 0, 'V' },
	{ "help", 0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

struct powerdebug_options {
	int findparent;
	int verbose;
	int regulators;
	int sensors;
	int clocks;
	int ticktime;
	char *clkarg;
};

int getoptions(int argc, char *argv[], struct powerdebug_options *options)
{
	int c;

	memset(options, 0, sizeof(*options));
	options->ticktime = 10;

	while (1) {
		int optindex = 0;

		c = getopt_long(argc, argv, "rscp:t:dvVh",
				long_options, &optindex);
		if (c == -1)
			break;

		switch (c) {
		case 'r':
			options->regulators = 1;
			selectedwindow = REGULATOR;
			break;
		case 's':
			options->sensors = 1;
			selectedwindow = SENSOR;
			break;
		case 'c':
			options->clocks = 1;
			selectedwindow = CLOCK;
			break;
		case 'p':
			options->findparent = 1;
			options->clkarg = strdup(optarg);
			if (!options->clkarg) {
				fprintf(stderr, "failed to allocate memory");
				return -1;
			}
			break;
		case 't':
			options->ticktime = atoi(optarg);
			break;
		case 'd':
			dump = 1;
			break;
		case 'v':
			options->verbose = 1;
			break;
		case 'V':
			version();
			break;
		case '?':
			fprintf(stderr, "%s: Unknown option %c'.\n",
				argv[0], optopt);
		default:
			return -1;
		}
	}

	if (dump && !(options->regulators ||
		      options->clocks || options->sensors)) {
		/* By Default lets show everything we have */
		options->regulators = options->clocks = options->sensors = 1;
	}

	if (options->findparent && (!options->clocks || !dump)) {
		fprintf(stderr, "-p option passed without -c and -d."
			" Exiting...\n");
		return -1;
	}

	if (!dump && selectedwindow == -1)
		selectedwindow = CLOCK;

	return 0;
}

int keystroke_callback(int *enter_hit, int *findparent_ncurses,
		       char *clkname_str, int *refreshwin,
		       struct powerdebug_options *options)
{
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
			*findparent_ncurses = 1;

		if ((keystroke == 27 || oldselectedwin !=
		     selectedwindow) && *findparent_ncurses) {
			*findparent_ncurses = 0;
			clkname_str[0] = '\0';
		}

		if (*findparent_ncurses && keystroke != 13) {
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
		*enter_hit = 1;

	if (keychar == 'Q' && !*findparent_ncurses)
		return 1;
	if (keychar == 'R') {
		*refreshwin = 1;
		options->ticktime = 3;
	} else
		*refreshwin = 0;

	return 0;
}

int mainloop(struct powerdebug_options *options)
{
	int findparent_ncurses = 0, refreshwin = 0;
	int enter_hit = 0;
	int firsttime[TOTAL_FEATURE_WINS];
	int i;
	char clkname_str[64];

	for (i = 0; i < TOTAL_FEATURE_WINS; i++)
		firsttime[i] = 1;

	while (1) {
		int key = 0;
		struct timeval tval;
		fd_set readfds;

		if (!dump) {
			if (firsttime[0])
				init_curses();
			create_windows();
			show_header();
		}

		if (options->regulators || selectedwindow == REGULATOR) {
			read_regulator_info();
			if (!dump) {
				create_selectedwindow();
				show_regulator_info(options->verbose);
			}
			else
				print_regulator_info(options->verbose);
		}

		if (options->clocks || selectedwindow == CLOCK) {
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
					int command = 0;

					if (enter_hit)
						command = CLOCK_SELECTED;
					if (refreshwin)
						command = REFRESH_WINDOW;
					hrow = read_and_print_clock_info(
						options->verbose,
						highlighted_row,
						command);
					highlighted_row = hrow;
					enter_hit = 0;
				} else
					find_parents_for_clock(clkname_str,
							enter_hit);
			}
			if (!ret && dump) {
				if (options->findparent)
					read_and_dump_clock_info_one(options->clkarg);
				else
					read_and_dump_clock_info(options->verbose);
			}
		}

		if (options->sensors || selectedwindow == SENSOR) {
			if (!dump) {
				create_selectedwindow();
				print_sensor_header();
			} else
				read_and_print_sensor_info(options->verbose);
		}

		if (dump)
			break;

		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		tval.tv_sec = options->ticktime;
		tval.tv_usec = (options->ticktime - tval.tv_sec) * 1000000;

		key = select(1, &readfds, NULL, NULL, &tval);
		if (!key)
			continue;

		if (keystroke_callback(&enter_hit, &findparent_ncurses,
				       clkname_str, &refreshwin, options))
			break;

	}

	return 0;
}

int main(int argc, char **argv)
{
	struct powerdebug_options *options;

	options = malloc(sizeof(*options));
	if (!options) {
		fprintf(stderr, "failed to allocated memory\n");
		return -1;
	}

	if (getoptions(argc, argv, options)) {
		usage();
		return 1;
	}

	if (init_regulator_ds())
		return 1;

	if (mainloop(options))
		return 1;

	return 0;
}
