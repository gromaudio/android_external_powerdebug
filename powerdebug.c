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
#include <stdbool.h>
#include <math.h>
#include "regulator.h"
#include "display.h"
#include "clocks.h"
#include "sensor.h"
#include "powerdebug.h"

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
	bool verbose;
	bool regulators;
	bool sensors;
	bool clocks;
	bool dump;
	unsigned int ticktime;
	int selectedwindow;
	char *clkname;
};

int getoptions(int argc, char *argv[], struct powerdebug_options *options)
{
	int c;

	memset(options, 0, sizeof(*options));
	options->ticktime = 10;
	options->selectedwindow = -1;

	while (1) {
		int optindex = 0;

		c = getopt_long(argc, argv, "rscp:t:dvVh",
				long_options, &optindex);
		if (c == -1)
			break;

		switch (c) {
		case 'r':
			options->regulators = true;
			options->selectedwindow = REGULATOR;
			break;
		case 's':
			options->sensors = true;
			options->selectedwindow = SENSOR;
			break;
		case 'c':
			options->clocks = true;
			options->selectedwindow = CLOCK;
			break;
		case 'p':
			options->clkname = strdup(optarg);
			if (!options->clkname) {
				fprintf(stderr, "failed to allocate memory");
				return -1;
			}
			options->dump = true;   /* Assume -dc in case of -p */
			options->clocks = true;
			break;
		case 't':
			options->ticktime = atoi(optarg);
			break;
		case 'd':
			options->dump = true;
			break;
		case 'v':
			options->verbose = true;
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

	/* No system specified to be dump, let's default to all */
	if (!options->regulators && !options->clocks && !options->sensors)
		options->regulators = options->clocks = options->sensors = true;

	if (options->selectedwindow == -1)
		options->selectedwindow = CLOCK;

	return 0;
}

int keystroke_callback(bool *enter_hit, bool *findparent_ncurses,
		       char *clkname_str, bool *refreshwin, bool *cont,
		       struct powerdebug_options *options)
{
	char keychar;
	int keystroke = getch();
	int oldselectedwin = options->selectedwindow;

	if (keystroke == EOF)
		exit(0);

	if (keystroke == KEY_RIGHT || keystroke == '\t')
		 options->selectedwindow = display_next_panel();

	if (keystroke == KEY_LEFT || keystroke == KEY_BTAB)
		 options->selectedwindow = display_prev_panel();

	if (keystroke == KEY_DOWN) {
		display_next_line();
		*cont = true;
	}

	if (keystroke == KEY_UP) {
		display_prev_line();
		*cont = true;
	}

	if (options->selectedwindow == CLOCK) {

#if 0
		/* TODO : fix with a new panel applicable for all subsystems */
		if (keystroke == '/')
			*findparent_ncurses = true;
#endif

		if ((keystroke == '\e' || oldselectedwin !=
		     options->selectedwindow) && *findparent_ncurses) {
			*findparent_ncurses = false;
			clkname_str[0] = '\0';
		}

		if (*findparent_ncurses && keystroke != '\r') {
			int len = strlen(clkname_str);
			char str[2];

			if (keystroke == KEY_BACKSPACE) {
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
	fini_curses();
	printf("key entered %d:%c\n", keystroke, keychar);
	exit(1);
#endif

	if (keystroke == '\r')
		*enter_hit = true;

	if (keychar == 'Q' && !*findparent_ncurses)
		return 1;
	if (keychar == 'R') {
		*refreshwin = true;
		options->ticktime = 3;
	} else
		*refreshwin = false;

	return 0;
}

int mainloop(struct powerdebug_options *options)
{
	bool findparent_ncurses = false;
	bool refreshwin = false;
	bool enter_hit = false;
	bool cont = false;
	char clkname_str[64];

	strcpy(clkname_str, "");

	while (1) {
		int key = 0;
		struct timeval tval;
		fd_set readfds;

		if (options->selectedwindow == REGULATOR)
			regulator_display();

		if (options->selectedwindow == SENSOR)
			sensor_display();

		if (options->selectedwindow == CLOCK) {

			if (!cont) {

				if (!findparent_ncurses) {

					if (enter_hit)
						clock_toggle_expanded();

					clock_display();
					enter_hit = false;
				} else
					find_parents_for_clock(clkname_str,
							       enter_hit);
			} else cont = false;
		}

		FD_ZERO(&readfds);
		FD_SET(0, &readfds);
		tval.tv_sec = options->ticktime;
		tval.tv_usec = (options->ticktime - tval.tv_sec) * 1000000;

	again:
		key = select(1, &readfds, NULL, NULL, &tval);
		if (!key)
			continue;

		if (key < 0) {
			if (errno == EINTR)
				goto again;
			break;
		}

		if (keystroke_callback(&enter_hit, &findparent_ncurses,
				       clkname_str, &refreshwin, &cont, options))
			break;

	}

	return 0;
}

static int powerdebug_dump(struct powerdebug_options *options)
{
	if (options->regulators)
		regulator_dump();

	if (options->clocks)
		clock_dump(options->clkname);

	if (options->sensors)
		sensor_dump();

	return 0;
}

static int powerdebug_display(struct powerdebug_options *options)
{
	if (display_init(options->selectedwindow)) {
		printf("failed to initialize display\n");
		return -1;
	}

	if (mainloop(options))
		return -1;

	return 0;
}

static struct powerdebug_options *powerdebug_init(void)
{
	struct powerdebug_options *options;

	options = malloc(sizeof(*options));
	if (!options)
		return NULL;

	memset(options, 0, sizeof(*options));

	return options;
}

int main(int argc, char **argv)
{
	struct powerdebug_options *options;
	int ret;

	options = powerdebug_init();
	if (!options) {
		fprintf(stderr, "not enough memory to allocate options\n");
		return 1;
	}

	if (getoptions(argc, argv, options)) {
		usage();
		return 1;
	}

	if (regulator_init()) {
		printf("not enough memory to allocate regulators info\n");
		options->regulators = false;
	}

	if (clock_init()) {
		printf("failed to initialize clock details (check debugfs)\n");
		options->clocks = false;
	}

	if (sensor_init()) {
		printf("failed to initialize sensors\n");
		options->sensors = false;
	}

	ret = options->dump ? powerdebug_dump(options) :
		powerdebug_display(options);

	return ret < 0;
}
