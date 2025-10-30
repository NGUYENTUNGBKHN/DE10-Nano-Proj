#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#define EXT
#include "../common/global.h"

#include "hog.h"
#ifdef MEASURE_TIME_
	//#include "time_measure.h"
#endif

#define compute_angle(dx, dy, dxy, theta) compute_angle_float(dx, dy, dxy, theta)

#define compute_angle_int(dx, dy, dxy, theta) \
{\
	dxy = dx * dx + dy * dy; \
	dxy = fast_rsqrt(dxy) * dxy; \
	theta = atan2f(dy, dx) * var; \
	if (theta < 0.0) \
		theta = theta + 180; \
}

#define compute_angle_float(dx, dy, dxy, theta)\
{\
	dxy = sqrtf(dx * dx + dy * dy); \
	theta = atan2(dy, dx) * var; \
	if (theta < 0.0) \
		theta = theta + 180; \
}

#define compute_angle_short(dx, dy, dxy, theta)\
{\
	dxy = sqrtf(dx * dx + dy * dy); \
	theta = atan2(dy, dx) * var; \
	if (theta < 0.0) \
		theta = theta + 180; \
}

float fast_rsqrt(float x) {
	u16 i;
	float x2;
	const float threehalfs = 1.5f;

	x2 = x * 0.5f;
	i = * (u16*) &x;
	i = 0x5f3759df - (i >> 1);
	x = * (float*) &i;
	x *= (threehalfs - (x2 * x * x));
	// x *= (threehalfs - (x2 * x * x));
	return x;
}

float fast_ptr_rsqrt(float x) {
	u16 *i;
	float x2;
	const float threehalfs = 1.5f;

	x2 = x * 0.5f;
	i = (u16*) &x;
	*i = 0x5f3759df - (*i >> 1);
	x = * (float*) i;
	x *= (threehalfs - (x2 * x * x));
	// x *= (threehalfs - (x2 * x * x));
	return x;
}


void img_resize_v3(u8* in_img, u8* out_img, 
	u16 in_width, u16 in_height, u16 out_width, u16 out_height)
{
	const u32 x_ratio = (u32) ((in_width << 16) / (out_width)) + 1;
	const u32 y_ratio = (u32) ((in_height << 16) / (out_height)) + 1;
	u8 *p;
	u16 i_out, j_out;
	u16 i_in, j_in;
	u32 rat;
	if ((out_width < 2) || (out_height < 2)) {
		return;
	}
	for (i_out = 0; i_out < out_height; ++i_out) {
		i_in = ((i_out*y_ratio) >> 16);
		p = in_img + i_in * in_width;
		rat = 0;
		for (j_out = 0; j_out < out_width; ++j_out) {
			j_in = rat >> 16;
			*out_img++ = p[j_in];
			rat += x_ratio;
			}
	}
}

void computeGradient(uchar *src, u16 hight, u16 width, float* dxy, float* theta) {
	u16 idx;
	float x1, x2, y1, y2;
	float dx, dy;
	u16 i = 0;
	u16 j = 0;
	for (i = 0; i < hight; i++)
	{
		for (j = 0; j < width; j++)
		{
			//    | y1 |
			// x1 |    | x2
			//    | y2 |
			idx = i*width+j;
			if (i == 0){
				y1 = src[idx];
				y2 = src[idx+width];
			} 
			else if (i == hight - 1){
				y1 = src[idx-width];
				y2 = src[idx];
			} 
			else{
				y1 = src[(i-1)*width+j];
				y2 = src[(i+1)*width+j];
			}

			if (j == 0){
				x1 = src[idx];
				x2 = src[idx+1];
			} 
			else if (j == width - 1){
				x1 = src[idx-1];
				x2 = src[idx];
			} 
			else{
				x1 = src[idx-1];
				x2 = src[idx+1];
			}
			
			dx = x2 - x1;
			dy = y2 - y1;
			dxy[idx] = sqrtf(dx * dx + dy * dy);
			theta[idx] = atan2(dy, dx) * var;
			if (theta[idx] < 0.0)
				theta[idx] = theta[idx] + 180;
		}
	}
}

