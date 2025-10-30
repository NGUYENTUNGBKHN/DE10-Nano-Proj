/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: MMC driver															*/
/* Comment:																		*/
/*==============================================================================*/

/*==============================================================================*/
/* インクルード																	*/
/*==============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "js_oswapi.h"
#include "js_io.h"
#include "js_mmc.h"
#include "js_mmc_reg.h"
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
/* ローカルデファイン															*/
/*==============================================================================*/

/* Other */
#define	MMC_LOCAL_TOUT			1000
#define	MMC_IE_VAL				0x0146
#define	MMC_IE_TC_VAL			0x417E
#define	MMC_BLK_SIZE			512

/* bus freq */
#define	MMC_BUS_FREQ_400KHZ		0
#define	MMC_BUS_FREQ_25MHZ		1
#define	MMC_BUS_FREQ_50MHZ		2

/* bus_mode */
#define	MMC_BUS_MODE_1BIT		0x00
#define	MMC_BUS_MODE_4BIT		0x01

/* cmd attr */
#define	MMC_CA_RESP_EX			(1<<6)			/* response_expect */
#define	MMC_CA_RESP_LEN136		(1<<7)			/* response_length */
#define	MMC_CA_RESP_CRC			(1<<8)			/* check_response_crc */
#define	MMC_CA_DATA_EX			(1<<9)			/* data_expected */
#define	MMC_CA_DATA_W			(1<<10)			/* read_write */
#define	MMC_CA_TRNS_STR			(1<<11)			/* transfer_mode */
#define	MMC_CA_AUTO_STOP		(1<<12)			/* send_auto_stop */
#define	MMC_CA_WAIT_PRV			(1<<13)			/* wait_prvdata_complete */
#define	MMC_CA_STOP_ABT			(1<<14)			/* stop_abort_cmd */
#define	MMC_CA_INIT				(1<<15)			/* send_initialization */
#define	MMC_CA_UPD_CLK			(1<<21)			/* update_clock_registers_only */
#define	MMC_CA_RDCD				(1<<22)			/* read_ceata_device */
#define	MMC_CA_CCS_EX			(1<<23)			/* ccs_expected */
#define	MMC_CA_ENA_BOOT			(1<<24)			/* enable_boot */
#define	MMC_CA_BOOT_ACK			(1<<25)			/* expect_boot_ack */
#define	MMC_CA_DIS_BOOT			(1<<26)			/* disable_boot */
#define	MMC_CA_BOOT_MD			(1<<27)			/* boot_mode */
#define	MMC_CA_VOLT_SW			(1<<28)			/* volt_switch */
#define	MMC_CA_HOLD				(1<<29)			/* use_hold_reg */
#define	MMC_CA_START			(0x80000000)	/* start_cmd */

/* Response */
#define	MMC_RSP_R1				(MMC_CA_RESP_EX|MMC_CA_RESP_CRC)
#define	MMC_RSP_R1b				(MMC_CA_RESP_EX|MMC_CA_RESP_CRC)
#define	MMC_RSP_R2				(MMC_CA_RESP_EX|MMC_CA_RESP_LEN136|MMC_CA_RESP_CRC)
#define	MMC_RSP_R3				(MMC_CA_RESP_EX)
#define	MMC_RSP_R6				(MMC_CA_RESP_EX|MMC_CA_RESP_CRC)
#define	MMC_RSP_R7				(MMC_CA_RESP_EX|MMC_CA_RESP_CRC)

/* cmd */
#define	MMC_CMD0				(0|MMC_CA_START|MMC_CA_HOLD|MMC_CA_INIT)
#define	MMC_CMD2				(2|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R2)
#define	MMC_CMD3				(3|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R6)
#define	MMC_CMD6				(6|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R1)
#define	MMC_CMD7				(7|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R1b)
#define	MMC_CMD8				(8|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R7)
#define	MMC_CMD9				(9|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R2)
#define	MMC_CMD16				(16|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R1)
#define	MMC_CMD17				(17|MMC_CA_START|MMC_CA_HOLD|MMC_CA_DATA_EX|MMC_RSP_R1)
#define	MMC_CMD18				(18|MMC_CA_START|MMC_CA_HOLD|MMC_CA_DATA_EX|MMC_CA_AUTO_STOP|MMC_RSP_R1)
#define	MMC_CMD24				(24|MMC_CA_START|MMC_CA_HOLD|MMC_CA_DATA_EX|MMC_CA_DATA_W|MMC_RSP_R1)
#define	MMC_CMD25				(25|MMC_CA_START|MMC_CA_HOLD|MMC_CA_DATA_EX|MMC_CA_DATA_W|MMC_CA_AUTO_STOP|MMC_RSP_R1)
#define	MMC_CMD55				(55|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R1)
#define	MMC_ACMD6				(6|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R1)
#define	MMC_ACMD41				(41|MMC_CA_START|MMC_CA_HOLD|MMC_RSP_R3)

