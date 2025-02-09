#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

void print_usage(char *command)
{
	int i = 0;
	while (command[i] == '.' || command[i] == '/')
		i++;

	printf("Usage: %s [-m mode] [image_file.%c%c%c]\n", command, command[i], command[i+1], command[i+2]);
	printf("source_file: image file to decode.\n");
	printf("mode: IBM PC BIOS mode, in hexadecimal - don't need to write the \"0x\".\n Supported: 0x10 (EGA 640x350 4-bit), 0x12 (VGA 640x480 4-bit) and 0x13 (320x200 8-bit).\n");
	printf("\n");
	printf("%s displays the image in the screen.\n", command);
	printf("\n");
}

int parse_args(int argc, char **argv, char **input_filename, uint16_t *mode_wanted)
{
	if (argc == 1)
	{
		print_usage(argv[0]);
		return -1;
	}

	for (int i = 0; i < argc; i++)
	{
		if (argv[i][0] == '-' && argv[i][1] != 0)
		{
			switch(argv[i][1])
			{
			case 'm':
				*mode_wanted = strtol(argv[i+1], NULL, 16);
				i++;
				break;
			case 'h':
				print_usage(argv[0]);
				return -1;
			default:
				print_usage(argv[0]);
				return -1;
			}
		}
	}

	*input_filename = argv[argc-1];
	return 0;
}
