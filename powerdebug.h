#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>
#include <errno.h>

#define VERSION "1.0"

//#define PATH_MAX 1024
//#define NAME_MAX 128
#define VALUE_MAX 16

struct regulator_info {
	char name[NAME_MAX];
	char state[VALUE_MAX];
	char status[VALUE_MAX];
	char type[VALUE_MAX];
	char opmode[VALUE_MAX];
	int microvolts;
	int min_microvolts;
	int max_microvolts;
	int microamps;
	int min_microamps;
	int max_microamps;
	int requested_microamps;
	int num_users;
} *regulators_info;

extern int numregulators;

extern void usage(char **argv);
extern void version(void);
extern void print_regulator_info(int verbose);
extern void get_sensor_info(char *path, char *name, char *sensor, int verbose);


