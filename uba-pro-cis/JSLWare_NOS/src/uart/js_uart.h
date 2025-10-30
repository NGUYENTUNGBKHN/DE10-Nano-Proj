/*==============================================================================*/
/* Copyright (C) 2011 JSL Technology. All right reserved.						*/
/* Tittle: UART driver															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_UART__
#define __J_UART__


/*==============================================================================*/
/* �o�[�W�������																*/
/*==============================================================================*/
#define	UART_VER				100

/*==============================================================================*/
/* �n���h����`																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} UART_HANDLE;

/*==============================================================================*/
/* �e��f�t�@�C��(�ύX�\)														*/
/*==============================================================================*/
#define	UART_ACCESS_TOUT		(50)		/* �^�C���A�E�g */
#define	UART_ACCESS_TOUT_CNT	(1000000)	/* �^�C���A�E�g�J�E���^ */

/*==============================================================================*/
/* API�����p��`																*/
/*==============================================================================*/

/* recv_cb_func()->stat */
#define	UART_STAT_RECV			0x1
#define	UART_STAT_TRANSRDY		0x2

typedef struct {
	UINT8 port;
	void (*recv_cb_func)( UINT8 c, UINT8 stat, void *arg );
	void *cb_arg;
} UART_PARAM;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
INT8 Uart_open( UART_HANDLE *handle, UART_PARAM *param );
void Uart_close( UART_HANDLE *handle );
INT8 Uart_send( UART_HANDLE *handle, UINT8 c, INT8 wait );
INT8 Uart_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* �O���Ő錾����R���t�B�O���[�V�����萔										*/
/*==============================================================================*/

/* CFG_UART_FMT */
#define	UART_FMT_RXEN				0x0200
#define	UART_FMT_TXEN				0x0100
#define	UART_FMT_5BIT				0x0000
#define	UART_FMT_6BIT				0x0020
#define	UART_FMT_7BIT				0x0040
#define	UART_FMT_8BIT				0x0060
#define	UART_FMT_STOPBIT1			0x0000
#define	UART_FMT_STOPBIT2			0x0008
#define	UART_FMT_PARITY_ODD			0x0002
#define	UART_FMT_PARITY_EVEN		0x0006

extern const UINT32 CFG_UART_SYSCLK[];		/* �y���t�F�����V�X�e���N���b�N���g��(Hz) */
extern const UINT32 CFG_UART_BPS[];			/* UART�ڕWbps */
extern const UINT16 CFG_UART_FMT[];			/* UART�t�H�[�}�b�g */


#endif /* __J_UART__ */