/* hEvt */
#define	MMC_EVT_CMDEND			0			/* Command End */
#define	MMC_EVT_XFEREND			1			/* Transfer End */
#define	MMC_EVT_ERR				2			/* Error */

/* type */
#define	MMC_TYPE_SDV2			0x0001		/* SD V2.0 or Leter */
#define	MMC_TYPE_SDHC			0x0002		/* SDHC Card */
#define	MMC_TYPE_SDXC			0x0004		/* SDXC Card */
#define	MMC_TYPE_MEM			0x0008		/* Memory Card */
#define	MMC_TYPE_WP				0x4000		/* Write Protect */
#define	MMC_TYPE_CONFIRM		0x8000		/* 判定確定 */


/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/
typedef struct {
	UINT16 rca;			/* RCA Address */
	UINT16 type;		/* Card Type */
	UINT32 blk_cnt;		/* Total Blocks */
} CARD_INFO;

typedef struct {
	volatile INT32 cnt;
	INT32 cmd12;
	UINT8 *buf;
} MMC_XFER;

typedef struct {
	UINT32 base;
	MMC_PARAM prm;
	OSW_ISR_HANDLE hIsr;
	OSW_EVT_HANDLE hEvt;
	UINT16 clkd[3];
	MMC_XFER xfer;
	CARD_INFO info;
} MMC_STR;

static MMC_STR *pshMmc[MMC_PORT_CNT];
static OSW_MEM_HANDLE *pshMemMmc = 0;

/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 mmc_base[MMC_PORT_CNT] = {
#if (MMC_PORT_CNT >= 1)
	MMC0_BASE
#endif
#if (MMC_PORT_CNT >= 2)
	,MMC1_BASE
#endif
#if (MMC_PORT_CNT >= 3)
	,MMC2_BASE
#endif
#if (MMC_PORT_CNT >= 4)
	,MMC3_BASE
#endif
#if (MMC_PORT_CNT >= 5)
	,MMC4_BASE
#endif
#if (MMC_PORT_CNT >= 6)
	,MMC5_BASE
#endif
#if (MMC_PORT_CNT >= 7)
	,MMC6_BASE
#endif
#if (MMC_PORT_CNT >= 8)
	,MMC7_BASE
#endif
};


/*==============================================================================*/
/* 割り込みIDテーブル															*/
/*==============================================================================*/
static const UINT16 mmc_interrupt_id[MMC_PORT_CNT] = {
#if (MMC_PORT_CNT >= 1)
	OSW_INT_SDMMC0_IRQ
#endif
#if (MMC_PORT_CNT >= 2)
	,OSW_INT_SDMMC1_IRQ
#endif
#if (MMC_PORT_CNT >= 3)
	,OSW_INT_SDMMC2_IRQ
#endif
#if (MMC_PORT_CNT >= 4)
	,OSW_INT_SDMMC3_IRQ
#endif
#if (MMC_PORT_CNT >= 5)
	,OSW_INT_SDMMC4_IRQ
#endif
#if (MMC_PORT_CNT >= 6)
	,OSW_INT_SDMMC5_IRQ
#endif
#if (MMC_PORT_CNT >= 7)
	,OSW_INT_SDMMC6_IRQ
#endif
#if (MMC_PORT_CNT >= 8)
	,OSW_INT_SDMMC7_IRQ
#endif
};


/*==============================================================================*/
/* FIFO読み込み																	*/
/*==============================================================================*/
static void mmc_fifo_read( MMC_STR *pMmc )
{
	UINT32 *p,i,j;
	UINT8 *c;
	
	if( pMmc->xfer.cnt > 0 ){
		/* Read */
		if( (UINT32)pMmc->xfer.buf & 0x3 ){
			/* 非アライメント */
			for( j = 0 ; j < 128 ; j++ ){
				c = &pMmc->xfer.buf[j<<2];
				i = IOREG32(pMmc->base,MMC_DATA);
				c[0] = i & 0xFF;
				c[1] = (i >> 8) & 0xFF;
				c[2] = (i >> 16) & 0xFF;
				c[3] = (i >> 24 ) & 0xFF;
			}
		}
		else {
			p = (UINT32 *)pMmc->xfer.buf;
			for( j = 0 ; j < 128 ; j++ ){
				p[j] = IOREG32(pMmc->base,MMC_DATA);
			}
		}
		pMmc->xfer.buf = &pMmc->xfer.buf[512];
		pMmc->xfer.cnt--;
	}
}


