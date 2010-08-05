#include <getopt.h>

#include "powerdebug.h"


int numregulators;

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
		fprintf(stderr, "init_regulator_ds: Not enough memory to read information for %d regulators!\n",
			numregulators);
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

/* 
	1. readdir /sys/class/hwmon
	2. foreach subdir in above, do following
	  a) check if its virtual or physical (check for device)
	  b) for each *type* (in, temp, pwm, fan, curr, power, energy)
		i> search for "_" (strrchr... check for last)
		ii> check the *item* depending on type
		    i.e for "in" type items=min, max, input, label
		 	for "fan" type items=min,max,input,div,target,label
			for "pwm" type items=enable, mode, freq
			for "temp" type items=type, max, min,input,crit,offset,label,lowest,highest,
			for "curr" type items=max,min,input
			for "power" type items=average,average_interval,average_min/max, average_highest/lowest, input, input_highest/lowest, accuracy,alarm, cap, cap_min/max
			for "energy" type items=input
			for "intrusion" type items=alarm,beep
*/

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


int read_regulator_info(int verbose)
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

		len = sprintf(filename, "/sys/class/regulator/%s",item->d_name);

		dir = opendir(filename);
		if (!dir) {
			//ret = 1;
			//goto exit2;
			continue;
		}

		count++;
		if (count > numregulators) {
			ret = 1;
			goto exit1;
		}

		strcpy(regulators_info[count-1].name, item->d_name);
		while((ritem = readdir(dir))) {
			if (strlen(ritem->d_name) < 3)
				continue;

			sprintf(filename + len, "/%s", ritem->d_name);
			// printf("Inside %s directory. Opening file %s!\n", item->d_name, ritem->d_name);

			file = fopen(filename, "r");
			if (!file)
				continue;

			memset(line, 0, 1024);
			fptr = fgets(line, 1024, file);
			fclose(file);
			if (!fptr) {
				//ret = 1;
				//goto exit1;
				continue;
			}
			// printf("Read file %s, data=%s, count = %d\n", filename, fptr, count);

			if (!strcmp(ritem->d_name, "name"))
				strcpy(regulators_info[count-1].name, fptr);
			if (!strcmp(ritem->d_name, "state"))
				strcpy(regulators_info[count-1].state, fptr);
			if (!strcmp(ritem->d_name, "status"))
				strcpy(regulators_info[count-1].status, fptr);

			/* Read following _only_ if verbose option specified */
			if(!verbose)
				continue;

			if (!strcmp(ritem->d_name, "type"))
				strcpy(regulators_info[count-1].type, fptr);
			if (!strcmp(ritem->d_name, "opmode"))
				strcpy(regulators_info[count-1].opmode, fptr);

			if (!strcmp(ritem->d_name, "microvolts"))
				regulators_info[count-1].microvolts = atoi(fptr);
			if (!strcmp(ritem->d_name, "min_microvolts"))
				regulators_info[count-1].min_microvolts = atoi(fptr);
			if (!strcmp(ritem->d_name, "max_microvolts"))
				regulators_info[count-1].max_microvolts = atoi(fptr);

			if (!strcmp(ritem->d_name, "microamps"))
				regulators_info[count-1].microamps = atoi(fptr);
			if (!strcmp(ritem->d_name, "min_microamps"))
				regulators_info[count-1].min_microamps = atoi(fptr);
			if (!strcmp(ritem->d_name, "max_microamps"))
				regulators_info[count-1].max_microamps = atoi(fptr);
			if (!strcmp(ritem->d_name, "requested_microamps"))
				regulators_info[count-1].requested_microamps = atoi(fptr);

			if (!strcmp(ritem->d_name, "num_users"))
				regulators_info[count-1].num_users = atoi(fptr);
		}
exit1:
		closedir(dir);
		if (ret)
			break;
	}	
//exit2:
	closedir(regdir);

	return ret;
}


int main(int argc, char **argv)
{
	int c;
	int regulators = 0, sensors = 0, verbose = 0;

	/*
	 * Options:
	 * -r, --regulator	: regulator
	 * -s, --sensor		: sensors
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
			{"verbose", 0, 0, 'v'},
			{"version", 0, 0, 'V'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "rsvVh", long_options, &optindex);
		if (c == -1)
			break;

		switch (c) {
			case 'r':
				regulators = 1;
				break;
			case 's':
				sensors = 1;
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


	/* Need atleast one option specified */
	if (!regulators && !sensors) {
		usage(argv);
	}

	init_regulator_ds();

	if (regulators) {
		read_regulator_info(verbose);
		print_regulator_info(verbose);
	}

	if (sensors) {
		read_and_print_sensor_info(verbose);
	}

	return 0;

}
