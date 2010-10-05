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
#include <errno.h>

static int  clk_tree_level = 1;
static char clk_dir_path[PATH_MAX];
static char highlighted_path[PATH_MAX];
static char clk_name[NAME_MAX];
static int  gadder = 0;


void init_clock_details(void)
{
        strcpy(clk_dir_path, "/debug/clock");
        strcpy(clk_name, "");
        strcpy(highlighted_path, "");
}

int get_int_from(char *file)
{
        FILE *filep;
        char result[NAME_MAX];
        int ret;

        filep = fopen(file, "r");

        if (!filep)
                return -1;  //TBD : What should we return on failure, here ?

        ret = fscanf(filep, "%s", result);
        fclose(filep);

        return atoi(result);
}

int read_and_print_clock_info(int verbose, int hrow, int selected)
{
        if (selected) {
        //        if (!(strcmp(clk_dir_path, "/debug/clock") &&
         //               strcmp(highlighted_path, ".."))) {
                strcpy(clk_dir_path, highlighted_path);
                hrow = 0;
                clk_tree_level += gadder;
                if (clk_tree_level <=0)
                        clk_tree_level = 1;
        }

        hrow = read_and_print_clock_one_level(verbose, hrow, selected);

        return hrow;
}

void set_hl_dir_path_to_parent(void)
{
        char *ptr;

        strcpy(highlighted_path, clk_dir_path);
        if (strcmp(clk_dir_path, "/clock/debug")) {
                ptr = strrchr(highlighted_path, '/');
                if (ptr)
                        ptr[0]='\0';
        }
}

int read_and_print_clock_one_level(int verbose, int hrow, int selected)
{
        int line = 0, usecount = 0, flags = 0, rate = 0;
//      int parent_dir_row = 1000;
        DIR *dir, *subdir;
        char filename[PATH_MAX], devpath[PATH_MAX], clockname[NAME_MAX];
        struct dirent *item, *subitem;

        (void)verbose;

        print_clock_header(clk_tree_level);

        sprintf(filename, "%s", clk_dir_path);

        dir = opendir(filename);
        if (!dir)
                return 0;

        while ((item = readdir(dir))) {
                /* skip hidden dirs except ".." */
                if (item->d_name[0] == '.' && strcmp(item->d_name, ".."))
                        continue;

                if (selected && hrow == line && !strcmp(item->d_name, "..")) {
                        sprintf(devpath, "%s", clk_dir_path);
                        strcpy(clockname, "..");
                } else {
                        sprintf(devpath, "%s/%s", clk_dir_path, item->d_name);
                        strcpy(clockname, item->d_name);
                }

                subdir = opendir(devpath);

                if (!subdir)
                        continue;

                while ((subitem = readdir(subdir))) {
                        if (subitem->d_name[0] == '.') /* skip hidden files */
                                continue;

                        sprintf(filename, "%s/%s", devpath, subitem->d_name);

                        if (!strcmp(subitem->d_name, "flags"))
                                flags = get_int_from(filename);

                        if (!strcmp(subitem->d_name, "rate"))
                                rate = get_int_from(filename);
        
                        if (!strcmp(subitem->d_name, "usecount"))
                                usecount = get_int_from(filename);
                }

                if (hrow == line) {
                        if (!strcmp(clockname, "..")) {
                                if (clk_tree_level != 1) {
                                        set_hl_dir_path_to_parent();
                                        gadder = -1;
                                } 
                        } else {
                                strcpy(highlighted_path, devpath);
                                gadder = 1;
                        }
                }

//                sprintf(clockname, "%s:dp-%s:n-%s:hp-%s",
//                        clockname, clk_dir_path, clk_name, highlighted_path);

/*                if (strcmp(clockname, "..")) {
                        int row = line + 1;

                        if (line > parent_dir_row)
                                row--;
                        print_clock_info_line(row, clockname, flags, rate, usecount,
                                              (hrow == line) ? 1 : 0);
                } else {
                        print_clock_info_line(0, clockname, flags, rate, usecount,
                                              (hrow == line) ? 1 : 0);
                        parent_dir_row = line;
                }
*/

                print_clock_info_line(line, clockname, flags, rate, usecount,
                                      (hrow == line) ? 1 : 0);
                line++;

                closedir(subdir);
        }

        closedir(dir);

        if (hrow >= (line - 1))
                hrow = -1;
        return hrow;
}

void dump_clock_info(int verbose)
{
        printf("Clock Tree :\n");
        printf("**********\n");
        printf("/\n");
        dump_clock_info_recur(verbose, clk_dir_path);
}

void dump_clock_info_recur(int verbose, char *clkdirpath)
{
        int usecount = 0, flags = 0, rate = 0;
        DIR *dir, *subdir;
        char filename[PATH_MAX], devpath[PATH_MAX];
        struct dirent *item, *subitem;
        char *clock, *clockp;

        sprintf(filename, "%s", clkdirpath);

        dir = opendir(filename);
        if (!dir)
                return;

        while ((item = readdir(dir))) {
                int cnt = 0;

                /* skip hidden dirs except ".." */
                if (item->d_name[0] == '.' )
                        continue;

                sprintf(devpath, "%s/%s", clkdirpath, item->d_name);

                subdir = opendir(devpath);

                if (!subdir)
                        continue;

                while ((subitem = readdir(subdir))) {
                        if (subitem->d_name[0] == '.') /* skip hidden
files */
                                continue;

                        sprintf(filename, "%s/%s", devpath, subitem->d_name);

                        if (!strcmp(subitem->d_name, "flags"))
                                flags = get_int_from(filename);

                        if (!strcmp(subitem->d_name, "rate"))
                                rate = get_int_from(filename);

                        if (!strcmp(subitem->d_name, "usecount"))
                                usecount = get_int_from(filename);
                }

                if (!usecount && !verbose)
                        continue;

                
                clockp = strrchr(devpath, '/');
                if (clockp)
                        clockp++;
                else
                        continue;

                clock = strchr(devpath, '/');
                if (clock) {
                        clock++;
                        clock = strchr(clock, '/');
                        if (clock)
                                clock++;
                }

                while (clock) {
                        clock = strchr(clock, '/');
                        if (clock)
                                clock++;
                        else
                                break;
                        cnt ++;
                }

                printf("|");
                if (cnt == 1) {
                        cnt --;
                        printf("-- ");
                } else
                        printf("   ");


                while (cnt) {
                        if (cnt == 2)
                                printf("|-");
                        else if (cnt == 1)
                                printf("- ");
                        else
                                printf("|   ");
                        cnt --;
                }

                printf("%s <flags=0x%x:rate=%d:usecount=%d>\n",
                        clockp, flags, rate, usecount);
                dump_clock_info_recur(verbose, devpath);
        }
}