/*==============================================================================*/
/* FIFO書き込み																	*/
/*==============================================================================*/
static void mmc_fifo_write( MMC_STR *pMmc )
{
	UINT32 *p,i,j;
	UINT8 *c;
	
	if( pMmc->xfer.cnt < 0 ){
		/* Write */
		if( (UINT32)pMmc->xfer.buf & 0x3 ){
			/* 非アライメント */
			for( j = 0 ; j < 128 ; j++ ){
				c = &pMmc->xfer.buf[j<<2];
				i = (c[3] << 24)|(c[2] << 16)|(c[1] << 8)|c[0];
				IOREG32(pMmc->base,MMC_DATA) = i;
			}
		}
		else {
			p = (UINT32 *)pMmc->xfer.buf;
			for( j = 0 ; j < 128 ; j++ ){
				IOREG32(pMmc->base,MMC_DATA) = p[j];
			}
		}
		pMmc->xfer.buf = &pMmc->xfer.buf[512];
		pMmc->xfer.cnt++;
	}
}


/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void mmc_isr( UINT8 num )
{
	MMC_STR *pMmc;
	UINT32 i;
	
	if( pshMmc[num] == 0 ) return;
	pMmc = pshMmc[num];
	
	i = IOREG32(pMmc->base,MMC_MINTSTS);
	
	if( i & 0x3C2 ){
		/* Error */
		DBG_TRACE2( "mmc_isr(%u):ERR %08X\n", pMmc->prm.port, i );
		OSW_EVT_set( &pMmc->hEvt, MMC_EVT_ERR );
	}
	else {
		if( i & (1<<2) ){
			/* Command complete */
			OSW_EVT_set( &pMmc->hEvt, MMC_EVT_CMDEND );
		}
		if( i & (1<<5) ){
			/* Read */
			mmc_fifo_read( pMmc );
			if( pMmc->xfer.cnt == 0 ){
				IOREG32(pMmc->base,MMC_INTMASK) = MMC_IE_TC_VAL & (~(1<<5));
			}
		}
		if( i & (1<<4) ){
			/* Write */
			mmc_fifo_write( pMmc );
			if( pMmc->xfer.cnt == 0 ){
				IOREG32(pMmc->base,MMC_INTMASK) = MMC_IE_TC_VAL & (~(1<<4));
			}
		}
		if( i & (1<<3) ){
			while( pMmc->xfer.cnt > 0 ){
				mmc_fifo_read( pMmc );
			};
			
			if( pMmc->xfer.cmd12 == 0 ){
				/* Transfer completed */
				OSW_EVT_set( &pMmc->hEvt, MMC_EVT_XFEREND );
			}
		}
		if( i & (1<<14) ){
			/* Auto Stop */
			OSW_EVT_set( &pMmc->hEvt, MMC_EVT_XFEREND );
		}
	}
	
	IOREG32(pMmc->base,MMC_RINTSTS) = i;
}


#if (MMC_PORT_CNT >= 1)
/*==============================================================================*/
/* ポート0割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr0( void ) { mmc_isr(0); }
#endif
#if (MMC_PORT_CNT >= 2)
/*==============================================================================*/
/* ポート1割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr1( void ) { mmc_isr(1); }
#endif
#if (MMC_PORT_CNT >= 3)
/*==============================================================================*/
/* ポート2割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr2( void ) { mmc_isr(2); }
#endif
#if (MMC_PORT_CNT >= 4)
/*==============================================================================*/
/* ポート3割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr3( void ) { mmc_isr(3); }
#endif
#if (MMC_PORT_CNT >= 5)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr4( void ) { mmc_isr(4); }
#endif
#if (MMC_PORT_CNT >= 6)
/*==============================================================================*/
/* ポート5割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr5( void ) { mmc_isr(5); }
#endif
#if (MMC_PORT_CNT >= 7)
/*==============================================================================*/
/* ポート6割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr6( void ) { mmc_isr(6); }
#endif
#if (MMC_PORT_CNT >= 8)
/*==============================================================================*/
/* ポート7割り込みエントリ														*/
/*==============================================================================*/
void mmc_isr7( void ) { mmc_isr(7); }
#endif

