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
 * @file kernel_config.c
 * @brief カーネル設定の関数を格納しています。
 * @date 2018.01.05
 */
/****************************************************************************/
#include <kernel.h>		// kernel.h内でEXTERN宣言を操作するので、EXTERNの前でincludeする, 18/09/14
#define EXTERN extern

#include <stdint.h>
#include <string.h>

#include "kernel_inc.h"
#include "kernel_config.h"
#include "txi_knl.h"

#include "js_oswapi.h"
#include "js_intc_reg.h"
#include "alt_interrupt.h"
#include "sub_functions.h"


#define TEMPLATE_TASK_CORE0		(1)
#define TEMPLATE_TASK_CORE1		(2)
#define TEMPLATE_TASK_DUAL		(3)

#define TEMPLATE_CORE_TYPE		(TEMPLATE_TASK_CORE0)

void isr_entry_init(void);
void main_task(void);
void dline_task(void);
void cline_task(void);
void timer_task(void);
void display_task(void);
void bezel_task(void);
void otg_task(void);
void subline_task(void);
void uart01_cb_task(void);
void usb0_cb_task(void);
void usb1_cb_task(void);
void fusb_det_task(void);
void dipsw_task(void);
void wdt_task(void);


#if 0
extern ER set_int_typ(INTNO intno, irq_type_t type);		///< 割込みタイプ設定
void _intr_system_timer_proc(void);
void _intr_led_timer_proc(void);
void _intr_dipsw_timer_proc(void);
#endif

#ifdef TX_THREAD_UITRON_ENABLE_OBJECT_NAME

/****************************************************************/
/*						タスク生成パラメータ			*/
/****************************************************************/
// Task configuration
// 第1パラメータ:タスク属性
//   TA_HLNG：高級言語用のインタフェースで処理単位を起動
//   TA_ACT:タスクを実行可能状態で生成
//   TA_FPU:FPUを使用するコンテキストでは指定。無いコンテキストからFPU命令を使用する例外が発生する。
// 第2パラメータ:タスクの拡張情報
// 第3パラメータ:タスクの起動番地
// 第4パラメータ:タスクの起動優先度
//   (TMIN_TPRI(1)～TMAX_TPRI(31))
// 第5パラメータ:タスクのスタック領域のサイズ（バイト数）
// 第6パラメータ:タスクのスタック領域の先頭番地
// 第7パラメータ:タスクの名称（文字列）
long long _kernel_main_stk[0x1000/sizeof(long long)];
T_CTSK const ctsk_main = {TA_HLNG|TA_ACT, (VP_INT)0, (FP)main_task, TPRIORITY_LOWEST, sizeof(_kernel_main_stk), (VP)_kernel_main_stk, "main_task"};
long long _kernel_dline_stk[0x1000/sizeof(long long)];
T_CTSK const ctsk_dline = {TA_HLNG, (VP_INT)0, (FP)dline_task, TPRIORITY_NORMAL, sizeof(_kernel_dline_stk), (VP)_kernel_dline_stk, "dline_task"};
long long _kernel_cline_stk[0x1000/sizeof(long long)];
T_CTSK const ctsk_cline = {TA_HLNG, (VP_INT)0, (FP)cline_task, TPRIORITY_HIGHEST, sizeof(_kernel_cline_stk), (VP)_kernel_cline_stk, "cline_task"};
long long _kernel_timer_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_timer = {TA_HLNG, (VP_INT)0, (FP)timer_task, TPRIORITY_HIGH, sizeof(_kernel_timer_stk), (VP)_kernel_timer_stk, "timer_task"};
long long _kernel_display_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_display = {TA_HLNG, (VP_INT)0, (FP)display_task, TPRIORITY_NORMAL, sizeof(_kernel_display_stk), (VP)_kernel_display_stk, "display_task"};
long long _kernel_bezel_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_bezel = {TA_HLNG, (VP_INT)0, (FP)bezel_task, TPRIORITY_NORMAL, sizeof(_kernel_bezel_stk), (VP)_kernel_bezel_stk, "bezel_task"};
long long _kernel_otg_stk[0x1000/sizeof(long long)];
T_CTSK const ctsk_otg = {TA_HLNG, (VP_INT)0, (FP)otg_task, TPRIORITY_HIGHEST, sizeof(_kernel_otg_stk), (VP)_kernel_otg_stk, "otg_task"};
long long _kernel_subline_stk[0x1000/sizeof(long long)];
T_CTSK const ctsk_subline = {TA_HLNG, (VP_INT)0, (FP)subline_task, TPRIORITY_NORMAL, sizeof(_kernel_subline_stk), (VP)_kernel_subline_stk, "subline_task"};
long long _kernel_uart01_cb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_uart01_cb = {TA_HLNG, (VP_INT)0, (FP)uart01_cb_task, TPRIORITY_HIGH, sizeof(_kernel_uart01_cb_stk), (VP)_kernel_uart01_cb_stk, "uart01_cb_task"};
long long _kernel_usb0_cb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_usb0_cb = {TA_HLNG, (VP_INT)0, (FP)usb0_cb_task, TPRIORITY_HIGH, sizeof(_kernel_usb0_cb_stk), (VP)_kernel_usb0_cb_stk, "usb0_cb_task"};
long long _kernel_usb1_cb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_usb1_cb = {TA_HLNG, (VP_INT)0, (FP)usb1_cb_task, TPRIORITY_HIGH, sizeof(_kernel_usb1_cb_stk), (VP)_kernel_usb1_cb_stk, "usb1_cb_task"};
long long _kernel_fusb_det_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_fusb_det = {TA_HLNG, (VP_INT)0, (FP)fusb_det_task, TPRIORITY_HIGHEST, sizeof(_kernel_fusb_det_stk), (VP)_kernel_fusb_det_stk, "fusbdet_task"};
long long _kernel_dipsw_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_dipsw = {TA_HLNG, (VP_INT)0, (FP)dipsw_task, TPRIORITY_NORMAL, sizeof(_kernel_dipsw_stk), (VP)_kernel_dipsw_stk, "dipsw_task"};
long long _kernel_wdt_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_wdt = {TA_HLNG, (VP_INT)0, (FP)wdt_task, TPRIORITY_LOWEST, sizeof(_kernel_wdt_stk), (VP)_kernel_wdt_stk, "wdt_task"};