void computeGradient_v2(uchar *src, u16 hight, u16 width, float* dxy, float* theta) {
	u16 idx;
	float x1, x2, y1, y2;
	float dx, dy;
	u16 i, j;

	u16 height1 = hight - 1;
	u16 width1 = width - 1;

	uchar *p1 = src;
	uchar *p2 = src + 2 * width;

	uchar *p3 = src + width;
	uchar *p4 = src + width + 2;

	idx = width1;
	for (i = 1; i < height1; i++)
	{
		// Calculate (i, 0)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);

		x1 = *(p3);
		x2 = *(p3+1);
		
		
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle(dx, dy, dxy[idx], theta[idx])
		
		// Calculate (i, 1...w-2)
		for (j = 1; j < width1; j++)
		{
			//    | y1 |
			// x1 |    | x2
			//    | y2 |
			++idx;
			y1 = *(p1++);
			y2 = *(p2++);

			x1 = *(p3++);
			x2 = *(p4++);
			
			
			dx = x2 - x1;
			dy = y2 - y1;
			compute_angle(dx, dy, dxy[idx], theta[idx])
		}
		// Calculate (i, w-1)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);
		x1 = *(p3);
		x2 = *(p3+1);
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle(dx, dy, dxy[idx], theta[idx])


		// Fix pointer of p1, p2, p3, p4
		// ++p1;
		// ++p2;

		++p3; ++p3;
		++p4; ++p4;
	}

	// Calculate row: 0 | col: 1...width-2
	p1 = src;
	p2 = src + width;
	p3 = src;
	p4 = src + 2;
	idx = -1;

	{
		// Calculate (i, 0)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);

		x1 = *(p3);
		x2 = *(p3+1);
		
		
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle(dx, dy, dxy[idx], theta[idx])
		
		// Calculate (i, 1...w-2)
		for (j = 1; j < width1; j++)
		{
			//    | y1 |
			// x1 |    | x2
			//    | y2 |
			++idx;
			y1 = *(p1++);
			y2 = *(p2++);

			x1 = *(p3++);
			x2 = *(p4++);
			
			
			dx = x2 - x1;
			dy = y2 - y1;
			compute_angle(dx, dy, dxy[idx], theta[idx])
		}
		// Calculate (i, w-1)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);
		x1 = *(p3);
		x2 = *(p3+1);
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle(dx, dy, dxy[idx], theta[idx])
	}
	

	// Calculate row: height-1 | col: 1...width-2
	p2 = src + height1 * width;
	p1 = p2 - width;

	p3 = src + height1 * width;
	p4 = src + height1 * width + 2;
	idx = height1 * width-1;
	
	{
		// Calculate (i, 0)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);

		x1 = *(p3);
		x2 = *(p3+1);
		
		
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle(dx, dy, dxy[idx], theta[idx])
		
		// Calculate (i, 1...w-2)
		for (j = 1; j < width1; j++)
		{
			//    | y1 |
			// x1 |    | x2
			//    | y2 |
			++idx;
			y1 = *(p1++);
			y2 = *(p2++);

			x1 = *(p3++);
			x2 = *(p4++);
			
			
			dx = x2 - x1;
			dy = y2 - y1;
			compute_angle(dx, dy, dxy[idx], theta[idx])
		}
		// Calculate (i, w-1)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);
		x1 = *(p3);
		x2 = *(p3+1);
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle(dx, dy, dxy[idx], theta[idx])
	}



}

