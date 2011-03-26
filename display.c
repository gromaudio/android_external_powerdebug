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

#include "powerdebug.h"
#include "regulator.h"
#include "display.h"

#define print(w, x, y, fmt, args...) do { mvwprintw(w, y, x, fmt, ##args); } while (0)
#define NUM_FOOTER_ITEMS 5

static WINDOW *header_win;
static WINDOW *regulator_win;
static WINDOW *clock_win;
static WINDOW *sensor_win;
static WINDOW *selected_win;
static WINDOW *footer_win;

int maxx, maxy;
char footer_items[NUM_FOOTER_ITEMS][64];

static char *win_names[TOTAL_FEATURE_WINS] = {
	"Clocks",
	"Regulators",
	"Sensors"
};

/* "all" : Kill header and footer windows too ? */
void killall_windows(int all)
{
	if (all && header_win) {
		delwin(header_win);
		header_win = NULL;
	}
	if (regulator_win) {
		delwin(regulator_win);
		regulator_win = NULL;
	}
	if (clock_win) {
		delwin(clock_win);
		clock_win = NULL;
	}
	if (sensor_win) {
		delwin(sensor_win);
		sensor_win = NULL;
	}
	if (all && footer_win) {
		delwin(footer_win);
		footer_win = NULL;
	}
}

static void display_fini(void)
{
	endwin();
}

