/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: GPIO driver															*/
/* Comment:																		*/
/* 	・初期化設定はドライバ側では行われません									*/
/*==============================================================================*/

/*==============================================================================*/
/* インクルード																	*/
/*==============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "js_oswapi.h"
#include "js_io.h"
#include "js_gpio.h"
#include "js_gpio_reg.h"


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

typedef struct {
	void *cb_isr_func[GPIO_PORT_BIT_CNT];
	void *cb_isr_arg[GPIO_PORT_BIT_CNT];
	UINT8 attr[GPIO_PORT_BIT_CNT];
} GPIO_VECT;

typedef struct {
	OSW_SEM_HANDLE hSem_Resource;
	OSW_ISR_HANDLE *phIsr[GPIO_PORT_CNT];
	GPIO_VECT *pVect[GPIO_PORT_CNT];
	OSW_MEM_HANDLE *phMem;
} GPIO_STR;

static GPIO_STR *pshGpi = 0;


/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 gpio_base[GPIO_PORT_CNT] = {
#if (GPIO_PORT_CNT >= 1)
	GPIO0_BASE
#endif
#if (GPIO_PORT_CNT >= 2)
	,GPIO1_BASE
#endif
#if (GPIO_PORT_CNT >= 3)
	,GPIO2_BASE
#endif
#if (GPIO_PORT_CNT >= 4)
	,GPIO3_BASE
#endif
#if (GPIO_PORT_CNT >= 5)
	,GPIO4_BASE
#endif
#if (GPIO_PORT_CNT >= 6)
	,GPIO5_BASE
#endif
#if (GPIO_PORT_CNT >= 7)
	,GPIO6_BASE
#endif
#if (GPIO_PORT_CNT >= 8)
	,GPIO7_BASE
#endif
#if (GPIO_PORT_CNT >= 9)
	,GPIO8_BASE
#endif
#if (GPIO_PORT_CNT >= 10)
	,GPIO9_BASE
#endif
#if (GPIO_PORT_CNT >= 11)
	,GPIO10_BASE
#endif
#if (GPIO_PORT_CNT >= 12)
	,GPIO11_BASE
#endif
#if (GPIO_PORT_CNT >= 13)
	,GPIO12_BASE
#endif
#if (GPIO_PORT_CNT >= 14)
	,GPIO13_BASE
#endif
#if (GPIO_PORT_CNT >= 15)
	,GPIO14_BASE
#endif
#if (GPIO_PORT_CNT >= 16)
	,GPIO15_BASE
#endif
};


/*==============================================================================*/
/* 割り込みIDテーブル															*/
/*==============================================================================*/
static const UINT16 gpio_interrupt_id[GPIO_PORT_CNT] = {
#if (GPIO_PORT_CNT >= 1)
	OSW_INT_GPIO0_IRQ
#endif
#if (GPIO_PORT_CNT >= 2)
	,OSW_INT_GPIO1_IRQ
#endif
#if (GPIO_PORT_CNT >= 3)
	,OSW_INT_GPIO2_IRQ
#endif
#if (GPIO_PORT_CNT >= 4)
	,OSW_INT_GPIO3_IRQ
#endif
#if (GPIO_PORT_CNT >= 5)
	,OSW_INT_GPIO4_IRQ
#endif
#if (GPIO_PORT_CNT >= 6)
	,OSW_INT_GPIO5_IRQ
#endif
#if (GPIO_PORT_CNT >= 7)
	,OSW_INT_GPIO6_IRQ
#endif
#if (GPIO_PORT_CNT >= 8)
	,OSW_INT_GPIO7_IRQ
#endif
#if (GPIO_PORT_CNT >= 9)
	,OSW_INT_GPIO8_IRQ
#endif
#if (GPIO_PORT_CNT >= 10)
	,OSW_INT_GPIO9_IRQ
#endif
#if (GPIO_PORT_CNT >= 11)
	,OSW_INT_GPIO10_IRQ
#endif
#if (GPIO_PORT_CNT >= 12)
	,OSW_INT_GPIO11_IRQ
#endif
#if (GPIO_PORT_CNT >= 13)
	,OSW_INT_GPIO12_IRQ
#endif
#if (GPIO_PORT_CNT >= 14)
	,OSW_INT_GPIO13_IRQ
#endif
#if (GPIO_PORT_CNT >= 15)
	,OSW_INT_GPIO14_IRQ
#endif
#if (GPIO_PORT_CNT >= 16)
	,OSW_INT_GPIO15_IRQ
#endif
};


