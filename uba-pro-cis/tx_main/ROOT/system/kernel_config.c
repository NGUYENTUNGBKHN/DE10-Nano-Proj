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
void usb2_cb_task(void);
void fusb_det_task(void);
void dipsw_task(void);
void apb_task(void);
void centering_task(void);
void discrimination_task(void);
void feed_task(void);
void fram_task(void);
void icb_task(void);
void mgu_task(void);
void motor_task(void);
void rfid_task(void);
void sensor_task(void);
void stacker_task(void);
void power_task(void);
void shutter_task(void);

#if defined(UBA_RTQ)
void rc_task(void);
#endif
void wdt_task(void);

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
T_CTSK const ctsk_display = {TA_HLNG, (VP_INT)0, (FP)display_task, TPRIORITY_LOW, sizeof(_kernel_display_stk), (VP)_kernel_display_stk, "display_task"};
long long _kernel_bezel_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_bezel = {TA_HLNG, (VP_INT)0, (FP)bezel_task, TPRIORITY_LOW, sizeof(_kernel_bezel_stk), (VP)_kernel_bezel_stk, "bezel_task"};
long long _kernel_otg_stk[0x1000/sizeof(long long)];
T_CTSK const ctsk_otg = {TA_HLNG, (VP_INT)0, (FP)otg_task, TPRIORITY_HIGHEST, sizeof(_kernel_otg_stk), (VP)_kernel_otg_stk, "otg_task"};
long long _kernel_subline_stk[0x1000/sizeof(long long)];
T_CTSK const ctsk_subline = {TA_HLNG, (VP_INT)0, (FP)subline_task, TPRIORITY_NORMAL, sizeof(_kernel_subline_stk), (VP)_kernel_subline_stk, "subline_task"};
long long _kernel_uart01_cb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_uart01_cb = {TA_HLNG, (VP_INT)0, (FP)uart01_cb_task, TPRIORITY_HIGH, sizeof(_kernel_uart01_cb_stk), (VP)_kernel_uart01_cb_stk, "uart01_cb_task"};
long long _kernel_usb0_cb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_usb0_cb = {TA_HLNG, (VP_INT)0, (FP)usb0_cb_task, TPRIORITY_HIGH, sizeof(_kernel_usb0_cb_stk), (VP)_kernel_usb0_cb_stk, "usb0_cb_task"};
long long _kernel_usb2_cb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_usb2_cb = {TA_HLNG, (VP_INT)0, (FP)usb2_cb_task, TPRIORITY_HIGH, sizeof(_kernel_usb2_cb_stk), (VP)_kernel_usb2_cb_stk, "usb2_cb_task"};
long long _kernel_fusb_det_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_fusb_det = {TA_HLNG, (VP_INT)0, (FP)fusb_det_task, TPRIORITY_HIGHEST, sizeof(_kernel_fusb_det_stk), (VP)_kernel_fusb_det_stk, "fusbdet_task"};
long long _kernel_dipsw_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_dipsw = {TA_HLNG, (VP_INT)0, (FP)dipsw_task, TPRIORITY_LOW, sizeof(_kernel_dipsw_stk), (VP)_kernel_dipsw_stk, "dipsw_task"};
long long _kernel_apb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_apb = {TA_HLNG, (VP_INT)0, (FP)apb_task, TPRIORITY_NORMAL, sizeof(_kernel_apb_stk), (VP)_kernel_apb_stk, "apb_task"};
long long _kernel_centering_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_centering = {TA_HLNG, (VP_INT)0, (FP)centering_task, TPRIORITY_NORMAL, sizeof(_kernel_centering_stk), (VP)_kernel_centering_stk, "centering_task"};
long long _kernel_discrimination_stk[0x40000/sizeof(long long)];
T_CTSK const ctsk_discrimination = {TA_HLNG, (VP_INT)0, (FP)discrimination_task, TPRIORITY_LOW, sizeof(_kernel_discrimination_stk), (VP)_kernel_discrimination_stk, "discrimination_task"};
long long _kernel_feed_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_feed = {TA_HLNG, (VP_INT)0, (FP)feed_task, TPRIORITY_NORMAL, sizeof(_kernel_feed_stk), (VP)_kernel_feed_stk, "feed_task"};
long long _kernel_fram_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_fram = {TA_HLNG, (VP_INT)0, (FP)fram_task, TPRIORITY_NORMAL, sizeof(_kernel_fram_stk), (VP)_kernel_fram_stk, "fram_task"};
long long _kernel_icb_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_icb = {TA_HLNG, (VP_INT)0, (FP)icb_task, TPRIORITY_NORMAL, sizeof(_kernel_icb_stk), (VP)_kernel_icb_stk, "icb_task"};
long long _kernel_mgu_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_mgu = {TA_HLNG, (VP_INT)0, (FP)mgu_task, TPRIORITY_LOW, sizeof(_kernel_mgu_stk), (VP)_kernel_mgu_stk, "mgu_task"};
long long _kernel_motor_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_motor = {TA_HLNG, (VP_INT)0, (FP)motor_task, TPRIORITY_NORMAL, sizeof(_kernel_motor_stk), (VP)_kernel_motor_stk, "motor_task"};
long long _kernel_rfid_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_rfid = {TA_HLNG, (VP_INT)0, (FP)rfid_task, TPRIORITY_NORMAL, sizeof(_kernel_rfid_stk), (VP)_kernel_rfid_stk, "rfid_task"};
long long _kernel_sensor_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_sensor = {TA_HLNG, (VP_INT)0, (FP)sensor_task, TPRIORITY_NORMAL, sizeof(_kernel_sensor_stk), (VP)_kernel_sensor_stk, "sensor_task"};
long long _kernel_stacker_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_stacker = {TA_HLNG, (VP_INT)0, (FP)stacker_task, TPRIORITY_NORMAL, sizeof(_kernel_stacker_stk), (VP)_kernel_stacker_stk, "stacker_task"};
long long _kernel_power_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_power = {TA_HLNG, (VP_INT)0, (FP)power_task, TPRIORITY_HIGHEST, sizeof(_kernel_power_stk), (VP)_kernel_power_stk, "power_task"};

