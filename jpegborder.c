#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>


int read_image(char * filepath, unsigned char ** image, int * ret_w, \
	int * ret_h, int * ret_num_comp, J_COLOR_SPACE * ret_colorspace) {

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* Data structure for storing one scanline from an image */
	JSAMPROW row_pointer[1];

	FILE *f = fopen(filepath, "rb");
	if (!f) {
		fprintf(stderr, \
			"Error: could not open jpeg %s\n!", \
			filepath);
		return -1;
	}

	cinfo.err = jpeg_std_error(&jerr); /* Setup error handler */
	/* Run all the functions that set the scene for reading an image */
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	/* Read some information about the image from the file */
	*(ret_w) = cinfo.image_width;
	*(ret_h) = cinfo.image_height;
	*(ret_num_comp) = cinfo.num_components;
	*(ret_colorspace) = cinfo.out_color_space;

	/* Make space to store the raw image data (the actual pixels) */
	*(image) = (unsigned char *) \
		malloc(cinfo.image_width * cinfo.image_height * cinfo.num_components);
	/* Also make space to store a row of the image for reading purposes */
	row_pointer[0] = (unsigned char *) \
		malloc(cinfo.image_width * cinfo.num_components);

	/* Read image into memory */
	unsigned long m = 0;
	unsigned long size_of_row = 0;

	while (cinfo.output_scanline < cinfo.image_height) {
		size_of_row = cinfo.image_width * cinfo.num_components;
		/* Index in the input image data where the
		 * pixel data for the mth row begins */
		m = cinfo.output_scanline * size_of_row;

		jpeg_read_scanlines(&cinfo, row_pointer, 1);

		for (unsigned long h = 0; h < size_of_row; h++) {
			(*(image))[m + h] = row_pointer[0][h];
		}
	}

	/* Clean up the stuff used for reading the image into memory */
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);
	fclose(f);

	return 0;
}


int write_image(char * filepath, unsigned char * image, int w, int h, \
	int num_comp, J_COLOR_SPACE colorspace) {

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* Data structure for storing one scanline from an image */
	JSAMPROW row_pointer[1];

	FILE *f = fopen(filepath, "wb");
	if (!f) {
		fprintf(stderr, \
			"Error: could not open output filepath %s\n!", \
			filepath);
		return -1;
	}

	cinfo.err = jpeg_std_error(&jerr); /* Setup error handler */
	/* Run all the functions that set the scene for writing an image */
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, f);
	/* Setting the parameters of the output file here */
	cinfo.image_width = w;
	cinfo.image_height = h;
	cinfo.input_components = num_comp;
	cinfo.in_color_space = colorspace;
	/* TODO: What are the defaults? */
	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);

	/* Write the resized image, row by row */
	while (cinfo.next_scanline < cinfo.image_height) {
		/* The index in the output image data where the pixel data
		 * for the mth row begins. */
		int m = \
			cinfo.next_scanline * cinfo.image_width * cinfo.input_components;
		row_pointer[0] = &image[m];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Clean up the stuff used for writing the image */
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(f);

	return 0;
}


