/*
 * pl_pos.c
 *
 *  Created on: 2019/04/22
 *      Author: yuji-kenta
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
//#include "custom.h"

#include "pl_gpio.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Private Definitions *****************************/
/************************** External Functions *****************************/
extern ER set_int_typ(INTNO intno, irq_type_t type);		///< 割込みタイプ設定

/************************** External Variables *****************************/
/************************** Private Variables *****************************/

#define FPGA_DACSPI_IRQ_INTR_ID			OSW_INT_FPGA_IRQ2
#define FPGA_CISSPI_IRQ_INTR_ID			OSW_INT_FPGA_IRQ4
#define FPGA_UVSCAN_IRQ_INTR_ID			OSW_INT_FPGA_IRQ5
#define FPGA_CISSCAN_IRQ_INTR_ID		OSW_INT_FPGA_IRQ8
#define FPGA_SCANILLEGAL_IRQ_INTR_ID	OSW_INT_FPGA_IRQ13
#define FPGA_SCANNUM_IRQ_INTR_ID		OSW_INT_FPGA_IRQ15
#define FPGA_ENCDN0_IRQ_INTR_ID			OSW_INT_FPGA_IRQ20
#define FPGA_ENCDN1_IRQ_INTR_ID			OSW_INT_FPGA_IRQ21
#define FPGA_ENCDN2_IRQ_INTR_ID			OSW_INT_FPGA_IRQ22
#define FPGA_ENCDN3_IRQ_INTR_ID			OSW_INT_FPGA_IRQ23
#define FPGA_RFIDTX_IRQ_INTR_ID			OSW_INT_FPGA_IRQ24
#define FPGA_RFIDRX_IRQ_INTR_ID			OSW_INT_FPGA_IRQ25
#define FPGA_SDRAMERR_IRQ_INTR_ID		OSW_INT_FPGA_IRQ31
#define FPGA_FEDPRM_IRQ_INTR_ID			OSW_INT_FPGA_IRQ32
#define FPGA_STKPRM_IRQ_INTR_ID			OSW_INT_FPGA_IRQ33
#define FPGA_FEDCNC_IRQ_INTR_ID			OSW_INT_FPGA_IRQ36
#define FPGA_STKCNC_IRQ_INTR_ID			OSW_INT_FPGA_IRQ37

/*********************************************************************//**
 * @brief		gpio initialize
 * @param[in]	None
 * @return 		None
 **********************************************************************/
OSW_ISR_HANDLE hplIsr2;
OSW_ISR_HANDLE hplIsr4;
OSW_ISR_HANDLE hplIsr5;
OSW_ISR_HANDLE hplIsr8;
//OSW_ISR_HANDLE hplIsr10;	/* Reserved */
OSW_ISR_HANDLE hplIsr13;
OSW_ISR_HANDLE hplIsr15;
OSW_ISR_HANDLE hplIsr20;
OSW_ISR_HANDLE hplIsr21;
OSW_ISR_HANDLE hplIsr22;
OSW_ISR_HANDLE hplIsr23;
//OSW_ISR_HANDLE hplIsr24;	/* Reserved */
//OSW_ISR_HANDLE hplIsr25;	/* Reserved */
OSW_ISR_HANDLE hplIsr29;
OSW_ISR_HANDLE hplIsr30;
OSW_ISR_HANDLE hplIsr31;
OSW_ISR_HANDLE hplIsr32;
OSW_ISR_HANDLE hplIsr33;
//OSW_ISR_HANDLE hplIsr34;	/* Reserved */
//OSW_ISR_HANDLE hplIsr36;	/* Reserved */
//OSW_ISR_HANDLE hplIsr37;	/* Reserved */
//OSW_ISR_HANDLE hplIsr38;	/* Reserved */
void _pl_intr_2(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_DACSPI_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_DACSPI_IRQ_INTR_ID);
}
void _pl_intr_4(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_CISSPI_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_CISSPI_IRQ_INTR_ID);
}
void _pl_intr_5(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_UVSCAN_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_UVSCAN_IRQ_INTR_ID);
}
void _pl_intr_8(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_CISSCAN_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_CISSCAN_IRQ_INTR_ID);
}
void _pl_intr_13(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_SCANILLEGAL_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_SCANILLEGAL_IRQ_INTR_ID);
}
void _pl_intr_15(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_SCANNUM_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_SCANNUM_IRQ_INTR_ID);
}
void _pl_intr_20(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_ENCDN0_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_ENCDN0_IRQ_INTR_ID);
}
void _pl_intr_21(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_ENCDN1_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_ENCDN1_IRQ_INTR_ID);
}
void _pl_intr_22(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_ENCDN2_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_ENCDN2_IRQ_INTR_ID);
}
void _pl_intr_23(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_ENCDN3_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_ENCDN3_IRQ_INTR_ID);
}
void _pl_intr_24(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_RFIDTX_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_RFIDTX_IRQ_INTR_ID);
}
void _pl_intr_25(void)
{
	RUART_RX_UNION ruart_rx;
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_RFIDRX_IRQ_INTR_ID);

	if(FPGA_REG.RUART_RX.BIT.ERR_F)
	{
		/* CLR ERR_F */
		ruart_rx = FPGA_REG.RUART_RX;
		FPGA_REG.RUART_RX.LWORD = ruart_rx.LWORD;
	}

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_RFIDRX_IRQ_INTR_ID);
}

