#ifndef _BINARIZE_H_
#define _BINARIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

void bilinear_resize(unsigned char* in_img, unsigned char* out_img, int in_width, int in_height, int out_width, int out_height);
void bilinear_resize_new(unsigned char* in_img, unsigned char* out_img, int in_width, int in_height, int out_width, int out_height);


/**
	Region thresholding with project sum on axis of bradley binary image.

	@param sum: Sum pixel on axis.
	@param bin: Return binary by threshold.
	@param len: lenght of axis.
	@param r1: Radius of region.
	@param r2: Radios of global.
	@param threshold: Threshold for binary.
*/
void region_avarage_bradley_threshold_1D(int* sum, unsigned char* bin, int len, int r1, int r2, float threshold);

/***
	Bradley binary thresholding - Convert image to binary image with 3 threshold ratio.

	@param height: Number row of image.
	@param width: Number col of image.
	@param gray_img: Input data image.
	@param img1: Return binary image calculate by threshold
	@param img2: Return binary image calculate by threshold * ratio1
	@param img3: Return binary image calculate by threshold * ratio2
	@param threshold: threshold for calculate binary image.
	@param window_r: Radius of kernel filter.
	@param ratio1: Ratio of threshold.
	@param ratio2: Ratio of threshold.
	@return: 3 Binary image.
***/
void triple_faster_bradley_threshold(int height, int width, unsigned char* gray_img, 
	unsigned char* img1, unsigned char* img2, unsigned char* img3, float threshold, int window_r, float ratio1, float ratio2);

// Huong•ÏŠ·20220720
unsigned char* get_roi(unsigned char* img, unsigned char* roi, int width, int height,
                       int roi_left, int roi_top, int roi_right, int roi_bottom);

void project_on_x(unsigned char* image, int* sum, int top, int bottom, int width);

/**
	Calculate sum pixel on each row.

	@param image: Data of image.
	@param sum: Return sum pixel of each row.
	@param height: Number row of image.
	@param width: Number col of image.
*/
void project_on_y(unsigned char* image, int* sum, int height, int width);

void remove_boundary_noise(unsigned char* bin, int len);
void filter_noise_segmentation(unsigned char* bin, int len, int threshold);
void merge_H_segmentation(unsigned char* bin, int len, int threshold, int delta, int nof_ref);

int get_largest_binarized_CC1D(unsigned char *bin, int* histogram, int len, int num, int* left, int* right);


void cut_bottom_top_offset(unsigned char* img, int height, int width, int *top, int *bottom, int region_r, int threshold);
unsigned char* copy_make_border(unsigned char* img, int height, int width);

void gauss_filter(unsigned char* img, int height, int width);
void gauss_filter_new(unsigned char* img, int height, int width);
#ifdef __cplusplus
}
#endif

#endif
