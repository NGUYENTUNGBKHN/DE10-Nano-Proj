#ifndef	_SJ_H
#define	_SJ_H

#define ERR_SIZE_MISS_MACH	0x1010				//�T�C�Y���s��v

typedef struct//���ʂ̃e���v���[�g
{
	//���̓p�����^
	u16 note_x_size;							//�D�T�C�Y�@
	u16 note_y_size;							//�D�T�C�Y
	u16 thr_note_x_size_min;					//�D�̉���(����)	�P�ʂ͇o
	u16 thr_note_y_size_min;					//�D�̉���(�呖���j
	u16 thr_note_x_size_max;					//�D�̏��(����)
	u16 thr_note_y_size_max;					//�D�̏��(�呖���j

	//�o�̓p�����^
	u32 res;

} ST_SJ;




#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void size_judge(ST_SJ* st);
u16 size_judge_demo(u8 bn);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NN_H */