void computeGradient_v3(uchar *src, u16 hight, u16 width, float* dxy, float* theta) {
	u16 idx;
	u16 i, j;
	short x1, x2, y1, y2;
	float dx, dy;


	u16 height1 = hight - 1;
	u16 width1 = width - 1;

	uchar *p1 = src;
	uchar *p2 = src + 2 * width;

	uchar *p3 = src + width;
	uchar *p4 = src + width + 2;

	idx = width1;
	for (i = 1; i < height1; i++)
	{
		// Calculate (i, 0)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);

		x1 = *(p3);
		x2 = *(p3+1);
		
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle_short(dx, dy, dxy[idx], theta[idx])
		
		// Calculate (i, 1...w-2)
		for (j = 1; j < width1; j++)
		{
			//    | y1 |
			// x1 |    | x2
			//    | y2 |
			++idx;
			y1 = *(p1++);
			y2 = *(p2++);

			x1 = *(p3++);
			x2 = *(p4++);
			
			dx = x2 - x1;
			dy = y2 - y1;
			compute_angle_short(dx, dy, dxy[idx], theta[idx])
		}
		// Calculate (i, w-1)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);
		x1 = *(p3);
		x2 = *(p3+1);
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle_short(dx, dy, dxy[idx], theta[idx])

		++p3; ++p3;
		++p4; ++p4;
	}

	// Calculate row: 0 | col: 1...width-2
	p1 = src;
	p2 = src + width;
	p3 = src;
	p4 = src + 2;
	idx = -1;
	{
		// Calculate (i, 0)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);

		x1 = *(p3);
		x2 = *(p3+1);
		
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle_short(dx, dy, dxy[idx], theta[idx])
		
		// Calculate (i, 1...w-2)
		for (j = 1; j < width1; j++)
		{
			//    | y1 |
			// x1 |    | x2
			//    | y2 |
			++idx;
			y1 = *(p1++);
			y2 = *(p2++);

			x1 = *(p3++);
			x2 = *(p4++);
			
			dx = x2 - x1;
			dy = y2 - y1;
			compute_angle_short(dx, dy, dxy[idx], theta[idx])
		}
		// Calculate (i, w-1)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);
		x1 = *(p3);
		x2 = *(p3+1);
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle_short(dx, dy, dxy[idx], theta[idx])
	}
	

	// Calculate row: height-1 | col: 1...width-2
	p2 = src + height1 * width;
	p1 = p2 - width;

	p3 = src + height1 * width;
	p4 = src + height1 * width + 2;
	idx = height1 * width-1;
	{
		// Calculate (i, 0)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);

		x1 = *(p3);
		x2 = *(p3+1);
		
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle_short(dx, dy, dxy[idx], theta[idx])
		
		// Calculate (i, 1...w-2)
		for (j = 1; j < width1; j++)
		{
			//    | y1 |
			// x1 |    | x2
			//    | y2 |
			++idx;
			y1 = *(p1++);
			y2 = *(p2++);

			x1 = *(p3++);
			x2 = *(p4++);
			
			
			dx = x2 - x1;
			dy = y2 - y1;
			compute_angle_short(dx, dy, dxy[idx], theta[idx])
		}
		// Calculate (i, w-1)
		++idx;
		y1 = *(p1++);
		y2 = *(p2++);
		x1 = *(p3);
		x2 = *(p3+1);
		dx = x2 - x1;
		dy = y2 - y1;
		compute_angle_short(dx, dy, dxy[idx], theta[idx])
	}

}

void computeOrientBin(  float *dxy, float *theta, u16 width_img, u16 orientations, u16 pixels_per_cell, u16 n_cell_i, u16 n_cell_j,
						float *orient_bin) 
{
	u16 idx;

	u16 cell_i, cell_j, bin, start_i, end_i, start_j, end_j;
	u16 idx_bin;
	float bin_A, bin_B;
	u16 i = 0;
	u16 j = 0;

	float bin_size = 180.0 / orientations;

	for (cell_i = 0; cell_i < n_cell_i; cell_i++)
	{
		for (cell_j = 0; cell_j < n_cell_j; cell_j++)
		{
			start_i = (cell_i) * pixels_per_cell;
			end_i = (cell_i + 1) * pixels_per_cell;
			start_j = (cell_j) * pixels_per_cell;
			end_j = (cell_j + 1) * pixels_per_cell;
			for (bin = 0; bin < orientations; bin++)
			{
				for (i = start_i; i < end_i; i++)
				{
					for (j = start_j; j < end_j; j++)
					{
						idx = i*width_img+j;
						bin_A = (float)bin*bin_size;
						bin_B = bin_A + bin_size;
						if (theta[idx] >= bin_A && theta[idx] < bin_B)
						{
							idx_bin = (cell_i * n_cell_j + cell_j) * orientations + bin;
							// if (cell_i == 1 && cell_j == 3 && bin == 0)
							//     printf("%d %f\n", idx, orient_bin[idx_bin]);
							// if (i < 5 && j < 5) printf("%d %d %d\n", i, j, idx_bin);
							if (bin < (orientations - 1))
							{
								orient_bin[idx_bin] += ((bin_B-theta[idx]) / bin_size) * dxy[idx];
								orient_bin[idx_bin + 1] += ((theta[idx]-bin_A) / bin_size) * dxy[idx];
							} 
							else if (bin == (orientations - 1))
							{
								orient_bin[idx_bin] += ((bin_B-theta[idx]) / bin_size) * dxy[idx];
								orient_bin[idx_bin + 1 - orientations] += ((theta[idx]-bin_A) / bin_size) * dxy[idx];
							}
						}
					}
				}
			}
		}
	}
}

