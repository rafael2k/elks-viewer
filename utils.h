#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

void print_usage(char *command);

int parse_args(int argc, char **argv, char **input_filename, uint16_t *mode_wanted);


#endif // UTILS_H_
