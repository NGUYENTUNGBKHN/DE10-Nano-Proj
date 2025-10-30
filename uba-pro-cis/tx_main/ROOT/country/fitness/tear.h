/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* �{�\�t�g�E�F�A�Ɋ܂܂��\�[�X�R�[�h�ɂ͓��{���K�@�B������ЌŗL��       */
/* ��Ƌ@�����܂�ł��܂��B                                               */
/* �閧�ێ��_�񖳂��Ƀ\�t�g�E�F�A�Ƃ����Ɋ܂܂����̑S�̂������͈ꕔ��   */
/* ���J���������s���܂���B                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file tear.h
 * @brief �􂯌��m
 * @date 2019/06/23 Created.
 */
/****************************************************************************/

#ifndef	_TEAR_H
#define	_TEAR_H

//�p�����^
#define LIMIT_TEAR_COUNT		20		// ���ʂ��i�[����z��̗v�f��
#define _1mm					8		// �}�[�W���l
#define LIMIT_CLIMB				5		// �Ǐ��ŏ����m�̍ۏ�֐i�߂��
#define MAX_WHILE				10000	// �X�L�������[�v�̍ő��
#define NOT_SERCH_VERTEX_AROUND	39		// ���_���W�̎���5�o�����m�ΏۊO�Ƃ���@dot�Ŏw�� 

//each
#define RES_WIDTH_BELOW_STANDARD				0x01	//����臒l�ȉ�
#define RES_DEPTH_BELOW_STANDARD				0x02	//�[������臒l��
#define RES_DEPTH_AND_WIDTH_PASS				0x03	//臒l�𖞂��� 臒l�ȏ�̊p�􂯂�����Ɋ܂܂��
#define RES_DEPTH_PASS_SCANING					0x04	//�X�L��������臒l�𒴂���
#define RES_OUT_OF_AREA_DEPTH_BELOW_STANDARD	0x05	//�X�L�������ɕ������Ŏ����G���A����o������𖞂������Ȃ��B	
														//�������p�􂯂��p�܂��ʕs�\
#define RES_OUT_OF_AREA_WIDTH_AND_DEPTH_PASS	0x07	//�X�L�������ɕ������Ŏ����G���A����o������𖞂����Ă���B
														//�p�􂯊p�܂�̋�ʉ\�@�T�C�Y���傫���ꍇ�p�܂ꌟ�m�ŋ�ʂ��Ă��邩��
#define RES_FALSE_DETECT_OF_DOG_EAR				0x06	//�p�܂ꕔ��������ăX�L��������								
#define RES_SCAN_OUT_OF_AREA_WIDTH				0x08	//�X�L�������ɕ������Ŏ����G���A����o��
#define RES_SCAN_OUT_OF_AREA_DEPTH				0x09	//�X�L�������ɏc�����Ŏ����G���A����o��
#define RES_SCAN_NOT_REFERENCE_AREA				0x0A	//�X�L�������ɏ��O�G���A���X�L���������B


//fitness
#define	RES_TOTAL_DEPTH				0x01	//���v�̗􂯂�臒l�ȏ�
#define	RES_SPECIFIC_PART			0x02	//����̕����ւ̗􂯂�臒l�ȏ�
#define	RES_TEAR_PENETRATION		0x03	//�������ђʂ��Ă���@�Ӓn�����̋^�f������
//#define RES_DEPTH_AND_WIDTH_PASS	0x03	//臒l�𖞂���

#define	ERR_MANY_DETECTION_TEARS	0x01	//�����̗􂯂���������
#define ERR_MAX_WHILE_OVER			0x02	//while�̍ő�񐔂𒴂���


enum tear_type	//�`��
{
		vertical,		//����
		horizontal,		//����
		diagonal,		//�΂�
		PENETRATION,
		not_required,	//�s���������ꂪ�������܂�Ă���Ȃ��臒l�ȏ�ƂȂ�
		
};

enum tear_side	//��
{
		upper_side,	//���
		lower_side,	//����
		left_side,	//����
		right_side,	//�E��
};


enum corner_tear
{
	not_corner,
	upper_left_tr,
	lower_left_tr,
	upper_right_tr,
	lower_right_tr,
};




typedef struct
{


	//���͏��	�֐��O�Őݒ肷��p�����^�@�itemplate�j
	float threshold_width;				//���m���ׂ��􂯂̕��@		�@�P�ʇo�Ŏw�肷��B
	float threshold_vertical_depth;		//���m���ׂ��􂯂̐[��(����)�@�P�ʇo�Ŏw�肷��B
	float threshold_horizontal_depth;	//���m���ׂ��􂯂̐[��(����)�@�P�ʇo�Ŏw�肷��B
	float threshold_diagonal_depth;		//���m���ׂ��􂯂̐[��(�΂�)�@�P�ʇo�Ŏw�肷��B
	//float	threshold_total_depth;		//���m�����􂯂̐[���̍��v�l��臒l�@�P�ʇo�Ŏw�肷��B
	u8 width_detection;					//�����m�����s���邩�ۂ��@�P�F���s�@�O�F���s���Ȃ�
	u8 total_depth_judge;				//�[���̍��v�l�Ŕ��肷��@�P�F���s�@�O�F���s���Ȃ�
	u8 plane;							//�p����v���[��

	//���ԏ��	
	u8 buf_num;							//�������̃o�b�t�@�ԍ����i�[����
	s16 corner_triangle_vertex_1[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//�p�܂�O�p�`�̐܂ꏉ�߂̍��W���L�^����
																			//����_���炘���ω����Ă��钸�_����L�^����
	s16 corner_triangle_vertex_2[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//�p�܂�O�p�`�̐܂ꏉ�߂̍��W���L�^����

	s16 threshold_width_dot;			//���m���ׂ��􂯂̕��@�P�ʂ��o����dot�ɕϊ���������
	s16 threshold_vertical_depth_dot;	//���m���ׂ��􂯂̐[��(����)�@�P�ʂ��o����dot�ɕϊ���������
	s16 threshold_horizontal_depth_dot;	//���m���ׂ��􂯂̐[��(����)�@�P�ʂ��o����dot�ɕϊ���������
	s16 threshold_diagonal_depth_dot;	//���m���ׂ��􂯂̐[��(�΂�)�@�P�ʂ��o����dot�ɕϊ���������
	

	s16	tear_width_coordinate_start;	//�􂯂͂��߂�x���W���L�^����B��Ӊ��ӁF�E���@���ʁF�㑤���L�^����
	s16	tear_width_coordinate_end;		//�􂯂�����x���W���L�^����B��Ӊ��ӁF�����@���ʁF�������L�^����

	s16 edge_coordinates_detected[VERTEX_POINT_NUM];	//���ۂɌ��m�����l���̃G�b�W�����W���L�^����

	u8	temporary_depth_thr;			//�֊s�X�L�����̍ۂɗp���鉼�􌈒��臒l
	u8	temporary_depth_thr_diff_x;		//�֊s�X�L�����̍ۂɗp���鉼�􌈒��臒l �P�O�Ƃ̍���
	u8	temporary_depth_thr_diff_y;		//�֊s�X�L�����̍ۂɗp���鉼�􌈒��臒l �P�O�Ƃ̍���
	u8 search_pitch;					//�֊s�����̃X�L�����̍ۂ̃X�L�����s�b�`�@
	u8  width_search_num_x;				//�����m�̍ۂɉ������
	u8  width_search_num_y;				//�����m�̍ۂɉ������
	u8 penetration;						//�O�F�ʏ팔�@�P�F�c�ђʌ��@�Q�F���ђʌ�
	u8 padding2;

	s16 note_size_x;
	s16 note_size_y;

	s16 a_third_note_size_x;
	s16 a_third_note_size_y;

	u8 dog_ear_res[4];					//�p�܂ꌟ�m�̌��ʂ��i�[����

	s16 not_seen_areas[MAX_NOT_SEEN_AREA_COUNT][X_Y];	//����Ȃ��G���A�͈̔�
	u8	not_seen_area_count;							//����Ȃ��G���A�̐�
	u8 padding3[3];


	//�o�͏��
	float res_tear_width[LIMIT_TEAR_COUNT];			//���m�����􂯂̕�
	float res_tear_depth[LIMIT_TEAR_COUNT];			//���m�����􂯂̐[��
	float res_tear_depth_total;						//���m�����􂯂̐[���̍��v�l
	u8 res_tear_type[LIMIT_TEAR_COUNT];				//���m�����􂯂̌`��
	u8 res_corner_tear_type[LIMIT_TEAR_COUNT];		//���m�����p�􂯂̏ꏊ
	u16 res_each_judge_reason[LIMIT_TEAR_COUNT];	//���m�����􂯂̗��R�@
	u8 res_tear_count;								//���m�����􂯂̐�
	u8 res_judge_tear;								//���m�����􂯂̒���臒l�𒴂�����̂��������B�@1�F�L�@0�F��
	u16 res_judge_reason;							//���m�����􂯂̗��R�@
	u16 res_err_code;								//�G���[�R�[�h
	u8	level;
	u8 padding4;


	//float	res_total_depth;						//���m�����􂯂̐[���̍��v�l�@�P�ʇo�Ŏw�肷��B




} ST_TEAR;

//�e���_�̂T�o���T�o�̍��W���v�Z���L�^����
typedef struct
{
	s16 area1_left_up_x;
	s16 area1_left_up_y;

	s16 area1_right_down_x;
	s16 area1_right_down_y;

	s16 area2_left_up_x;
	s16 area2_left_up_y;
			
	s16 area2_right_down_x;
	s16 area2_right_down_y;

} ST_NOT_SECURITY_AREA;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	tear(u8 buf_num, ST_TEAR *tr);
	void ini_tear_param(ST_TEAR *tr);

	void get_dog_ear_res( ST_TEAR *tr );

	void search_tear_contour(ST_VERTEX pver , ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param, ST_TEAR *tr);

	u8 search_tear_depth_upper_and_lower(ST_SPOINT spt ,ST_VERTEX pver ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y ,s16 vartex_y ,s8 location  , s16 limit_min ,s16 limit_max, ST_TEAR *tr );
	u8 search_tear_width_upper_and_lower(ST_SPOINT spt ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y , s16 limit_min ,s16 limit_max, s8 increment ,u8* corner_flg, ST_TEAR *tr);
	void depth_scan_module_upper_and_lower(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y, u8 res[] ,s8 inc_num);

	u8 search_tear_depth_left_and_right(ST_SPOINT spt ,ST_VERTEX pver ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y ,s16 vartex_x ,s8 location  , s16 limit_min ,s16 limit_max, ST_TEAR *tr);
	u8 search_tear_width_left_and_right(ST_SPOINT spt ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y , s16 limit_min ,s16 limit_max, s8 increment ,u8* corner_flg , ST_TEAR *tr);
	void depth_scan_module_left_and_right(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x ,s16 y, u8 res[] ,s8 inc_num);

	u8	tear_total_depth(ST_TEAR *tr);

	//u8  get_temporary_depth(ST_BS *pbs);
	//u8  get_width_search_num(ST_BS *pbs);
	//s16 get_search_pitch(void);

	void get_cof(ST_BS *pbs, ST_TEAR *tr);

	void matching_tear_res_and_dog_ear_res(s16 x, s16, ST_TEAR *tr);

	u8	tear_level_detect(ST_TEAR *tr);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TEAR_H */