void computeOrientBin_v2(  float *dxy, float *theta, u16 width_img, u16 orientations, u16 pixels_per_cell, u16 n_cell_i, u16 n_cell_j,
						float *orient_bin) 
{
	u16 bin;
	u16 idx_cell;
	u16 idx_bin;
	float bin_A, bin_B;
	u16 idx, prev_idx;

	float bin_size = 180.0 / orientations;

	u16 n_cell = n_cell_i * n_cell_j;

	idx_cell = 0;
	idx_bin = 0;

	prev_idx = 0;
	for (idx = 1; idx < width_img; ++idx) {
		dxy[idx] += dxy[idx-1];
		theta[idx] += theta[idx-1];
	}

	for (idx_cell = 0; idx_cell < n_cell; ++idx_cell) {
		for (bin = 0; bin < orientations; ++bin) {
			bin_A = bin_size * bin;
			bin_B = bin_A + bin_size;

			if (bin_A <= theta[idx_cell] && theta[idx_cell] < bin_B) {
				if (bin + 1 < orientations) {
					orient_bin[idx_bin] += ((bin_B-theta[idx_cell]) / bin_size) * dxy[idx_cell];
					orient_bin[idx_bin + 1] += ((theta[idx_cell]-bin_A) / bin_size) * dxy[idx_cell];
				} 
				else {
					orient_bin[idx_bin] += ((bin_B-theta[idx_cell]) / bin_size) * dxy[idx_cell];
					orient_bin[idx_bin + 1 - orientations] += ((theta[idx_cell]-bin_A) / bin_size) * dxy[idx_cell];
				}
				++idx_bin;
			}
		}
	}
}

void computeOrientBin_v3(  float *dxy, float *theta, u16 width_img, u16 orientations, u16 pixels_per_cell, u16 n_cell_i, u16 n_cell_j,
						float *orient_bin) 
{
	u16 idx;

	u16 cell_i, cell_j, bin, start_i, end_i, start_j, end_j;
	u16 idx_bin = 0;
	u16 bin_lef, bin_rig;
	float bin_A, bin_B;
	u16 i = 0;
	u16 j = 0;

	float bin_size = 180.0 / orientations;

	start_i = 0;
	end_i = pixels_per_cell;
	for (cell_i = 0; cell_i < n_cell_i; cell_i++)
	{
		start_j = 0;
		end_j = pixels_per_cell;
		for (cell_j = 0; cell_j < n_cell_j; cell_j++)
		{
			for (i = start_i; i < end_i; i++)
			{
				for (j = start_j; j < end_j; j++)
				{
					idx = i * width_img + j;
					bin_lef = theta[idx] / bin_size;
					if (bin_lef >= orientations) continue;
					bin_rig = bin_lef + 1;
					if (bin_rig == orientations) bin_rig = 0;
					
					bin_A = bin_size * bin_lef;
					bin_B = bin_A + bin_size;
					orient_bin[idx_bin + bin_lef] += ((bin_B-theta[idx]) / bin_size) * dxy[idx];
					orient_bin[idx_bin + bin_rig] += ((theta[idx]-bin_A) / bin_size) * dxy[idx];
				}
			}
			start_j += pixels_per_cell;
			end_j += pixels_per_cell;

			idx_bin += orientations;
		}
		start_i += pixels_per_cell;
		end_i += pixels_per_cell;
	}
}