long long _kernel_shutter_stk[0x800/sizeof(long long)];
T_CTSK const ctsk_shutter = {TA_HLNG, (VP_INT)0, (FP)shutter_task, TPRIORITY_NORMAL, sizeof(_kernel_shutter_stk), (VP)_kernel_shutter_stk, "shutter_task"};

#if defined(UBA_RTQ)
	long long _kernel_rc_stk[0x800/sizeof(long long)];
	T_CTSK const ctsk_rc = {TA_HLNG, (VP_INT)0, (FP)rc_task, TPRIORITY_NORMAL, sizeof(_kernel_rc_stk), (VP)_kernel_rc_stk, "rc_task"};
#endif

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
T_CFLG const _kernel_flg_usb2_cb = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "USB2_CB_FLAG"};
T_CFLG const _kernel_flg_fusb_det = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "FUSB_DET_FLAG"};
T_CFLG const _kernel_flg_sensor = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "SENSOR_DET_FLAG"};
T_CFLG const _kernel_flg_feed = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "FEED_DET_FLAG"};
T_CFLG const _kernel_flg_stacker = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "STACKER_DET_FLAG"};
T_CFLG const _kernel_flg_centering = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "CENTERING_DET_FLAG"};
T_CFLG const _kernel_flg_power = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "POWER_FLAG"};
T_CFLG const _kernel_flg_rfid = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "RFID_FLAG"};


T_CFLG const _kernel_flg_apb = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "APB_FLAG"};	//
T_CFLG const _kernel_flg_shutter = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "SHUTTER_FLAG"};	//
T_CFLG const _kernel_flg_fram = {TA_TFIFO | TA_WMUL | TA_CLR, 0x00000000U, "FRAM_FLAG"};	// 2023-12-04



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

VP_INT _kernel_dtq2_buf[10];
T_CDTQ const _kernel_dtq_icb = {TA_TFIFO, 10, _kernel_dtq2_buf, "ICB_DTQ"};