/****************************************************************/
/*						イベントフラグ生成パラメータ			*/
/****************************************************************/
// Flg configuration
// 第1パラメータ:イベントフラグ属性、（( TA_TFIFO || TA_TPRI ) | ( TA_WSGL || TA_WMUL ) | [ TA_CLR ]）
//   TA_TFIFO：FIFO 順
//   TA_TPRI:タスクの優先度順
//   TA_WSGL:一つのイベントフラグで同時に複数のタスクが待ち状態となることができません
//   TA_WMUL:同時に複数のタスクが待ち状態にできます
//   TA_CLR:イベントフラグ待ちの解除条件が成り立った時に、タスクをイベントフラグ待ちから解除すると同時に、イベントフラグのビットパターンのすべてのビットをクリアします
// 第2パラメータ:タスクの拡張情報
T_CFLG const _kernel_flg_otg = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "OTG_FLAG"};
T_CFLG const _kernel_flg_uart01_cb = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "UART01_CB_FLAG"};
T_CFLG const _kernel_flg_usb0_cb = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "USB0_CB_FLAG"};
T_CFLG const _kernel_flg_usb1_cb = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "USB1_CB_FLAG"};
T_CFLG const _kernel_flg_fusb_det = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "FUSB_DET_FLAG"};
T_CFLG const _kernel_flg_power = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "POWER_FLAG"};
T_CFLG const _kernel_flg_wdt = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "WDT_FLAG"};


