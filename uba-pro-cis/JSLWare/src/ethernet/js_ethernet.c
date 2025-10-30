/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: Ethernet driver														*/
/* Comment:																		*/
/* 	・																			*/
/*==============================================================================*/

/*==============================================================================*/
/* インクルード																	*/
/*==============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "js_oswapi.h"
#include "js_io.h"
#include "js_ethernet.h"
#include "js_ethernet_def.h"
#include "js_ethernet_reg.h"
#if JS_E_DEBUG
#include "js_e_debug.h"
#else
#define	_E_DEBUG()
#endif


/*==============================================================================*/
/* デバッグトレース宣言(有効にするとトレース出力あり)							*/
/*==============================================================================*/
#define	DBG_ERR()			//osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#define DBG_TRACE1(...)		//osw_printf(__VA_ARGS__)
#define DBG_TRACE2(...)		//osw_printf(__VA_ARGS__)
#if DBG_ERR_ALL_ENABLE
#ifdef DBG_ERR
#undef DBG_ERR
#define	DBG_ERR() osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#endif
#endif


/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/
ETH_STR *pshEth[MAC_PORT_CNT];
OSW_MEM_HANDLE *pshMemEth = 0;
static volatile UINT32 eth_dummy_dw;


/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 mac_base[MAC_PORT_CNT] = {
#if (MAC_PORT_CNT >= 1)
	MAC0_BASE
#endif
#if (MAC_PORT_CNT >= 2)
	,MAC1_BASE
#endif
#if (MAC_PORT_CNT >= 3)
	,MAC2_BASE
#endif
};


/*==============================================================================*/
/* 割り込みIDテーブル															*/
/*==============================================================================*/
static const UINT16 eth_interrupt_id[MAC_PORT_CNT] = {
#if (MAC_PORT_CNT >= 1)
	OSW_INT_EMAC0_IRQ
#endif
#if (MAC_PORT_CNT >= 2)
	,OSW_INT_EMAC1_IRQ
#endif
#if (MAC_PORT_CNT >= 3)
	,OSW_INT_EMAC2_IRQ
#endif
};


/*==============================================================================*/
/* ディバイダー選択用テーブル													*/
/*==============================================================================*/
static const UINT8 eth_mdc_div_tbl[] = {
	124,	5,
	102,	4,
	62,		1,
	42,		0,
	26,		3,
	16,		2,
	18,		15,
	16,		14,
	14,		13,
	12,		12,
	10,		11,
	8,		10,
	6,		9,
	4,		8
};


/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void eth_isr( UINT8 num )
{
	ETH_STR *pEth;
	EMAC_Desc *desc;
	UINT32 i,j,stat;
	
	if( pshEth[num] == NULL ) return;
	pEth = pshEth[num];
	
	if( IOREG32(pEth->base,EMAC_INTERRUPT_STATUS) & 0x1 ){
		eth_dummy_dw = IOREG32(pEth->base,EMAC_SGMII_RGMII_SMII_CONTROL_STATUS);
		OSW_SEM_post( &pEth->hSem[ETH_SEM_LINK] );
	}
	
	stat = IOREG32(pEth->base,EMAC_STATUS);
	
	if( stat & (1<<6) ){
		for( j = 0 ; j < ETH_RXDESC_CNT ; j ++ ){
			desc = &pEth->rxd[pEth->rxd_index];
			if( desc->dsc0 & DMA_DESC0_RX_OWN ) break;
			i = (desc->dsc0 >> 16) & 0x3FFF;
			
			if( pEth->rxd_index >= (ETH_RXDESC_CNT-1) ){
				desc->dsc1 = DMA_DESC1_RX_RER|ETH_RX_BUF_SIZE;
			}
			else {
				desc->dsc1 = ETH_RX_BUF_SIZE;
			}
			desc->dsc0 = DMA_DESC0_RX_OWN;
			
			if( i > ETH_RX_BUF_SIZE ) i = ETH_RX_BUF_SIZE;
			if( pEth->phy.link && pEth->enable ){
				if( i && (pEth->rx_q_rem < ETH_RXQBUF_CNT) ){
					OSW_CAC_writeclean( (void *)desc->dsc2, i, 1 );
					memcpy( (void *)&pEth->rx_buf_q[ETH_RX_BUF_SIZE*pEth->rx_q_wp], (void *)desc->dsc2, i );
					pEth->rx_q_size[pEth->rx_q_wp] = i;
					pEth->rx_q_wp++;
					if( pEth->rx_q_wp >= ETH_RXQBUF_CNT ) pEth->rx_q_wp = 0;
					pEth->rx_q_rem++;
				}
			}
			
			pEth->rxd_index++;
			if( pEth->rxd_index >= ETH_RXDESC_CNT ) pEth->rxd_index = 0;
		}
	}
	if( stat & (1<<0) ){
		if( (pEth->txd[0].dsc0 & DMA_DESC0_TX_OWN) == 0 ){
			pEth->tx_stat = 1;
			OSW_SEM_post( &pEth->hSem[ETH_SEM_SEND] );
		}
	}
	
	IOREG32(pEth->base,EMAC_STATUS) = stat;
	
	if( pEth->rx_q_rem ){
		if( pEth->phy.link && pEth->enable ){
			(*pEth->prm.cb_func)( ETH_STAT_RECV_NOTIFY, pEth->prm.cb_arg );
		}
	}
}