void computeOrientBin_v4(  float *dxy, float *theta, u16 width_img, u16 orientations, u16 pixels_per_cell, u16 n_cell_i, u16 n_cell_j,
						float *orient_bin) 
{
	u16 idx;
	u16 i, j;
	u16 cell_i, cell_j, bin, start_i, end_i, start_j, end_j;
	u16 bin_lef, bin_rig;
	float bin_A, bin_B;
	float *ret = orient_bin;

	float bin_size = 180.0 / orientations;
	float *bin_table = (float*)malloc_float(sizeof(float) * (orientations+1));
	for (i = 0; i <= orientations; ++i)
		bin_table[i] = bin_size * i;
	

	start_i = 0;
	end_i = pixels_per_cell;
	for (cell_i = 0; cell_i < n_cell_i; ++cell_i)
	{
		start_j = 0;
		end_j = pixels_per_cell;
		for ( cell_j = 0; cell_j < n_cell_j; ++cell_j)
		{
			for (i = start_i; i < end_i; ++i)
			{
				for (j = start_j; j < end_j; ++j)
				{
					idx = i * width_img + j;
					bin_lef = theta[idx] / bin_size;
					if (bin_lef >= orientations) continue;
					bin_rig = bin_lef + 1;
					bin_A = bin_table[bin_lef];
					bin_B = bin_table[bin_rig];
					bin_rig = bin_rig == orientations ? 0 : bin_rig;
					
					
					ret[bin_lef] += (bin_B-theta[idx]) * dxy[idx];
					ret[bin_rig] += (theta[idx]-bin_A) * dxy[idx];
				}
			}
			start_j += pixels_per_cell;
			end_j += pixels_per_cell;

			for (i = 0; i < orientations; ++i)
				ret[i] /= bin_size;
			ret += orientations;
		}
		start_i += pixels_per_cell;
		end_i += pixels_per_cell;
	}

	free(bin_table);
}

#ifdef HOG_EXTRACTION_DEBUG_
float *_computeHOG(uchar *src, u16 width, u16 hight,
	u16 orientations, u16 pixels_per_cell, u16 cells_per_block,
	u16 *feature_size)
{
	/*
	src: gray image resize for HOG calculator
	orientations = nbins
	pixels_per_cell = CellSize
	cells_per_block = BlockSize
	return: pointer float, need free
	
	*/
	float *dxy = (float*)malloc_float(sizeof(float) * width * hight);
	float *theta = (float*)malloc_float(sizeof(float) * width * hight);
	u16 idx;

	// Calculate gradient and theta
	start_time(pro4);
	computeGradient_v2(src, hight, width, dxy, theta);
	end_time(pro4, NULL);
	printf("ver1 dxy\n");
	for (u16 i = 0; i < 5; ++i) {
		for (u16 j = 0; j < 5; ++j) {
			idx = i * width + j;
			printf("%f ", dxy[idx]);
		}
		printf("\n");
	}
	printf("ver1 theta\n");
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			idx = i * width + j;
			printf("%f ", theta[idx]);
		}
		printf("\n");
	}

	start_time(pro5);
	computeGradient_v3(src, hight, width, dxy, theta);
	end_time(pro5, NULL);
	printf("ver1 dxy\n");
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			idx = i * width + j;
			printf("%f ", dxy[idx]);
		}
		printf("\n");
	}
	printf("ver1 theta\n");
	for (u16 i = 0; i < 5; ++i) {
		for (u16 j = 0; j < 5; ++j) {
			idx = i * width + j;
			printf("%f ", theta[idx]);
		}
		printf("\n");
	}
	printf("done\n");

	// Orientation binning
	u16 n_cell_i = hight / pixels_per_cell;
	u16 n_cell_j = width / pixels_per_cell;

	u16 n_size_orient_bin = orientations * n_cell_i * n_cell_j;
	float *orient_bin = (float*)malloc_float(sizeof(float) * n_size_orient_bin);
	float *orient_bin_2 = (float*)malloc_float(sizeof(float) * n_size_orient_bin);
	for (u16 n = 0; n < n_size_orient_bin; n++)
	{
		orient_bin[n] = 0.0;
		orient_bin_2[n] = 0.0;
	}

	
	

	start_time(pro5);
	computeOrientBin(dxy, theta, width, orientations, pixels_per_cell, n_cell_i, n_cell_j, orient_bin);
	end_time(pro5, NULL);