//#if defiend(UBA_RTQ)	//2025-02-02
VP_INT _kernel_dtq3_buf[10];
T_CDTQ const _kernel_dtq_rtq = {TA_TFIFO, 10, _kernel_dtq3_buf, "RTQ_DTQ"};
//#endif

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
T_CMBX const _kernel_mbx_usb2_cb = {TA_TFIFO | TA_MFIFO, 0, 0, "USB2_CB_MBX"};
T_CMBX const _kernel_mbx_fusb_det = {TA_TFIFO | TA_MFIFO, 0, 0, "FUSB_DET_MBX"};
T_CMBX const _kernel_mbx_dipsw = {TA_TFIFO | TA_MFIFO, 0, 0, "DIPSW_MBX"};
T_CMBX const _kernel_mbx_apb = {TA_TFIFO | TA_MFIFO, 0, 0, "APB_MBX"};
T_CMBX const _kernel_mbx_centering = {TA_TFIFO | TA_MFIFO, 0, 0, "CENTERING_MBX"};
T_CMBX const _kernel_mbx_discrimination = {TA_TFIFO | TA_MFIFO, 0, 0, "DISCRIMINATION_MBX"};
T_CMBX const _kernel_mbx_feed = {TA_TFIFO | TA_MFIFO, 0, 0, "FEED_MBX"};
T_CMBX const _kernel_mbx_fram = {TA_TFIFO | TA_MFIFO, 0, 0, "FRAM_MBX"};
T_CMBX const _kernel_mbx_icb = {TA_TFIFO | TA_MFIFO, 0, 0, "ICB_MBX"};
T_CMBX const _kernel_mbx_mgu = {TA_TFIFO | TA_MFIFO, 0, 0, "MGU_MBX"};
T_CMBX const _kernel_mbx_motor = {TA_TFIFO | TA_MFIFO, 0, 0, "MOTOR_MBX"};
T_CMBX const _kernel_mbx_rfid = {TA_TFIFO | TA_MFIFO, 0, 0, "RFID_MBX"};

T_CMBX const _kernel_mbx_sensor = {TA_TFIFO | TA_MFIFO, 0, 0, "SENSOR_MBX"};
T_CMBX const _kernel_mbx_stacker = {TA_TFIFO | TA_MFIFO, 0, 0, "STACKER_MBX"};
T_CMBX const _kernel_mbx_power = {TA_TFIFO | TA_MFIFO, 0, 0, "POWER_MBX"};

T_CMBX const _kernel_mbx_shutter = {TA_TFIFO | TA_MFIFO, 0, 0, "SHUTTER_MBX"};	//
T_CMBX const _kernel_mbx_rc = {TA_TFIFO | TA_MFIFO, 0, 0, "RC_MBX"};	//
T_CMBX const _kernel_mbx_rc_uart_cb = {TA_TFIFO | TA_MFIFO, 0, 0, "RC_UART_CB_MBX"};	//

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
T_CSEM const _kernel_sem_state_tbl = {TA_TFIFO, 1, 1, "STATE_TBL_SEM"};
T_CSEM const _kernel_sem_i2c0 = {TA_TFIFO, 1, 1, "I2C0_SEM"};
T_CSEM const _kernel_sem_i2c3 = {TA_TFIFO, 1, 1, "I2C3_SEM"};
T_CSEM const _kernel_sem_spi = {TA_TFIFO, 1, 1, "SPI_SEM"};


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
//UBA_RFID
extern void _intr_system_100ms_timer_proc(void);
T_CCYC _kernel_cyc_100ms = { TA_HLNG, NULL, (FP) _intr_system_100ms_timer_proc, 100, 1, "SYSTEM TIMER" };

#endif

T_CDTQ  cdtq_main = {TA_TFIFO, DTQ_DEF_CNT, 0};
T_CDTQ  cdtq_wdt = {TA_TFIFO, DTQ_DEF_CNT, 0};
T_CSEM csem_fram = { TA_TFIFO, 1, 1};