int image_add_border(unsigned char * image, int iw, int ih, \
	int i_num_comp, unsigned char ** resized_image, double widthratio, \
	double heightratio, char makesquare, int * ret_ow, int * ret_oh) {

	int ow = iw * widthratio;
	int oh = ih * heightratio;

	/* If makesquare is set, adjust the other dimension
	 * to be the same size... */
	if (makesquare == 1) {
		/* ...but only if one, and only one of the ratios is set... */
		if (widthratio == 1.0 && heightratio != 1.0) {
			ow = oh;
			/* border_w = ow - iw; */
		}
		if (heightratio == 1.0 && widthratio != 1.0) {
			oh = ow;
			/* border_h = oh - ih; */
		}
	}
	/* The height/width of the border in pixels */
	int border_h = (oh - ih) / 2.0;
	int border_w = (ow - iw) / 2.0;
	/* Adjust output dimensions to be precisely the size of the original image
	 * + 2 borders on the top/bottom, 2 borders on the sides */
	ow = iw + (2 * border_w);
	oh = ih + (2 * border_h);
	*(ret_ow) = ow;
	*(ret_oh) = oh;
	*(resized_image) = (unsigned char *) malloc(ow * oh * i_num_comp);

	/* Read through image one pixel, (i, j) at a time */
	for (int h = 0; h < oh; h++) {
		for (int w = 0; w < ow; w++) {
			/* Resized image x pixel memory offset,
			 * resized image y pixel offset */
			int rx = w * i_num_comp;
			int ry = h * ow * i_num_comp;

			/* If the iteration is /between/ the borders */
			if (h >= border_h && h < (oh - border_h)
				&& w >= border_w && w < (ow - border_w)) {

				/* Input image x pixel memory offset,
				 * input image y pixel offset. */
				int ix = (w - border_w) * i_num_comp;
				int iy = (h - border_h) * iw * i_num_comp;

				// TODO: grayscale images only have luminance?
				/* Red pixel */
				(*(resized_image))[ry + rx + 0] = image[iy + ix + 0];
				/* Green pixel */
				(*(resized_image))[ry + rx + 1] = image[iy + ix + 1];
				/* Blue pixel */
				(*(resized_image))[ry + rx + 2] = image[iy + ix + 2];
			/* Else the loop is within the bounds of the borders,
			 * make the pixel white */
			} else {
				// TODO: grayscale images only have luminance?
				(*(resized_image))[ry + rx + 0] = 255; /* Red pixel */
				(*(resized_image))[ry + rx + 1] = 255; /* Green pixel */
				(*(resized_image))[ry + rx + 2] = 255; /* Blue pixel */
			}
		}
	}

	return 0;
}


int main(int argc, char **argv) {
	int opt;
	struct option opt_table[] = {
		{ "input",          required_argument,  NULL,  'i' },
		{ "height",         required_argument,  NULL,  'h' },
		{ "output",         required_argument,  NULL,  'o' },
		{ "overwrite",      no_argument,        NULL,  'O' },
		{ "square",         no_argument,        NULL,  's' },
		{ "width",          required_argument,  NULL,  'w' },
		{ 0, 0, 0, 0 }
	};
	char opt_string[] = { "i:h:o:Osw:" };

	double heightratio = 1;
	double widthratio = 1;
	char makesquare = 0;
	char *inputfilepath;
	char *outputfilepath = NULL;

	while ((opt = getopt_long(argc, argv, opt_string, opt_table, NULL)) \
		!= -1) {

		switch (opt) {
			case 'i':
				inputfilepath = malloc(strlen(optarg) + 1);
				strncpy(inputfilepath, optarg, strlen(optarg));
				break;
			case 'h':
				heightratio = strtod(optarg, NULL);
				break;
			case 'o':
				outputfilepath = malloc(strlen(optarg) + 1);
				strncpy(outputfilepath, optarg, strlen(optarg));
				break;
			case 'O':
				outputfilepath = malloc(strlen(inputfilepath) + 1);
				strncpy(outputfilepath, inputfilepath, strlen(inputfilepath));
				break;
			case 's':
				makesquare = 1;
				break;
			case 'w':
				widthratio = strtod(optarg, NULL);
				break;
		}
	}

	if (outputfilepath == NULL) {
		fprintf(stderr, \
			"Error: no output option was given! Please use the -o or -O flags!\n");
		free(inputfilepath);
		free(outputfilepath);
		return -1;
	}

	unsigned char *input_image_data = NULL;
	unsigned char *resized_image_data = NULL;
	int input_image_w;
	int input_image_h;
	int input_image_num_comp;
	J_COLOR_SPACE input_image_colorspace;

	if (0 != read_image(inputfilepath, &input_image_data, &input_image_w, \
		&input_image_h, &input_image_num_comp, &input_image_colorspace)) {
		return -1;
	}

	free(inputfilepath);
	int output_image_w;
	int output_image_h;

	if (0 != image_add_border(input_image_data, input_image_w, \
		input_image_h, input_image_num_comp, &resized_image_data, widthratio, \
		heightratio, makesquare, &output_image_w, &output_image_h)) return -2;

	free(input_image_data);

	if (0 != write_image(outputfilepath, resized_image_data, output_image_w, \
		output_image_h, input_image_num_comp, input_image_colorspace)) {
		return -3;
	}

	free(resized_image_data);
	free(outputfilepath);

	return 0;
}