// 最小二乗法の計算
void lsm(u32 x[], u32 y[], u32 N, double *a, double *b)
{
	int i;
	double A00 = 0 ,A01 = 0, A02 = 0, A11 = 0, A12 = 0;

	for(i = 0; i < N; i++)
	{
		A00 += 1.0;
		A01 += (double)x[i];
		A02 += (double)y[i];
		A11 += (double)x[i] * (double)x[i];
		A12 += (double)x[i] * (double)y[i];
	}

	*a = (A02 * A11 - A01 * A12) / (A00 * A11 - A01 * A01);
	*b = (A00 * A12 - A01 * A02) / (A00 * A11 - A01 * A01);
}
u32 lsmpoint(double a, double b, u32 point)
{
	u32 val = 0;

	val = (u32)(a + b * (double)point);

	return val;
}
#define PI	3.141592653589793
float calc_skew(u32 start_y, u32 end_y, u32* tmp_x, u32* tmp_y, u32 N)
{
	u32 y1 = start_y;
	u32 y2 = end_y;
	u32 start_x = 0;
	u32 end_x = 0;
	double a;
	double b;
	float skew = 0.0f;
	float radian = 0.0f;

	lsm(tmp_x, tmp_y, N, &a, &b);

	start_x = lsmpoint(a, b, y1);
	end_x = lsmpoint(a, b, y2);

	if(start_x == end_x)
	{
		skew = 0.0f;
	}
	else
	{
		radian = atanf(((float)((s32)end_x - (s32)start_x)*2) / (float)((s32)y2 - (s32)y1));
		skew = (float)(radian * 180.0 / PI);
	}
	return skew;
}
/*********************************************************************//**
 * @brief sub function
 *  detect banknote edge skew
 * @param 		None
 * @return 		SKEW		: true
 *				OTHER		: false
 **********************************************************************/
