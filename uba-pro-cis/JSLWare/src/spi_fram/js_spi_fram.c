/*==============================================================================*/
/* Copyright (C) 2018 JCM Co., Ltd. All right reserved.						    */
/* Tittle: SPI F-RAM driver														*/
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
#include "js_spi_fram.h"
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
#ifndef SPI_FRAM_MAX_PORT
#define	SPI_FRAM_MAX_PORT		1
#endif
//#define	SPI_FRAM_MAX_LEN		0xFFFC
#define	SPI_FRAM_MAX_LEN		0x00020000


/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/

typedef struct {
	SPI_FRAM_PARAM prm;
	OSW_SEM_HANDLE hSem;
	UINT32 cmd_mode;
	UINT32 size;
} SPI_FRAM_STR;

static SPI_FRAM_STR *pshSpi[SPI_FRAM_MAX_PORT];
static OSW_MEM_HANDLE *pshMemSpi = 0;
static UINT32 spi_device_cnt;


/*==============================================================================*/
/* コマンドコード																*/
/*==============================================================================*/
#if defined(__ARMCC_VERSION)
#define WREN  (0x06)
#define WRDI  (0x04)
#define RDSR  (0x05)
#define WRSR  (0x01)
#define READ  (0x03)
#define FSTRD (0x0B)
#define WRITE (0x02)
#define SLEEP (0xB9)
#define RDID  (0x9F)
#else
#define WREN  (0b00000110)
#define WRDI  (0b00000100)
#define RDSR  (0b00000101)
#define WRSR  (0b00000001)
#define READ  (0b00000011)
#define FSTRD (0b00001011)
#define WRITE (0b00000010)
#define SLEEP (0b10111001)
#define RDID  (0b10011111)
#endif