/****************************************************************/
/*						割り込みハンドラ						*/
/****************************************************************/
extern void isr_entry_init(void);
extern void isr_irq2_proc(void);
extern void isr_irq4_proc(void);
extern void isr_irq5_proc(void);
extern void isr_irq8_proc(void);
extern void isr_irq10_proc(void);
extern void isr_irq13_proc(void);
extern void isr_irq15_proc(void);
extern void isr_irq20_proc(void);
extern void isr_irq21_proc(void);
extern void isr_irq22_proc(void);
extern void isr_irq23_proc(void);
extern void isr_irq24_proc(void);
extern void isr_irq25_proc(void);
extern void isr_irq31_proc(void);
extern void isr_irq32_proc(void);
extern void isr_irq33_proc(void);
extern void isr_irq34_proc(void);
extern void isr_irq36_proc(void);
extern void isr_irq37_proc(void);
extern void isr_irq38_proc(void);
T_CISR cisr_irq2 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ2, (FP) isr_irq2_proc, IPL_USER_NORMAL};		// DACシリアル通信完了割込み
T_CISR cisr_irq4 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ4, (FP) isr_irq4_proc, IPL_USER_NORMAL};		// CISシリアル通信完了
T_CISR cisr_irq5 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ5, (FP) isr_irq5_proc, IPL_USER_NORMAL};		// UVセンサー採取完了
T_CISR cisr_irq8 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ8, (FP) isr_irq8_proc, IPL_USER_NORMAL};		// CISスキャン完了
T_CISR cisr_irq10 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ10, (FP) isr_irq10_proc, IPL_USER_NORMAL};	// 磁気センサスキャン完了
T_CISR cisr_irq13 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ13, (FP) isr_irq13_proc, IPL_USER_NORMAL};	// データスキャン異常
T_CISR cisr_irq15 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ15, (FP) isr_irq15_proc, IPL_USER_NORMAL};	// 採取番号確定
T_CISR cisr_irq20 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ20, (FP) isr_irq20_proc, IPL_USER_NORMAL};	// ENC_DN0
T_CISR cisr_irq21 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ21, (FP) isr_irq21_proc, IPL_USER_NORMAL};	// ENC_DN1
T_CISR cisr_irq22 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ22, (FP) isr_irq22_proc, IPL_USER_NORMAL};	// ENC_DN2
T_CISR cisr_irq23 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ23, (FP) isr_irq23_proc, IPL_USER_NORMAL};	// ENC_DN3
T_CISR cisr_irq24 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ24, (FP) isr_irq24_proc, IPL_USER_NORMAL};	// RFID-UART 送信完了
T_CISR cisr_irq25 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ25, (FP) isr_irq25_proc, IPL_USER_NORMAL};	// RFID-UART 受信割込み
T_CISR cisr_irq31 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ31, (FP) isr_irq31_proc, IPL_USER_NORMAL};	// データ採取異常-SDRAM転送指示無効
T_CISR cisr_irq32 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ32, (FP) isr_irq32_proc, IPL_USER_NORMAL};	// 搬送モーターパラメーターセット切り替わり
T_CISR cisr_irq33 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ33, (FP) isr_irq33_proc, IPL_USER_NORMAL};	// 収納モーターパラメーターセット切り替わり
T_CISR cisr_irq34 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ34, (FP) isr_irq34_proc, IPL_USER_NORMAL};	// 予約(PBモーターパラメーターセット切り替わり)
T_CISR cisr_irq36 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ36, (FP) isr_irq36_proc, IPL_USER_NORMAL};	// 搬送モーターエンコーダー
T_CISR cisr_irq37 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ37, (FP) isr_irq37_proc, IPL_USER_NORMAL};	// 収納モーターエンコーダー
T_CISR cisr_irq38 = {TA_HLNG, NULL, OSW_INT_FPGA_IRQ38, (FP) isr_irq38_proc, IPL_USER_NORMAL};	// 予約(PBモーターエンコーダー)


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
ID Usb2CbTaskID;
ID FusbTaskID;
ID DipswTaskID;
ID ApbTaskID;
ID CenteringTaskID;
ID DiscriminationTaskID;
ID FeedTaskID;
ID FramTaskID;
ID IcbTaskID;
ID MguTaskID;
ID MotorTaskID;
ID RfidTaskID;
ID SensorTaskID;
ID StackerTaskID;
ID PowerTaskID;
ID ShutterTaskID;	//
ID RcTaskID;
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
	{ID_USB2_CB_TASK, &Usb2CbTaskID,&ctsk_usb2_cb},
	{ID_FUSB_DET_TASK, &FusbTaskID,&ctsk_fusb_det},
	{ID_DIPSW_TASK, &DipswTaskID,&ctsk_dipsw},
	{ID_APB_TASK, &ApbTaskID,&ctsk_apb},
	{ID_CENTERING_TASK, &CenteringTaskID,&ctsk_centering},
	{ID_DISCRIMINATION_TASK, &DiscriminationTaskID,&ctsk_discrimination},
	{ID_FEED_TASK, &FeedTaskID,&ctsk_feed},
	{ID_FRAM_TASK, &FramTaskID,&ctsk_fram},
	{ID_ICB_TASK, &IcbTaskID,&ctsk_icb},
	{ID_MGU_TASK, &MguTaskID,&ctsk_mgu},
	{ID_MOTOR_TASK, &MotorTaskID,&ctsk_motor},
	{ID_RFID_TASK, &RfidTaskID,&ctsk_rfid},
	{ID_SENSOR_TASK, &SensorTaskID,&ctsk_sensor},
	{ID_STACKER_TASK, &StackerTaskID,&ctsk_stacker},
	{ID_POWER_TASK, &PowerTaskID,&ctsk_power},	
	{ID_SHUTTER_TASK, &ShutterTaskID,&ctsk_shutter},
	
