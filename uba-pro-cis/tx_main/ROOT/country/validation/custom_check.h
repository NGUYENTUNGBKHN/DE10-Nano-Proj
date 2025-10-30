#ifndef	_CUSTOMCHECK_INCLUDE_H
#define	_CUSTOMCHECK_INCLUDE_H

/****************************************************************************/
/**
 * @file customcheck.h
 * @brief �J�X�^���`�F�b�N
 * @date 2023/02/16�@ver1.1
 */
 /****************************************************************************/

#define CUSTOMCHECK_CALCULATION_NUM 16
#define CUSTOMCHECK_USEDPLANE 8
#define CUSTOMCHECK_CALCULATION_USEDPLANE_NUM 5
#define CUSTOMCHECK_PARA_NUM 20
#define CUSTOMCHECK_2CL_BIAS 127
#define CUSTOMCHECK_3CL_RATIO 255
#define CUSTOMCHECK_X_SKIP 2
#define CUSTOMCHECK_Y_SKIP 2

#define CUSTOMCHECK_CF_JUDGE_LEVEL 40

typedef struct//�J�X�^���`�F�b�N�@�v�Z���e�\����
{
	s16 type;//�����ށ@0�F�G���A�����@1�F1�F���ρ@2�F2�F���@3�F3�F��
	s16 margin;//
	s16 min;//
	s16 max;//
	s8 usedplane[CUSTOMCHECK_CALCULATION_USEDPLANE_NUM];//�v�Z�Ŏg�p����v���[��
	s8 padding[3];

}CUSTOMCHECK_PARA_CAL;

typedef struct//�J�X�^���`�F�b�N�@�v���[���\����
{
	s16 plane;//�`�F�b�N����v���[��
	s16 level;//臒l

}CUSTOMCHECK_PARA_USEDPLANE;

typedef struct //�J�X�^���`�F�b�N�@�G���A�\����
{
	s16 startx;//����
	s16 starty;
	s16 endx;//�E��
	s16 endy;
	
}CUSTOMCHECK_PARA_AREA;

//�J�X�^���`�F�b�N�p�����^�\����
typedef struct
{
	CUSTOMCHECK_PARA_AREA cca;
	CUSTOMCHECK_PARA_USEDPLANE ccu[CUSTOMCHECK_USEDPLANE];
	CUSTOMCHECK_PARA_CAL ccc[CUSTOMCHECK_CALCULATION_NUM];

	u8 usedplane_num;
	u8 cal_num;	
	
}CUSTOMCHECK_PARA;

//�J�X�^���`�F�b�N�\����
typedef struct
{
	CUSTOMCHECK_PARA para[CUSTOMCHECK_PARA_NUM];
	u8 paranum;

	//����
	u16 judge[CUSTOMCHECK_PARA_NUM];//�G���A���Ƃ̔��茋��
	u8 result_num;//�ŏ��o�̓��x���̔���ԍ�
	u8 result_area;//�ŏ��o�̓��x���̃G���A
	u8 sum_num;//CF����ɂȂ������萔
	u8 sum_area;//CF����ɂȂ����G���A��
	s32 value;//�ŏ��o�͌v�Z����
	u8 level;//�ŏ��o�̓��x��

}ST_CUSTOMCHECK_PARA;

//�e���v���[�g����p�����^���󂯎��
typedef struct
{
	s16 startx;
	s16 starty;
	s16 endx;
	s16 endy;
	u16	usedplane_num;
	u16	cal_num;
	u32	usedplane_ofs;
	u32	cal_ofs;

}ST_CUSTOMCHECK;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

u8 customcheck_judge(s32 result, s16 max, s16 min, s16 margin);
s32 customcheck_calculation(u32* sum, u32 usedpoint, u8 type, s8* usedplane, u32 area);
s16 customcheck(u8 buf_num, ST_CUSTOMCHECK_PARA* cc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
