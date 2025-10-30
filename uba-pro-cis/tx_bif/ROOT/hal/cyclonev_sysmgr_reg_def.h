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
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file baule17_gpio_def.h
 * @brief BAU-LE17 SYSTEMレジスタ定義
 * @date 2018.03.05 Created
 */
/****************************************************************************/
#if !defined(__CYCLONE_V_SYSMGR_REG_DEF_H_INCLUDED__)
#define __CYCLONE_V_SYSMGR_REG_DEF_H_INCLUDED__

#define SYSMGR_BASE					(0xFFD08000)
#define FLASHIO5_REG_ADDRESS		(0xFFD08464)
#define FLASHIO6_REG_ADDRESS		(0xFFD08468)
#define FLASHIO7_REG_ADDRESS		(0xFFD0846C)
#define FLASHIO8_REG_ADDRESS		(0xFFD08470)
#define GENERALIO12_REG_ADDRESS		(0xFFD084B0)

// EMACIO
#define SYS_EMACIO0 0x400
#define SYS_EMACIO1 0x404
#define SYS_EMACIO2 0x408
#define SYS_EMACIO3 0x40c
#define SYS_EMACIO4 0x410
#define SYS_EMACIO5 0x414
#define SYS_EMACIO6 0x418
#define SYS_EMACIO7 0x41c
#define SYS_EMACIO8 0x420
#define SYS_EMACIO9 0x424
#define SYS_EMACIO10 0x428
#define SYS_EMACIO11 0x42c
#define SYS_EMACIO12 0x430
#define SYS_EMACIO13 0x434
//EMACIO
#define SYS_FLASHIO0 0x450
#define SYS_FLASHIO1 0x454
#define SYS_FLASHIO2 0x458
#define SYS_FLASHIO3 0x45c
#define SYS_FLASHIO4 0x460
#define SYS_FLASHIO5 0x464
#define SYS_FLASHIO6 0x468
#define SYS_FLASHIO7 0x46C
#define SYS_FLASHIO8 0x470
#define SYS_FLASHIO9 0x474
#define SYS_FLASHIO10 0x478
#define SYS_FLASHIO11 0x47c
//GENERALIO
#define SYS_GENERALIO0 0x480
#define SYS_GENERALIO1 0x484
#define SYS_GENERALIO2 0x488
#define SYS_GENERALIO3 0x48c
#define SYS_GENERALIO4 0x490
#define SYS_GENERALIO5 0x494
#define SYS_GENERALIO6 0x498
#define SYS_GENERALIO7 0x49c
#define SYS_GENERALIO8 0x4a0
#define SYS_GENERALIO9 0x4a4
#define SYS_GENERALIO10 0x4a8
#define SYS_GENERALIO11 0x4ac
#define SYS_GENERALIO12 0x4b0
#define SYS_GENERALIO13 0x4b4
#define SYS_GENERALIO14 0x4b8
#define SYS_GENERALIO15 0x4bc
#define SYS_GENERALIO16 0x4c0
#define SYS_GENERALIO17 0x4c4
#define SYS_GENERALIO18 0x4c8
//MIXED1IO
#define SYS_MIXED1IO0 0x500
#define SYS_MIXED1IO1 0x504
#define SYS_MIXED1IO2 0x508
#define SYS_MIXED1IO3 0x50c
#define SYS_MIXED1IO4 0x510
#define SYS_MIXED1IO5 0x514
#define SYS_MIXED1IO6 0x518
#define SYS_MIXED1IO7 0x51c
#define SYS_MIXED1IO8 0x520
#define SYS_MIXED1IO9 0x524
#define SYS_MIXED1IO10 0x528
#define SYS_MIXED1IO11 0x52c
#define SYS_MIXED1IO12 0x530
#define SYS_MIXED1IO13 0x534
#define SYS_MIXED1IO14 0x538
#define SYS_MIXED1IO15 0x53c
#define SYS_MIXED1IO16 0x540
#define SYS_MIXED1IO17 0x544
#define SYS_MIXED1IO18 0x548
#define SYS_MIXED1IO19 0x54c
#define SYS_MIXED1IO20 0x550
#define SYS_MIXED1IO21 0x554

/****************************************************************/
/**
 * @brief GENERALIO12
 */
/****************************************************************/
union GENERALIO12_REG_UNION
{
	u32 _DWORD;
	struct
	{
		u32 sel: 2;
		u32 : 30;
	} BIT;
};


#define GENERALIO12_REG  (*(volatile union GENERALIO12_REG_UNION*)(GENERALIO12_REG_ADDRESS))



#endif /* __CYCLONE_V_SYSMGR_REG_DEF_H_INCLUDED__ */

/* End of file */