/****************************************************************/
/*						データキュー生成パラメータ			*/
/****************************************************************/
// DTQ configuration
// 第1パラメータ:データキュー属性、（TA_TFIFO || TA_TPRI）
//   TA_TFIFO：FIFO 順
//   TA_TPRI:タスクの優先度順、タスクをイベントフラグ待ちから解除すると同時に、イベントフラグのビットパターンのすべてのビットをクリアします
// 第2パラメータ:データキュー領域の容量（データの個数）
// 第3パラメータ:データキュー領域の先頭番地
// 第4パラメータ:データキューの名称（文字列）
VP_INT _kernel_dtq1_buf[10];
T_CDTQ const _kernel_dtq1 = {TA_TFIFO, 10, _kernel_dtq1_buf, "UART_DTQ"};

/****************************************************************/
/*						メールボックス生成パラメータ			*/
/****************************************************************/
// Mail box configuration
// 第1パラメータ:メールボックス属性、（TA_TFIFO || TA_TPRI）
//   TA_TFIFO: FIFO 順
//   TA_TPRI: タスクの優先度順
//   TA_MFIFO: FIFO 順
//   TA_MPRI:mprihd で指定された番地から、送信されるメッ
//      セージの優先度の最大値がmaxmpri の場合に必要なサイズのメモリ領域を、優先度別のメッ
//      セージキューヘッダ領域として使用します。mprihd にNULL（＝0）が指定された場合には、
//      必要なサイズのメモリ領域をコンフィグレーションで定義したシステム用メモリ領域から自
//      動で確保します。
// 第2パラメータ:送信されるメッセージの優先度の最大値
// 第3パラメータ:優先度別のメッセージキューヘッダ領域の先頭番地
// 第4パラメータ:メールボックスの名称（文字列）
T_CMBX const _kernel_mbx_main = {TA_TFIFO | TA_MFIFO, 0, 0, "MAIN_MBX"};
T_CMBX const _kernel_mbx_dline = {TA_TFIFO | TA_MFIFO, 0, 0, "DLINE_MBX"};
T_CMBX const _kernel_mbx_cline = {TA_TFIFO | TA_MFIFO, 0, 0, "CLINE_MBX"};
T_CMBX const _kernel_mbx_otg = {TA_TFIFO | TA_MFIFO, 0, 0, "OTG_MBX"};
T_CMBX const _kernel_mbx_timer = {TA_TFIFO | TA_MFIFO, 0, 0, "TIMER_MBX"};
T_CMBX const _kernel_mbx_display = {TA_TFIFO | TA_MFIFO, 0, 0, "DISPLAY_MBX"};
T_CMBX const _kernel_mbx_bezel = {TA_TFIFO | TA_MFIFO, 0, 0, "BEZEL_MBX"};
T_CMBX const _kernel_mbx_subline = {TA_TFIFO | TA_MFIFO, 0, 0, "SUBLINE_MBX"};
T_CMBX const _kernel_mbx_uart01_cb = {TA_TFIFO | TA_MFIFO, 0, 0, "UART01_CB_MBX"};
T_CMBX const _kernel_mbx_usb0_cb = {TA_TFIFO | TA_MFIFO, 0, 0, "USB0_CB_MBX"};
T_CMBX const _kernel_mbx_usb1_cb = {TA_TFIFO | TA_MFIFO, 0, 0, "USB1_CB_MBX"};
T_CMBX const _kernel_mbx_fusb_det = {TA_TFIFO | TA_MFIFO, 0, 0, "FUSB_DET_MBX"};
T_CMBX const _kernel_mbx_dipsw = {TA_TFIFO | TA_MFIFO, 0, 0, "DIPSW_MBX"};

/****************************************************************/
/*						セマフォ生成パラメータ			*/
/****************************************************************/
// Semaphore pool configuration
// 第1パラメータ:セマフォ属性、（TA_TFIFO || TA_TPRI）
//   TA_TFIFO: FIFO 順
//   TA_TPRI: タスクの優先度順
// 第2パラメータ:セマフォの資源数の初期値
// 第3パラメータ:セマフォの最大資源数
// 第4パラメータ:セマフォの名称（文字列）
T_CSEM const _kernel_sem_i2c0 = {TA_TFIFO, 1, 1, "I2C0_SEM"};
T_CSEM const _kernel_sem_i2c3 = {TA_TFIFO, 1, 1, "I2C3_SEM"};


