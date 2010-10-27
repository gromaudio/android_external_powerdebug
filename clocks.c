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
#include "clocks.h"
#include <errno.h>
#include <sys/stat.h>

static int  clk_tree_level = 1;
static char clk_dir_path[PATH_MAX];
static char highlighted_path[PATH_MAX];
static char clk_name[NAME_MAX];
static int  gadder = 0;


void init_clock_details(void)
{
        char *path = debugfs_locate_mpoint();
        struct stat buf;


        if (path)
                strcpy(clk_dir_path, path);
        else {
                fprintf(stderr, "powerdebug: Unable to locate debugfs mount"
                        " point. Mount debugfs and try again..\n");
                exit(1);
        }
        sprintf(clk_dir_path, "%s/clock", clk_dir_path);
        //strcpy(clk_dir_path, "/debug/clock"); // Hardcoded for testing..
        if (stat(clk_dir_path, &buf)) {
                fprintf(stderr, "powerdebug: Unable to find clock tree"
                        " information at %s. Exiting..\n", clk_dir_path);
                exit(1);
        }
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
        (void)verbose;
        printf("Clock Tree :\n");
        printf("**********\n");
        read_clock_info(clk_dir_path);
        print_clock_info(clocks_info, 1, 1);
}

void read_clock_info(char *clkpath)
{
        DIR *dir;
        struct dirent *item;
        char filename[NAME_MAX], clockname[NAME_MAX];
        struct clock_info *child;
        struct clock_info *cur;

        dir = opendir(clkpath);
        if (!dir)
                return;

        clocks_info = (struct clock_info *)malloc(sizeof(struct clock_info));
        memset(clocks_info, 0, sizeof(clocks_info));
        strcpy(clocks_info->name, "/");

        while ((item = readdir(dir))) {
                /* skip hidden dirs except ".." */
                if (item->d_name[0] == '.')
                        continue;

                strcpy(clockname, item->d_name);
                sprintf(filename, "%s/%s", clkpath, item->d_name);
                cur = (struct clock_info *)malloc(sizeof(struct clock_info));
                memset(cur, 0, sizeof(cur));
                strcpy(cur->name, clockname);
                cur->parent = clocks_info;
                insert_children(&clocks_info, cur);
                child = read_clock_info_recur(filename, 2, cur);
        }
        closedir(dir);
}

struct clock_info *read_clock_info_recur(char *clkpath, int level,
                        struct clock_info *parent)
{
        int ret = 0;
        DIR *dir;
        char filename[PATH_MAX];
        struct dirent *item;
        struct clock_info *cur = NULL;
        struct stat buf;

        dir = opendir(clkpath);
        if (!dir)
                return NULL;

        while ((item = readdir(dir))) {
                struct clock_info *child;
                /* skip hidden dirs except ".." */
                if (item->d_name[0] == '.' )
                        continue;

                sprintf(filename, "%s/%s", clkpath, item->d_name);

                ret = stat(filename, &buf);

                if (ret < 0) {
                        printf("Error doing a stat on %s\n", filename);
                        exit(1);
                }

                if (S_ISREG(buf.st_mode))
                {
                        if (!strcmp(item->d_name, "flags"))
                                parent->flags = get_int_from(filename);
                        if (!strcmp(item->d_name, "rate"))
                                parent->rate = get_int_from(filename);
                        if (!strcmp(item->d_name, "usecount"))
                                parent->usecount = get_int_from(filename);
                        continue;
                }

                if (!S_ISDIR(buf.st_mode))
                        continue;

                cur = (struct clock_info *)malloc(sizeof(struct clock_info));
                memset(cur, 0, sizeof(cur));
                strcpy(cur->name, item->d_name);
                cur->children = NULL;
                cur->parent = NULL;
                cur->num_children = 0;
                child = read_clock_info_recur(filename, level + 1, cur);

                insert_children(&parent, cur);
                cur->parent = parent;
        }
        closedir(dir);

        return cur;
}

void insert_children(struct clock_info **parent, struct clock_info *clk)
{
        if (!(*parent)->children) {
                (*parent)->children = (struct clock_info **)
                                      malloc(sizeof(struct clock_info *)*2);
                (*parent)->num_children = 0;
        } else
                (*parent)->children = (struct clock_info **)
                                      realloc((*parent)->children,
                                      sizeof(struct clock_info *) *
                                      ((*parent)->num_children + 2));
        if ((*parent)->num_children > 0)
                (*parent)->children[(*parent)->num_children - 1]->last_child = 0;
        clk->last_child = 1;
        (*parent)->children[(*parent)->num_children] = clk;
        (*parent)->children[(*parent)->num_children + 1] = NULL;
        (*parent)->num_children++;
}


void print_clock_info(struct clock_info *clk, int level, int bmp)
{
        int i, j;

        if (!clk)
                return;

        for (i = 1, j = 0; i < level; i++, j = (i - 1)) {
                if (i == (level - 1)) {
                        if (clk->last_child)
                                printf("`-- ");
                        else
                                printf("|-- ");
                } else {
                        if ((1<<j) & bmp)
                                printf("|   ");
                        else
                                printf("    ");
                }
        }
        if (clk == clocks_info)
                printf("%s\n", clk->name);
        else {
                char *unit = "Hz";
                double drate = (double)clk->rate;
                
                if (drate > 1000 && drate < 1000000) {
                        unit = "KHz";
                        drate /= 1000;
                }
                if (drate > 1000000) {
                        unit = "MHz";
                        drate /= 1000000;
                }
                printf("%s (flags:%d,usecount:%d,rate:%5.2f %s)\n",
                        clk->name, clk->flags, clk->usecount, drate, unit);
                //printf("%s (flags:%d,usecount:%d,rate:%5.2f %s, bmp=0x%x)\n",
                //        clk->name, clk->flags, clk->usecount, drate, unit, bmp);
        }
        if (clk->children) {
                int tbmp = bmp;
                int xbmp = -1;

                if (clk->last_child) {
                        xbmp ^= 1 << (level - 2 );
        
                        xbmp = tbmp & xbmp;
                } else
                        xbmp = bmp;
                for (i = 0; i<clk->num_children; i++) {
                        //if (clk->children[i]->last_child)
                        tbmp = xbmp | (1<<level);
                        print_clock_info(clk->children[i], level + 1, tbmp);
                }
        } 
}

char *debugfs_locate_mpoint(void)
{
	int ret;
	FILE *filep;
	char **path;
	char fsname[64];
	struct statfs sfs;

	path = likely_mpoints;
	while (*path) {
                ret = statfs(*path, &sfs);
                if (ret >= 0 && sfs.f_type == (long)DEBUGFS_MAGIC)
			return *path;
		path++;
	}

	filep = fopen("/proc/mounts", "r");
	if (filep == NULL) {
		fprintf(stderr, "powerdebug: Error opening /proc/mounts.");
                exit(1);
        }

	while (fscanf(filep, "%*s %s %s %*s %*d %*d\n",
		      debugfs_mntpoint, fsname) == 2)
		if (!strcmp(fsname, "debugfs"))
			break;
	fclose(filep);

	if (strcmp(fsname, "debugfs"))
		return NULL;

	return debugfs_mntpoint;
}