bool _is_banknote_edge_skew(void)
{
	bool result = false;
#if BANKNOTE_EDGE_SKEW_ENABLE
#if 1//#if (BANKNOTE_EDGE_SKEW_FIX==1)
	bool left_result = false;
	bool right_result = false;
	float left_skew = 255.0f;
	float right_skew = 255.0f;
#endif

	u32 tmp_y_left = 0;		//2024-08-19
	u32 tmp_y_right = 0;	//2024-08-19

#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif
	u32 i = 0;
	u32 ii = 0;
	float skew = 255.0f;

	if((BV_CISEDG_ARRAY.PIX[EDGE_PAPER_CHECK_PIX_MIN/2].BIT.PIX0 == 0x3FF)	//230pix
	|| (BV_CISEDG_ARRAY.PIX[EDGE_PAPER_CHECK_PIX_MAX/2].BIT.PIX0 == 0x3FF))	//490pix
	{//エッジ未検出(短券検出　2点間の片方のエッジ未検出)
		result = true;
	}
	else
	{//エッジ検出
#if 1//#if (BANKNOTE_EDGE_SKEW_FIX==1) /* 左右別にスキュー角度を求める */
		// left_skew
		memset(tmp_x, 0, sizeof(tmp_x));
		memset(tmp_y, 0, sizeof(tmp_y));
		ii = 0;
		for(i = (EDGE_PAPER_CHECK_PIX_MIN)/2; i < (EDGE_PAPER_CHECK_PIX_MIN+80)/2; i++)
		{
			tmp_x[ii] = i*2;
			tmp_y[ii] = BV_CISEDG_ARRAY.PIX[i].BIT.PIX0;
			ii++;
			tmp_x[ii] = i*2+1;
			tmp_y[ii] = BV_CISEDG_ARRAY.PIX[i].BIT.PIX1;
			ii++;
		}
		left_skew = calc_skew((EDGE_PAPER_CHECK_PIX_MIN), (EDGE_PAPER_CHECK_PIX_MIN+80)-1, tmp_x, tmp_y, ii);
		if((left_skew <= -EDGE_PAPER_CHECK_SKEW_THR) || (left_skew >= EDGE_PAPER_CHECK_SKEW_THR))
		{
			left_result = true;
		}
		// right_skew
		memset(tmp_x, 0, sizeof(tmp_x));
		memset(tmp_y, 0, sizeof(tmp_y));
		ii = 0;
		for(i = (EDGE_PAPER_CHECK_PIX_MAX-80)/2; i < (EDGE_PAPER_CHECK_PIX_MAX)/2; i++)
		{
			tmp_x[ii] = i*2;
			tmp_y[ii] = BV_CISEDG_ARRAY.PIX[i].BIT.PIX0;
			ii++;
			tmp_x[ii] = i*2+1;
			tmp_y[ii] = BV_CISEDG_ARRAY.PIX[i].BIT.PIX1;
			ii++;
		}
		right_skew = calc_skew((EDGE_PAPER_CHECK_PIX_MAX-80), (EDGE_PAPER_CHECK_PIX_MAX)-1, tmp_x, tmp_y, ii);
		if((right_skew <= -EDGE_PAPER_CHECK_SKEW_THR) || (right_skew >= EDGE_PAPER_CHECK_SKEW_THR))
		{
			right_result = true;
		}
		if(right_result && left_result)
		{
			result = true;
		}
		if(fabs(left_skew) < fabs(right_skew))
		{
			skew = left_skew;
		}
		else
		{
			skew = right_skew;
		}
#else
		memset(tmp_x, 0, sizeof(tmp_x));
		memset(tmp_y, 0, sizeof(tmp_y));
		for(i = EDGE_PAPER_CHECK_PIX_MIN/2; i < EDGE_PAPER_CHECK_PIX_MAX/2; i++)
		{
			tmp_x[ii] = i*2;
			tmp_y[ii] = BV_CISEDG_ARRAY.PIX[i].BIT.PIX0;
			ii++;
			tmp_x[ii] = i*2+1;
			tmp_y[ii] = BV_CISEDG_ARRAY.PIX[i].BIT.PIX1;
			ii++;
		}
		skew = calc_skew(EDGE_PAPER_CHECK_PIX_MIN, EDGE_PAPER_CHECK_PIX_MAX-1, tmp_x, tmp_y, ii);
		if((skew <= -EDGE_PAPER_CHECK_SKEW_THR) || (skew >= EDGE_PAPER_CHECK_SKEW_THR))
		{
			result = true;
		}
#endif

		//2024-08-19 リサイクルするかの判定に使用 UBA500同様、L4,R4の札検知のタイミングで判定
		for(i = 82; i < 106; i++)
		{
			tmp_y_left = tmp_y_left + BV_CISEDG_ARRAY.PIX[i].BIT.PIX0;
			tmp_y_left = tmp_y_left + BV_CISEDG_ARRAY.PIX[i].BIT.PIX1;
		}
		for(i = 255; i < 279; i++)
		{
			tmp_y_right = tmp_y_right + BV_CISEDG_ARRAY.PIX[i].BIT.PIX0;
			tmp_y_right = tmp_y_right + BV_CISEDG_ARRAY.PIX[i].BIT.PIX1;
		}

		tmp_y_left = tmp_y_left / 48;
		tmp_y_right = tmp_y_right / 48;

		if(tmp_y_left > tmp_y_right)
		{
			ex_rc_skew_pulse = tmp_y_left - tmp_y_right;
		}
		else
		{
			ex_rc_skew_pulse = tmp_y_right - tmp_y_left;
		}

	}
	pbill_data->st_model_area.skew_edge = skew;
#endif
	return result;
}

/*********************************************************************//**
 * @brief sub function
 *  detect banknote edge length
 * @param 		None
 * @return 		float		: edge length
 **********************************************************************/
float _is_banknote_edge_length(void)
{
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif
	float length = 0;
	u32 pix = 0;
	u32 cnt = 0;

	for(cnt = 0; cnt < 360; cnt++)
	{
		if((BV_CISEDG_ARRAY.PIX[cnt].BIT.PIX0 != 0x00)
		&& (BV_CISEDG_ARRAY.PIX[cnt].BIT.PIX0 != 0x3FF))
		{
			pix++;
		}
		if((BV_CISEDG_ARRAY.PIX[cnt].BIT.PIX1 != 0x00)
		&& (BV_CISEDG_ARRAY.PIX[cnt].BIT.PIX1 != 0x3FF))
		{
			pix++;
		}
	}
	length = (float)pix * CIS_MAIN_PITCH;
	pbill_data->st_model_area.length_edge = length;
	return length;
}
void _pl_intr_29(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_SCANBIN_IRQ_INTR_ID);
#if BANKNOTE_EDGE_LENGTH_ENABLE
	if(_is_banknote_edge_length() < 60.0f)
	{
		iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_CIS_SHORT_EDGE);
	}