/****************************************************************/
/*						固定長メモリプール生成パラメータ			*/
/****************************************************************/
// Memory pool configuration
// 第1パラメータ:固定長メモリプール属性、（TA_TFIFO || TA_TPRI）
//   TA_TFIFO：FIFO 順
//   TA_TPRI:タスクの優先度順
// 第2パラメータ:獲得できるメモリブロック数（個数）
// 第3パラメータ:メモリブロックのサイズ（バイト数）
// 第4パラメータ:固定長メモリプール領域の先頭番地
//   (TMIN_TPRI(1)～TMAX_TPRI(31))
// 第5パラメータ:固定長メモリプールの名称（文字列）
long long _kernel_main_mbx_mpf_buf[(16*32)/sizeof(long long)];
T_CMPF const _kernel_main_mbx_mpf = {TA_TFIFO, 16, 32, (VP)_kernel_main_mbx_mpf_buf, "_kernel_main_mbx_mpf"};
long long _kernel_sub_mbx_mpf_buf[(16*32)/sizeof(long long)];
T_CMPF const _kernel_sub_mbx_mpf = {TA_TFIFO, 16, 32, (VP)_kernel_sub_mbx_mpf_buf, "_kernel_sub_mbx_mpf"};

struct {
	struct {
		uint8_t system[MPL_SYSTEM_SIZE];
	} pool;
} mem __attribute__ ((aligned (4)));
/****************************************************************/
/*						可変長メモリプール生成パラメータ		*/
/****************************************************************/
T_CMPL const _kernel_system_mpl = { TA_TFIFO, sizeof(mem.pool.system), mem.pool.system, "MPL_SYSTEM"};

/****************************************************************/
/*						周期タイマ生成パラメータ					*/
/****************************************************************/
// Cyclic system timer configuration
// 第1パラメータ:周期ハンドラ属性
//   TA_HLNG：高級言語用のインタフェースで処理単位を起動
//   TA_ASM:アセンブリ言語用のインタフェースで処理単位を起動
//   TA_STA:、動作している状態で周期ハンドラを生成。
//   TA_PHS:周期ハンドラの生成時の位相を保存します。
// 第2パラメータ:周期ハンドラの拡張情報
// 第3パラメータ:周期ハンドラの起動番地
// 第4パラメータ:周期ハンドラの起動周期
// 第5パラメータ:周期ハンドラの起動位相
// 第6パラメータ:周期ハンドラの名称（文字列）
extern void _intr_system_timer_proc(void);
T_CCYC _kernel_cyc_system = { TA_HLNG, NULL, (FP) _intr_system_timer_proc, 1, 1, "SYSTEM TIMER" };
#if 0
extern void _intr_led_timer_proc(void);
T_CCYC _kernel_cyc_led = { TA_HLNG, NULL, (FP) _intr_led_timer_proc, 250, 1, "LED TIMER" };
extern void _intr_dipsw_timer_proc(void);
T_CCYC _kernel_cyc_dipsw = { TA_HLNG, NULL, (FP) _intr_dipsw_timer_proc, 100, 1, "DIPSW TIMER" };
#endif

#endif

T_CDTQ  cdtq_main = {TA_TFIFO, DTQ_DEF_CNT, 0};
T_CDTQ  cdtq_wdt = {TA_TFIFO, DTQ_DEF_CNT, 0};
T_CSEM csem_fram = { TA_TFIFO, 1, 1};

/* Kernel configration data */
_KERNEL_UITRON_TSK _kernel_tsk[TID_MAX];
VP _kernel_tsk_id[TID_MAX];
INT _kernel_tsk_maxid = TID_MAX;

_KERNEL_UITRON_SEM _kernel_sem[SEMID_MAX];
VP _kernel_sem_id[SEMID_MAX];
INT _kernel_sem_maxid = SEMID_MAX;

_KERNEL_UITRON_FLG _kernel_flg[FLGID_MAX];
VP _kernel_flg_id[FLGID_MAX];
INT _kernel_flg_maxid = FLGID_MAX;