/*==============================================================================*/
/* 割り込みエントリテーブル														*/
/*==============================================================================*/
static const osw_isr_func mmc_isr_entry[MMC_PORT_CNT] = {
#if (MMC_PORT_CNT >= 1)
	mmc_isr0
#endif
#if (MMC_PORT_CNT >= 2)
	,mmc_isr1
#endif
#if (MMC_PORT_CNT >= 3)
	,mmc_isr2
#endif
#if (MMC_PORT_CNT >= 4)
	,mmc_isr3
#endif
#if (MMC_PORT_CNT >= 5)
	,mmc_isr4
#endif
#if (MMC_PORT_CNT >= 6)
	,mmc_isr5
#endif
#if (MMC_PORT_CNT >= 7)
	,mmc_isr6
#endif
#if (MMC_PORT_CNT >= 8)
	,mmc_isr7
#endif
};


/*==============================================================================*/
/* I/O CLK																		*/
/*==============================================================================*/
static const UINT32 mmc_sd_clk[3] = {
	400000,
	25000000,
	50000000
};


/*==============================================================================*/
/* ビジー解除待ち																*/
/*==============================================================================*/
static INT8 mmc_busy_wait( MMC_STR *pMmc )
{
	UINT32 i,t;
	
	t = OSW_TIM_value();
	while( (IOREG32(pMmc->base,MMC_STATUS) & (1<<9)) ){
		i = OSW_TIM_value() - t;
		if( i >= MMC_LOCAL_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		if( i ) OSW_TSK_sleep( 1 );
	};
	return( TRUE );
}


/*==============================================================================*/
/* レスポンスリード																*/
/*==============================================================================*/
static void mmc_resp_read( MMC_STR *pMmc, UINT8 *buf, UINT32 len )
{
	UINT32 rsp[4];
	
	rsp[0] = IOREG32(pMmc->base,MMC_RESP0);
	rsp[1] = IOREG32(pMmc->base,MMC_RESP1);
	rsp[2] = IOREG32(pMmc->base,MMC_RESP2);
	rsp[3] = IOREG32(pMmc->base,MMC_RESP3);
	if( len >= sizeof(rsp) ) len = sizeof(rsp);
	memcpy( (void *)buf, (void *)rsp, len );
}


/*==============================================================================*/
/* コマンド送信レジスタ制御														*/
/*==============================================================================*/
static void mmc_cmd_write( MMC_STR *pMmc, UINT32 cmd, UINT32 arg, UINT32 nblk_blen )
{
	mmc_busy_wait( pMmc );
	OSW_EVT_wait( &pMmc->hEvt, 0 );
	IOREG32(pMmc->base,MMC_BYTCNT) = nblk_blen;
	IOREG32(pMmc->base,MMC_CMDARG) = arg;
	IOREG32(pMmc->base,MMC_CMD) = cmd;
}


/*==============================================================================*/
/* コマンド送信																	*/
/*==============================================================================*/
static void mmc_cmd_send( MMC_STR *pMmc, UINT32 cmd, UINT32 arg, UINT32 nblk_blen )
{
	DBG_TRACE2( "mmc(%u):CMD=%u\n", pMmc->prm.port, (cmd&0x1F) );
	mmc_cmd_write( pMmc, cmd, arg, nblk_blen );
}


/*==============================================================================*/
/* アプリケーションコマンド送信													*/
/*==============================================================================*/
static INT8 mmc_acmd_send( MMC_STR *pMmc, UINT32 cmd, UINT32 arg, UINT32 nblk_blen )
{
	DBG_TRACE2( "mmc(%u):ACMD=%u\n", pMmc->prm.port, (cmd&0x1F) );
	mmc_cmd_write( pMmc, MMC_CMD55, (pMmc->info.rca<<16), 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	mmc_cmd_write( pMmc, cmd, arg, nblk_blen );
	return( TRUE );
}


/*==============================================================================*/
/* バス設定																		*/
/*==============================================================================*/
static INT8 mmc_bus_cfg( MMC_STR *pMmc, UINT8 mode, UINT8 freq )
{
	UINT32 ctype = 0,t;
	
	if( mode & MMC_BUS_MODE_4BIT ){
		/* 4bit */
		ctype = 0x1;
	}
	if( freq >= MMC_BUS_FREQ_50MHZ ){
		/* High Speed */
		DBG_TRACE1( "mmc(%u):HS Mode\n", pMmc->prm.port );
	}
	
	if( mmc_busy_wait( pMmc ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	IOREG32(pMmc->base,MMC_CLKENA) = 0;
	IOREG32(pMmc->base,MMC_CLKSRC) = 0;
	IOREG32(pMmc->base,MMC_CLKDIV) = pMmc->clkd[freq];
	IOREG32(pMmc->base,MMC_CMD) = (MMC_CA_START|MMC_CA_UPD_CLK|MMC_CA_WAIT_PRV);
	
	t = OSW_TIM_value();
	while( (IOREG32(pMmc->base,MMC_CMD) & MMC_CA_START) ){
		if( (OSW_TIM_value() - t) >= MMC_LOCAL_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		OSW_TSK_sleep( 1 );
	};
	
	IOREG32(pMmc->base,MMC_CLKDIV) = pMmc->clkd[freq];
	IOREG32(pMmc->base,MMC_CLKENA) = 0x1001;
	IOREG32(pMmc->base,MMC_CMD) = (MMC_CA_START|MMC_CA_UPD_CLK|MMC_CA_WAIT_PRV);
	
	t = OSW_TIM_value();
	while( (IOREG32(pMmc->base,MMC_CMD) & MMC_CA_START) ){
		if( (OSW_TIM_value() - t) >= MMC_LOCAL_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		OSW_TSK_sleep( 1 );
	};
	
	IOREG32(pMmc->base,MMC_CTYPE) = ctype;
	
	return( TRUE );
}


/*==============================================================================*/
/* カード識別																	*/
/*==============================================================================*/
static INT8 mmc_card_ident( MMC_STR *pMmc )
{
	UINT32 i,j,t,hcs = 0,hs = 0;
	UINT8 freq = MMC_BUS_FREQ_25MHZ;
	UINT8 rsp[16];
	
	memset( (void *)&pMmc->info, 0, sizeof(CARD_INFO) );
	pMmc->xfer.cnt = 0;
	IOREG32(pMmc->base,MMC_RINTSTS) = 0xFFFFFFFF;
	IOREG32(pMmc->base,MMC_INTMASK) = MMC_IE_VAL;
	
	if( mmc_bus_cfg( pMmc, MMC_BUS_MODE_1BIT, MMC_BUS_FREQ_400KHZ ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	/* Initial */
	mmc_cmd_send( pMmc, MMC_CMD0, 0, 0 );
	OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT );
	OSW_TSK_sleep( 2 );
	
	/* GO_IDLE_STATE */
	mmc_cmd_send( pMmc, MMC_CMD0, 0, 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	OSW_TSK_sleep( 2 );
	
	/* ===== IDLE State ===== */
	
	/* SEND_IF_COND */
	mmc_cmd_send( pMmc, MMC_CMD8, 0x1AA, 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) ){
		mmc_resp_read( pMmc, rsp, 2 );
		OSW_TSK_sleep( 1 );
		if( (rsp[0] != 0xAA) && (rsp[1] != 0x1) ){
			/* voltage mismatch */
			DBG_ERR();
			return( FALSE );
		}
		DBG_TRACE1( "mmc(%u):SDV2\n", pMmc->prm.port );
		pMmc->info.type |= MMC_TYPE_SDV2;
		hcs = 1;
	}
	else {
		OSW_TSK_sleep( 1 );
	}
	
	t = OSW_TIM_value();
	while( 1 ){
		if( (OSW_TIM_value() - t) >= 1500 ){
			/* Time Out */
			DBG_ERR();
			return( FALSE );
		}
		/* SD_SEND_OP_COND */
		if( mmc_acmd_send( pMmc, MMC_ACMD41, (0xFF8000|(hcs<<30)), 0 ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
			DBG_ERR();
			return( FALSE );
		}
		mmc_resp_read( pMmc, rsp, 4 );
		if( rsp[3] & 0x80 ){
			/* Initialization Complete */
			if( hcs && (rsp[3] & 0x40) ){
				DBG_TRACE1( "mmc(%u):SDHC\n", pMmc->prm.port );
				pMmc->info.type |= MMC_TYPE_SDHC;
			}
			break;
		}
		OSW_TSK_sleep( 50 );
	};
	
	/* ===== Ready State ===== */
	
	/* ALL_SEND_CID */
	mmc_cmd_send( pMmc, MMC_CMD2, 0, 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	OSW_TSK_sleep( 1 );
	
	/* ===== Ident State ===== */
	
	/* SEND_RELATIVE_ADDR */
	mmc_cmd_send( pMmc, MMC_CMD3, 0, 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	mmc_resp_read( pMmc, rsp, 4 );
	OSW_TSK_sleep( 1 );
	pMmc->info.rca = rsp[2] | (rsp[3] << 8);
	
	/* ===== Stanby State ===== */
	
	/* SEND_CSD */
	mmc_cmd_send( pMmc, MMC_CMD9, (pMmc->info.rca<<16), 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	mmc_resp_read( pMmc, rsp, 16 );
	OSW_TSK_sleep( 1 );
	
	i = rsp[15] >> 6;
	if( i == 1 ){
		/* CSD Version 2.0 */
		DBG_TRACE1( "mmc(%u):CSD Version 2.0\n", pMmc->prm.port );
		i = rsp[6] | (rsp[7]<<8) | ((rsp[8]&0x3F)<<16); /* C_SIZE */
		if( i >= 0xFFFF ){
			DBG_TRACE1( "mmc(%u):SDXC\n", pMmc->prm.port );
			pMmc->info.type |= MMC_TYPE_SDXC;
		}
		i = (i + 1) * 1024;
	}
	else if( i == 0 ){
		/* CSD Version 1.0 */
		DBG_TRACE1( "mmc(%u):CSD Version 1.0\n", pMmc->prm.port );
		i = (rsp[7]>>6) | (rsp[8]<<2) | ((rsp[9]&0x3)<<10); /* C_SIZE */
		j = (rsp[5]>>7) | ((rsp[6]&0x3)<<1); /* C_SIZE_MULT */
		j = 1 << ( j + 2 );	/* MULT */
		i = (i + 1) * j;	/* BLOCKNR */
		j = rsp[10] & 0xF;	/* READ_BL_LEN */
		j = 1 << j;			/* BLOCK_LEN */
		i *= j / 512;
	}
	else {
		DBG_ERR();
		return( FALSE );
	}
	pMmc->info.blk_cnt = i;
	DBG_TRACE1( "mmc(%u):blk_cnt=%u\n", pMmc->prm.port, pMmc->info.blk_cnt );
	
	/* SELECT/DESELECT_CARD */
	mmc_cmd_send( pMmc, MMC_CMD7, (pMmc->info.rca<<16), 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	if( mmc_busy_wait( pMmc ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	OSW_TSK_sleep( 1 );
	
	/* ===== Transfer State ===== */
	
	/* SET_BLOCKLEN */
	mmc_cmd_send( pMmc, MMC_CMD16, 512, 0 );
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	OSW_TSK_sleep( 1 );
	
	/* SET_BUS_WIDTH */
	if( mmc_acmd_send( pMmc, MMC_ACMD6, 2, 0 ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	OSW_TSK_sleep( 1 );
	
	if( hs ){
		/* SWITCH */
		mmc_cmd_send( pMmc, MMC_CMD6, 0x80FFFFF1, 0 );
		if( (OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT ) & (1<<MMC_EVT_CMDEND)) == 0 ){
			DBG_ERR();
			return( FALSE );
		}
		freq = MMC_BUS_FREQ_50MHZ;
		OSW_TSK_sleep( 1 );
	}
	
	if( mmc_bus_cfg( pMmc, MMC_BUS_MODE_4BIT, freq ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	pMmc->info.type |= (MMC_TYPE_MEM|MMC_TYPE_CONFIRM);
	
	return( TRUE );
}


/*==============================================================================*/
/* リセット終了待ち																*/
/*==============================================================================*/
static INT8 mmc_reset_wait( MMC_STR *pMmc )
{
	UINT32 t,i;
	
	t = OSW_TIM_value();
	while( (IOREG32(pMmc->base,MMC_CTRL) & 0x7) ){
		i = OSW_TIM_value() - t;
		if( i >= MMC_LOCAL_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		if( i ){
			OSW_TSK_sleep( 1 );
		}
	};
	
	return( TRUE );
}


/*==============================================================================*/
/* レジスタ初期化																*/
/*==============================================================================*/
static INT8 mmc_reg_init( MMC_STR *pMmc )
{
	UINT32 div,n,i,j;
	
	n = pMmc->prm.port;
	for( i = 0 ; i < 3 ; i ++ ){
		for( div = 0 ; div <= 255 ; div ++ ){
			j = div * 2;
			if( j == 0 ){
				j = 1;
			}
			if( (CFG_MMC_SYSCLK[n] / j) <= mmc_sd_clk[i] ){
				break;
			}
		}
		if( div >= 255 ){
			DBG_ERR();
			return( FALSE );
		}
		pMmc->clkd[i] = div;
		j = div;
		if( j == 0 ) j = 1;
		DBG_TRACE1( "mmc(%u):clk(%u)=%uHz\n", n, i, (CFG_MMC_SYSCLK[n]/j) );
	}
	
	IOREG32(pMmc->base,MMC_CTRL) = 0x7;
	if( mmc_reset_wait( pMmc ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	IOREG32(pMmc->base,MMC_PWREN) = 0x1;
	OSW_TSK_sleep( 1 );
	
	/* Bus Mode */
	if( mmc_bus_cfg( pMmc, MMC_BUS_MODE_1BIT, MMC_BUS_FREQ_400KHZ ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	IOREG32(pMmc->base,MMC_RINTSTS) = 0xFFFFFFFF;
	IOREG32(pMmc->base,MMC_INTMASK) = MMC_IE_VAL;
	IOREG32(pMmc->base,MMC_CTRL) |= (1<<4);
	IOREG32(pMmc->base,MMC_BLKSIZ) = MMC_BLK_SIZE;
	IOREG32(pMmc->base,MMC_CARDTHRCTL) = (MMC_BLK_SIZE<<16)|0x1;
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 Mmc_open( MMC_HANDLE *handle, MMC_PARAM *param )
{
	MMC_STR *pMmc;
	UINT32 n;
	
	if( (param == 0) || 
		(param->port >= MMC_PORT_CNT) ||
		(handle == 0) ||
		(pshMmc[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	pMmc = (MMC_STR *)OSW_MEM_alloc( pshMemMmc, sizeof(MMC_STR), 4 );
	if( pMmc == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pMmc, 0, sizeof(MMC_STR) );
	handle->hdl = (void *)pMmc;
	
	pMmc->prm = *param;
	n = pMmc->prm.port;
	pMmc->base = mmc_base[n];
	
	if( OSW_ISR_create( &pMmc->hIsr, mmc_interrupt_id[n], mmc_isr_entry[n] ) == FALSE ){
		Mmc_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	if( OSW_EVT_create( &pMmc->hEvt, 16 ) == FALSE ){
		Mmc_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	if( mmc_reg_init( pMmc ) == FALSE ){
		Mmc_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	pshMmc[n] = pMmc;
	OSW_ISR_enable( mmc_interrupt_id[n] );
	
	DBG_TRACE1( "Mmc_open(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void Mmc_close( MMC_HANDLE *handle )
{
	MMC_STR *pMmc;
	UINT32 n;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	pMmc = (MMC_STR *)handle->hdl;
	n = pMmc->prm.port;
	
	OSW_ISR_disable( mmc_interrupt_id[n] );
	IOREG32(pMmc->base,MMC_INTMASK) = 0;
	IOREG32(pMmc->base,MMC_RINTSTS) = 0xFFFFFFFF;
	IOREG32(pMmc->base,MMC_CTRL) = 0;
	pshMmc[n] = 0;
	OSW_EVT_delete( &pMmc->hEvt );
	OSW_ISR_delete( &pMmc->hIsr );
	OSW_MEM_free( pshMemMmc, pMmc, sizeof(MMC_STR) );
	handle->hdl = 0;
	
	DBG_TRACE1( "Mmc_close(%u)\n", n );
}


/*==============================================================================*/
/* メモリーR/W転送																*/
/*==============================================================================*/
static INT8 mmc_rw_blk( MMC_STR *pMmc, UINT8 *buf, UINT32 lba, UINT32 blk, UINT32 write )
{
	UINT32 adr,len,i;
	
	if( (pMmc->info.type & (MMC_TYPE_MEM|MMC_TYPE_CONFIRM)) != (MMC_TYPE_MEM|MMC_TYPE_CONFIRM)  ){
		return( MMC_RWSTAT_ERR_CARD );
	}
	
	if( write && (pMmc->info.type & MMC_TYPE_WP) ) return( MMC_RWSTAT_ERR_WP );
	if( blk == 0 ) return( MMC_RWSTAT_OK );
	
	IOREG32(pMmc->base,MMC_CTRL) |= 0x2;
	if( mmc_reset_wait( pMmc ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	while( blk ){
		if( (pMmc->info.type & (MMC_TYPE_SDHC|MMC_TYPE_SDXC)) == 0 ){
			adr = lba << 9;
		}
		else {
			adr = lba;
		}
		len = blk;
		if( len >= 0xFFFF ) len = 0xFFFF;
		
		IOREG32(pMmc->base,MMC_RINTSTS) = 0xFFFFFFFF;
		IOREG32(pMmc->base,MMC_INTMASK) = MMC_IE_TC_VAL;
		pMmc->xfer.buf = buf;
		pMmc->xfer.cmd12 = 0;
		
		if( write ){
			pMmc->xfer.cnt = (-(INT32)len);
			mmc_fifo_write( pMmc );
			if( len == 1 ){
				mmc_cmd_send( pMmc, MMC_CMD24, adr, MMC_BLK_SIZE );
			}
			else {
				pMmc->xfer.cmd12 = 1;
				mmc_cmd_send( pMmc, MMC_CMD25, adr, (MMC_BLK_SIZE*len) );
			}
		}
		else {
			pMmc->xfer.cnt = (INT32)len;
			if( len == 1 ){
				mmc_cmd_send( pMmc, MMC_CMD17, adr, MMC_BLK_SIZE );
			}
			else {
				pMmc->xfer.cmd12 = 1;
				mmc_cmd_send( pMmc, MMC_CMD18, adr, (MMC_BLK_SIZE*len) );
			}
		}
		
		while( 1 ){
			i = OSW_EVT_wait( &pMmc->hEvt, MMC_ACCESS_TOUT );
			if( i & (1<<MMC_EVT_ERR) ){
				DBG_ERR();
				return( MMC_RWSTAT_ERR_DATA );
			}
			else if( i & (1<<MMC_EVT_XFEREND) ){
				if( pMmc->xfer.cnt ){
					DBG_ERR();
					return( MMC_RWSTAT_ERR_DATA );
				}
				break;
			}
			else if( i == 0 ){
				DBG_ERR();
				return( MMC_RWSTAT_ERR_DATA );
			}
		};
		
		blk -= len;
		lba += len;
		buf = &buf[(len<<9)];
	};
	
	return( MMC_RWSTAT_OK );
}


/*==============================================================================*/
/* カード識別API																*/
/*==============================================================================*/
INT8 Mmc_ident( MMC_HANDLE *handle, UINT8 attr, MMC_INFO *info )
{
	MMC_STR *pMmc;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pMmc = (MMC_STR *)handle->hdl;
	
	if( (attr & MMC_ATTR_INS) == 0 ){
		pMmc->info.type = 0;
		return( FALSE );
	}
	
	if( mmc_card_ident( pMmc ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	if( attr & MMC_ATTR_WP ) pMmc->info.type |= MMC_TYPE_WP;
	if( info == 0 ) return( TRUE );
	
	memset( (void *)info, 0, sizeof(MMC_INFO) );
	if( pMmc->info.type & MMC_TYPE_MEM ){
		info->card_type = MMC_INFO_TYPE_MEM;
	}
	info->sector_cnt = pMmc->info.blk_cnt;
	
	return( TRUE );
}


/*==============================================================================*/
/* メモリーリードAPI															*/
/*==============================================================================*/
INT8 Mmc_mem_read( MMC_HANDLE *handle, UINT8 *buf, UINT32 lba, UINT32 blk )
{
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( MMC_RWSTAT_ERR_CARD );
	}
	_E_DEBUG();
	return( mmc_rw_blk( (MMC_STR *)handle->hdl, buf, lba, blk, 0 ) );
}


/*==============================================================================*/
/* メモリーライトAPI															*/
/*==============================================================================*/
INT8 Mmc_mem_write( MMC_HANDLE *handle, UINT8 *buf, UINT32 lba, UINT32 blk )
{
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( MMC_RWSTAT_ERR_CARD );
	}
	_E_DEBUG();
	return( mmc_rw_blk( (MMC_STR *)handle->hdl, buf, lba, blk, 1 ) );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 Mmc_init( OSW_MEM_HANDLE *mem_handle )
{
	pshMemMmc = mem_handle;
	memset( (void *)pshMmc, 0, sizeof(pshMmc) );
	DBG_TRACE1( "Mmc_init()\n" );
	
	return( TRUE );
}






