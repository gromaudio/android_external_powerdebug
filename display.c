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
static WINDOW *regulator_win;
static WINDOW *clock_pad;
static WINDOW *clock_labels;
static WINDOW *sensor_win;
static WINDOW *footer_win;

int maxx, maxy;

/* Number of lines in the virtual window */
static const int maxrows = 1024;

static char footer_items[NUM_FOOTER_ITEMS][64];

struct rowdata {
	int attr;
	void *data;
};

struct windata {
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

int display_init(void)
{
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

	regulator_win = subwin(stdscr, maxy - 2, maxx, 1, 0);
	if (!regulator_win)
		return -1;

	clock_labels = subwin(stdscr, maxy - 2, maxx, 1, 0);
	if (!clock_labels)
		return -1;

	clock_pad = newpad(maxrows, maxx);
	if (!clock_pad)
		return -1;

	sensor_win = subwin(stdscr, maxy - 2, maxx, 1, 0);
	if (!sensor_win)
		return -1;

	header_win = subwin(stdscr, 1, maxx, 0, 0);
	if (!header_win)
		return -1;

	footer_win = subwin(stdscr, 1, maxx, maxy-1, 0);
	if (!footer_win)
		return -1;

	return 0;
}

void create_windows(int selectedwindow)
{
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
	switch (selectedwindow) {
	case REGULATOR:
		wrefresh(regulator_win);
		break;

	case SENSOR:
		wrefresh(sensor_win);
		break;
	}
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

		print(header_win, curr_pointer, 0, " %s ", windata[i].name);
		curr_pointer += strlen(windata[i].name) + 2;
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

void print_regulator_header(void)
{
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
}

void print_clock_header(void)
{
	werase(clock_labels);
	wattron(clock_labels, A_BOLD);
	print(clock_labels, 0, 0, "Name");
	print(clock_labels, 56, 0, "Flags");
	print(clock_labels, 75, 0, "Rate");
	print(clock_labels, 88, 0, "Usecount");
	print(clock_labels, 98, 0, "Children");
	wattroff(clock_labels, A_BOLD);
	wrefresh(clock_labels);
}

#if 0
void show_regulator_info(struct regulator_info *reg_info, int nr_reg, int verbose)
{
	int i, count = 1;

	print_regulator_header();

	wrefresh(regulator_win);

	return;

	(void)verbose;

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
#endif

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

int display_refresh_pad(int win)
{
	return prefresh(clock_pad, windata[win].scrolling,
			0, 2, 0, maxy - 2, maxx);
}

static int inline display_clock_un_select(int win, int line,
					  bool highlight, bool bold)
{
	if (mvwchgat(clock_pad, line, 0, -1,
		     highlight ? WA_STANDOUT :
		     bold ? WA_BOLD: WA_NORMAL, 0, NULL) < 0)
		return -1;

	return display_refresh_pad(win);
}

int display_select(int win, int line)
{
	return display_clock_un_select(win, line, true, false);
}

int display_unselect(int win, int line, bool bold)
{
	return display_clock_un_select(win, line, false, bold);
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

int display_reset_cursor(win)
{
	windata[win].nrdata = 0;
	werase(clock_pad);
	return wmove(clock_pad, 0, 0);
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
		wattron(clock_pad, attr);

	wprintw(clock_pad, "%s\n", str);

	if (attr)
		wattroff(clock_pad, attr);

	return 0;
}

int display_next_line(int win)
{
	int cursor = windata[win].cursor;
	int nrdata = windata[win].nrdata;
	int scrolling = windata[win].scrolling;
	struct rowdata *rowdata = windata[win].rowdata;

	if (cursor >= nrdata)
		return cursor;

	display_unselect(win, cursor, rowdata[cursor].attr);
	if (cursor < nrdata - 1) {
		if (cursor >= (maxy - 4 + scrolling))
			scrolling++;
		cursor++;
	}
	display_select(win, cursor);

	windata[win].scrolling = scrolling;
	windata[win].cursor = cursor;

	return cursor;
}

int display_prev_line(int win)
{
	int cursor = windata[win].cursor;
	int nrdata = windata[win].nrdata;
	int scrolling = windata[win].scrolling;
	struct rowdata *rowdata = windata[win].rowdata;

	if (cursor >= nrdata)
		return cursor;

	display_unselect(win, cursor, rowdata[cursor].attr);
	if (cursor > 0) {
		if (cursor <= scrolling)
			scrolling--;
		cursor--;
	}
	display_select(win, cursor);

	windata[win].scrolling = scrolling;
	windata[win].cursor = cursor;

	return cursor;
}