_KERNEL_UITRON_DTQ _kernel_dtq[DTQID_MAX];
VP _kernel_dtq_id[DTQID_MAX];
INT _kernel_dtq_maxid = DTQID_MAX;

_KERNEL_UITRON_MBX _kernel_mbx[MBXID_MAX];
VP _kernel_mbx_id[MBXID_MAX];
INT _kernel_mbx_maxid = MBXID_MAX;

_KERNEL_UITRON_MPF _kernel_mpf[MPFID_MAX];
VP _kernel_mpf_id[MPFID_MAX];
INT _kernel_mpf_maxid = MPFID_MAX;

_KERNEL_UITRON_MPL _kernel_mpl[MPLID_MAX];
VP _kernel_mpl_id[MPLID_MAX];
INT _kernel_mpl_maxid = MPLID_MAX;

_KERNEL_UITRON_CYC _kernel_cyc[CYCID_MAX];
VP _kernel_cyc_id[CYCID_MAX];
INT _kernel_cyc_maxid = CYCID_MAX;

// ThreadX-uitron4はISRを未サポートのため構造体を宣言する, 19/04/10
VP kernel_isr_id[ISRID_MAX];		// ISR領域
const INT kernel_isr_maxid = ISRID_MAX;

#ifdef TX_ENABLE_EVENT_TRACE
UCHAR	event_buffer[65536];
#endif

/*----------------------------------------------------------------------*/
/* MAIN Task Variables                                                  */
/*----------------------------------------------------------------------*/
// Driver Instantiations
ID MainTaskID;
ID DlineTaskID;
ID ClineTaskID;
ID TimerTaskID;
ID DisplayTaskID;
ID BezelTaskID;
ID OtgTaskID;
ID SublineTaskID;
ID Uart01CbTaskID;
ID Usb0CbTaskID;
ID Usb1CbTaskID;
ID FusbTaskID;
ID DipswTaskID;
ID WdtTaskID;

typedef struct _T_CTSK_INFO{
	ID id;
	ID *pid;
	const T_CTSK *info;
} T_CTSK_INFO;
const T_CTSK_INFO ctsk_info[] = {
	{ID_DLINE_TASK, &DlineTaskID,&ctsk_dline},
	{ID_CLINE_TASK, &ClineTaskID,&ctsk_cline},
	{ID_TIMER_TASK, &TimerTaskID,&ctsk_timer},
	{ID_DISPLAY_TASK, &DisplayTaskID,&ctsk_display},
	{ID_BEZEL_TASK, &BezelTaskID,&ctsk_bezel},
	{ID_OTG_TASK, &OtgTaskID,&ctsk_otg},
	{ID_SUBLINE_TASK, &SublineTaskID,&ctsk_subline},
	{ID_UART01_CB_TASK, &Uart01CbTaskID,&ctsk_uart01_cb},
	{ID_USB0_CB_TASK, &Usb0CbTaskID,&ctsk_usb0_cb},
	{ID_USB1_CB_TASK, &Usb1CbTaskID,&ctsk_usb1_cb},
	{ID_FUSB_DET_TASK, &FusbTaskID,&ctsk_fusb_det},
	{ID_DIPSW_TASK, &DipswTaskID,&ctsk_dipsw},
	{ID_WDT_TASK, &WdtTaskID,&ctsk_wdt},
	{0,0,0}
};
// Driver Instantiations
ID MainMbxID;
ID DlineMbxID;
ID ClineMbxID;
ID OtgMbxID;
ID TimerMbxID;
ID DisplayMbxID;
ID BezelMbxID;
ID SublineMbxID;
ID Uart01RcvMbxID;
ID Usb0RcvMbxID;
ID Usb1RcvMbxID;
ID FusbMbxID;
ID DipswMbxID;

