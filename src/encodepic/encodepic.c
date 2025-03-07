/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>
#include <errno.h>
#include "../libwasteland/wasteland.h"
#include "config.h"

/**
 * Displays the usage text.
 */

static void display_usage(void)
{
	printf("Usage: wl_encodepic [OPTION]... PNGFILE PICFILE\n");
	printf("Converts a PNG image file into a PIC image file.\n");
	printf("\nThe PNG file can have any dimension and colors. It is "
	       "automatically converted.\n");
	printf("\nOptions\n");
	printf("  -h, --help          Display help and exit\n");
	printf("  -V, --version       Display version and exit\n");
	printf("\nReport bugs to %s <%s>\n", AUTHOR, EMAIL);
}

/**
 * Displays the version information.
 */

static void display_version(void)
{
	printf("wl_encodepic %s\n", VERSION);
	printf("\n%s\n", COPYRIGHT);
	printf("This is free software; see the source for copying conditions. ");
	printf("There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS ");
	printf("FOR A PARTICULAR PURPOSE.\n\nWritten by %s <%s>\n", AUTHOR,
	       EMAIL);
}

/**
 * Terminate the program with code 1 and the specified error message.
 *
 * @param message
 *            The error message
 */

static void die(char *message, ...)
{
	va_list args;

	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	exit(1);
}

/**
 * Check options.
 *
 * @param argc
 *            The number of arguments
 * @param argv
 *            The argument array
 */

static void check_options(int argc, char *argv[])
{
	char opt;
	int index;
	static struct option options[] = { { "help", 0, NULL, 'h' },
					   { "version", 0, NULL, 'V' } };

	opterr = 0;
	while ((opt = getopt_long(argc, argv, "hV", options, &index)) != -1) {
		switch (opt) {
		case 'V':
			display_version();
			exit(1);
			break;
		case 'h':
			display_usage();
			exit(1);
			break;
		default:
			die("Unknown option: %s\nUse --help to show valid options.\n",
			    argv[optind - 1]);
			break;
		}
	}
}

/**
 * Writes the pic into the specified file in PNG format.
 * 
 * @param filename
 *            The output filename
 * @param pic
 *            The wasteland pic
 */

static void writePic(char *filename, gdImagePtr image)
{
	gdImagePtr output;
	int x, y, i;
	wlImage pic;

	/* Create a temporary second image for palette conversion */
	output = gdImageCreate(288, 128);
	for (i = 0; i < 16; i++) {
		gdImageColorAllocate(output, wlPalette[i].red,
				     wlPalette[i].green, wlPalette[i].blue);
	}
	for (i = 16; i < 256; i++)
		gdImageColorAllocate(output, 0, 0, 0);
	gdImageCopyResampled(output, image, 0, 0, 0, 0, 288, 128,
			     gdImageSX(image), gdImageSY(image));

	/* Create the pic */
	pic = wlImageCreate(288, 128);

	/* Copy pixels from image to pic */
	for (y = 0; y < pic->height; y++) {
		for (x = 0; x < pic->width; x++) {
			pic->pixels[y * pic->width + x] =
				gdImageGetPixel(output, x, y);
		}
	}

	/* Write the pic file */
	wlPicWriteFile(pic, filename);

	/* Free resources */
	wlImageFree(pic);
	gdImageDestroy(output);
}

/**
 * Main method
 *
 * @param argc
 *            The number of arguments
 * @param argv
 *            The argument array
 * @return Exit value
 */

int main(int argc, char *argv[])
{
	char *source, *dest;
	gdImagePtr image;
	FILE *file;

	/* Process options and reset argument pointer */
	check_options(argc, argv);
	argc -= optind;
	argv += optind;

	/* Terminate if wrong number of parameters are specified */
	if (argc != 2)
		die("Wrong number of parameters.\nUse --help to show syntax.\n");

	/* Process parameters */
	source = argv[0];
	dest = argv[1];

	/* Read the PNG file */
	file = fopen(source, "rb");
	if (!file) {
		die("Unable to read PNG file %s: %s\n", source,
		    strerror(errno));
	}
	image = gdImageCreateFromPng(file);
	fclose(file);

	/* Write the PIC file */
	writePic(dest, image);

	/* Free resources */
	gdImageDestroy(image);

	/* Success */
	return 0;
}