#if (MAC_PORT_CNT >= 1)
/*==============================================================================*/
/* ポート0割り込みエントリ														*/
/*==============================================================================*/
void eth_isr0( void ) { eth_isr(0); }
#endif
#if (MAC_PORT_CNT >= 2)
/*==============================================================================*/
/* ポート1割り込みエントリ														*/
/*==============================================================================*/
void eth_isr1( void ) { eth_isr(1); }
#endif
#if (MAC_PORT_CNT >= 3)
/*==============================================================================*/
/* ポート2割り込みエントリ														*/
/*==============================================================================*/
void eth_isr2( void ) { eth_isr(2); }
#endif


/*==============================================================================*/
/* 割り込みエントリテーブル														*/
/*==============================================================================*/
static const osw_isr_func eth_isr_entry[MAC_PORT_CNT] = {
#if (MAC_PORT_CNT >= 1)
	eth_isr0
#endif
#if (MAC_PORT_CNT >= 2)
	,eth_isr1
#endif
#if (MAC_PORT_CNT >= 3)
	,eth_isr2
#endif
};


/*==============================================================================*/
/* 受信																			*/
/*==============================================================================*/
static void eth_recv( ETH_STR *pEth )
{
	EMAC_Desc *desc;
	UINT32 j,k = 0;
	UINT8 *p;
	
	pEth->rxd_index = 0;
	p = pEth->rx_buf;
	
	for( j = 0 ; j < ETH_RXDESC_CNT ; j ++ ){
		desc = &pEth->rxd[j];
		if( j == 0 ){
			desc->dsc0 = 0;
		}
		else {
			desc->dsc0 = DMA_DESC0_RX_OWN;
		}
		desc->dsc1 = ETH_RX_BUF_SIZE;
		desc->dsc2 = (UINT32)&p[ETH_RX_BUF_SIZE*k];
		desc->dsc3 = 0;
		desc->dsc4 = 0;
		desc->dsc5 = 0;
		desc->dsc6 = 0;
		desc->dsc7 = 0;
		
		k++;
		if( k >= ETH_RXBUF_CNT ) k = 0;
	}
	pEth->rxd[j-1].dsc1 |= DMA_DESC1_RX_RER;
	
	OSW_CAC_writeclean( (void *)pEth->rx_buf, (ETH_RX_BUF_SIZE*ETH_RXBUF_CNT), 1 );
	
	pEth->rxd[0].dsc0 |= DMA_DESC0_RX_OWN;
}