void display_init(void)
{
	initscr();
	start_color();
	keypad(stdscr, TRUE);
	noecho();
	cbreak();
	curs_set(0);
	nonl();
	use_default_colors();

	init_pair(PT_COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
	init_pair(PT_COLOR_ERROR, COLOR_BLACK, COLOR_RED);
	init_pair(PT_COLOR_HEADER_BAR, COLOR_WHITE, COLOR_BLACK);
	init_pair(PT_COLOR_YELLOW, COLOR_WHITE, COLOR_YELLOW);
	init_pair(PT_COLOR_GREEN, COLOR_WHITE, COLOR_GREEN);
	init_pair(PT_COLOR_BRIGHT, COLOR_WHITE, COLOR_BLACK);
	init_pair(PT_COLOR_BLUE, COLOR_WHITE, COLOR_BLUE);
	init_pair(PT_COLOR_RED, COLOR_WHITE, COLOR_RED);

	atexit(display_fini);
}

void create_windows(int selectedwindow)
{

	getmaxyx(stdscr, maxy, maxx);
	killall_windows(1);

	header_win = subwin(stdscr, 1, maxx, 0, 0);
	footer_win = subwin(stdscr, 1, maxx, maxy-1, 0);

	strcpy(footer_items[0], " Q (Quit) ");
	strcpy(footer_items[1], " R (Refresh) ");

	if (selectedwindow == CLOCK)
		strcpy(footer_items[2], " Other Keys: 'Left', 'Right', 'Up', 'Down', 'enter', "
			" '/', 'Esc' ");
	else
		strcpy(footer_items[2], " Other Keys: 'Left', 'Right' ");

	strcpy(footer_items[3], "");

	werase(stdscr);
	refresh();

}

void create_selectedwindow(int selectedwindow)
{
	WINDOW *win;

	killall_windows(0);

	getmaxyx(stdscr, maxy, maxx);

	win = subwin(stdscr, maxy - 2, maxx, 1, 0);

	switch (selectedwindow) {
	case REGULATOR: regulator_win = win;
		break;

	case CLOCK:     clock_win = win;
		break;

	case SENSOR:    sensor_win = win;
		break;
	}

	selected_win = win;

	refresh();
}

void show_header(int selectedwindow)
{
	int i, j = 0;
	int curr_pointer = 0;

	wattrset(header_win, COLOR_PAIR(PT_COLOR_HEADER_BAR));
	wbkgd(header_win, COLOR_PAIR(PT_COLOR_HEADER_BAR));
	werase(header_win);

	print(header_win, curr_pointer, 0, "PowerDebug %s", VERSION);
	curr_pointer += 20;

	for (i = 0; i < TOTAL_FEATURE_WINS; i++) {
		if (selectedwindow == i)
			wattron(header_win, A_REVERSE);
		else
			wattroff(header_win, A_REVERSE);

		print(header_win, curr_pointer, 0, " %s ", win_names[i]);
		curr_pointer += strlen(win_names[i]) + 2;
	}
	wrefresh(header_win);
	werase(footer_win);

	for (i = 0; i < NUM_FOOTER_ITEMS; i++) {
		if (strlen(footer_items[i]) == 0)
			continue;
		wattron(footer_win, A_REVERSE);
		print(footer_win, j, 0, "%s", footer_items[i]);
		wattroff(footer_win, A_REVERSE);
		j+= strlen(footer_items[i])+1;
	}
	wrefresh(footer_win);
}


void show_regulator_info(struct regulator_info *reg_info, int nr_reg, int verbose)
{
	int i, count = 1;

	(void)verbose;

	werase(regulator_win);
	wattron(regulator_win, A_BOLD);
	print(regulator_win, 0, 0, "Name");
	print(regulator_win, 12, 0, "Status");
	print(regulator_win, 24, 0, "State");
	print(regulator_win, 36, 0, "Type");
	print(regulator_win, 48, 0, "Users");
	print(regulator_win, 60, 0, "Microvolts");
	print(regulator_win, 72, 0, "Min u-volts");
	print(regulator_win, 84, 0, "Max u-volts");
	wattroff(regulator_win, A_BOLD);

	for (i = 0; i < nr_reg; i++) {
		int col = 0;

		if ((i + 2) > (maxy-2))
			break;

		if (reg_info[i].num_users > 0)
			wattron(regulator_win, WA_BOLD);
		else
			wattroff(regulator_win, WA_BOLD);

		print(regulator_win, col, count, "%s",
			reg_info[i].name);
		col += 12;
		print(regulator_win, col, count, "%s",
			reg_info[i].status);
		col += 12;
		print(regulator_win, col, count, "%s",
			reg_info[i].state);
		col += 12;
		print(regulator_win, col, count, "%s",
			reg_info[i].type);
		col += 12;
		print(regulator_win, col, count, "%d",
			reg_info[i].num_users);
		col += 12;
		print(regulator_win, col, count, "%d",
			reg_info[i].microvolts);
		col += 12;
		print(regulator_win, col, count, "%d",
			reg_info[i].min_microvolts);
		col += 12;
		print(regulator_win, col, count, "%d",
			reg_info[i].max_microvolts);

		count++;
	}
	wrefresh(regulator_win);
}


void print_clock_header(void)
{
	werase(clock_win);
	wattron(clock_win, A_BOLD);
	print(clock_win, 0, 0, "Name");
	print(clock_win, 54, 0, "Flags");
	print(clock_win, 64, 0, "Rate");
	print(clock_win, 72, 0, "Usecount");
	print(clock_win, 84, 0, "Children");
	wattroff(clock_win, A_BOLD);
	wrefresh(clock_win);
}

void print_sensor_header(void)
{
	werase(sensor_win);
	wattron(sensor_win, A_BOLD);
	print(sensor_win, 0, 0, "Name");
	print(sensor_win, 36, 0, "Temperature");
	wattroff(sensor_win, A_BOLD);
	wattron(sensor_win, A_BLINK);
	print(sensor_win, 0, 1, "Currently Sensor information available"
		" only in Dump mode!");
	wattroff(sensor_win, A_BLINK);
	wrefresh(sensor_win);
}

void print_one_clock(int line, char *str, int bold, int highlight)
{
	if (bold)
		wattron(clock_win, WA_BOLD);
	if (highlight)
		wattron(clock_win, WA_STANDOUT);

	print(clock_win, 0, line + 1, "%s", str);
	if (bold)
		wattroff(clock_win, WA_BOLD);
	if (highlight)
		wattroff(clock_win, WA_STANDOUT);
	wrefresh(clock_win);
}
