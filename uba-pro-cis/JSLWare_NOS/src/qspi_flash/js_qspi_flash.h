/*==============================================================================*/
/* Copyright (C) 2012 JSL Technology. All right reserved.						*/
/* Tittle: QSPI Flash driver														*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_QSPI_FLASH__
#define __J_QSPI_FLASH__


/*==============================================================================*/
/* �o�[�W�������																*/
/*==============================================================================*/
#define	QSPI_FLASH_VER			102

/*==============================================================================*/
/* �n���h����`																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} QSPI_FLASH_HANDLE;


/*==============================================================================*/
/* �e��f�t�@�C��(�ύX�\)														*/
/*==============================================================================*/
#define	QSPI_FLASH_ACCESS_TOUT	(1000*10)	/* Read�A�N�Z�X�^�C���A�E�g */
#define	QSPI_FLASH_WRITE_TOUT	(1000*10)	/* Write�^�C���A�E�g */
#define	QSPI_FLASH_CERASE_TOUT	(60000*10)	/* �`�b�v�C���[�X�^�C���A�E�g */
#define	QSPI_FLASH_INTERVAL		(10)		/* �������ݒ��̘A��CPU��L���� */

/*==============================================================================*/
/* API�����p��`																*/
/*==============================================================================*/

/* status_cb_func()->stat */
#define	QSPI_FLASH_STAT_ERASING		0x01
#define	QSPI_FLASH_STAT_WRITEADDR	0x02
#define	QSPI_FLASH_STAT_ERASE_ERR	0xE1
#define	QSPI_FLASH_STAT_WRITE_ERR	0xE2

typedef struct {
	UINT8 port;
	void (*status_cb_func)( UINT8 stat, UINT32 addr, void *arg );
	void *cb_arg;
} QSPI_FLASH_PARAM;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/

typedef struct {
	UINT8 *buf;
	UINT32 addr;
	UINT32 len;
	UINT32 *byte_count;
} QSPI_BUF_INFO;

/* QSPI_Flash_Write()->erase */
#define	QSPI_FLASH_ERASE_NONE		0
#define	QSPI_FLASH_ERASE_AUTO		1
#define	QSPI_FLASH_ERASE_ONLY		2

INT8 QSPI_Flash_open( QSPI_FLASH_HANDLE *handle, QSPI_FLASH_PARAM *param, UINT32 *size );
void QSPI_Flash_close( QSPI_FLASH_HANDLE *handle );
INT8 QSPI_Flash_Read( QSPI_FLASH_HANDLE *handle, QSPI_BUF_INFO *buf );
INT8 QSPI_Flash_Write( QSPI_FLASH_HANDLE *handle, QSPI_BUF_INFO *buf, INT8 erase );
INT8 QSPI_Flash_ChipErase( QSPI_FLASH_HANDLE *handle );
INT8 QSPI_Flash_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* �O���Ő錾����R���t�B�O���[�V�����萔										*/
/*==============================================================================*/

/* CFG_QSPI_ATTR */
#define	QSPI_ATTR_TYPE_MICRON			0x0003
#define	QSPI_ATTR_TYPE_SPANCION			0x0004
#define QSPI_ATTR_BULK_ERACE_CODE(n)	((UINT32)n<<24)

extern const UINT32 CFG_QSPI_SYSCLK[];					/* �y���t�F�����V�X�e���N���b�N���g��(Hz) */
extern const UINT32 CFG_QSPI_CLK[];						/* QSPI�ڕW�r�b�gCLK���g�� */
extern const UINT32 CFG_QSPI_ATTR[];					/* QSPI Flash���샂�[�h */
extern const UINT32 CFG_QSPI_FLASH_PAGE_SIZE[];			/* QSPI Flash�y�[�W�T�C�Y */
extern const UINT32 *CFG_QSPI_FLASH_INFO[];				/* QSPI Flash�Z�N�^�[��� */


#endif /* __J_QSPI_FLASH__ */