/*==============================================================================*/
/* 送信																			*/
/*==============================================================================*/
static void eth_send( ETH_STR *pEth, UINT8 *buf, UINT16 len )
{
	EMAC_Desc *desc;
	UINT32 i,j,k;
	
	OSW_CAC_writeclean( (void *)buf, len, 1 );
	
	k = len;
	for( j = 0 ; j < ETH_TXDESC_CNT ; j ++ ){
		i = k;
		if( i >= ETH_VALID_DATA_SIZE ) i = ETH_VALID_DATA_SIZE;
		desc = &pEth->txd[j];
		if( j == 0 ){
			desc->dsc0 = 0;
		}
		else {
			desc->dsc0 = DMA_DESC0_TX_OWN;
		}
		desc->dsc1 = i;
		desc->dsc2 = (UINT32)buf;
		desc->dsc3 = 0;
		desc->dsc4 = 0;
		desc->dsc5 = 0;
		desc->dsc6 = 0;
		desc->dsc7 = 0;
		
		k -= i;
		if( k == 0 ){
			j++;
			break;
		}
		buf = &buf[i];
	}
	pEth->txd[0].dsc0 |= DMA_DESC0_TX_FS;
	pEth->txd[j-1].dsc0 |= (DMA_DESC0_TX_LS|DMA_DESC0_TX_IC|DMA_DESC0_TX_TER);
	pEth->txd[0].dsc0 |= DMA_DESC0_TX_OWN;
	
	IOREG32(pEth->base,EMAC_TRANSMIT_POLL_DEMAND) = 1;
}


