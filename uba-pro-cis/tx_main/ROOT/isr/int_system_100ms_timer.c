/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* �{�\�t�g�E�F�A�Ɋ܂܂��\�[�X�R�[�h�ɂ͓��{���K�@�B������ЌŗL��       */
/* ��Ƌ@�����܂�ł��܂��B                                               */
/* �閧�ێ��_�񖳂��Ƀ\�t�g�E�F�A�Ƃ����Ɋ܂܂����̑S�̂������͈ꕔ��   */
/* ���J���������s���܂���B                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file int_system_timer.c
 * @brief
 * @date 2018.01.05 Created.
 */
/****************************************************************************/
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"

#define EXTERN extern
#include "system_timer.h"
#include <time.h>
#include "common.h"
#include "custom.h"
#include "cyc.h"

#define EXT
#include "com_ram.c"


void _intr_system_100ms_timer_proc(UArg32 arg)
{
	/* 理想は10msだがとりあえず100msに入れる */
	#if defined(CHECKER)
	checkr_check_port();
	#endif
	/* 理想は10msだがとりあえず100msに入れる */
	if( ex_100msec_timer != 0)
	{
		--ex_100msec_timer;
	}

	if (ex_rfid_flag_setTimeInit_done)
	{
		ex_rfid_setTime_cnt ++; //今は使用していないが、ICBの30分書き込み用
	}
	
	if( ex_100msec_motor != 0) //2024-11-21 1core
	{
		--ex_100msec_motor;
	}
	if( ex_100msec_wait != 0) //2024-11-21 1core
	{
		--ex_100msec_wait;
	}

}

/* End of File */
