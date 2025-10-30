#ifndef _HOG_H_
#define _HOG_H_

typedef unsigned char uchar;

#define PI   3.14159265358979323846
#define var  57.2957795131

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void img_resize_v3(u8* in_img, u8* out_img, 
                   u16 in_width, u16 in_height, u16 out_width, u16 out_height);

float *_computeHOG(uchar *src, u16 width, u16 hight,
                u16 orientations, u16 pixels_per_cell, u16 cells_per_block,
                u16 *feature_size);

float *computeHOG(uchar *src, u16 width, u16 hight, u16 *feature_size,
                u16 orientations, u16 pixels_per_cell, u16 cells_per_block,
                u16 new_width, u16 new_hight);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif/*_HOG_H_*/
