#include "powerdebug.h"

void usage(char **argv)
{
	printf("Usage: %s [OPTIONS]\n", argv[0]);
	printf("  -r, --regulator 	Show regulator information\n");
	printf("  -s, --sensor		Show sensor information\n");
	printf("  -v, --verbose		Verbose mode (use with -r and/or -s)\n");
	printf("  -V, --version		Show Version\n");
	printf("  -h, --help 		Help\n");

	exit(0);
}

void version()
{
	printf("powerdebug version %s\n", VERSION);
	exit(0);
}

void print_string_val(char *name, char *val)
{
	printf("\t%s=%s", name, val);
	if(!strchr(val, '\n'))
		printf("\n");
}

void print_regulator_info(int verbose)
{
	int i;

	for (i=0; i<numregulators; i++) {
		printf("Regulator # %d\n", i+1);
			print_string_val("name", regulators_info[i].name);
		if (strcmp(regulators_info[i].status, ""))
			print_string_val("status", regulators_info[i].status);
		if (strcmp(regulators_info[i].state, ""))
			print_string_val("state", regulators_info[i].state);

		if (!verbose)
			continue;

		if (strcmp(regulators_info[i].type, ""))
			print_string_val("type", regulators_info[i].type);
		if (strcmp(regulators_info[i].opmode, ""))
			print_string_val("opmode", regulators_info[i].opmode);

		if (regulators_info[i].microvolts)
			printf("\tmicrovolts=%d\n",
				regulators_info[i].microvolts);
		if (regulators_info[i].min_microvolts)
			printf("\tmin_microvolts=%d\n",
				regulators_info[i].min_microvolts);
		if (regulators_info[i].max_microvolts)
			printf("\tmax_microvolts=%d\n",
				regulators_info[i].max_microvolts);

		if (regulators_info[i].microamps)
			printf("\tmicroamps=%d\n",
				regulators_info[i].microamps);
		if (regulators_info[i].min_microamps)
			printf("\tmin_microamps=%d\n",
				regulators_info[i].min_microamps);
		if (regulators_info[i].max_microamps)
			printf("\tmax_microamps=%d\n",
				regulators_info[i].max_microamps);
		if (regulators_info[i].requested_microamps)
			printf("\trequested_microamps=%d\n",
				regulators_info[i].requested_microamps);

		if (regulators_info[i].num_users)
			printf("\tnum_users=%d\n",
				regulators_info[i].num_users);
		printf("\n");
	}

	if (!numregulators && verbose) {
		printf("Could not find regulator information!");
		printf(" Looks like /sys/class/regulator is empty.\n\n");
	}
}
