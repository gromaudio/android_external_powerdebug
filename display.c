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

#define print(w, x, y, fmt, args...) do { mvwprintw(w, y, x, fmt, ##args); } while (0)
#define NUM_FOOTER_ITEMS 5

static WINDOW *header_win;
static WINDOW *regulator_win;
static WINDOW *clock_win;
static WINDOW *sensor_win;
static WINDOW *footer_win;

int maxx, maxy;
char footer_items[NUM_FOOTER_ITEMS][64];


void fini_curses(void) {
	endwin();
}

void killall_windows(void)
{
	if (header_win) {
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
	if (footer_win) {
		delwin(footer_win);
		footer_win = NULL;
	}
}

void init_curses(void)
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
        init_pair(PT_COLOR_HEADER_BAR, COLOR_BLACK, COLOR_WHITE);
        init_pair(PT_COLOR_YELLOW, COLOR_WHITE, COLOR_YELLOW);
        init_pair(PT_COLOR_GREEN, COLOR_WHITE, COLOR_GREEN);
        init_pair(PT_COLOR_BRIGHT, COLOR_WHITE, COLOR_BLACK);
        init_pair(PT_COLOR_BLUE, COLOR_WHITE, COLOR_BLUE);
        init_pair(PT_COLOR_RED, COLOR_WHITE, COLOR_RED);

        atexit(fini_curses);
}


void create_windows(void)
{

	getmaxyx(stdscr, maxy, maxx);
	killall_windows();

	header_win = subwin(stdscr, 1, maxx, 0, 0);
//	regulator_win = subwin(stdscr, maxy/2 - 2, maxx, 1, 0);
//	clock_win = subwin(stdscr, maxy/2 - 2, maxx, maxy/2, 0);

	footer_win = subwin(stdscr, 1, maxx, maxy-1, 0);

	strcpy(footer_items[0], " Q (Quit) ");
	strcpy(footer_items[1], " R (Refresh) ");

	werase(stdscr);
	refresh();

}

int create_regulator_win(int row, int maxrows)
{
        int numrows;

        if (regulator_win) {
                delwin(regulator_win);
                regulator_win = NULL;
        }

        getmaxyx(stdscr, maxy, maxx);
        if (maxrows < (maxy/2 - 2))
                numrows = maxrows;
        else
                numrows = maxy/2 - 2;
        regulator_win = subwin(stdscr, numrows, maxx, row, 0);

        refresh();

        return numrows + row;
}

int create_clock_win(int row, int maxrows)
{
        int numrows;

        if (clock_win) {
                delwin(clock_win);
                clock_win = NULL;
        }

        getmaxyx(stdscr, maxy, maxx);
        if (maxrows < (maxy/2 - 2))
                numrows = maxrows;
        else
                numrows = maxy/2 - 2;
        clock_win = subwin(stdscr, numrows, maxx, row, 0);

        refresh();

        return numrows + row;
}

int create_sensor_win(int row, int maxrows)
{
        int numrows;

        if (sensor_win) {
                delwin(sensor_win);
                sensor_win = NULL;
        }

        getmaxyx(stdscr, maxy, maxx);
        if (maxrows < 4)
                numrows = maxrows;
        else
                numrows = 4;
        sensor_win = subwin(stdscr, numrows, maxx, row, 0);

        refresh();

        return numrows + row;
}

void show_header(void)
{
	int i, j = 0;

	wattrset(header_win, COLOR_PAIR(PT_COLOR_HEADER_BAR));
	wbkgd(header_win, COLOR_PAIR(PT_COLOR_HEADER_BAR));
	werase(header_win);

	print(header_win, 0, 0, "PowerDebug version %s         (C) Linaro",
	      VERSION);

	wrefresh(header_win);

	werase(footer_win);

	for (i=0; i<NUM_FOOTER_ITEMS; i++) {
		if (strlen(footer_items[i])==0)
			continue;
		wattron(footer_win, A_REVERSE);
		print(footer_win, j, 0, "%s", footer_items[i]);
		wattroff(footer_win, A_REVERSE);
		j+= strlen(footer_items[i])+1;
	}
	wrefresh(footer_win);
}