#if defined(UBA_RTQ)
	{ID_RC_TASK, &RcTaskID,&ctsk_rc},
#endif
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
ID Usb2RcvMbxID;
ID FusbMbxID;
ID DipswMbxID;
ID ApbMbxID;
ID CenteringMbxID;
ID DiscriminationMbxID;
ID FeedMbxID;
ID FramMbxID;
ID IcbMbxID;
ID MguMbxID;
ID MotorMbxID;
ID RfidMbxID;
ID SensorMbxID;
ID StackerMbxID;
ID PowerMbxID;

ID ShutterMbxID;	//
ID RcMbxID;	//
ID RcUartRcvMbxID;	//

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
	{ID_USB2_CB_MBX, &Usb2RcvMbxID,&_kernel_mbx_usb2_cb},
	{ID_FUSB_DET_MBX, &FusbMbxID,&_kernel_mbx_fusb_det},
	{ID_DIPSW_MBX, &DipswMbxID,&_kernel_mbx_dipsw},
	{ID_APB_MBX, &ApbMbxID,&_kernel_mbx_apb},
	{ID_CENTERING_MBX, &CenteringMbxID,&_kernel_mbx_centering},
	{ID_DISCRIMINATION_MBX, &DiscriminationMbxID,&_kernel_mbx_discrimination},
	{ID_FEED_MBX, &FeedMbxID,&_kernel_mbx_feed},
	{ID_FRAM_MBX, &FramMbxID,&_kernel_mbx_fram},
	{ID_ICB_MBX, &IcbMbxID,&_kernel_mbx_icb},
	{ID_MGU_MBX, &MguMbxID,&_kernel_mbx_mgu},
	{ID_MOTOR_MBX, &MotorMbxID,&_kernel_mbx_motor},
	{ID_RFID_MBX, &RfidMbxID,&_kernel_mbx_rfid},

	{ID_SENSOR_MBX, &SensorMbxID,&_kernel_mbx_sensor},
	{ID_STACKER_MBX, &StackerMbxID,&_kernel_mbx_stacker},
	{ID_POWER_MBX, &PowerMbxID,&_kernel_mbx_power},

	{ID_SHUTTER_MBX, &ShutterMbxID,&_kernel_mbx_shutter},	//
#if defined(UBA_RTQ)
	{ID_RC_MBX, &RcMbxID,&_kernel_mbx_rc},	//
	{ID_RC_UART_CB_MBX, &RcUartRcvMbxID,&_kernel_mbx_rc_uart_cb},	//
#endif
	{0,0,0}
};

