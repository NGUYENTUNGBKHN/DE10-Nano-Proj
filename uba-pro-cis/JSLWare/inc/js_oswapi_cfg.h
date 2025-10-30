/*==============================================================================*/
/* Copyright (C) 2011 JSL Technology. All right reserved.						*/
/* Tittle: OS Wrraper API Configuration											*/
/* Comment:																		*/
/* 	・OS Wrraper APIの機能制限設定(メモリ節約用)								*/
/*==============================================================================*/

#ifndef __J_OSWIF_CFG__
#define __J_OSWIF_CFG__

#define	OSW_FAIL_STOP		0		/* OS引数異常検出 */
#define	OSW_MSG_DISABLE		0		/* メッセージキューAPI無効 */
#define	OSW_SEM_DISABLE		0		/* セマフォAPI無効 */
#define	OSW_EVT_DISABLE		0		/* イベントフラグAPI無効 */
#define	OSW_ISR_DISABLE		1		/* ISR API無効 */
#define	OSW_MEM_DISABLE		0		/* ヒープAPI無効 */

/* Non OS用 */
#define	OSW_DEF_HEAP		(1024*2)



#endif /* __J_OSWIF_CFG__ */









