/*==============================================================================*/
/* Copyright (C) 2011 JSL Technology. All right reserved.						*/
/* Tittle: OS Wrraper API header(Stellaris SYS/BIOS)							*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_OSWIF_ENV__
#define __J_OSWIF_ENV__


/*==============================================================================*/
/* �G���[�g���[�X																*/
/*==============================================================================*/
#define	DBG_ERR_ALL_ENABLE		0

/*==============================================================================*/
/* �݊����}�N��																	*/
/*==============================================================================*/
extern int osw_printf( const INT8 *format, ... );


/*==============================================================================*/
/* �n���h����`																	*/
/*==============================================================================*/

/* �^�X�N�n���h�� */
typedef struct {
	void *hdl;
} OSW_TSK_HANDLE;

/* ���b�Z�[�W�L���[�n���h�� */
typedef struct {
	void *post_hdl;
	void *pend_hdl;
	UINT32 *q_mem;
	UINT32 post_idx;
	UINT32 pend_idx;
	UINT32 q_total;
} OSW_MSG_HANDLE;

/* �Z�}�t�H�n���h�� */
typedef struct {
	void *hdl;
} OSW_SEM_HANDLE;

/* �C�x���g�t���O�n���h�� */
typedef struct {
	void *hdl;
} OSW_EVT_HANDLE;

/* ���荞�݃n���h�� */
typedef struct {
	void *hdl;
	UINT16 interrupt_id;
} OSW_ISR_HANDLE;

/* �q�[�v�n���h�� */
typedef struct {
	void *hdl;
	UINT32 total;
	UINT32 free;
	UINT32 max_use;
} OSW_MEM_HANDLE;


#endif /* __J_OSWIF_ENV__ */









