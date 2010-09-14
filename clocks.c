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
#include<error.h>
#include<errno.h>

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

int read_and_print_clock_info(int verbose, int hrow)
{
        int line = 0, usecount = 0, flags = 0, rate = 0;
        DIR *dir, *subdir;
        char filename[PATH_MAX], devpath[PATH_MAX], clockname[NAME_MAX];
        struct dirent *item, *subitem;

        (void)verbose;

        print_clock_header(1);

        sprintf(filename, "%s", "/debug/clock");

        dir = opendir(filename);
        if (!dir)
                return 0;

        while ((item = readdir(dir))) {
                if (item->d_name[0] == '.')  /* skip the hidden files */
                        continue;

                sprintf(devpath, "/debug/clock/%s", item->d_name);
                strcpy(clockname, item->d_name);

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

                print_clock_info_line(line, clockname, flags, rate,
                                      usecount, (hrow == line) ? 1 : 0);
                line++;
                closedir(subdir);
        }

        closedir(dir);

        if (hrow >= (line - 1))
                hrow = -1;
        return hrow;
}