/*==============================================================================*/
/* Write Enable																	*/
/*==============================================================================*/
static INT8 spi_write_enable( SPI_FRAM_STR *pSpi )
{
	UINT32 n = pSpi->prm.port;
	UINT8 c;

	c = 0x06; /* Write Enable */
	if( (*CFG_SPI_FRAM_SEND_FUNC[n])( n, &c, 1, (SPI_FRAM_SMODE_WRITE|SPI_FRAM_SMODE_PACKEND) ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	return( TRUE );
}


/*==============================================================================*/
/* Read 																		*/
/*==============================================================================*/
static INT8 spi_read( SPI_FRAM_STR *pSpi, UINT8 *buf, UINT32 addr, UINT32 len )
{
	UINT32 n = pSpi->prm.port;
	UINT32 i;
	UINT8 c[4];
	INT8 stat = TRUE;

	while( len ){
		i = len;
		if( i > SPI_FRAM_MAX_LEN ) i = SPI_FRAM_MAX_LEN;

		c[0] = READ;
		c[1] = (UINT8)( ( addr & 0x00FF0000 ) >> 16 );
		c[2] = (UINT8)( ( addr & 0x00FF00 ) >> 8 );
		c[3] = (UINT8)( addr & 0x0000FF );

		if( (*CFG_SPI_FRAM_SEND_FUNC[n])( n, c, 4, SPI_FRAM_SMODE_WRITE ) == FALSE ){
			DBG_ERR();
			stat = FALSE;
		}
		if( (*CFG_SPI_FRAM_SEND_FUNC[n])( n, buf, (UINT16)i, (SPI_FRAM_SMODE_READ|SPI_FRAM_SMODE_PACKEND) ) == FALSE ){
			DBG_ERR();
			stat = FALSE;
		}
		if( stat == FALSE ) break;

		len -= i;
		addr += i;
		buf = &buf[i];
	};

	return( stat );
}


/*==============================================================================*/
/* Program Command																*/
/*==============================================================================*/
static INT8 spi_program_sect( SPI_FRAM_STR *pSpi,
				UINT8 *buf, UINT32 addr, UINT32 len, UINT32 *byte )
{
	UINT32 n = pSpi->prm.port;
	UINT8 c[4];
	INT8 ret = TRUE;

	DBG_TRACE2( "spi:spi_program_sect(%u,0x%08X,0x%08X)\n", n, addr,len );
	if( spi_write_enable( pSpi ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}

	if( pSpi->prm.status_cb_func ){
		(*pSpi->prm.status_cb_func)( SPI_FRAM_STAT_WRITEADDR, addr, pSpi->prm.cb_arg );
	}

	c[0] = WRITE; /* Bulk Program */
	c[1] = (UINT8)( ( addr & 0x00FF0000 ) >> 16 );
	c[2] = (UINT8)( ( addr & 0x00FF00 ) >> 8 );
	c[3] = (UINT8)( addr & 0x0000FF );

	if( (*CFG_SPI_FRAM_SEND_FUNC[n])( n, c, 4, SPI_FRAM_SMODE_WRITE ) == FALSE ){
		DBG_ERR();
		ret = FALSE;
	}
	if( (*CFG_SPI_FRAM_SEND_FUNC[n])( n, buf, (UINT16)len, (SPI_FRAM_SMODE_WRITE|SPI_FRAM_SMODE_PACKEND) ) == FALSE ){
		DBG_ERR();
		ret = FALSE;
	}
	if( ret == TRUE ){
		*byte = *byte + len;
	}

	if( (ret == FALSE) && pSpi->prm.status_cb_func ){
		(*pSpi->prm.status_cb_func)( SPI_FRAM_STAT_WRITE_ERR, addr, pSpi->prm.cb_arg );
	}

	return( ret );
}


/*==============================================================================*/
/* Verify																		*/
/*==============================================================================*/
static INT8 spi_verify( SPI_FRAM_STR *pSpi, UINT8 *buf, UINT32 addr, UINT32 len )
{
	UINT32 i;
	UINT8 c[32];

	while( len ){
		i = len;
		if( i >= sizeof(c) ) i = sizeof(c);
		if( spi_read( pSpi, c, addr, i ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		if( memcmp( (void *)c, (void *)buf, i ) != 0 ){
			DBG_ERR();
			return( FALSE );
		}
		len -= i;
		addr += i;
		buf = &buf[i];
	};
	return( TRUE );
}


/*==============================================================================*/
/* Program 																		*/
/*==============================================================================*/
static INT8 spi_program( SPI_FRAM_STR *pSpi, UINT8 *buf, UINT32 addr, UINT32 len, UINT32 *byte )
{
	UINT32 i;

	DBG_TRACE2( "spi:spi_program\n");

	if( len == 0 ) return( TRUE );

	while( len ){
		i = len;
		if( i > SPI_FRAM_MAX_LEN ) i = SPI_FRAM_MAX_LEN;

		if( spi_program_sect( pSpi, buf, addr, i, byte ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
#if 0 // FRAMの仕様（ReadでもWriteする）上ベリファイの意味がないので無効。
		if( spi_verify( pSpi, buf, addr, i ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
#endif

		len -= i;
		addr += i;
		buf = &buf[i];
	}

	return( TRUE );
}



/*==============================================================================*/
/* リードAPI																	*/
/*==============================================================================*/
INT8 SPI_FRAM_Read( SPI_FRAM_HANDLE *handle, SPI_BUF_INFO *buf )
{
	SPI_FRAM_STR *pSpi;
	UINT32 i,j;
	INT8 ret = TRUE;

	if( (handle == 0) || (handle->hdl == 0) || (buf == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pSpi = (SPI_FRAM_STR *)handle->hdl;

	if( buf->addr >= pSpi->size ){
		/* 領域超え */
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}
	j = buf->len;
	i = pSpi->size - buf->addr;
	if( j > i ) j = i;
	if( j == 0 ){
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}

	if( OSW_SEM_pend( &pSpi->hSem, SPI_FRAM_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}

	if( spi_read( pSpi, buf->buf, buf->addr, j ) == FALSE ){
		ret = FALSE;
		j = 0;
	}

	if( buf->byte_count ) *buf->byte_count = j;

	OSW_SEM_post( &pSpi->hSem );

	return( ret );
}


/*==============================================================================*/
/* ライトAPI																	*/
/*==============================================================================*/
INT8 SPI_FRAM_Write( SPI_FRAM_HANDLE *handle, SPI_BUF_INFO *buf )
{
	SPI_FRAM_STR *pSpi;
	UINT32 i,j,k = 0;
	INT8 ret = TRUE;

	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pSpi = (SPI_FRAM_STR *)handle->hdl;

	if( buf->addr >= pSpi->size ){
		/* 領域超え */
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}
	j = buf->len;
	i = pSpi->size - buf->addr;
	if( j > i ) j = i;
	if( j == 0 ){
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}

	if( OSW_SEM_pend( &pSpi->hSem, SPI_FRAM_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}

	if( spi_program( pSpi, buf->buf, buf->addr, j, &k ) == FALSE ){
		DBG_ERR();
		ret = FALSE;
	}

	if( buf->byte_count ) *buf->byte_count = k;

	OSW_SEM_post( &pSpi->hSem );
	DBG_TRACE1( "SPI_FRAM_Write(%u)\n", pSpi->prm.port );

	return( ret );
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 SPI_FRAM_open( SPI_FRAM_HANDLE *handle, SPI_FRAM_PARAM *param, UINT32 *size )
{
	SPI_FRAM_STR *pSpi;
	UINT32 n,*wp,i;

	if( (param == 0) ||
		(handle == 0) ||
		(param->port >= spi_device_cnt) ||
		(pshSpi[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	pSpi = (SPI_FRAM_STR *)OSW_MEM_alloc( pshMemSpi, sizeof(SPI_FRAM_STR), 4 );
	if( pSpi == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pSpi, 0, sizeof(SPI_FRAM_STR) );
	handle->hdl = (void *)pSpi;

	pSpi->prm = *param;
	n = pSpi->prm.port;

	if( OSW_SEM_create( &pSpi->hSem, 1 ) == FALSE ){
		DBG_ERR();
		SPI_FRAM_close( handle );
		return( FALSE );
	}

	wp = (UINT32 *)CFG_SPI_FRAM_INFO[n];
	i = wp[0];
	pSpi->size = i;
	if( size ) *size = i;

	pshSpi[n] = pSpi;
	DBG_TRACE1( "SPI_FRAM_open(%u)\n", n );

	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void SPI_FRAM_close( SPI_FRAM_HANDLE *handle )
{
	SPI_FRAM_STR *pSpi;
	UINT32 n;

	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	pSpi = (SPI_FRAM_STR *)handle->hdl;
	n = pSpi->prm.port;
	pshSpi[n] = 0;
	OSW_SEM_delete( &pSpi->hSem );
	OSW_MEM_free( pshMemSpi, pSpi, sizeof(SPI_FRAM_STR) );
	handle->hdl = 0;

	DBG_TRACE1( "SPI_FRAM_close(%u)\n", n );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 SPI_FRAM_init( OSW_MEM_HANDLE *mem_handle )
{
	UINT32 i;

	pshMemSpi = mem_handle;
	memset( (void *)pshSpi, 0, sizeof(pshSpi) );

	for( i = 0 ; i < SPI_FRAM_MAX_PORT ; i ++ ){
		if( CFG_SPI_FRAM_INFO[i] == NULL ) break;
	}

	if( i == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	spi_device_cnt = i;

	DBG_TRACE2( "spi:device = %u\n", i );
	DBG_TRACE1( "SPI_FRAM_init()\n" );

	return( TRUE );
}

/* EOF */
