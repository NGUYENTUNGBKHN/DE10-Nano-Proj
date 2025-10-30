/*==============================================================================*/
/* Copyright (C) 2013 JSL Technology. All right reserved.						*/
/* Tittle: Ethernet driver														*/
/* Comment:																		*/
/* 	・																			*/
/*==============================================================================*/

#ifndef __J_ETHERNET_DEF__
#define __J_ETHERNET_DEF__

/*==============================================================================*/
/* ローカルデファイン															*/
/*==============================================================================*/
/* Option */
#define	ETH_TXBUF_CNT			1
#define	ETH_TXDESC_CNT			1
#define	ETH_RXDESC_CNT			90	/* ETH_RXBUF_CNTの整数倍 */
#define	ETH_RXBUF_CNT			30
#define	ETH_RXQBUF_CNT			30	/* ETH_RXBUF_CNT以上の値 */
#define	ETH_TX_CH				0
#define	ETH_RX_CH				0
#define	ETH_LOOPBACK			0
#define	ETH_PORT_NUM			0

/* Fixed Define */
#define	ETH_MDIO_CLK			2000000
#define	ETH_SEM_CNT				3
#define	ETH_CH_CNT				8

/* Sem */
#define	ETH_SEM_API				0
#define	ETH_SEM_SEND			1
#define	ETH_SEM_LINK			2

/* _EMAC_Desc */
#define DMA_DESC0_TX_OWN 		(1U<<31)
#define DMA_DESC0_TX_IC  		(1<<30)
#define DMA_DESC0_TX_LS	    	(1<<29)
#define DMA_DESC0_TX_FS    		(1<<28)
#define DMA_DESC0_TX_DC   		(1<<27)
#define DMA_DESC0_TX_DP   		(1<<26)
#define DMA_DESC0_TX_TTSE   	(1<<25)
#define DMA_DESC0_TX_CIC(x)   	(x<<22)
#define DMA_DESC0_TX_TER   		(1<<21)
#define DMA_DESC0_TX_TCH   		(1<20)
#define DMA_DESC0_TX_TTSS   	(1<<17)
#define DMA_DESC0_TX_IHE   		(1<<16)
#define DMA_DESC0_TX_ES  		(1<<15)
#define DMA_DESC0_TX_JT  		(1<<14)
#define DMA_DESC0_TX_FF 		(1<<13)
#define DMA_DESC0_TX_IPE  		(1<<12)
#define DMA_DESC0_TX_LC   	 	(1<<11)
#define DMA_DESC0_TX_NC    		(1<<10)
#define DMA_DESC0_TX_EC   		(1<<8)
#define DMA_DESC0_TX_VF   		(1<<7)
#define DMA_DESC0_TX_CC(x) 		(x<<3)
#define DMA_DESC0_TX_ED   		(1<<2)
#define DMA_DESC0_TX_UF  		(1<<1)
#define DMA_DESC0_TX_DB   		(1<<0)

#define DMA_DESC0_RX_OWN 		(1U<<31)
#define DMA_DESC0_RX_AFN 		(1<<30)
#define DMA_DESC0_RX_FL(x)    	(x<<16)
#define DMA_DESC0_RX_ES    		(1<<15)
#define DMA_DESC0_RX_DEE		(1<<14)
#define DMA_DESC0_RX_SAF   		(1<<13)
#define DMA_DESC0_RX_LE   		(1<<12)
#define DMA_DESC0_RX_OE  		(1<<11)
#define DMA_DESC0_RX_VLAN  		(1<<10)
#define DMA_DESC0_RX_FS  		(1<<9)
#define DMA_DESC0_RX_LS   		(1<<8)
#define DMA_DESC0_RX_IPC   		(1<<7)
#define DMA_DESC0_RX_LC   		(1<<6)
#define DMA_DESC0_RX_FT   		(1<<5)
#define DMA_DESC0_RX_RWT   		(1<<4)
#define DMA_DESC0_RX_RE   		(1<<3)
#define DMA_DESC0_RX_DE   		(1<<2)
#define DMA_DESC0_RX_CE  		(1<<1)
#define DMA_DESC0_RX_RX  		(1<<0)

#define DMA_DESC1_RX_DIC   		(1<<31)
#define DMA_DESC1_RX_RBS2(x)	(x<<16)
#define DMA_DESC1_RX_RER  		(1<<15)
#define DMA_DESC1_RX_RCH  		(1<<14)
#define DMA_DESC1_RX_RBS1(x)	(x<<0)

/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/

typedef struct _EMAC_Desc {
	volatile UINT32 dsc0;
	volatile UINT32 dsc1;
	volatile UINT32 dsc2;
	volatile UINT32 dsc3;
	volatile UINT32 dsc4;
	volatile UINT32 dsc5;
	volatile UINT32 dsc6;
	volatile UINT32 dsc7;
} EMAC_Desc;

typedef struct {
	UINT8 link;		/* 0:Down, 1:Up */
	UINT8 duplex;	/* 0:Harf, 1:Full */
	UINT8 band;		/* 0:10MBps, 1:100MBps, 2:Giga */
} ETH_PHY;

typedef struct {
	UINT32 base;
	OSW_ISR_HANDLE hIsr;
	OSW_SEM_HANDLE hSem[ETH_SEM_CNT];
	ETH_PARAM prm;
	ETH_PHY phy;
	volatile INT32 enable;
	volatile INT32 tx_stat;
	UINT32 phy_div;
	
	EMAC_Desc *txd;
	EMAC_Desc *rxd;
	UINT32 rxd_index;
	UINT8 *tx_buf;
	UINT8 *rx_buf;
	UINT32 rx_q_wp;
	UINT32 rx_q_rp;
	UINT32 rx_q_rem;
	UINT32 rx_q_size[ETH_RXQBUF_CNT];
	UINT8 *rx_buf_q;
} ETH_STR;

extern ETH_STR *pshEth[MAC_PORT_CNT];
extern OSW_MEM_HANDLE *pshMemEth;

extern INT8 eth_phy_reset( ETH_STR *pEth );
extern INT8 eth_phy_link_status( ETH_STR *pEth );


#endif /* __J_ETHERNET_DEF__ */









