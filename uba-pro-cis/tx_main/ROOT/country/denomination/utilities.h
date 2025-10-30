#ifndef _DENOMI_CPP_UTILITIES_H_
#define _DENOMI_CPP_UTILITIES_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void add_grid(  unsigned char *src, u16 h, u16 w, u16 row, u16 col,
                unsigned char *dst, u16 num_block_h, u16 num_block_w);
u8 get_pixel(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, s16 x_coordinates, s16 y_coordinates, u8 plane[]); // get ROI denomi

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif/*_DENOMI_CPP_UTILITIES_H_*/
