#include "powerdebug.h"

void usage(char **argv)
{
	printf("Usage: %s [OPTIONS]\n", argv[0]);
	printf("  -r 		Show regulator information\n");
	printf("  -s 		Show sensor information\n");
	printf("  -v 		Verbose\n");
	printf("  -h 		Help\n");

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
			printf("\tmicrovolts=%d\n", regulators_info[i].microvolts);
		if (regulators_info[i].min_microvolts)
			printf("\tmin_microvolts=%d\n", regulators_info[i].min_microvolts);
		if (regulators_info[i].max_microvolts)
			printf("\tmax_microvolts=%d\n", regulators_info[i].max_microvolts);

		if (regulators_info[i].microamps)
			printf("\tmicroamps=%d\n", regulators_info[i].microamps);
		if (regulators_info[i].min_microamps)
			printf("\tmin_microamps=%d\n", regulators_info[i].min_microamps);
		if (regulators_info[i].max_microamps)
			printf("\tmax_microamps=%d\n", regulators_info[i].max_microamps);
		if (regulators_info[i].requested_microamps)
			printf("\trequested_microamps=%d\n", regulators_info[i].requested_microamps);

		if (regulators_info[i].num_users)
			printf("\tnum_users=%d\n", regulators_info[i].num_users);
		printf("\n");
	}
}