#endif
#if BANKNOTE_EDGE_SKEW_ENABLE
	if(_is_banknote_edge_skew())
	{
		iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_CIS_SKEW);
	}
#endif
}
void _pl_intr_30(void)
{
#if BANKNOTE_MLT_PAPER_ENABLE
	int cnt = 0;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif
#endif
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_SCANMLT_IRQ_INTR_ID);
#if BANKNOTE_MLT_PAPER_ENABLE
	for(cnt = 0; cnt < 32; cnt++)
	{
		pbill_data->st_model_area.mlti_edge[cnt] = (u16)BV_CISMLT_ARRAY.VRLINE[cnt].VRCH[0].BIT.VRCH;
	}
	iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_CIS_MLT);
#endif
}
void _pl_intr_31(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_SDRAMERR_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_SDRAMERR_IRQ_INTR_ID);
}
void _pl_intr_32(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_FEDPRM_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_FEDPRM_IRQ_INTR_ID);
}
void _pl_intr_33(void)
{
	// Disable GPIO interrupts
	OSW_ISR_disable(FPGA_STKPRM_IRQ_INTR_ID);

	// Enable GPIO interrupts
	OSW_ISR_enable(FPGA_STKPRM_IRQ_INTR_ID);
}
void _pl_gpio_init(void)
{
	// IRQ Handler
	if(hplIsr2.hdl == 0)
	{
		OSW_ISR_create( &hplIsr2, FPGA_CISSPI_IRQ_INTR_ID, _pl_intr_2);
	}
	if(hplIsr5.hdl == 0)
	{
		OSW_ISR_create( &hplIsr5, FPGA_UVSCAN_IRQ_INTR_ID, _pl_intr_5);
	}
	if(hplIsr8.hdl == 0)
	{
		OSW_ISR_create( &hplIsr8, FPGA_CISSCAN_IRQ_INTR_ID, _pl_intr_8);
	}
	if(hplIsr13.hdl == 0)
	{
		OSW_ISR_create( &hplIsr13, FPGA_SCANILLEGAL_IRQ_INTR_ID, _pl_intr_13);
	}
	if(hplIsr15.hdl == 0)
	{
		OSW_ISR_create( &hplIsr15, FPGA_SCANNUM_IRQ_INTR_ID, _pl_intr_15);
	}
	if(hplIsr20.hdl == 0)
	{
		OSW_ISR_create( &hplIsr20, FPGA_ENCDN0_IRQ_INTR_ID, _pl_intr_20);
	}
	if(hplIsr21.hdl == 0)
	{
		OSW_ISR_create( &hplIsr21, FPGA_ENCDN1_IRQ_INTR_ID, _pl_intr_21);
	}
	if(hplIsr22.hdl == 0)
	{
		OSW_ISR_create( &hplIsr22, FPGA_ENCDN2_IRQ_INTR_ID, _pl_intr_22);
	}
	if(hplIsr23.hdl == 0)
	{
		OSW_ISR_create( &hplIsr23, FPGA_ENCDN3_IRQ_INTR_ID, _pl_intr_23);
	}
	if(hplIsr29.hdl == 0)
	{
		OSW_ISR_create( &hplIsr29, FPGA_SCANBIN_IRQ_INTR_ID, _pl_intr_29);
		OSW_ISR_set_priority(FPGA_SCANBIN_IRQ_INTR_ID, IPL_USER_HIGHEST);
		set_int_typ(FPGA_SCANBIN_IRQ_INTR_ID, IRQ_EDGE_RISING);
	}
	OSW_ISR_disable(FPGA_SCANBIN_IRQ_INTR_ID);
	if(hplIsr30.hdl == 0)
	{
		OSW_ISR_create( &hplIsr30, FPGA_SCANMLT_IRQ_INTR_ID, _pl_intr_30);
		OSW_ISR_set_priority(FPGA_SCANMLT_IRQ_INTR_ID, IPL_USER_HIGHEST);
		set_int_typ(FPGA_SCANMLT_IRQ_INTR_ID, IRQ_EDGE_RISING);
	}
	OSW_ISR_disable(FPGA_SCANMLT_IRQ_INTR_ID);
	if(hplIsr31.hdl == 0)
	{
		OSW_ISR_create( &hplIsr31, FPGA_SDRAMERR_IRQ_INTR_ID, _pl_intr_31);
	}
	if(hplIsr32.hdl == 0)
	{
		OSW_ISR_create( &hplIsr32, FPGA_FEDPRM_IRQ_INTR_ID, _pl_intr_32);
	}
	if(hplIsr33.hdl == 0)
	{
		OSW_ISR_create( &hplIsr33, FPGA_STKPRM_IRQ_INTR_ID, _pl_intr_33);
	}
}


/* EOF */
