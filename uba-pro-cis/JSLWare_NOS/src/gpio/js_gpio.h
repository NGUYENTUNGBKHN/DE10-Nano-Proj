/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: GPIO driver															*/
/* Comment:																		*/
/* 	�E�������ݒ�̓h���C�o���ł͍s���܂���									*/
/*==============================================================================*/

#ifndef __J_GPIO__
#define __J_GPIO__


/*==============================================================================*/
/* �o�[�W�������																*/
/*==============================================================================*/
#define	GPIO_VER				100

/*==============================================================================*/
/* �n���h����`																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} GPIO_ISR_HANDLE;

/*==============================================================================*/
/* �e��f�t�@�C��(�ύX�\)														*/
/*==============================================================================*/
#define	GPIO_ACCESS_TOUT		(1000)	/* �^�C���A�E�g */

/*==============================================================================*/
/* �e��f�t�@�C��(�ύX�s��)														*/
/*==============================================================================*/
#define	GPIO_BIT_SHIFT			5
#define	GPIO_PORT_BIT_CNT		(1<<GPIO_BIT_SHIFT)

/*==============================================================================*/
/* API�����p��`																*/
/*==============================================================================*/

/* gpio_id�}�N�� */
#define	GpioID(port,bit)		((port*GPIO_PORT_BIT_CNT)+bit)

/* attr */
#define	GPIO_ATTR_LOW_EDGE		0x00
#define	GPIO_ATTR_HIGH_EDGE		0x01
#define	GPIO_ATTR_LOW_LEVEL		0x02
#define	GPIO_ATTR_HIGH_LEVEL	0x03

/* mode */
#define	GPIO_MODE_INPUT			0
#define	GPIO_MODE_OUTPUT		1
#define	GPIO_MODE_OPEN_DRAIN	2

typedef struct {
	UINT8 attr;
	void (*cb_isr_func)( void *arg );
	void *cb_isr_arg;
} GPIO_ISR_PARAM;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
void Gpio_out( UINT16 gpio_id, UINT8 val );
INT8 Gpio_in( UINT16 gpio_id );
INT8 Gpio_mode( UINT16 gpio_id, UINT8 mode );
INT8 GpioIsr_open( UINT16 gpio_id, GPIO_ISR_PARAM *param );
void GpioIsr_close( UINT16 gpio_id );
void GpioIsr_enable( UINT16 gpio_id );
void GpioIsr_disable( UINT16 gpio_id );
void GpioIsr_clear( UINT16 gpio_id );
INT8 Gpio_init( OSW_MEM_HANDLE *mem_handle );


#endif /* __J_GPIO__ */









