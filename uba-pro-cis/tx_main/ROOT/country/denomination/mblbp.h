#ifndef _MBLBP_H_
#define _MBLBP_H_
//#include "config_structure.h"

//#define EXT
//#include "../../common/global.h"
typedef unsigned char uchar;


struct Mat
{
    uchar *data;
	u16 width;
	u16 height;
};

struct Rect
{
	u16 x;
	u16 y;
	u16 width;
	u16 height;
};

struct Size
{
	u16 width;
	u16 height;
};

typedef struct Mat Mat;
typedef struct Rect Rect;
typedef struct Size Size;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

u16 inImage(u16 u, u16 v, u16 limit_u, u16 limit_v);

u16 getValueOfBinary3(u16 bit[]);

float *getMblbpFeature(Mat img, 
                       Rect rect, 
                       Size gridSize, 
                       Size blockSize, 
                       Size cellSize);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif/*_MBLBP_H_*/
