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
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "powerdebug.h"
#include "regulator.h"
#include "display.h"

#define print(w, x, y, fmt, args...) do { mvwprintw(w, y, x, fmt, ##args); } while (0)

enum { PT_COLOR_DEFAULT = 1,
       PT_COLOR_HEADER_BAR,
       PT_COLOR_ERROR,
       PT_COLOR_RED,
       PT_COLOR_YELLOW,
       PT_COLOR_GREEN,
       PT_COLOR_BRIGHT,
       PT_COLOR_BLUE,
};

static WINDOW *header_win;
static WINDOW *footer_win;
static int current_win;

int maxx, maxy;

/* Number of lines in the virtual window */
static const int maxrows = 1024;

#define footer_label " Q (Quit)  R (Refresh) Other Keys: 'Left', " \
	"'Right' , 'Up', 'Down', 'enter', , 'Esc'"

struct rowdata {
	int attr;
	void *data;
};

struct windata {
	WINDOW *win;
	WINDOW *pad;
	struct display_ops *ops;
	struct rowdata *rowdata;
	char *name;
	int nrdata;
	int scrolling;
	int cursor;
};

struct windata windata[TOTAL_FEATURE_WINS] = {
	{ .name = "Clocks"     },
	{ .name = "Regulators" },
	{ .name = "Sensors"    },
};

static void display_fini(void)
{
	endwin();
}

static int show_header_footer(int win)
{
	int i;
	int curr_pointer = 0;

	wattrset(header_win, COLOR_PAIR(PT_COLOR_HEADER_BAR));
	wbkgd(header_win, COLOR_PAIR(PT_COLOR_HEADER_BAR));
	werase(header_win);

	print(header_win, curr_pointer, 0, "PowerDebug %s", VERSION);
	curr_pointer += 20;

	for (i = 0; i < TOTAL_FEATURE_WINS; i++) {
		if (win == i)
			wattron(header_win, A_REVERSE);
		else
			wattroff(header_win, A_REVERSE);

		print(header_win, curr_pointer, 0, " %s ", windata[i].name);
		curr_pointer += strlen(windata[i].name) + 2;
	}
	wrefresh(header_win);
	werase(footer_win);

	wattron(footer_win, A_REVERSE);
	print(footer_win, 0, 0, "%s", footer_label);
	wattroff(footer_win, A_REVERSE);
	wrefresh(footer_win);

	return 0;
}

int display_init(int wdefault)
{
	int i;
	size_t array_size = sizeof(windata) / sizeof(windata[0]);

	current_win = wdefault;

	if (!initscr())
		return -1;

	start_color();
	use_default_colors();

	keypad(stdscr, TRUE);
	noecho();
	cbreak();
	curs_set(0);
	nonl();

	if (init_pair(PT_COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK) ||
	    init_pair(PT_COLOR_ERROR, COLOR_BLACK, COLOR_RED) ||
	    init_pair(PT_COLOR_HEADER_BAR, COLOR_WHITE, COLOR_BLACK) ||
	    init_pair(PT_COLOR_YELLOW, COLOR_WHITE, COLOR_YELLOW) ||
	    init_pair(PT_COLOR_GREEN, COLOR_WHITE, COLOR_GREEN) ||
	    init_pair(PT_COLOR_BRIGHT, COLOR_WHITE, COLOR_BLACK) ||
	    init_pair(PT_COLOR_BLUE, COLOR_WHITE, COLOR_BLUE) ||
	    init_pair(PT_COLOR_RED, COLOR_WHITE, COLOR_RED))
		return -1;

	if (atexit(display_fini))
		return -1;

	getmaxyx(stdscr, maxy, maxx);

	for (i = 0; i < array_size; i++) {

		windata[i].win = subwin(stdscr, maxy - 2, maxx, 1, 0);
		if (!windata[i].win)
			return -1;

		windata[i].pad = newpad(maxrows, maxx);
		if (!windata[i].pad)
			return -1;

	}

	header_win = subwin(stdscr, 1, maxx, 0, 0);
	if (!header_win)
		return -1;

	footer_win = subwin(stdscr, 1, maxx, maxy-1, 0);
	if (!footer_win)
		return -1;

	return show_header_footer(wdefault);
}

void print_regulator_header(void)
{
	WINDOW *regulator_win = windata[REGULATOR].win;

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
	wrefresh(regulator_win);

	show_header_footer(REGULATOR);
}

void print_clock_header(void)
{
	WINDOW *clock_win = windata[CLOCK].win;

	werase(clock_win);
	wattron(clock_win, A_BOLD);
	print(clock_win, 0, 0, "Name");
	print(clock_win, 56, 0, "Flags");
	print(clock_win, 75, 0, "Rate");
	print(clock_win, 88, 0, "Usecount");
	print(clock_win, 98, 0, "Children");
	wattroff(clock_win, A_BOLD);
	wrefresh(clock_win);

	show_header_footer(CLOCK);
}

