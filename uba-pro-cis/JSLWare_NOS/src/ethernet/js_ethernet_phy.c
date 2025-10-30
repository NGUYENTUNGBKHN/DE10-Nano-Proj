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
/* PHY MDIOビジー解除待ち														*/
/*==============================================================================*/
static INT8 eth_phy_busywait( ETH_STR *pEth )
{
	UINT32 i;
	i = OSW_TIM_value();
	while( IOREG32(pEth->base,EMAC_GMII_ADDRESS) & 0x1 ){
		if( (OSW_TIM_value()-i) >= ETH_ACCESS_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		OSW_TSK_sleep( 1 );
	};
	return( TRUE );
}


/*==============================================================================*/
/* PHYレジスタリード															*/
/*==============================================================================*/
static INT8 eth_phy_read( ETH_STR *pEth, UINT8 phy, UINT8 reg, UINT16 *val )
{
	if( eth_phy_busywait( pEth ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	IOREG32(pEth->base,EMAC_GMII_ADDRESS) = (phy<<11)|(reg<<6)|(pEth->phy_div<<2)|0x1;
	if( eth_phy_busywait( pEth ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	*val = (UINT16)IOREG32(pEth->base,EMAC_GMII_DATA);
	return( TRUE );
}


/*==============================================================================*/
/* PHYレジスタライト															*/
/*==============================================================================*/
static INT8 eth_phy_write( ETH_STR *pEth, UINT8 phy, UINT8 reg, UINT16 val )
{
	if( eth_phy_busywait( pEth ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	IOREG32(pEth->base,EMAC_GMII_DATA) = val;
	IOREG32(pEth->base,EMAC_GMII_ADDRESS) = (phy<<11)|(reg<<6)|(pEth->phy_div<<2)|0x3;
	if( eth_phy_busywait( pEth ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	return( TRUE );
}


/*==============================================================================*/
/* PHYソフトリセット															*/
/*==============================================================================*/
INT8 eth_phy_reset( ETH_STR *pEth )
{
	UINT32 i,n = pEth->prm.port;
	UINT16 s,*p;
	
	DBG_TRACE1( "eth_phy_reset(%u)\n", pEth->prm.port );
	/* Reset */
	if( eth_phy_write( pEth, CFG_ETH_PHYADDR[n], EPHY_CTRL, 0x8000 ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	i = OSW_TIM_value();
	while( 1 ){
		if( (OSW_TIM_value()-i) >= ETH_ACCESS_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		if( eth_phy_read( pEth, CFG_ETH_PHYADDR[n], EPHY_CTRL, &s ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		if( (s & 0x8000) == 0 ) break;
		OSW_TSK_sleep( 1 );
	};
	
	/* Custom Configration */
	p = (UINT16 *)CFG_ETH_PHYCFG[n];
	while( *p != 0xFFFF ){
		DBG_TRACE2( "phy:REG%u = 0x%04X\n", p[0], p[1] );
		if( eth_phy_write( pEth, CFG_ETH_PHYADDR[n], p[0], p[1] ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		p = &p[2];
	};
	
	if( eth_phy_read( pEth, CFG_ETH_PHYADDR[n], EPHY_CTRL, &s ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	/* Auto negotiate */
	s |= 0x1200;
	if( eth_phy_write( pEth, CFG_ETH_PHYADDR[n], EPHY_CTRL, s ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	return( TRUE );
}


/*==============================================================================*/
/* PHY LINKステータス取得														*/
/*==============================================================================*/
INT8 eth_phy_link_status( ETH_STR *pEth )
{
	UINT32 i,n = pEth->prm.port;
	UINT16 s,m;
	UINT8 link,duplex,band;
	
	_E_DEBUG();
	duplex = 0;
	band = 0;
	if( eth_phy_read( pEth, CFG_ETH_PHYADDR[n], EPHY_STATUS, &s ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	if( s & 0x4 ){
		if( pEth->phy.link == 0 ){
			i = OSW_TIM_value();
			while( 1 ){
				if( (OSW_TIM_value()-i) >= ETH_ACCESS_TOUT ){
					DBG_ERR();
					return( FALSE );
				}
				if( eth_phy_read( pEth, CFG_ETH_PHYADDR[n], EPHY_STATUS, &s ) == FALSE ){
					DBG_ERR();
					return( FALSE );
				}
				if( s & 0x20 ) break;
				OSW_TSK_sleep( 1 );
			};
			m = 0;
			if( CFG_ETH_PHYATTR[n] & ETH_PHY_ATTR_GIGA ){
				if( eth_phy_read( pEth, CFG_ETH_PHYADDR[n], EPHY_1000BT_STATUS, &m ) == FALSE ){
					DBG_ERR();
					return( FALSE );
				}
			}
			if( eth_phy_read( pEth, CFG_ETH_PHYADDR[n], EPHY_AN_LINK, &s ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
			if( m & 0x800 ){
				/* Giga Full Duplex */
				band = 2;
				duplex = 1;
			}
			else if( m & 0x400 ){
				/* Giga Harf Duplex */
				band = 2;
			}
			else if( s & 0x100 ){
				/* 100MBps, Full */
				band = 1;
				duplex = 1;
			}
			else if( s & 0x040 ){
				/* 10Mbps, Full */
				duplex = 1;
			}
			else if( s & 0x080 ){
				/* 100Mbps Harf */
				band = 1;
			}
			else if( s & 0x020 ){
				/* 10Mbps, Harf */
			}
		}
		link = 1;
	}
	else {
		link = 0;
	}
	
	if( pEth->phy.link ){
		if( link == 0 ){
			pEth->phy.link = 0;
			(*pEth->prm.cb_func)( ETH_STAT_LINK_DISCONNECT, pEth->prm.cb_arg );
			DBG_TRACE1( "Eth Link Down\n" );
			return( TRUE );
		}
	}
	else {
		if( link ){
			pEth->phy.duplex = duplex;
			pEth->phy.band = band;
			pEth->phy.link = 1;
			(*pEth->prm.cb_func)( ETH_STAT_LINK_CONNECT, pEth->prm.cb_arg );
			DBG_TRACE1( "Eth Link Up(duplex=%u,band=%u)\n", duplex, band );
			return( TRUE );
		}
	}
	return( FALSE );
}