void show_regulator_info(int verbose)
{
	int i, count = 2;

	werase(regulator_win);
	wattron(regulator_win, A_BOLD);
	wattron(regulator_win, A_STANDOUT);
	print(regulator_win, 0, 0, "Regulator Information");
	wattroff(regulator_win, A_STANDOUT);
	print(regulator_win, 0, 1, "Name");
	print(regulator_win, 12, 1, "Status");
	print(regulator_win, 24, 1, "State");
	print(regulator_win, 36, 1, "Type");
	print(regulator_win, 48, 1, "Microvolts");
	print(regulator_win, 60, 1, "Min u-volts");
	print(regulator_win, 72, 1, "Max u-volts");
	wattroff(regulator_win, A_BOLD);

	for (i=0; i<numregulators; i++) {
		int col = 0;

		if((i + 2) > (maxy-2))
			break;

		if(!verbose && !strncmp(regulators_info[i].state, "disabled", 8))
			continue;

		print(regulator_win, col, count, "%s",
		      regulators_info[i].name);
		col += 12;
		print(regulator_win, col, count, "%s",
		      regulators_info[i].status);
		col += 12;
		print(regulator_win, col, count, "%s",
		      regulators_info[i].state);
		col += 12;
		print(regulator_win, col, count, "%s",
		      regulators_info[i].type);
		col += 12;
		print(regulator_win, col, count, "%d",
		      regulators_info[i].microvolts);
		col += 12;
		print(regulator_win, col, count, "%d",
		      regulators_info[i].min_microvolts);
		col += 12;
		print(regulator_win, col, count, "%d",
		      regulators_info[i].max_microvolts);

		count++;
	}
	wrefresh(regulator_win);
}


void print_clock_header(int level)
{
        char lev[NAME_MAX];

        sprintf(lev, "(Level %d)\n", level);
        werase(clock_win);
        wattron(clock_win, A_BOLD);
        wattron(clock_win, A_STANDOUT);
        print(clock_win, 0, 0, "Clock Information");
        wattroff(clock_win, A_STANDOUT);
        print(clock_win, 0, 1, "Name");
        print(clock_win, 24, 1, "Flags");
        print(clock_win, 36, 1, "Rate");
        print(clock_win, 48, 1, "Usecount");
        print(clock_win, 60, 1, lev);
        wattroff(clock_win, A_BOLD);
	wrefresh(clock_win);
}

void print_sensor_header(void)
{
        werase(sensor_win);
        wattron(sensor_win, A_BOLD);
        wattron(sensor_win, A_STANDOUT);
        print(sensor_win, 0, 0, "Sensor Information");
        wattroff(sensor_win, A_STANDOUT);
        print(sensor_win, 0, 1, "Name");
        print(sensor_win, 36, 1, "Temperature");
        wattroff(sensor_win, A_BOLD);
        wattron(sensor_win, A_BLINK);
        print(sensor_win, 0, 2, "Currently Sensor information available"
                                  " only in Dump mode!");
        wattroff(sensor_win, A_BLINK);
	wrefresh(sensor_win);
}

void print_clock_info_line(int line, char *clockname, int flags, int rate,
                           int usecount, int highlight)
{
        if (highlight) {
                //wattrset(clock_win, COLOR_PAIR(PT_COLOR_RED));
                //wbkgd(clock_win, COLOR_PAIR(PT_COLOR_RED));
                wattron(clock_win, WA_BOLD);
        }
        print(clock_win, 0, line + 2, "%s", clockname); 
        print(clock_win, 24, line + 2, "%d", flags); 
        print(clock_win, 36, line + 2, "%d", rate); 
        print(clock_win, 48, line + 2, "%d", usecount);
        if (highlight) {
                //use_default_colors();
                //wattrset(clock_win, COLOR_PAIR(PT_COLOR_DEFAULT));
                //wbkgd(clock_win, COLOR_PAIR(PT_COLOR_DEFAULT));
                wattroff(clock_win, WA_BOLD);
        }
        wrefresh(clock_win);
}