/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void gpio_isr( UINT8 num )
{
	GPIO_VECT *vect;
	void (*cb_isr_func)( void *arg );
	UINT32 bit,port;
	UINT16 i;
	
	port = gpio_base[num];
	bit = IOREG32(port,GPIO_INTSTATUS);
	IOREG32(port,GPIO_INTSTATUS) = bit;
	
	if( pshGpi->pVect[num] ){
		vect = pshGpi->pVect[num];
		
		for( i = 0 ; i < GPIO_PORT_BIT_CNT ; i ++ ){
			if( bit & (1<<i) ){
				if( vect->cb_isr_func[i] ){
					cb_isr_func = (void(*)(void*))vect->cb_isr_func[i];
					(*cb_isr_func)( vect->cb_isr_arg[i] );
				}
			}
			
		}
	}
	IOREG32(port,GPIO_PORTA_EOI) = bit;
	
	DBG_TRACE2( "gpio_isr(%u)\n", num );
}


#if (GPIO_PORT_CNT >= 1)
/*==============================================================================*/
/* ポート0割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr0( void ) { gpio_isr(0); }
#endif
#if (GPIO_PORT_CNT >= 2)
/*==============================================================================*/
/* ポート1割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr1( void ) { gpio_isr(1); }
#endif
#if (GPIO_PORT_CNT >= 3)
/*==============================================================================*/
/* ポート2割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr2( void ) { gpio_isr(2); }
#endif
#if (GPIO_PORT_CNT >= 4)
/*==============================================================================*/
/* ポート3割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr3( void ) { gpio_isr(3); }
#endif
#if (GPIO_PORT_CNT >= 5)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr4( void ) { gpio_isr(4); }
#endif
#if (GPIO_PORT_CNT >= 6)
/*==============================================================================*/
/* ポート5割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr5( void ) { gpio_isr(5); }
#endif
#if (GPIO_PORT_CNT >= 7)
/*==============================================================================*/
/* ポート6割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr6( void ) { gpio_isr(6); }
#endif
#if (GPIO_PORT_CNT >= 8)
/*==============================================================================*/
/* ポート7割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr7( void ) { gpio_isr(7); }
#endif
#if (GPIO_PORT_CNT >= 9)
/*==============================================================================*/
/* ポート8割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr8( void ) { gpio_isr(8); }
#endif
#if (GPIO_PORT_CNT >= 10)
/*==============================================================================*/
/* ポート9割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr9( void ) { gpio_isr(9); }
#endif
#if (GPIO_PORT_CNT >= 11)
/*==============================================================================*/
/* ポート10割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr10( void ) { gpio_isr(10); }
#endif
#if (GPIO_PORT_CNT >= 12)
/*==============================================================================*/
/* ポート11割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr11( void ) { gpio_isr(11); }
#endif
#if (GPIO_PORT_CNT >= 13)
/*==============================================================================*/
/* ポート12割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr12( void ) { gpio_isr(12); }
#endif
#if (GPIO_PORT_CNT >= 14)
/*==============================================================================*/
/* ポート13割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr13( void ) { gpio_isr(13); }
#endif
#if (GPIO_PORT_CNT >= 15)
/*==============================================================================*/
/* ポート14割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr14( void ) { gpio_isr(14); }
#endif
#if (GPIO_PORT_CNT >= 16)
/*==============================================================================*/
/* ポート15割り込みエントリ														*/
/*==============================================================================*/
void gpio_isr15( void ) { gpio_isr(15); }
#endif