typedef struct _T_CMBX_INFO{
	ID id;
	ID *pid;
	const T_CMBX *info;
} T_CMBX_INFO;
const T_CMBX_INFO cmbx_info[] = {
	{ID_MAIN_MBX, &MainMbxID,&_kernel_mbx_main},
	{ID_DLINE_MBX, &DlineMbxID,&_kernel_mbx_dline},
	{ID_CLINE_MBX, &ClineMbxID,&_kernel_mbx_cline},
	{ID_TIMER_MBX, &TimerMbxID,&_kernel_mbx_timer},
	{ID_DISPLAY_MBX, &DisplayMbxID,&_kernel_mbx_display},
	{ID_BEZEL_MBX, &BezelMbxID,&_kernel_mbx_bezel},
	{ID_SUBLINE_MBX, &SublineMbxID,&_kernel_mbx_subline},
	{ID_UART01_CB_MBX, &Uart01RcvMbxID,&_kernel_mbx_uart01_cb},
	{ID_USB0_CB_MBX, &Usb0RcvMbxID,&_kernel_mbx_usb0_cb},
	{ID_USB1_CB_MBX, &Usb1RcvMbxID,&_kernel_mbx_usb1_cb},
	{ID_FUSB_DET_MBX, &FusbMbxID,&_kernel_mbx_fusb_det},
	{ID_DIPSW_MBX, &DipswMbxID,&_kernel_mbx_dipsw},
	{0,0,0}
};

// Driver Instantiations
ID I2C0SemID;
ID I2C3SemID;
typedef struct _T_CSEM_INFO{
	ID id;
	ID *pid;
	const T_CSEM *info;
} T_CSEM_INFO;
const T_CSEM_INFO csem_info[] = {
	{ID_I2C0_SEM, &I2C0SemID, &_kernel_sem_i2c0},
	{ID_I2C3_SEM, &I2C3SemID, &_kernel_sem_i2c3},
	{0,0,0}
};

// Driver Instantiations
ID OtgFlagID;
ID Uart01RcvFlagID;
ID Usb0RcvFlagID;
ID Usb1RcvFlagID;
ID FusbDetFlagID;
ID WdtFlagID;
typedef struct _T_CFLG_INFO{
	ID id;
	ID *pid;
	const T_CFLG *info;
} T_CFLG_INFO;
const T_CFLG_INFO cflg_info[] = {
	{ID_UART01_CB_FLAG, &Uart01RcvFlagID, &_kernel_flg_uart01_cb},
	{ID_USB0_CB_FLAG, &Usb0RcvFlagID, &_kernel_flg_usb0_cb},
	{ID_USB1_CB_FLAG, &Usb1RcvFlagID, &_kernel_flg_usb1_cb},
	{ID_FUSB_DET_FLAG, &FusbDetFlagID, &_kernel_flg_fusb_det},
	{ID_FLGID_WDT, &WdtFlagID, &_kernel_flg_wdt},
	{0,0,0}
};
// Driver Instantiations
extern T_CMPF const _kernel_main_mbx_mpf;
extern T_CMPF const _kernel_sub_mbx_mpf;
ID MainMpfID;
ID SubMpfID;
typedef struct _T_CMPF_INFO{
	ID id;
	ID *pid;
	const T_CMPF *info;
} T_CMPF_INFO;
const T_CMPF_INFO cmpf_info[] = {
	{ID_MBX_MAIN_MPF, &MainMpfID,&_kernel_main_mbx_mpf},
	{ID_MBX_MPF, &SubMpfID,&_kernel_sub_mbx_mpf},
	{0,0,0}
};

// Driver Instantiations
extern T_CMPL const _kernel_system_mpl;
ID SystemMplID;
typedef struct _T_CMPL_INFO{
	ID id;
	ID *pid;
	const T_CMPL *info;
} T_CMPL_INFO;
const T_CMPL_INFO cmpl_info[] = {
	{ID_SYSTEM_MPL, &SystemMplID,&_kernel_system_mpl},
	{0,0,0}
};


ID SystemCycID;
ID LedCycID;
ID DipswCycID;
typedef struct _T_CCYC_INFO{
	ID id;
	ID *pid;
	const T_CCYC *info;
} T_CCYC_INFO;
const T_CCYC_INFO ccyc_info[] = {
	{ID_SYSTEM_CYC, &SystemCycID,&_kernel_cyc_system},
#if 0
	{ID_LED_CYC, &LedCycID,&_kernel_cyc_led},
	{ID_DIPSW_CYC, &DipswCycID,&_kernel_cyc_dipsw},
#endif
	{0,0,0}
};

