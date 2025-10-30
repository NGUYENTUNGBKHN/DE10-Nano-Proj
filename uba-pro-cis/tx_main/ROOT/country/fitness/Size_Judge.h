#ifndef	_SJ_H
#define	_SJ_H

#define ERR_SIZE_MISS_MACH	0x1010				//サイズが不一致

typedef struct//識別のテンプレート
{
	//入力パラメタ
	u16 note_x_size;							//札サイズ　
	u16 note_y_size;							//札サイズ
	u16 thr_note_x_size_min;					//札の下限(搬送)	単位は㎜
	u16 thr_note_y_size_min;					//札の下限(主走査）
	u16 thr_note_x_size_max;					//札の上限(搬送)
	u16 thr_note_y_size_max;					//札の上限(主走査）

	//出力パラメタ
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