/*==============================================================================*/
/* 割り込みエントリテーブル														*/
/*==============================================================================*/
static const osw_isr_func gpio_isr_entry[GPIO_PORT_CNT] = {
#if (GPIO_PORT_CNT >= 1)
	gpio_isr0
#endif
#if (GPIO_PORT_CNT >= 2)
	,gpio_isr1
#endif
#if (GPIO_PORT_CNT >= 3)
	,gpio_isr2
#endif
#if (GPIO_PORT_CNT >= 4)
	,gpio_isr3
#endif
#if (GPIO_PORT_CNT >= 5)
	,gpio_isr4
#endif
#if (GPIO_PORT_CNT >= 6)
	,gpio_isr5
#endif
#if (GPIO_PORT_CNT >= 7)
	,gpio_isr6
#endif
#if (GPIO_PORT_CNT >= 8)
	,gpio_isr7
#endif
#if (GPIO_PORT_CNT >= 9)
	,gpio_isr8
#endif
#if (GPIO_PORT_CNT >= 10)
	,gpio_isr9
#endif
};


/*==============================================================================*/
/* IO出力																		*/
/*==============================================================================*/
void Gpio_out( UINT16 gpio_id, UINT8 val )
{
	UINT16 n;
	UINT32 port,isr_stat;
	
	n = gpio_id & (GPIO_PORT_BIT_CNT-1);
	port = gpio_base[gpio_id>>GPIO_BIT_SHIFT];
	
	isr_stat = OSW_ISR_global_disable();
	if( val ){
		IOREG32(port,GPIO_SWPORTA_DR) |= (1<<n);
	}
	else {
		IOREG32(port,GPIO_SWPORTA_DR) &= ~(1<<n);
	}
	OSW_ISR_global_restore( isr_stat );
}


/*==============================================================================*/
/* IO入力																		*/
/*==============================================================================*/
INT8 Gpio_in( UINT16 gpio_id )
{
	UINT16 n = gpio_id & (GPIO_PORT_BIT_CNT-1);
	return( (INT8)((IOREG32(gpio_base[gpio_id>>GPIO_BIT_SHIFT],GPIO_EXT_PORTA) >> n) & 0x1) );
}


/*==============================================================================*/
/* IOモード変更																	*/
/*==============================================================================*/
INT8 Gpio_mode( UINT16 gpio_id, UINT8 mode )
{
	UINT32 isr_stat = OSW_ISR_global_disable();
	UINT32 port;
	UINT16 n;
	INT8 ret = TRUE;
	
	n = gpio_id & (GPIO_PORT_BIT_CNT-1);
	port = gpio_base[gpio_id>>GPIO_BIT_SHIFT];
	
	if( mode == GPIO_MODE_INPUT ){
		IOREG32(port,GPIO_SWPORTA_DDR) &= ~(1<<n);
	}
	else if( mode == GPIO_MODE_OUTPUT ){
		IOREG32(port,GPIO_SWPORTA_DDR) |= (1<<n);
	}
	else {
		ret = FALSE;
		DBG_ERR();
	}
	OSW_ISR_global_restore( isr_stat );
	
	return( ret );
}


