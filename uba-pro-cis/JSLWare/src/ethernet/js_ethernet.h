/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: Ethernet driver														*/
/* Comment:																		*/
/* 	・																			*/
/*==============================================================================*/

#ifndef __J_ETHERNET__
#define __J_ETHERNET__


/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	ETH_VER						100

/*==============================================================================*/
/* ハンドル定義																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} ETH_HANDLE;

/*==============================================================================*/
/* 各種デファイン(変更可能)														*/
/*==============================================================================*/
#define	ETH_ACCESS_TOUT				(1000*4)	/* タイムアウト */
#define	ETH_VALID_DATA_SIZE			1514		/* dst + src + len + data */
#define	ETH_TX_BUF_SIZE				1516
#define	ETH_RX_BUF_SIZE				1520


/*==============================================================================*/
/* API引数用定義																*/
/*==============================================================================*/

/* eth_status */
#define	ETH_STAT_LINK_DISCONNECT	0		/* 切断通知 */
#define	ETH_STAT_LINK_CONNECT		1		/* 接続通知 */
#define	ETH_STAT_RECV_NOTIFY		2		/* 受信通知 */

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
/* 外部で宣言するコンフィグレーション定数										*/
/*==============================================================================*/

/* CFG_ETH_PHYATTR */
#define	ETH_PHY_ATTR_GIGA			0x0001

extern const UINT32 CFG_ETH_SYSCLK[];	/* ペリフェラルシステムクロック周波数(Hz) */
extern const UINT8 CFG_ETH_PHYADDR[];	/* PHYアドレス */
extern const UINT16 *CFG_ETH_PHYCFG[];	/* PHYレジスタ設定 */
extern const UINT16 CFG_ETH_PHYATTR[];	/* PHY機能指定 */
extern UINT8 CFG_ETH_MACADDR[][6];		/* MACアドレス */
extern OSW_MEM_HANDLE *CFG_ETH_NC_HEAP;	/* 非キャッシュ領域ヒープ */


#endif /* __J_ETHERNET__ */









