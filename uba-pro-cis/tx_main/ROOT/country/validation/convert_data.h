#ifndef _DATA_CONVERT_H_
#define _DATA_CONVERT_H_

#define VALI_DIR 4				//4����
#define MAX_DENOMI 100		//�ő����
#define NOMAL_THERSHOLD 200	//臒l 
#define CIR_RATIO_MULTIPLIER 1000		//�萔
#define CIR_CHECK_POINT_COUNT 121	//�ő�|�C���g��



//cir�V���[�Y�p ���W���~�b�g�f�[�^
typedef struct
{
	s16  x;
	s16  y;
	s16 limit_min;	//臒l
	s16 limit_max;	//臒l
} ST_LIM_AND_POINTS;

typedef struct
{
	u8 point_num;		//�v�f��
	u8 blank[3];
	ST_LIM_AND_POINTS	*ppoint[CIR_CHECK_POINT_COUNT];//�|�C���g�f�[�^�̃p�����^
	
} ST_CIR_MODES;

//�ӕ�nn���W�p�����^�\����
typedef struct
{
	s8  x;		//x
	s8  y;		//y
	u16 side;	//�\��
	
} ST_NN_POINTS;

// �V�ӕʁE�V�t�B�b�g�l�X�p�p�����^�\����..ichijo
typedef struct
{
	s16 x_s;//�n�_x
	s16 y_s;//�n�_y
	s16 x_e;//�I�_x
	s16 y_e;//�I�_y
	float threshold1;//�������l1
	float threshold2;//�������l2
	float threshold3;//�������l3
	float threshold4;//�������l4
} ST_IMUF_POITNS;// IrMagUvFolding->IMUF

typedef struct
{
	s16 x_s;//�n�_x
	s16 y_s;//�n�_y
	s16 x_e;//�I�_x
	s16 y_e;//�I�_y
	float threshold;//�������l1
	u8 side;			//�\���t���O�F�O���\�A�P����
	u8 padding[3];//�p�f�B���O
	float reserved1;//�\��1
	float reserved2;//�\��2
}ST_UV_POINTS;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_note_x_boundary(void *p);
int get_note_y_boundary(void *p);
int get_mesh_x_boundary(int diameter, void* p);
int get_mesh_y_boundary(int diameter, void* p);
unsigned char get_pixel_value(int x, int y, unsigned char plane, void* p);
s32 get_mesh_data(s32 x, s32 y, s32 plane, s32 diameter_x , s32 diameter_y, u8* mask, u8 buf_num, float divide_num);
int get_mesh_data2(int x, int y, int plane, int buf_num);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