/*******************************
        create semaphore
 *******************************/
void _main_init_semaphore(void)
{
	ER er;
	int index = 0;

	while(csem_info[index].id != 0)
	{
		er = cre_sem(csem_info[index].id, (T_CSEM *)csem_info[index].info);
		if(er != E_OK)
		{
	    	// create semaphore failed.
			/* system error */
			program_error();
		}
		*csem_info[index].pid = csem_info[index].id;
		index++;
	}
}
/*******************************
        create event flag
 *******************************/
void _main_init_event(void)
{
	ER er;
	int index = 0;

	while(cflg_info[index].id != 0)
	{
		er = cre_flg(cflg_info[index].id,(T_CFLG *)cflg_info[index].info);
		if(er != E_OK)
		{
	    	// create event flag failed.
			/* system error */
			program_error();
		}
		*cflg_info[index].pid = cflg_info[index].id;
		index++;
	}
}
/*******************************
        create dtq
 *******************************/
void _main_init_dtq(void)
{
#if 0
	ER er;
	er = cre_dtq(ID_UART_RX_DTQ, (T_CDTQ *)&_kernel_dtq1);
	if(er != E_OK)
	{
    	// create event flag failed.
		/* system error */
		program_error();
	}
#endif
}
/*******************************
        create mail box
 *******************************/
void _main_init_mbx(void)
{
	ER er;
	int index = 0;

	while(cmbx_info[index].id != 0)
	{
		er = cre_mbx(cmbx_info[index].id,(T_CMBX *)cmbx_info[index].info);
		if(er != E_OK)
		{
	    	// create mail box failed.
			/* system error */
			program_error();
		}
		*cmbx_info[index].pid = cmbx_info[index].id;
		index++;
	}
}

/*******************************
        create cyclic handler
 *******************************/
void _main_init_cyc(void)
{
	ER er;
	int index = 0;

	while(ccyc_info[index].id != 0)
	{
		er = cre_cyc(ccyc_info[index].id,(T_CCYC *)ccyc_info[index].info);
		if(er != E_OK)
		{
	    	// create cyclic handler failed.
			/* system error */
			program_error();
		}
		*ccyc_info[index].pid = ccyc_info[index].id;
		index++;
	}
}


/*******************************
        start cyclic handler
 *******************************/
void _main_start_cyc(void)
{
	ER er;
	int index = 0;

	while(ccyc_info[index].id != 0)
	{
		er = sta_cyc(ccyc_info[index].id);
		if(er != E_OK)
		{
	    	// create cyclic handler failed.
			/* system error */
			program_error();
		}
		index++;
	}
}
/*******************************
        create memory pool
 *******************************/
void _main_init_mpl(void)
{
	ER er;
	int index = 0;

	while(cmpl_info[index].id != 0)
	{
		er = cre_mpl(cmpl_info[index].id,(T_CMPL *)cmpl_info[index].info);
		if(er != E_OK)
		{
	    	// create mpl failed.
			/* system error */
			program_error();
		}
		*cmpl_info[index].pid = cmpl_info[index].id;
		index++;
	}
}
/*******************************
        create memory pool
 *******************************/
void _main_init_mpf(void)
{
	ER er;
	int index = 0;

	while(cmpf_info[index].id != 0)
	{
		er = cre_mpf(cmpf_info[index].id,(T_CMPF *)cmpf_info[index].info);
		if(er != E_OK)
		{
	    	// create mpf failed.
			/* system error */
			program_error();
		}
		*cmpf_info[index].pid = cmpf_info[index].id;
		index++;
	}
}

/*******************************
        create task
 *******************************/
