/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: MMC driver															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_MMC__
#define __J_MMC__


/*==============================================================================*/
/* �o�[�W�������																*/
/*==============================================================================*/
#define	MMC_VER				100

/*==============================================================================*/
/* �n���h����`																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} MMC_HANDLE;

/*==============================================================================*/
/* �e��f�t�@�C��(�ύX�\)														*/
/*==============================================================================*/
#define	MMC_ACCESS_TOUT			(4000)		/* �^�C���A�E�g */
#define	MMC_ACCESS_TOUT_CNT		(10000000)	/* �^�C���A�E�g�J�E���^ */

/*==============================================================================*/
/* API�����p��`																*/
/*==============================================================================*/

typedef struct {
	UINT8 port;
} MMC_PARAM;

/* card_type */
#define	MMC_INFO_TYPE_UNKNOWN	0x00
#define	MMC_INFO_TYPE_MEM		0x01

typedef struct {
	UINT8 card_type;
	UINT32 sector_cnt;
} MMC_INFO;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
/* Mmc_ident->attr */
#define	MMC_ATTR_INS			0x01	/* �J�[�h�}���L�� */
#define	MMC_ATTR_WP				0x02	/* ���C�g�v���e�N�g�L�� */

/* Mmc_read(),Mmc_write() return */
#define	MMC_RWSTAT_OK			0		/* ����I�� */
#define	MMC_RWSTAT_ERR_CARD		1		/* �J�[�h����s�� */
#define	MMC_RWSTAT_ERR_DATA		2		/* �f�[�^�]���G���[ */
#define	MMC_RWSTAT_ERR_WP		3		/* ���C�g�v���e�N�g�G���[ */

INT8 Mmc_open( MMC_HANDLE *handle, MMC_PARAM *param );
void Mmc_close( MMC_HANDLE *handle );
INT8 Mmc_ident( MMC_HANDLE *handle, UINT8 attr, MMC_INFO *info );
INT8 Mmc_mem_read( MMC_HANDLE *handle, UINT8 *buf, UINT32 lba, UINT32 blk );
INT8 Mmc_mem_write( MMC_HANDLE *handle, UINT8 *buf, UINT32 lba, UINT32 blk );
INT8 Mmc_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* �O���Ő錾����R���t�B�O���[�V�����萔										*/
/*==============================================================================*/

extern const UINT32 CFG_MMC_SYSCLK[];		/* �y���t�F�����V�X�e���N���b�N���g��(Hz) */


#endif /* __J_MMC__ */









