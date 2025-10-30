#ifndef	_HOLE_INCLUDE_H
#define	_HOLE_INCLUDE_H

/****************************************************************************/
/**
 * @file hole.h
 * @brief �����m ver2.5
 * @date 2020/8/18 Created.
 */
/****************************************************************************/

/*
	 2020/9/7

*/


//�ݒ�l
#define HOLE_ROI_BUFF 800
#define HOLE_ROI_LIST 200
#define HOLE_MAX_COUNT 8
#define EXCLUDE_COUNT 5
#define CONTOUR_SEARCH_MOVE_LIMIT 2000
#define MOVE_FOR_CONTOUR_MOVE_LIMIT 50
#define INI_MAX -4096//0xf000
#define INI_MIN 4095//0x0fff
#define HOLE_AREA_LIMIT 300

//�G���[�R�[�h
#define ERR_CONTOUR_LOST 0x1814//6164
#define ERR_CONTOUR_LOOP 0x1815//6165
#define ERR_LIST_OVER 0x1817//6167

//���m����
#define LEVEL_OVER 0x01
#define TOTAL_OVER 0x02
#define TOO_MANY_HOLE 0x03
#define TOTAL_LIMIT_OVER 0x04

//
#define NORMAL_HOLE 0
#define UNFIT_HOLE 1
#define	ERR_HOLE -1

#define OUTLINE_FLAG_HOLE 1
#define ONE_DOT_FLAG_HOLE 2
#define IN_HOLE 1
#define OUT_HOLE 0


typedef struct
{
	s16 exclude_point_x1;
	s16 exclude_point_x2;//x1��x2
	s16 exclude_point_y1;
	s16 exclude_point_y2;//y1��y2

} ST_EXCLUDE_HOLE;

typedef struct
{
	u8 t_plane;//�g�p���铧�߃v���[���ԍ��@���ߗ�
	u8 threshold;//臒l
	u8 skip;//�֊s���m�T���Ԋu dot
	u8 edge_margin;	//�O�`�}�[�W�� mm
	float x_step;//�呖�������T���Ԋu mm
	float y_step;//�����������T���Ԋu mm
	
	u16 threshold_level_area;//���Ɣ��肷��ʐϊ mm^2
	u16 threshold_total_area;//���v���o�ʐϊ mm^2
	u16 threshold_min_area;//�ŏ����o�ʐϊ mm^2
	
	u8	exclude_area_count;	//���O�͈͌�
	ST_EXCLUDE_HOLE exclude_hole[EXCLUDE_COUNT];//���O�͈͍��W�̍\����
	
	//���ԏ��
	s16 hole_min_list[HOLE_ROI_LIST];//ROI�p�z��
	s16 hole_max_list[HOLE_ROI_LIST];//ROI�p�z��
	s16 hole_max_buff[HOLE_ROI_BUFF];//�ő�x���W�L�^�p�z��
	s16 hole_min_buff[HOLE_ROI_BUFF];//�ŏ�x���W�L�^�p�z��

	u8 result;//�o�͌���
	u8 hole_count;//����
	u16 total_hole_area;//���v���o�ʐ�
	u16 max_hole_area;//���o�ő�ʐ�
	u16 err_code;//�G���[�R�[�h
	s16 holes[HOLE_MAX_COUNT];//�e���̖ʐ�
	u8 level;//���x��
	u8 padding[3];

} ST_HOLE;

typedef struct
{
	ST_HOLE* ho;

	ST_SPOINT t_spt;//���߃v���[��
	ST_EXCLUDE_HOLE outline;//���_���W
	ST_EXCLUDE_HOLE outline_margin;//�֊s�T�����̗􂯁E�p�܂ꔻ��J�n�ʒu�p
	ST_EXCLUDE_HOLE exlude[EXCLUDE_COUNT];//���O�͈͍��W

	ST_BS* pbs;
	ST_NOTE_PARAMETER* para;
	
	float mmfordot;//mm����dot��
	float mm2fordot;//mm2����dot��

	s8 x_step;//x�����Ԋu
	s8 y_step;//y�����Ԋu
	u8 buf;
	u8 skip;//�֊s�T���Ԋu

	s16 margin;
	s16 y_offset;//�_�����W��z��ɋL�^����

	u16 total_judge;//�ő升�v�ʐ�
	u16 min_area_dot;//�ŏ����m�ʐ�

	u16 total_limit;
	u16 res_total_judge;//���m�������v�ʐ�

	u8 res_hole_count;//���m�������̌�
	u8 padding;
	s16 roi_list_last;//�L�^����ROI�z��̍Ō�̗v�f�ԍ�

	s16 y_max_list[HOLE_MAX_COUNT];//�L�^���������Ƃ̍ő�y���W
	s16 y_min_list[HOLE_MAX_COUNT];//�L�^���������Ƃ̍ŏ�y���W
	s16* x_min_list[HOLE_MAX_COUNT];//�L�^���������Ƃ̍ő�x���W
	s16* x_max_list[HOLE_MAX_COUNT];//�L�^���������Ƃ̍ŏ�x���W
	s16	list_num[HOLE_MAX_COUNT];//�L�^����ROI�z��̍ŏ��̗v�f�ԍ�

} ST_HOLE_SETTING;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 hole(u8 buf_num, ST_HOLE* ho);

	s8 hole_lsearch(ST_HOLE_SETTING* setting);
	s8 contour_setup(ST_HOLE_SETTING* setting, s16 lx, s16 ly);
	s8 contour_lsearch(ST_HOLE_SETTING* setting, s16 x_start, s16 y_start, s16* ly_max, s16* ly_min, s8 check_count);
	s8 move_for_contour(ST_HOLE_SETTING* setting, s16* lx, s16* ly);

	u8 hole_thershold(ST_HOLE_SETTING* setting, s16 lx, s16 ly, u8 thr);
	s8 check_outline(ST_HOLE_SETTING* setting, s16 lx, s16 ly, s8 vnum);
	s8 check_exlude(ST_EXCLUDE_HOLE* exclude, s16 lx, s16 ly, u8 num);
	s8 check_area(ST_EXCLUDE_HOLE* rectangle, s16 lx, s16 ly);
	s16 set_max_remainder_0(s16 value, s8 step);
	s16 set_min_remainder_0(s16 value, s8 step);
	void initialize_hole_roi_buff(s16 from, s16 to, ST_HOLE* ho);
	void set_hole_lvertex(ST_HOLE_SETTING* setting, ST_EXCLUDE_HOLE* outline, ST_EXCLUDE_HOLE* outline_margin);

	u8 hole_level_detect(ST_HOLE* ho, ST_HOLE_SETTING* setting);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*  */