//     printf("ver1 orient_bin\n");
//     for (u16 i = 0; i < 5; ++i) {
//         for (u16 j = 0; j < 5; ++j) {
//             idx = i * width + j;
//             printf("%f ", orient_bin[idx]);
//         }
//         printf("\n");
//     }

	start_time(pro6);
	computeOrientBin_v4(dxy, theta, width, orientations, pixels_per_cell, n_cell_i, n_cell_j, orient_bin_2);
	end_time(pro6, NULL);
//     printf("ver 2 orient_bin\n");
//     for (u16 i = 0; i < 5; ++i) {
//         for (u16 j = 0; j < 5; ++j) {
//             idx = i * width + j;
//             printf("%f ", orient_bin_2[idx]);
//         }
//         printf("\n");
//     }
	printf("Check result\n");
	for (u16 n = 0; n < n_size_orient_bin; ++n)
	if (abs(orient_bin[n] - orient_bin_2[n]) > DEBUG_EPS) {
		u16 tmp = n;
		u16 k = n%orientations;
		tmp -= k;
		u16 j = tmp%n_cell_j;
		u16 i = tmp/n_cell_j;
		printf("%d %d %d, origin %f <> new %f\n", i, j, k, orient_bin[n], orient_bin_2[n]);
	}
	printf("compute orient bin done\n");

	u16 n_block_i = n_cell_i - cells_per_block + 1;
	u16 n_block_j = n_cell_j - cells_per_block + 1;

	u16 size_block = orientations * cells_per_block * cells_per_block;
	*feature_size = size_block * n_block_i * n_block_j;
	float *feature = (float*)malloc_float(sizeof(float) * *feature_size);

	u16 start_idx_bin, end_idx_bin;
	u16 start_idx_fd, end_idx_fd, idx_fd;
	start_time(pro6);
	for (u16 block_i = 0; block_i < n_block_i; block_i++)
	{
		for (u16 block_j = 0; block_j < n_block_j; block_j++)
		{
			start_idx_fd = (block_i * n_block_j + block_j) * size_block;
			end_idx_fd = start_idx_fd + size_block;
			idx_fd = start_idx_fd;

			for (u16 i = block_i; i < block_i + cells_per_block; i++)
			{
				for (u16 j = block_j; j < block_j + cells_per_block; j++)
				{
					start_idx_bin = (i * n_cell_j + j) * orientations;
					end_idx_bin = start_idx_bin + orientations;
					for (u16 ii = start_idx_bin; ii < end_idx_bin; ii++)
					{
						feature[idx_fd] = orient_bin[ii];
						idx_fd += 1;
					}
				}
			}
			// Normalize fd vector L2-norm
			float k = 0;
			for (u16 i = start_idx_fd; i < end_idx_fd; i++)
			{
				k += feature[i] * feature[i];
			}
			k = sqrt(k);
			if (k != 0)
			{
				for (u16 i = start_idx_fd; i < end_idx_fd; i++)
				{
					feature[i] = feature[i] / k;
				}
			}
		}
	}
	end_time(pro6, NULL);

	// free(dx);
	// free(dy);
	free_proc(dxy);
	free_proc(theta);
	free_proc(orient_bin);

	return feature;
}