/*==============================================================================*/
/* 割り込みエントリ登録															*/
/*==============================================================================*/
static INT8 gpio_isr_regist( UINT16 gpio_id, GPIO_ISR_PARAM *param )
{
	UINT16 n,num = gpio_id >> GPIO_BIT_SHIFT;
	
	n = gpio_id & (GPIO_PORT_BIT_CNT-1);
	
	if( pshGpi->phIsr[num] == NULL ){
		pshGpi->phIsr[num] = (OSW_ISR_HANDLE *)OSW_MEM_alloc( pshGpi->phMem, sizeof(OSW_ISR_HANDLE), 4 );
		if( pshGpi->phIsr[num] == 0 ){
			DBG_ERR();
			return( FALSE );
		}
		memset( (void *)pshGpi->phIsr[num], 0, sizeof(OSW_ISR_HANDLE) );
		
		if( OSW_ISR_create( pshGpi->phIsr[num], gpio_interrupt_id[num], gpio_isr_entry[num] ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		OSW_ISR_enable( gpio_interrupt_id[num] );
		DBG_TRACE2( "gpio_isr_regist(1)\n" );
	}
	
	if( pshGpi->pVect[num] == NULL ){
		pshGpi->pVect[num] = OSW_MEM_alloc( pshGpi->phMem, sizeof(GPIO_VECT), 4 );
		if( pshGpi->pVect[num] == 0 ){
			DBG_ERR();
			return( FALSE );
		}
		memset( (void *)pshGpi->pVect[num], 0, sizeof(GPIO_VECT) );
		
		DBG_TRACE2( "gpio_isr_regist(3)\n" );
	}
	
	pshGpi->pVect[num]->cb_isr_func[n] = (void *)param->cb_isr_func;
	pshGpi->pVect[num]->cb_isr_arg[n] = param->cb_isr_arg;
	pshGpi->pVect[num]->attr[n] = param->attr;
	
	return( TRUE );
}


/*==============================================================================*/
/* 割り込みエントリ登録解除														*/
/*==============================================================================*/
static void gpio_isr_unregist( UINT16 gpio_id )
{
	UINT16 num = gpio_id >> GPIO_BIT_SHIFT;
	UINT16 i,n;
	
	if( pshGpi->pVect[num] ){
		n = gpio_id & (GPIO_PORT_BIT_CNT-1);
		pshGpi->pVect[num]->cb_isr_func[n] = NULL;
		pshGpi->pVect[num]->cb_isr_arg[n] = NULL;
		pshGpi->pVect[num]->attr[n] = 0;
		
		for( i = 0 ; i < GPIO_PORT_BIT_CNT ; i++ ){
			if( pshGpi->pVect[num]->cb_isr_func[i] ) break;
		}
		if( i >= GPIO_PORT_BIT_CNT ){
			OSW_MEM_free( pshGpi->phMem, (void *)pshGpi->pVect[num], sizeof(GPIO_VECT) );
			pshGpi->pVect[num] = NULL;
			DBG_TRACE2( "gpio_isr_unregist(2)\n" );
		}
	}
	
	if( pshGpi->phIsr[num] && (pshGpi->pVect[num] == NULL) ){
		OSW_ISR_disable( gpio_interrupt_id[num] );
		OSW_ISR_delete( pshGpi->phIsr[num] );
		OSW_MEM_free( pshGpi->phMem, (void *)pshGpi->phIsr[num], sizeof(OSW_ISR_HANDLE) );
		pshGpi->phIsr[num] = NULL;
		DBG_TRACE2( "gpio_isr_unregist(1)\n" );
	}
}


/*==============================================================================*/
/* 割り込みポートオープン														*/
/*==============================================================================*/
INT8 GpioIsr_open( UINT16 gpio_id, GPIO_ISR_PARAM *param )
{
	UINT16 num,n;
	UINT32 port,stat;
	INT8 i;
	
	if( param == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	if( param->attr > 0x03 ){
		/* Not Support */
		DBG_ERR();
		return( FALSE );
	}
	
	num = gpio_id >> GPIO_BIT_SHIFT;
	n = gpio_id & (GPIO_PORT_BIT_CNT-1);
	port = gpio_base[num];
	
	if( OSW_SEM_pend( &pshGpi->hSem_Resource, GPIO_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	stat = OSW_ISR_global_disable();
	IOREG32(port,GPIO_INTTYPE_LEVEL) &= (~(1 << n));
	IOREG32(port,GPIO_INT_POLARITY) &= (~(1 << n));
	IOREG32(port,GPIO_INTSTATUS) = (1 << n);
	OSW_ISR_global_restore( stat );
	
	i = gpio_isr_regist( gpio_id, param );
	OSW_SEM_post( &pshGpi->hSem_Resource );
	
	if( i == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	stat = OSW_ISR_global_disable();
	switch( pshGpi->pVect[num]->attr[n] ){
	case GPIO_ATTR_HIGH_EDGE:
		IOREG32(port,GPIO_INTTYPE_LEVEL) |= (1 << n);
		IOREG32(port,GPIO_INT_POLARITY) |= (1 << n);
		break;
	case GPIO_ATTR_LOW_EDGE:
		IOREG32(port,GPIO_INTTYPE_LEVEL) |= (1 << n);
		break;
	case GPIO_ATTR_LOW_LEVEL:
		break;
	case GPIO_ATTR_HIGH_LEVEL:
		IOREG32(port,GPIO_INT_POLARITY) |= (1 << n);
		break;
	default:
		break;
	}
	OSW_ISR_global_restore( stat );
	
	DBG_TRACE1( "GpioIsr_open(%u)\n", gpio_id );
	
	return( TRUE );
}


/*==============================================================================*/
/* 割り込みポートクローズ														*/
/*==============================================================================*/
void GpioIsr_close( UINT16 gpio_id )
{
	if( OSW_SEM_pend( &pshGpi->hSem_Resource, GPIO_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return;
	}
	
	gpio_isr_unregist( gpio_id );
	
	OSW_SEM_post( &pshGpi->hSem_Resource );
	
	DBG_TRACE1( "GpioIsr_close(%u)\n", gpio_id );
}


/*==============================================================================*/
/* 割り込み許可																	*/
/*==============================================================================*/
void GpioIsr_enable( UINT16 gpio_id )
{
	UINT16 num,n;
	UINT32 port,isr;
	
	num = gpio_id >> GPIO_BIT_SHIFT;
	n = gpio_id & (GPIO_PORT_BIT_CNT-1);
	port = gpio_base[gpio_id >> GPIO_BIT_SHIFT];
	
	if( pshGpi->pVect[num] ){
		isr = OSW_ISR_global_disable();
		IOREG32(port,GPIO_INTEN) |= (1 << n);
		OSW_ISR_global_restore( isr );
	}
	else {
		DBG_ERR();
	}
	
	DBG_TRACE2( "GpioIsr_enable(%u)\n", gpio_id );
}


/*==============================================================================*/
/* 割り込み禁止																	*/
/*==============================================================================*/
void GpioIsr_disable( UINT16 gpio_id )
{
	UINT16 num,n;
	UINT32 port,isr;
	
	num = gpio_id >> GPIO_BIT_SHIFT;
	n = gpio_id & (GPIO_PORT_BIT_CNT-1);
	port = gpio_base[gpio_id >> GPIO_BIT_SHIFT];
	
	if( pshGpi->pVect[num] ){
		isr = OSW_ISR_global_disable();
		IOREG32(port,GPIO_INTEN) &= (~(1 << n));
		OSW_ISR_global_restore( isr );
	}
	else {
		DBG_ERR();
	}
	
	DBG_TRACE2( "GpioIsr_disable(%u)\n", gpio_id );
}


/*==============================================================================*/
/* 割り込み要因クリア															*/
/*==============================================================================*/
void GpioIsr_clear( UINT16 gpio_id )
{
	UINT32 port;
	
	port = gpio_base[gpio_id >> GPIO_BIT_SHIFT];
	
	IOREG32(port,GPIO_INTSTATUS) = (1 << (gpio_id & (GPIO_PORT_BIT_CNT-1)));
	
	DBG_TRACE2( "GpioIsr_clear(%u)\n", gpio_id );
}


/*==============================================================================*/
/* ドライバ初期化																*/
/*==============================================================================*/
INT8 Gpio_init( OSW_MEM_HANDLE *mem_handle )
{
	pshGpi = (GPIO_STR *)OSW_MEM_alloc( mem_handle, sizeof(GPIO_STR), 4 );
	if( pshGpi == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	
	memset( (void *)pshGpi, 0, sizeof(GPIO_STR) );
	pshGpi->phMem = mem_handle;
	
	if( OSW_SEM_create( &pshGpi->hSem_Resource, 1 ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	DBG_TRACE1( "Gpio_init()\n" );
	
	return( TRUE );
}