void print_sensor_header(void)
{
	WINDOW *sensor_win = windata[SENSOR].win;

	werase(sensor_win);
	wattron(sensor_win, A_BOLD);
	print(sensor_win, 0, 0, "Name");
	print(sensor_win, 36, 0, "Value");
	wattroff(sensor_win, A_BOLD);
	wrefresh(sensor_win);

	show_header_footer(SENSOR);
}

int display_register(int win, struct display_ops *ops)
{
	if (win < 0 || win >= TOTAL_FEATURE_WINS)
		return -1;

	windata[win].ops = ops;

	return 0;
}

int display_refresh(void)
{
	if (windata[current_win].ops && windata[current_win].ops->display)
		return windata[current_win].ops->display();

	return 0;
}

int display_select(void)
{
	if (windata[current_win].ops && windata[current_win].ops->select)
		return windata[current_win].ops->select();

	return 0;
}

int display_next_panel(void)
{
	current_win++;
	current_win %= TOTAL_FEATURE_WINS;

	return current_win;
}

int display_prev_panel(void)
{
	current_win--;
	if (current_win < 0)
		current_win = TOTAL_FEATURE_WINS - 1;

	return current_win;
}

int display_refresh_pad(int win)
{
	return prefresh(windata[win].pad, windata[win].scrolling,
			0, 2, 0, maxy - 2, maxx);
}

static int inline display_show_un_selection(int win, int line,
					    bool highlight, bool bold)
{
	if (mvwchgat(windata[win].pad, line, 0, -1,
		     highlight ? WA_STANDOUT :
		     bold ? WA_BOLD: WA_NORMAL, 0, NULL) < 0)
		return -1;

	return display_refresh_pad(win);
}

int display_show_selection(int win, int line)
{
	return display_show_un_selection(win, line, true, false);
}

int display_show_unselection(int win, int line, bool bold)
{
	return display_show_un_selection(win, line, false, bold);
}

void *display_get_row_data(int win)
{
	return windata[win].rowdata[windata[win].cursor].data;
}

int display_set_row_data(int win, int line, void *data, int attr)
{
	struct rowdata *rowdata =  windata[win].rowdata;

	if (line >= windata[win].nrdata) {
		rowdata = realloc(rowdata, sizeof(struct rowdata) * (line + 1));
		if (!rowdata)
			return -1;
		windata[win].nrdata = line + 1;
	}

	rowdata[line].data = data;
	rowdata[line].attr = attr;
	windata[win].rowdata = rowdata;

	return 0;
}

int display_reset_cursor(int win)
{
	windata[win].nrdata = 0;
	werase(windata[win].pad);
	return wmove(windata[win].pad, 0, 0);
}

int display_print_line(int win, int line, char *str, int bold, void *data)
{
	int attr = 0;

	if (bold)
		attr |= WA_BOLD;

	if (line == windata[win].cursor)
		attr |= WA_STANDOUT;

	if (display_set_row_data(win, line, data, attr))
		return -1;

	if (attr)
		wattron(windata[win].pad, attr);

	wprintw(windata[win].pad, "%s\n", str);

	if (attr)
		wattroff(windata[win].pad, attr);

	return 0;
}

int display_next_line(void)
{
	int cursor = windata[current_win].cursor;
	int nrdata = windata[current_win].nrdata;
	int scrolling = windata[current_win].scrolling;
	struct rowdata *rowdata = windata[current_win].rowdata;

	if (cursor >= nrdata)
		return cursor;

	display_show_unselection(current_win, cursor, rowdata[cursor].attr);
	if (cursor < nrdata - 1) {
		if (cursor >= (maxy - 4 + scrolling))
			scrolling++;
		cursor++;
	}
	display_show_selection(current_win, cursor);

	windata[current_win].scrolling = scrolling;
	windata[current_win].cursor = cursor;

	return cursor;
}

int display_prev_line(void)
{
	int cursor = windata[current_win].cursor;
	int nrdata = windata[current_win].nrdata;
	int scrolling = windata[current_win].scrolling;
	struct rowdata *rowdata = windata[current_win].rowdata;

	if (cursor >= nrdata)
		return cursor;

	display_show_unselection(current_win, cursor, rowdata[cursor].attr);
	if (cursor > 0) {
		if (cursor <= scrolling)
			scrolling--;
		cursor--;
	}
	display_show_selection(current_win, cursor);

	windata[current_win].scrolling = scrolling;
	windata[current_win].cursor = cursor;

	return cursor;
}