#else
float *_computeHOG(uchar *src, u16 width, u16 hight,
	u16 orientations, u16 pixels_per_cell, u16 cells_per_block,
	u16 *feature_size)
{
	/*
	src: gray image resize for HOG calculator
	orientations = nbins
	pixels_per_cell = CellSize
	cells_per_block = BlockSize
	return: pointer float, need free
	
	*/
	float *dxy = (float*)malloc_float(sizeof(float) * width * hight);
	float *theta = (float*)malloc_float(sizeof(float) * width * hight);
	u16 idx;
	u16 n_cell_i = 0;
	u16 n_cell_j = 0;

	u16 n_size_orient_bin = 0;
	float *orient_bin;
	u16 n = 0;

	u16 n_block_i = 0;
	u16 n_block_j = 0;
	u16 size_block = 0;
	float *feature;
	u16 start_idx_bin, end_idx_bin;
	u16 start_idx_fd, end_idx_fd, idx_fd;
	u16 block_i = 0;
	u16 block_j = 0;
	u16 i = 0;
	u16 ii = 0;
	u16 j = 0;
	float k = 0;

	// Calculate gradient and theta
#ifdef MEASURE_TIME_
	start_time(pro4);
#endif
	computeGradient_v2(src, hight, width, dxy, theta);
#ifdef MEASURE_TIME_
	end_time(pro4, NULL);
#endif

	// Orientation binning
	n_cell_i = hight / pixels_per_cell;
	n_cell_j = width / pixels_per_cell;

	n_size_orient_bin = orientations * n_cell_i * n_cell_j;
	orient_bin = (float*)malloc_float(sizeof(float) * n_size_orient_bin);
	for (n = 0; n < n_size_orient_bin; n++)
	{
		orient_bin[n] = 0.0;
	}

#ifdef MEASURE_TIME_
	start_time(pro5);
#endif
	computeOrientBin_v4(dxy, theta, width, orientations, pixels_per_cell, n_cell_i, n_cell_j, orient_bin);
#ifdef MEASURE_TIME_
	end_time(pro5, NULL);
#endif

	n_block_i = n_cell_i - cells_per_block + 1;
	n_block_j = n_cell_j - cells_per_block + 1;

	size_block = orientations * cells_per_block * cells_per_block;
	*feature_size = size_block * n_block_i * n_block_j;
	feature = (float*)malloc_float(sizeof(float) * *feature_size);

	start_idx_bin, end_idx_bin;
	start_idx_fd, end_idx_fd, idx_fd;
#ifdef MEASURE_TIME_
	start_time(pro6);
#endif
	for (block_i = 0; block_i < n_block_i; block_i++)
	{
		for (block_j = 0; block_j < n_block_j; block_j++)
		{
			start_idx_fd = (block_i * n_block_j + block_j) * size_block;
			end_idx_fd = start_idx_fd + size_block;
			idx_fd = start_idx_fd;

			for (i = block_i; i < block_i + cells_per_block; i++)
			{
				for (j = block_j; j < block_j + cells_per_block; j++)
				{
					start_idx_bin = (i * n_cell_j + j) * orientations;
					end_idx_bin = start_idx_bin + orientations;
					for (ii = start_idx_bin; ii < end_idx_bin; ii++)
					{
						feature[idx_fd] = orient_bin[ii];
						idx_fd += 1;
					}
				}
			}
			// Normalize fd vector L2-norm
			k = 0;
			for (i = start_idx_fd; i < end_idx_fd; i++)
			{
				k += feature[i] * feature[i];
			}
			k = sqrt(k);
			if (k != 0)
			{
				for (i = start_idx_fd; i < end_idx_fd; i++)
				{
					feature[i] = feature[i] / k;
				}
			}
		}
	}
#ifdef MEASURE_TIME_
	end_time(pro6, NULL);
#endif

	free_proc(dxy);
	free_proc(theta);
	free_proc(orient_bin);

	return feature;
}

#endif

float *computeHOG(uchar *src, u16 width, u16 hight, u16 *feature_size,
	u16 orientations, u16 pixels_per_cell, u16 cells_per_block,
	u16 new_width, u16 new_hight)
{
	u16 good_width;
	u16 good_hight;
	float *out;
	good_width = (new_width / pixels_per_cell) * pixels_per_cell;
	good_hight= (new_hight / pixels_per_cell) * pixels_per_cell;
	
//	u8 *img_resize = (u8*)malloc_char(sizeof(u8) * good_width * good_hight);
//#ifdef MEASURE_TIME_
//    start_time(pro2);
//#endif
//    img_resize_v3(src, img_resize, width, hight, good_width, good_hight);
//#ifdef MEASURE_TIME_
//    end_time(pro2, NULL);
//    start_time(pro3);
//#endif
//    float *out = _computeHOG(img_resize, good_width, good_hight, orientations, 
//                pixels_per_cell, cells_per_block, feature_size);
//#ifdef MEASURE_TIME_
//    end_time(pro3, NULL);
//#endif
//	free_proc(img_resize);

	out = _computeHOG(src, good_width, good_hight, orientations,
		pixels_per_cell, cells_per_block, feature_size);
	return out;
}