// Driver Instantiations
ID I2C0SemID;
ID I2C3SemID;
ID StateTblSemID;
ID SPISemID;
typedef struct _T_CSEM_INFO{
	ID id;
	ID *pid;
	const T_CSEM *info;
} T_CSEM_INFO;
const T_CSEM_INFO csem_info[] = {
	{ID_STATE_TBL_SEM, &StateTblSemID, &_kernel_sem_state_tbl},
	{ID_I2C0_SEM, &I2C0SemID, &_kernel_sem_i2c0},
	{ID_I2C3_SEM, &I2C3SemID, &_kernel_sem_i2c3},
	{ID_SPI_SEM, &SPISemID, &_kernel_sem_spi},
	{0,0,0}
};

// Driver Instantiations
ID OtgFlagID;
ID Uart01RcvFlagID;
ID Usb0RcvFlagID;
ID Usb2RcvFlagID;
ID FusbDetFlagID;
ID SensorFlagID;
ID FeedFlagID;
ID StackerFlagID;
ID CenteringFlagID;
ID PowerFlagID;
ID RfidFlagID;
ID ApbFlagID;	//
ID ShutterFlagID;	//
ID IcbFlagID;	//
ID FramFlagID;	//
ID RcFlagID;	//
ID WdtFlagID;
typedef struct _T_CFLG_INFO{
	ID id;
	ID *pid;
	const T_CFLG *info;
} T_CFLG_INFO;
const T_CFLG_INFO cflg_info[] = {
	{ID_OTG_CTRL_FLAG, &OtgFlagID, &_kernel_flg_otg},
	{ID_UART01_CB_FLAG, &Uart01RcvFlagID, &_kernel_flg_uart01_cb},
	{ID_USB0_CB_FLAG, &Usb0RcvFlagID, &_kernel_flg_usb0_cb},
	{ID_USB2_CB_FLAG, &Usb2RcvFlagID, &_kernel_flg_usb2_cb},
	{ID_FUSB_DET_FLAG, &FusbDetFlagID, &_kernel_flg_fusb_det},
	{ID_SENSOR_FLAG, &SensorFlagID, &_kernel_flg_sensor},
	{ID_FEED_CTRL_FLAG, &FeedFlagID, &_kernel_flg_feed},
	{ID_STACKER_CTRL_FLAG, &StackerFlagID, &_kernel_flg_stacker},
	{ID_CENTERING_CTRL_FLAG, &CenteringFlagID, &_kernel_flg_centering},
	{ID_POWER_FLAG, &PowerFlagID, &_kernel_flg_power},
	{ID_RFID_FLAG, &RfidFlagID, &_kernel_flg_rfid},
	{ID_APB_CTRL_FLAG, &ApbFlagID, &_kernel_flg_apb},	//
	{ID_SHUTTER_CTRL_FLAG, &ShutterFlagID, &_kernel_flg_shutter},	//
	{ID_FRAM_FLAG, &FramFlagID, &_kernel_flg_fram},	//2023-12-04
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
//UBA_RFID
ID SystemCycID_100ms;

typedef struct _T_CCYC_INFO{
	ID id;
	ID *pid;
	const T_CCYC *info;
} T_CCYC_INFO;
const T_CCYC_INFO ccyc_info[] = {
	{ID_SYSTEM_CYC, &SystemCycID,&_kernel_cyc_system},
	{ID_100MSEC_CYC, &SystemCycID_100ms,&_kernel_cyc_100ms},
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
	ER er;
	er = cre_dtq(ID_ICB_DTQ, (T_CDTQ *)&_kernel_dtq_icb);
	if(er != E_OK)
	{
    	// create event flag failed.
		/* system error */
		program_error();
	}

#if defined(UBA_RTQ)	//2025-02-02
	er = cre_dtq(ID_UART_RC_RX_DTQ, (T_CDTQ *)&_kernel_dtq_rtq);
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
	//UBA500はこのタイミングで起動しているが、DIP-SWでのテストモードの場合起動しない方がいい、
	//DIP-SWタスクはこのタイミングで起動するので、ここで ID_CLINE_TASK の起動判断はできない為、
	//この後でDIP-SWタスクが動きだした後で、テストモードか判断し、テストモードでない場合、起動
		if( (ctsk_info[index].id != ID_UART01_CB_TASK)
		 && (ctsk_info[index].id != ID_OTG_TASK)
		 && (ctsk_info[index].id != ID_CLINE_TASK)
		 && (ctsk_info[index].id != ID_SUBLINE_TASK)
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


