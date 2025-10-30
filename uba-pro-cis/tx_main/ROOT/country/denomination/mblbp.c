#include <stdio.h>
#include <stdlib.h>


#define EXT
#include "../common/global.h"
//#include "../../ocr/cod_malloc.h"
#include "mblbp.h"

u16 inImage(u16 u, u16 v, u16 limit_u, u16 limit_v) {
	return (0 <= u) && (u < limit_u) && (0 <= v) && (v < limit_v);
}


u16 getValueOfBinary3(u16 bit[]) {
	return	(bit[1]) + (bit[2] << 1) + (bit[4] << 2) + (bit[7] << 3)
		+ (bit[6] << 4) + (bit[5] << 5) + (bit[3] << 6) + (bit[0] << 7);
}

unsigned char getValueOfBinary4(unsigned char bit[]) {
	return	(bit[1]) + (bit[2] << 1) + (bit[4] << 2) + (bit[7] << 3)
		+ (bit[6] << 4) + (bit[5] << 5) + (bit[3] << 6) + (bit[0] << 7);
}


float *getMblbpFeature(Mat img, Rect rect, Size gridSize, Size blockSize, Size cellSize) {
	u16 resSize = rect.width * rect.height;
	float *res = (float*)malloc_float(sizeof(float) * resSize);
	u16 xCell = blockSize.width / cellSize.width, yCell = blockSize.height / cellSize.height;
	unsigned char * imgData = img.data;
	u16 imgWidth = img.width, imgHeight = img.height;
	u16 rectHeight = rect.height, rectWidth = rect.width;
	u16 rectX = rect.y, rectY = rect.x;
	u16 cellHeight = cellSize.height, cellWidth = cellSize.width;
	u16 tmp[8];
	float values[9];
	u16 nbCell = xCell * yCell;
	u16 val = 0, dem = 0;
	u16 u1 = 0;
	u16 v1 = 0;
	u16 i = 0;
	u16 j = 0;
	u16 xBegin = 0;
	u16 yBegin = 0;
	u16 xEnd = 0;
	u16 yEnd = 0;
	u16 x = 0;
	u16 y = 0;
	u16 id = 0;
	u16 gridHeight = 0;
	u16 gridWidth = 0;
	u16 feature_size = 0;
	float *value;
	float *cnt;
	u16 xGrid = 0;
	u16 yGrid = 0;
	u16 ind = 0;

	for (u1 = 0; u1 < rectHeight; ++u1)
		for (v1 = 0; v1 < rectWidth; ++v1) {
			for (i = 0; i < xCell; ++i)
				for (j = 0; j < yCell; ++j) {
					xBegin = rectX + u1 - cellHeight / 2 + i*cellHeight, xEnd = xBegin + cellHeight;
					yBegin = rectY + v1 - cellWidth / 2 + j*cellWidth, yEnd = yBegin + cellWidth;
					val = 0;
					dem = 0;
					for (x = xBegin; x < xEnd; ++x)
						for (y = yBegin; y < yEnd; ++y)
							if (inImage(x, y, imgHeight, imgWidth)) {
								val += imgData[x * imgWidth + y];
								++dem;
							}
					values[i * yCell + j] = (float) val / dem;
				}
			id = 0;
			for (i = 0; i < nbCell; ++i)
				if (i != 4) {
					if (values[i] > values[4]) tmp[id++] = 1;
					else tmp[id++] = 0;
				}
			res[u1*rectWidth + v1] = (float)getValueOfBinary3(tmp) / 256;
		}

	gridHeight = rect.height / gridSize.height, gridWidth = rect.width / gridSize.width;
	feature_size = gridSize.height * gridSize.width;
	value = (float*)malloc_float(sizeof(float) * feature_size);
	cnt = (float*)malloc_float(sizeof(float) * feature_size);

	for (i = 0; i < feature_size; i++)
	{
		value[i] = 0.0;
		cnt[i] = 0.0;
	}

	for (i = 0; i < resSize; ++i) {
		x = i / rect.width;
		y = i - x * rect.width;
		xGrid = x / gridHeight;
		yGrid = y / gridWidth;
		if (xGrid >= gridSize.height || yGrid >= gridSize.width) continue;
		ind = xGrid * gridSize.width + yGrid;
		value[ind] += res[i];
		++cnt[ind];
	}

	for (i = 0; i < feature_size; ++i) {
		value[i] /= cnt[i];
	}

	free_proc(res);
	free_proc(cnt);

	return value;
}
