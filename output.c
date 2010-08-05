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

void print_regulator_info(int verbose)
{
	int i;

	for (i=0; i<numregulators; i++) {
		printf("Regulator # %d\n", i+1);
		printf("\tname=%s\n", regulators_info[i].name);
		if (strcmp(regulators_info[i].status, ""))
			printf("\tstatus=%s\n", regulators_info[i].status);
		if (strcmp(regulators_info[i].state, ""))
			printf("\tstate=%s\n", regulators_info[i].state);

		if (!verbose)
			continue;

		if (strcmp(regulators_info[i].type, ""))
			printf("\ttype=%s\n", regulators_info[i].type);
		if (strcmp(regulators_info[i].opmode, ""))
			printf("\topmode=%s\n", regulators_info[i].opmode);

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
