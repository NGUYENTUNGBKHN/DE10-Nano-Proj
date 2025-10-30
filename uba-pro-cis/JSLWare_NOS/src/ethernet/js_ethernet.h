/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: Ethernet driver														*/
/* Comment:																		*/
/* 	�E																			*/
/*==============================================================================*/

#ifndef __J_ETHERNET__
#define __J_ETHERNET__


/*==============================================================================*/
/* �o�[�W�������																*/
/*==============================================================================*/
#define	ETH_VER						100

/*==============================================================================*/
/* �n���h����`																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} ETH_HANDLE;

/*==============================================================================*/
/* �e��f�t�@�C��(�ύX�\)														*/
/*==============================================================================*/
#define	ETH_ACCESS_TOUT				(1000*4)	/* �^�C���A�E�g */
#define	ETH_VALID_DATA_SIZE			1514		/* dst + src + len + data */
#define	ETH_TX_BUF_SIZE				1516
#define	ETH_RX_BUF_SIZE				1520


/*==============================================================================*/
/* API�����p��`																*/
/*==============================================================================*/

/* eth_status */
#define	ETH_STAT_LINK_DISCONNECT	0		/* �ؒf�ʒm */
#define	ETH_STAT_LINK_CONNECT		1		/* �ڑ��ʒm */
#define	ETH_STAT_RECV_NOTIFY		2		/* ��M�ʒm */

typedef struct {
	UINT8 port;
	void (*cb_func)( UINT8 eth_status, void *arg );
	void *cb_arg;
} ETH_PARAM;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
INT8 Ethernet_open( ETH_HANDLE *handle, ETH_PARAM *param, UINT8 *mac_adr );
void Ethernet_close( ETH_HANDLE *handle );
INT8 Ethernet_enable( ETH_HANDLE *handle, INT8 enable );
UINT16 Ethernet_send( ETH_HANDLE *handle, UINT8 *buf, UINT16 len );
UINT16 Ethernet_recv( ETH_HANDLE *handle, UINT8 *buf, UINT16 len );
INT8 Ethernet_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* �O���Ő錾����R���t�B�O���[�V�����萔										*/
/*==============================================================================*/

/* CFG_ETH_PHYATTR */
#define	ETH_PHY_ATTR_GIGA			0x0001

extern const UINT32 CFG_ETH_SYSCLK[];	/* �y���t�F�����V�X�e���N���b�N���g��(Hz) */
extern const UINT8 CFG_ETH_PHYADDR[];	/* PHY�A�h���X */
extern const UINT16 *CFG_ETH_PHYCFG[];	/* PHY���W�X�^�ݒ� */
extern const UINT16 CFG_ETH_PHYATTR[];	/* PHY�@�\�w�� */
extern UINT8 CFG_ETH_MACADDR[][6];		/* MAC�A�h���X */
extern OSW_MEM_HANDLE *CFG_ETH_NC_HEAP;	/* ��L���b�V���̈�q�[�v */


#endif /* __J_ETHERNET__ */