/*==============================================================================*/
/* レジスタ初期化																*/
/*==============================================================================*/
static INT8 eth_reg_init( ETH_STR *pEth )
{
	UINT32 i,j,k;
	UINT32 n;
	
	DBG_TRACE1( "eth_reg_init(%u)\n", pEth->prm.port );
	n = pEth->prm.port;
	
	k = sizeof(eth_mdc_div_tbl)>>1;
	i = CFG_ETH_SYSCLK[n] / ETH_MDIO_CLK;
	for( j = 0 ; j < k ; j ++ ){
		if( i > eth_mdc_div_tbl[j<<1] ) break;
	}
	if( j == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	j--;
	
	DBG_TRACE2( "eth:mdio_clk=%uHz\n", (CFG_ETH_SYSCLK[n]/eth_mdc_div_tbl[j<<1]) );
	pEth->phy_div = eth_mdc_div_tbl[(j<<1)+1];
	
	/* DMAリセット */
	IOREG32(pEth->base,EMAC_BUS_MODE) |= 0x1;
	i = OSW_TIM_value();
	while( (IOREG32(pEth->base,EMAC_BUS_MODE) & 0x1) || (IOREG32(pEth->base,EMAC_AHB_OR_AXI_STATUS) & 0x3) ){
		if( (OSW_TIM_value()-i) >= ETH_ACCESS_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		OSW_TSK_sleep( 1 );
	};
	
	memset( (void *)pEth->txd, 0, (sizeof(EMAC_Desc)*ETH_TXDESC_CNT) );
	memset( (void *)pEth->rxd, 0, (sizeof(EMAC_Desc)*ETH_RXDESC_CNT) );
	
	IOREG32(pEth->base,EMAC_STATUS) = 0xFFFFFFFF;
	IOREG32(pEth->base,EMAC_INTERRUPT_ENABLE) = 0x0;
	IOREG32(pEth->base,EMAC_BUS_MODE) = 0x1100880;
	IOREG32(pEth->base,EMAC_AXI_BUS_MODE) = 0x1;
	IOREG32(pEth->base,EMAC_TRANSMIT_DESCRIPTOR_LIST_ADDRESS) = (UINT32)pEth->txd;
	IOREG32(pEth->base,EMAC_RECEIVE_DESCRIPTOR_LIST_ADDRESS) = (UINT32)pEth->rxd;
	IOREG32(pEth->base,EMAC_OPERATION_MODE) = 0x2200000|(1<<13)|(1<<1);
	IOREG32(pEth->base,EMAC_GMII_ADDRESS) = 0;
	IOREG32(pEth->base,EMAC_MMC_CONTROL) = 0x8;
	
	eth_dummy_dw = IOREG32(pEth->base,EMAC_INTERRUPT_STATUS);
	IOREG32(pEth->base,EMAC_INTERRUPT_MASK) = 0x200;
	IOREG32(pEth->base,EMAC_MAC_CONFIGURATION) = 0x0060808C;
	
	OSW_ISR_enable( eth_interrupt_id[pEth->prm.port] );
	
	/* Phy reset */
	if( eth_phy_reset( pEth ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	return( TRUE );
}


/*==============================================================================*/
/* 通信開始																		*/
/*==============================================================================*/
static INT8 eth_start( ETH_STR *pEth )
{
	UINT8 *mac;
	
	DBG_TRACE1( "eth_start(%u)\n", pEth->prm.port );
	
	if( OSW_SEM_count( &pEth->hSem[ETH_SEM_LINK] ) == 0 ) OSW_SEM_post( &pEth->hSem[ETH_SEM_LINK] );
	
	mac = (UINT8 *)CFG_ETH_MACADDR[pEth->prm.port];
	
	IOREG32(pEth->base,EMAC_MAC_ADDRESS_HIGH(0)) = mac[4] | (mac[5] << 8) | (1U<<31);
	IOREG32(pEth->base,EMAC_MAC_ADDRESS_LOW(0)) = mac[0] | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
	IOREG32(pEth->base,EMAC_MAC_FRAME_FILTER) = 0x10;
	
	IOREG32(pEth->base,EMAC_STATUS) = 0xFFFFFFFF;
	IOREG32(pEth->base,EMAC_INTERRUPT_ENABLE) = 0x1A061;
	
	OSW_SEM_reset( &pEth->hSem[ETH_SEM_SEND] );
	eth_recv( pEth );
	
	pEth->enable = 1;
	
	if( OSW_SEM_count( &pEth->hSem[ETH_SEM_LINK] ) == 0 ) OSW_SEM_post( &pEth->hSem[ETH_SEM_LINK] );
	
	return( TRUE );
}


/*==============================================================================*/
/* 通信停止																		*/
/*==============================================================================*/
static INT8 eth_end( ETH_STR *pEth )
{
	UINT32 isr;
	
	DBG_TRACE1( "eth_end(%u)\n", pEth->prm.port );
	
	isr = OSW_ISR_global_disable();
	IOREG32(pEth->base,EMAC_OPERATION_MODE) = 0;
	OSW_ISR_global_restore( isr );
	IOREG32(pEth->base,EMAC_STATUS) = 0xFFFFFFFF;
	IOREG32(pEth->base,EMAC_INTERRUPT_ENABLE) = 0x0;
	
	if( pEth->phy.link ){
		pEth->phy.link = 0;
		(*pEth->prm.cb_func)( ETH_STAT_LINK_DISCONNECT, pEth->prm.cb_arg );
		DBG_TRACE1( "Eth Link Down\n" );
	}
	pEth->enable = 0;
	
	return( TRUE );
}


/*==============================================================================*/
/* リンクステータスチェック														*/
/*==============================================================================*/
static void eth_link_status( ETH_STR *pEth )
{
	UINT32 isr,i;
	
	if( OSW_SEM_pend( &pEth->hSem[ETH_SEM_LINK], 0 ) == TRUE ){
		/* Link Status Read */
		if( eth_phy_link_status( pEth ) == TRUE ){
			
			isr = OSW_ISR_global_disable();
			pEth->rx_q_wp = 0;
			pEth->rx_q_rp = 0;
			pEth->rx_q_rem = 0;
			OSW_ISR_global_restore( isr );
			
			/* Change */
			isr = OSW_ISR_global_disable();
			IOREG32(pEth->base,EMAC_OPERATION_MODE) &= (~(1<<13)|(1<<1));
			i = IOREG32(pEth->base,EMAC_MAC_CONFIGURATION) & (~((1<<15)|(1<<14)|(1<<11)));
			
			if( pEth->phy.link ){
				if( pEth->phy.band == 2 ){
					i |= (1<<11);
				}
				else {
					i |= (1<<15);
					if( pEth->phy.band == 1 ){
						i |= (1<<14);
					}
					if( pEth->phy.duplex ){
						i |= (1<<11);
					}
				}
			}
			
			IOREG32(pEth->base,EMAC_MAC_CONFIGURATION) = i;
			IOREG32(pEth->base,EMAC_OPERATION_MODE) |= ((1<<13)|(1<<1));
			
			IOREG32(pEth->base,EMAC_RECEIVE_POLL_DEMAND) = 1;
			OSW_ISR_global_restore( isr );
		}
	}
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 Ethernet_open( ETH_HANDLE *handle, ETH_PARAM *param, UINT8 *mac_adr )
{
	ETH_STR *pEth;
	UINT16 n,i;
	
	if( (param == 0) || 
		(param->port >= MAC_PORT_CNT) ||
		(param->cb_func == 0 ) ||
		(handle == 0) ||
		(pshEth[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	pEth = (ETH_STR *)OSW_MEM_alloc( pshMemEth, sizeof(ETH_STR), 4 );
	if( pEth == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pEth, 0, sizeof(ETH_STR) );
	handle->hdl = pEth;
	
	pEth->prm = *param;
	n = pEth->prm.port;
	pEth->base = mac_base[n];
	
	pEth->txd = (EMAC_Desc *)OSW_MEM_alloc( CFG_ETH_NC_HEAP, (sizeof(EMAC_Desc)*ETH_TXDESC_CNT), 4 );
	if( pEth->txd == 0 ){
		Ethernet_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	pEth->rxd = (EMAC_Desc *)OSW_MEM_alloc( CFG_ETH_NC_HEAP, (sizeof(EMAC_Desc)*ETH_RXDESC_CNT), 4 );
	if( pEth->rxd == 0 ){
		Ethernet_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	pEth->rx_buf = (UINT8 *)OSW_MEM_alloc( pshMemEth, (ETH_RX_BUF_SIZE*ETH_RXBUF_CNT), 32 );
	if( pEth->rx_buf == 0 ){
		Ethernet_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	pEth->rx_buf_q = (UINT8 *)OSW_MEM_alloc( pshMemEth, (ETH_RX_BUF_SIZE*ETH_RXQBUF_CNT), 4 );
	if( pEth->rx_buf_q == 0 ){
		Ethernet_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	if( OSW_ISR_create( &pEth->hIsr, eth_interrupt_id[n], eth_isr_entry[n] ) == FALSE ){
		Ethernet_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	for( i = 0 ; i < ETH_SEM_CNT ; i ++ ){
		if( OSW_SEM_create( &pEth->hSem[i], 0 ) == FALSE ){
			Ethernet_close( handle );
			DBG_ERR();
			return( FALSE );
		}
	}
	OSW_SEM_post( &pEth->hSem[ETH_SEM_API] );
	
	pshEth[n] = pEth;
	if( eth_reg_init( pEth ) == FALSE ){
		Ethernet_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	if( mac_adr ){
		memcpy( (void *)mac_adr, (void *)&CFG_ETH_MACADDR[n][0], 6 );
	}
	DBG_TRACE1( "Ethernet_open(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void Ethernet_close( ETH_HANDLE *handle )
{
	ETH_STR *pEth;
	UINT32 i,n;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	pEth = (ETH_STR *)handle->hdl;
	n = pEth->prm.port;
	pshEth[n] = 0;
	
	OSW_ISR_disable( eth_interrupt_id[pEth->prm.port] );
	eth_end( pEth );
	
	for( i = 0 ; i < ETH_SEM_CNT ; i ++ ){
		OSW_SEM_delete( &pEth->hSem[i] );
	}
	OSW_ISR_delete( &pEth->hIsr );
	if( pEth->rx_buf_q ){
		OSW_MEM_free( pshMemEth, pEth->rx_buf_q, (ETH_RX_BUF_SIZE*ETH_RXQBUF_CNT) );
		pEth->rx_buf_q = 0;
	}
	if( pEth->rx_buf ){
		OSW_MEM_free( pshMemEth, pEth->rx_buf, (ETH_RX_BUF_SIZE*ETH_RXBUF_CNT) );
		pEth->rx_buf = 0;
	}
	if( pEth->rxd ){
		OSW_MEM_free( CFG_ETH_NC_HEAP, pEth->rxd, (sizeof(EMAC_Desc)*ETH_RXDESC_CNT) );
		pEth->rxd = 0;
	}
	if( pEth->txd ){
		OSW_MEM_free( CFG_ETH_NC_HEAP, pEth->txd, (sizeof(EMAC_Desc)*ETH_TXDESC_CNT) );
		pEth->txd = 0;
	}
	OSW_MEM_free( pshMemEth, pEth, sizeof(ETH_STR) );
	handle->hdl = 0;
	
	DBG_TRACE1( "Ethernet_close(%u)\n", n );
}


/*==============================================================================*/
/* ポートEnable API																*/
/*==============================================================================*/
INT8 Ethernet_enable( ETH_HANDLE *handle, INT8 enable )
{
	ETH_STR *pEth;
	INT8 ret = TRUE;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pEth = (ETH_STR *)handle->hdl;
	
	if( OSW_SEM_pend( &pEth->hSem[ETH_SEM_API], ETH_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	if( enable && (pEth->enable == 0) ){
		ret = eth_start( pEth );
	}
	else if( pEth->enable ){
		ret = eth_end( pEth );
	}
	OSW_SEM_post( &pEth->hSem[ETH_SEM_API] );
	DBG_TRACE1( "Ethernet_enable(%u)\n", pEth->prm.port );
	
	return( ret );
}


/*==============================================================================*/
/* フレーム送信API																*/
/*==============================================================================*/
UINT16 Ethernet_send( ETH_HANDLE *handle, UINT8 *buf, UINT16 len )
{
	ETH_STR *pEth;
	UINT32 i,j;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( 0 );
	}
	_E_DEBUG();
	pEth = (ETH_STR *)handle->hdl;
	
	if( (pEth->phy.link && pEth->enable) == 0 ) return( 0 );
	if( len == 0 ) return( 0 );
	
	if( OSW_SEM_pend( &pEth->hSem[ETH_SEM_API], ETH_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( 0 );
	}
	i = len;
	
	for( j = 0 ; j < 10 ; j ++ ){
		pEth->tx_stat = 0;
		OSW_SEM_reset( &pEth->hSem[ETH_SEM_SEND] );
		eth_send( pEth, buf, i );
		if( OSW_SEM_pend( &pEth->hSem[ETH_SEM_SEND], ETH_ACCESS_TOUT ) == FALSE ){
			DBG_ERR();
			len = 0;
			break;
		}
		if( pEth->tx_stat ) break;
	};
	if( j >= 10 ){
		len = 0;
		DBG_ERR();
	}
	eth_link_status( (ETH_STR *)handle->hdl );
	OSW_SEM_post( &pEth->hSem[ETH_SEM_API] );
	
	return( len );
}


/*==============================================================================*/
/* フレーム受信API																*/
/*==============================================================================*/
UINT16 Ethernet_recv( ETH_HANDLE *handle, UINT8 *buf, UINT16 len )
{
	ETH_STR *pEth;
	UINT16 ret = 0;
	UINT32 isr;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pEth = (ETH_STR *)handle->hdl;
	
	if( OSW_SEM_pend( &pEth->hSem[ETH_SEM_API], ETH_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( 0 );
	}
	if( pEth->phy.link && pEth->enable && len ){
		isr = OSW_ISR_global_disable();
		if( pEth->rx_q_rem ){
			ret = pEth->rx_q_size[pEth->rx_q_rp];
			if( ret > len ) ret = len;
			memcpy( (void *)buf, (void *)&pEth->rx_buf_q[ETH_RX_BUF_SIZE*pEth->rx_q_rp], ret );
			pEth->rx_q_rp++;
			if( pEth->rx_q_rp >= ETH_RXQBUF_CNT ) pEth->rx_q_rp = 0;
			pEth->rx_q_rem--;
			if( pEth->rx_q_rem ){
				(*pEth->prm.cb_func)( ETH_STAT_RECV_NOTIFY, pEth->prm.cb_arg );
			}
		}
		OSW_ISR_global_restore( isr );
	}
	
	eth_link_status( (ETH_STR *)handle->hdl );
	OSW_SEM_post( &pEth->hSem[ETH_SEM_API] );
	
	return( ret );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 Ethernet_init( OSW_MEM_HANDLE *mem_handle )
{
	pshMemEth = mem_handle;
	memset( (void *)pshEth, 0, sizeof(pshEth) );
	
	DBG_TRACE1( "Ethernet_init()\n" );
	
	return( TRUE );
}