void _main_create_peripheral_task(void)
{
	ER er;
	int index = 0;


	while(ctsk_info[index].id != 0)
	{
		er = cre_tsk(ctsk_info[index].id,(T_CTSK *)ctsk_info[index].info);
		if(er != E_OK)
		{
	    	// create main task failed.
			/* system error */
			program_error();
		}
		*ctsk_info[index].pid = ctsk_info[index].id;
		index++;
	}
};
/*******************************
        activate task
 *******************************/
void _main_act_peripheral_task(void)
{
	ER er;
	int index = 0;

	while(ctsk_info[index].id != 0)
	{
		if( (ctsk_info[index].id != ID_UART01_CB_TASK)
		 && (ctsk_info[index].id != ID_CLINE_TASK)
		){
			er = act_tsk(ctsk_info[index].id);
			if(er != E_OK)
			{
				// act task failed.
				/* system error */
				program_error();
			}
		}
		index++;
	}
}

/****************************************************************/
/*						割込みハンドラ						*/
/****************************************************************/
#if 0
T_CISR cisr_irq0 = {TA_HLNG, NULL, FPGA_IRQ0, (FP) isr_irq0_proc, IPL_USER_NORMAL};		// 停電検知割込み
T_DINH cinh_irq9 = {TA_HLNG, (FP)isr_inh_irq9_proc, 7};
#endif


extern int _txi_inh_handler_entry(int);		// def_inh.cサービスコール
void txi_inh_entry(uint32_t icciar)
{
	uint32_t ackintid = ALT_INT_ICCIAR_ACKINTID_GET(icciar);
	
	_txi_inh_handler_entry(ackintid);
	
	IOREG32(INTC_BASE, INTC_ICCEOIR) = icciar;
}

/**
 * @brife ポインタ範囲チェック
 * @param p チェック対象ポインタ
**/
bool system_mpl(void* p)
{
	bool ret;
	
	if ((p >= mem.pool.system) && (p < mem.pool.system + sizeof(mem.pool.system))) {		// デバッグ用, 18/08/16
		ret = true;
	} else {
		ret = false;
	}
	
	return ret;
}

/****************************************************************/
/**
 * @brief タスク設定
 */
/****************************************************************/
void task_config(void)
{
	ER err;
	
	// initialize stack
	memset(&mem.pool.system, 0xff, sizeof(mem.pool.system));
	
	err = cre_tsk(ID_MAIN_TASK, (T_CTSK *)&ctsk_main);
	if (err != E_OK)
	{
		program_error();
	}
	_main_create_peripheral_task();
}

/****************************************************************/
/**
 * @brief メッセージボックス設定
 */
/****************************************************************/
void mbx_config(void)
{
	_main_init_mbx();
}

/****************************************************************/
/**
 * @brief データキュー設定
 */
/****************************************************************/
void dtq_config(void)
{
	_main_init_dtq();
}


/****************************************************************/
/**
 * @brief 固定長メモリブロック設定
 */
/****************************************************************/
void mpf_config(void)
{
	_main_init_mpf();
}

/****************************************************************/
/**
 * @brief 可変長メモリプール設定
 */
/****************************************************************/
void mpl_config(void)
{
	_main_init_mpl();
}

/****************************************************************/
/**
 * @brief イベントフラグ設定
 */
/****************************************************************/
void flg_config(void)
{
	_main_init_event();
}

/****************************************************************/
/**
 * @brief 周期タイマ設定
 */
/****************************************************************/
void cyc_config(void)
{
	_main_init_cyc();
}

/****************************************************************/
/**
 * @brief セマフォ設定, 18/12/04
 */
/****************************************************************/
void sem_config(void)
{
	_main_init_semaphore();
}

/****************************************************************/
/**
 * @brief 割込みサービスルーチン定義
 */
/****************************************************************/
void isr_config(void)
{
	isr_entry_init();
	memset(kernel_isr_id, 0, sizeof(kernel_isr_id));		// ISR領域クリア, 19/04/15
}

/****************************************************************/
/**
 * @brief カーネル設定
 */
/****************************************************************/
void kernel_config(void)
{
	task_config();
	mbx_config();
	dtq_config();
	mpf_config();
	mpl_config();
	flg_config();
	cyc_config();
	sem_config();
	isr_config();
}


