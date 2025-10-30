/****************************************************************************/
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2017                          */
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
 * MODEL NAME : （モデル名）
 * @file jdl_conf.h
 * @brief  JCM Device Log Configuration Header
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/
#pragma once


/*==========================================================================*/
/*==========================================================================*/
/* Common definitions                                                       */
/*==========================================================================*/
//#define _ENABLE_JDL

/*----------------------------------------------------------*/
/* JCM Device Log DEBUG                                     */
/*----------------------------------------------------------*/
//#define _DEBUG_JDL /* yamazaki JDL DEBUG */

/*----------------------------------------------------------*/
/* JCM Device Log Revision                                  */
/*----------------------------------------------------------*/
#define JDL_FORMAT_REV 0x0001

/*----------------------------------------------------------*/
/* Compiler Type                                            */
/*----------------------------------------------------------*/
//#define _JDL_IAR_EWARM
//#define _JDL_TI_CCS
//#define _JDL_XILINX_SDK
#define _JDL_ARMDS_SDK

/*----------------------------------------------------------*/
/* Back Up Memory Access Type                               */
/*----------------------------------------------------------*/
//#define _JDL_PMIO

/*----------------------------------------------------------*/
/* Byte order type                                          */
/*----------------------------------------------------------*/
#define _JDL_LITTLE_ENDIAN  /* Undefine : BIG ENDIAN */

/*----------------------------------------------------------*/
/* Number of 10 msec ticks for JDL Time                     */
/*----------------------------------------------------------*/
#define JDL_NUM_OF_10MSEC_TICK  10


/*----------------------------------------------------------*/
/* Task Mode size                                           */
/*----------------------------------------------------------*/
#define JDL_SIZE_TASK_MODE  2


/*----------------------------------------------------------*/
/* Sensor state size                                        */
/*----------------------------------------------------------*/
//#define JDL_SIZE_SENS_STAT  5	//default
#define JDL_SIZE_SENS_STAT  5  /* 24-11-05 */


/*----------------------------------------------------------*/
/* Error code size                                          */
/*----------------------------------------------------------*/
#define JDL_SIZE_ERR_CODE  2


/*==========================================================================*/
/*==========================================================================*/
/* JDL Common                                                               */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/
/* Base Address of JDL common buffer */
#define JDL_ADR_BUFF_BASE  0                                    /* [Buffer Base:0] */
#define JDL_ADR_REV        JDL_ADR_BUFF_BASE                    /* BADR:     0, CADR:     0 */


/*----------------------------------------------------------*/
/* Address for the reserved                                 */
/*----------------------------------------------------------*/
#define JDL_ADR_FOR_RESERVED (JDL_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Address for the reserved:2] */


/*----------------------------------------------------------*/
/* Address for the next category                            */
/*----------------------------------------------------------*/
#define JDL_ADR_FOR_1ST_CATEGORY (JDL_ADR_FOR_RESERVED + 0)          /* [Address for the 1st category:2] */



/*==========================================================================*/
/*==========================================================================*/
/* System                                                                   */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/

/*----------------------------------------------------------*/
/* Revision                                                 */
/*----------------------------------------------------------*/
#define JDL_SYS_REV            0x0001


/*----------------------------------------------------------*/
/* Base Address of System Category                          */
/*----------------------------------------------------------*/
/* Base Address of System Category buffer */
#define JDL_SYS_ADR_BUFF_BASE  JDL_ADR_FOR_1ST_CATEGORY                     /* [Buffer Base:2] */
                                                                            /* BADR:Buffer Address, CADR:Category Data Address */
#define JDL_SYS_ADR_REV        JDL_SYS_ADR_BUFF_BASE                        /* BADR:     2, CADR:     0 */

/* Base Address of System Category data to send */
#define JDL_SYS_ADR_SEND_BASE  (JDL_SYS_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:4] */


/*----------------------------------------------------------*/
/* Required Datas Address                                   */
/*----------------------------------------------------------*/
/* !!! Required datas >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
#define JDL_SYS_ADR_MODEL      JDL_SYS_ADR_SEND_BASE                       /* BADR:     4, CADR:     2 */
#define JDL_SYS_ADR_SERIAL1    (JDL_SYS_ADR_MODEL + JDL_SIZE_MODEL_NAME)   /* BADR:    36, CADR:    34 */
#define JDL_SYS_ADR_SERIAL2    (JDL_SYS_ADR_SERIAL1 + JDL_SIZE_SERIAL_NO)  /* BADR:    48, CADR:    46 */
#define JDL_SYS_ADR_FIRM_VER   (JDL_SYS_ADR_SERIAL2 + JDL_SIZE_SERIAL_NO)  /* BADR:    60, CADR:    58 */
#define JDL_SYS_ADR_BOOT_VER   (JDL_SYS_ADR_FIRM_VER + JDL_SIZE_FIRM_VER)  /* BADR:   124, CADR:   122 */
#define JDL_SYS_ADR_SET_TIME   (JDL_SYS_ADR_BOOT_VER + JDL_SIZE_BOOT_VER)  /* BADR:   140, CADR:   138 */
#define JDL_SYS_ADR_ELAP_TIME  (JDL_SYS_ADR_SET_TIME + JDL_SIZE_TIME)      /* BADR:   148, CADR:   146 */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Required datas !!! */


/*----------------------------------------------------------*/
/* Optional Datas Address                                   */
/*----------------------------------------------------------*/
#define JDL_SYS_ADR_OPT_DIPSW  (JDL_SYS_ADR_ELAP_TIME + JDL_SIZE_TIME)     /* BADR:   154, CADR:   156 */
#define JDL_SYS_ADR_OPT_OPTION (JDL_SYS_ADR_OPT_DIPSW + 2)                 /* BADR:   156, CADR:   158 */


/*----------------------------------------------------------*/
/* Address for the reserved                                 */
/*----------------------------------------------------------*/
#define JDL_SYS_ADR_FOR_RESERVED (JDL_SYS_ADR_OPT_OPTION + 4)    /* [Address for the reserved] BADR:   162, CADR:   160 */


/*----------------------------------------------------------*/
/* Address for the next category                            */
/*----------------------------------------------------------*/
#define JDL_SYS_ADR_FOR_NEXT_CATEGORY (JDL_SYS_ADR_FOR_RESERVED + 0) /* [Address for the next category] BADR:   162, CADR:   160  (Reserved area = 0) */


/*----------------------------------------------------------*/
/* System Total size                                        */
/*----------------------------------------------------------*/
/* Total size of System Category data to send */
#define JDL_SYS_SEND_TOTAL (JDL_SYS_ADR_FOR_RESERVED - JDL_SYS_ADR_SEND_BASE)      /* [System Send Data Total:158] Without Reserved and Revision */
/* Total size of System Category buffer without reserved area */
#define JDL_SYS_BUFF_TOTAL (JDL_SYS_ADR_FOR_NEXT_CATEGORY - JDL_SYS_ADR_BUFF_BASE) /* [System Buffer Total:160] 162 - 2(Next Addr - Base Addr) */



/*==========================================================================*/
/*==========================================================================*/
/* Statistics                                                               */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/

/*----------------------------------------------------------*/
/* Revision                                                 */
/*----------------------------------------------------------*/
#define JDL_STAT_REV               0x0001


/*----------------------------------------------------------*/
/* Base Address of Statistics Category                      */
/*----------------------------------------------------------*/
/* Base Address of Statistics Category buffer */
#define JDL_STAT_ADR_BUFF_BASE  JDL_SYS_ADR_FOR_NEXT_CATEGORY                           /* [Buffer Base:162] */
                                                                                        /* BADR:Buffer Address, CADR:Category Data Address */
#define JDL_STAT_ADR_REV        JDL_STAT_ADR_BUFF_BASE                                  /* BADR:   162, CADR:     0 */

/* Base Address of Statistics Category data to send */
#define JDL_STAT_ADR_SEND_BASE  (JDL_STAT_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:164] */


/*----------------------------------------------------------*/
/* Counters Address     Statistics                          */
/*----------------------------------------------------------*/
/*- Counters of Movements and Control related --------------*/
/* !!! Required datas >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
#define JDL_STAT_ADR_MOV_BASE  JDL_STAT_ADR_SEND_BASE /* [MOV counter BASE Address:164] */

    /* Counter settings */
    #define JDL_STAT_SIZE_MOV_CNTR JDL_DATA_TYPE_SIZE_DWORD /* [SIZE:4] */

#define JDL_STAT_ADR_MOV_POWERUP   JDL_STAT_ADR_MOV_BASE                                        /* BADR:   164, CADR:     2 */
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Required datas !!! */
#define JDL_STAT_ADR_MOV_FMOT_CNT  (JDL_STAT_ADR_MOV_POWERUP + JDL_STAT_SIZE_MOV_CNTR)          /* BADR:   168, CADR:     6 */
#define JDL_STAT_ADR_MOV_LMOT_CNT (JDL_STAT_ADR_MOV_FMOT_CNT + JDL_STAT_SIZE_MOV_CNTR)         /* BADR:   172, CADR:    10 *///shutter count
#define JDL_STAT_ADR_MOV_CMOT_CNT  (JDL_STAT_ADR_MOV_LMOT_CNT + JDL_STAT_SIZE_MOV_CNTR)        /* BADR:   176, CADR:    14 */
#define JDL_STAT_ADR_MOV_SMOT_CNT (JDL_STAT_ADR_MOV_CMOT_CNT + JDL_STAT_SIZE_MOV_CNTR)         /* BADR:   180, CADR:    18 *///stacker count
#define JDL_STAT_ADR_MOV_AMOT_CNT  (JDL_STAT_ADR_MOV_SMOT_CNT + JDL_STAT_SIZE_MOV_CNTR)        /* BADR:   184, CADR:    22 */
#define JDL_STAT_ADR_MOV_AMOT_TIME (JDL_STAT_ADR_MOV_AMOT_CNT + JDL_STAT_SIZE_MOV_CNTR)         /* BADR:   188, CADR:    26 */
#define JDL_STAT_ADR_MOV_FRAM      (JDL_STAT_ADR_MOV_AMOT_TIME + JDL_STAT_SIZE_MOV_CNTR)        /* BADR:   192, CADR:    30 */
#define JDL_STAT_ADR_MOV_ENER_TIME (JDL_STAT_ADR_MOV_FRAM + JDL_STAT_SIZE_MOV_CNTR)             /* BADR:   196, CADR:    34 */
#define JDL_STAT_ADR_MOV_RESET     (JDL_STAT_ADR_MOV_ENER_TIME + JDL_STAT_SIZE_MOV_CNTR)        /* BADR:   200, CADR:    38 */
#define JDL_STAT_ADR_MOV_RESERVED1 (JDL_STAT_ADR_MOV_RESET + JDL_STAT_SIZE_MOV_CNTR)            /* BADR:   204, CADR:    42 */
#define JDL_STAT_ADR_MOV_CHECKSUM  (JDL_STAT_ADR_MOV_RESERVED1 + JDL_STAT_SIZE_MOV_CNTR)        /* BADR:   208, CADR:    46 */

#define JDL_STAT_NEXTADR_FROM_MOV  (JDL_STAT_ADR_MOV_CHECKSUM + JDL_SIZE_CHECKSUM) /* [Next Item Address] BADR:   210, CADR:    48 */

/* Counters of Accepting related */
#define _JDL_STAT_USE_ACC_CNT
#if defined(_JDL_STAT_USE_ACC_CNT) //受け取り返却など
    #define JDL_STAT_ADR_ACC_BASE JDL_STAT_NEXTADR_FROM_MOV /* [ACC counter BASE Address:210] */
    
        /* Counter settings */
        #define JDL_STAT_SIZE_ACC_CNTR JDL_DATA_TYPE_SIZE_DWORD /* [SIZE:4] */
        
    #define JDL_STAT_ADR_ACC_INST       JDL_STAT_ADR_ACC_BASE                                   /* BADR:   210, CADR:    48 *///取り込み
    #define JDL_STAT_ADR_ACC_BILL_INST  (JDL_STAT_ADR_ACC_INST + JDL_STAT_SIZE_ACC_CNTR)        /* BADR:   214, CADR:    52 *///紙幣収納でカウントアップ、紙幣返却でカウントアップ
    #define JDL_STAT_ADR_ACC_BIIL_ACCT  (JDL_STAT_ADR_ACC_BILL_INST + JDL_STAT_SIZE_ACC_CNTR)   /* BADR:   218, CADR:    56 *///紙幣収納でカウントアップ、リサイクルドラムでもカウントアップ
    #define JDL_STAT_ADR_ACC_TICK_INST  (JDL_STAT_ADR_ACC_BIIL_ACCT + JDL_STAT_SIZE_ACC_CNTR)   /* BADR:   222, CADR:    60 */
    #define JDL_STAT_ADR_ACC_TICK_ACCT  (JDL_STAT_ADR_ACC_TICK_INST + JDL_STAT_SIZE_ACC_CNTR)   /* BADR:   226, CADR:    64 */
    #define JDL_STAT_ADR_ACC_CHECKSUM   (JDL_STAT_ADR_ACC_TICK_ACCT + JDL_STAT_SIZE_ACC_CNTR)   /* BADR:   230, CADR:    68 */

    #define JDL_STAT_NEXTADR_FROM_ACC   (JDL_STAT_ADR_ACC_CHECKSUM + JDL_SIZE_CHECKSUM) /* [Next Item Address] BADR:   232, CADR:    70 */
#else  /* _JDL_STAT_USE_ACC_CNT */
    #define JDL_STAT_NEXTADR_FROM_ACC   JDL_STAT_NEXTADR_FROM_MOV
#endif /* _JDL_STAT_USE_ACC_CNT */

/* Counters of RC related */
#define _JDL_STAT_USE_RC_CNT
#if defined(_JDL_STAT_USE_RC_CNT)
    #define JDL_STAT_ADR_RC_BASE JDL_STAT_NEXTADR_FROM_ACC /* [RC counter BASE Address:232] */
    
        /* Counter settings */
        #define JDL_STAT_SIZE_RC_CNTR JDL_DATA_TYPE_SIZE_DWORD /* [SIZE:4] */
        
    #define JDL_STAT_ADR_RC_STACK       JDL_STAT_ADR_RC_BASE                                    /* BADR:   232, CADR:    70 *///ドラムに収納でカウントアップ
    #define JDL_STAT_ADR_RC_PAYOUT      (JDL_STAT_ADR_RC_STACK + JDL_STAT_SIZE_RC_CNTR)         /* BADR:   236, CADR:    74 *///払い出しPay valid後カウントアップ
    #define JDL_STAT_ADR_RC_COLLECT     (JDL_STAT_ADR_RC_PAYOUT + JDL_STAT_SIZE_RC_CNTR)        /* BADR:   240, CADR:    78 *///回収でカウントアップ
    #define JDL_STAT_ADR_RC_REJECT      (JDL_STAT_ADR_RC_COLLECT + JDL_STAT_SIZE_RC_CNTR)       /* BADR:   244, CADR:    82 */
    #define JDL_STAT_ADR_RC_RESERVED1   (JDL_STAT_ADR_RC_REJECT + JDL_STAT_SIZE_RC_CNTR)        /* BADR:   248, CADR:    86 */
    #define JDL_STAT_ADR_RC_CHECKSUM    (JDL_STAT_ADR_RC_RESERVED1 + JDL_STAT_SIZE_RC_CNTR)     /* BADR:   252, CADR:    90 */

    #define JDL_STAT_NEXTADR_FROM_RC    (JDL_STAT_ADR_RC_CHECKSUM + JDL_SIZE_CHECKSUM) /* [Next Item Address] BADR:   254, CADR:    92 */
#else  /* _JDL_STAT_USE_RC_CNT */
    #define JDL_STAT_NEXTADR_FROM_RC    JDL_STAT_NEXTADR_FROM_ACC
#endif /* _JDL_STAT_USE_RC_CNT */

/* Counters of Rejecting related */
#define _JDL_STAT_USE_REJ_CNT
#if defined(_JDL_STAT_USE_REJ_CNT)
    #define JDL_STAT_ADR_REJ_BASE JDL_STAT_NEXTADR_FROM_RC /* [REJ counter BASE Address:254] */
    
        /* Counter settings */
        #define JDL_STAT_SIZE_REJ_CNTR JDL_DATA_TYPE_SIZE_WORD /* [SIZE:2] */
    /* Statistics-返却Totalカウンター*/
    #define JDL_STAT_ADR_REJ_SKEW       JDL_STAT_ADR_REJ_BASE                                   /* BADR:   254, CADR:    92 */
    #define JDL_STAT_ADR_REJ_MAG_PATTN  (JDL_STAT_ADR_REJ_SKEW + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   256, CADR:    94 */
    #define JDL_STAT_ADR_REJ_MAG_AMOUN  (JDL_STAT_ADR_REJ_MAG_PATTN + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   258, CADR:    96 */
    #define JDL_STAT_ADR_REJ_POSI_AT    (JDL_STAT_ADR_REJ_MAG_AMOUN + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   260, CADR:    98 */
    #define JDL_STAT_ADR_REJ_XRATE      (JDL_STAT_ADR_REJ_POSI_AT + JDL_STAT_SIZE_REJ_CNTR)     /* BADR:   262, CADR:   100 */
    #define JDL_STAT_ADR_REJ_CANCEL     (JDL_STAT_ADR_REJ_XRATE + JDL_STAT_SIZE_REJ_CNTR)       /* BADR:   264, CADR:   102 */
    #define JDL_STAT_ADR_REJ_SLIP       (JDL_STAT_ADR_REJ_CANCEL + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   266, CADR:   104 */
    #define JDL_STAT_ADR_REJ_FMOT_LOCK  (JDL_STAT_ADR_REJ_SLIP + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   268, CADR:   106 */
    #define JDL_STAT_ADR_REJ_FEED_TOUT  (JDL_STAT_ADR_REJ_FMOT_LOCK + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   270, CADR:   108 */
    #define JDL_STAT_ADR_REJ_RESERVED1  (JDL_STAT_ADR_REJ_FEED_TOUT + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   272, CADR:   110 */
    #define JDL_STAT_ADR_REJ_RESERVED2  (JDL_STAT_ADR_REJ_RESERVED1 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   274, CADR:   112 */
    #define JDL_STAT_ADR_REJ_APB_HOME   (JDL_STAT_ADR_REJ_RESERVED2 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   276, CADR:   114 */
    #define JDL_STAT_ADR_REJ_CENT_HOME  (JDL_STAT_ADR_REJ_APB_HOME + JDL_STAT_SIZE_REJ_CNTR)    /* BADR:   278, CADR:   116 */
    #define JDL_STAT_ADR_REJ_PRECOMP    (JDL_STAT_ADR_REJ_CENT_HOME + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   280, CADR:   118 */
    #define JDL_STAT_ADR_REJ_PHOT_PATT  (JDL_STAT_ADR_REJ_PRECOMP + JDL_STAT_SIZE_REJ_CNTR)     /* BADR:   282, CADR:   120 */
    #define JDL_STAT_ADR_REJ_PHOT_LEVE  (JDL_STAT_ADR_REJ_PHOT_PATT + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   284, CADR:   122 */
    #define JDL_STAT_ADR_REJ_INHIBIT    (JDL_STAT_ADR_REJ_PHOT_LEVE + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   286, CADR:   124 */
    #define JDL_STAT_ADR_REJ_ESCR_TOUT  (JDL_STAT_ADR_REJ_INHIBIT + JDL_STAT_SIZE_REJ_CNTR)     /* BADR:   288, CADR:   126 *///not use
    #define JDL_STAT_ADR_REJ_RETURN     (JDL_STAT_ADR_REJ_ESCR_TOUT + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   290, CADR:   128 */
    #define JDL_STAT_ADR_REJ_OPERATION  (JDL_STAT_ADR_REJ_RETURN + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   292, CADR:   130 *///not use
    #define JDL_STAT_ADR_REJ_LENGTH     (JDL_STAT_ADR_REJ_OPERATION + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   294, CADR:   132 */
    #define JDL_STAT_ADR_REJ_SHORT      (JDL_STAT_ADR_REJ_LENGTH + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   296, CADR:   134 */
    #define JDL_STAT_ADR_REJ_LONG       (JDL_STAT_ADR_REJ_SHORT + JDL_STAT_SIZE_REJ_CNTR)       /* BADR:   298, CADR:   136 */
    #define JDL_STAT_ADR_REJ_SYNC       (JDL_STAT_ADR_REJ_LONG + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   300, CADR:   138 */
    #define JDL_STAT_ADR_REJ_DYE        (JDL_STAT_ADR_REJ_SYNC + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   302, CADR:   140 */
    #define JDL_STAT_ADR_REJ_HOLE       (JDL_STAT_ADR_REJ_DYE + JDL_STAT_SIZE_REJ_CNTR)         /* BADR:   304, CADR:   142 */
    #define JDL_STAT_ADR_REJ_TEAR       (JDL_STAT_ADR_REJ_HOLE + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   306, CADR:   144 */
    #define JDL_STAT_ADR_REJ_DOG_EAR    (JDL_STAT_ADR_REJ_TEAR + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   308, CADR:   146 */
    #define JDL_STAT_ADR_REJ_CF         (JDL_STAT_ADR_REJ_DOG_EAR + JDL_STAT_SIZE_REJ_CNTR)     /* BADR:   310, CADR:   148 */
    #define JDL_STAT_ADR_REJ_MCIR       (JDL_STAT_ADR_REJ_CF + JDL_STAT_SIZE_REJ_CNTR)          /* BADR:   312, CADR:   150 */
    #define JDL_STAT_ADR_REJ_M3C        (JDL_STAT_ADR_REJ_MCIR + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   314, CADR:   152 */
    #define JDL_STAT_ADR_REJ_M4C        (JDL_STAT_ADR_REJ_M3C + JDL_STAT_SIZE_REJ_CNTR)         /* BADR:   316, CADR:   154 */
    #define JDL_STAT_ADR_REJ_IR         (JDL_STAT_ADR_REJ_M4C + JDL_STAT_SIZE_REJ_CNTR)         /* BADR:   318, CADR:   156 */
    #define JDL_STAT_ADR_REJ_THREAD     (JDL_STAT_ADR_REJ_IR + JDL_STAT_SIZE_REJ_CNTR)          /* BADR:   320, CADR:   158 */
    #define JDL_STAT_ADR_REJ_LOST       (JDL_STAT_ADR_REJ_THREAD + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   322, CADR:   160 */
    #define JDL_STAT_ADR_REJ_RESERVED3  (JDL_STAT_ADR_REJ_LOST + JDL_STAT_SIZE_REJ_CNTR)        /* BADR:   324, CADR:   162 */
    #define JDL_STAT_ADR_REJ_RESERVED4  (JDL_STAT_ADR_REJ_RESERVED3 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   326, CADR:   164 */
    #define JDL_STAT_ADR_REJ_RESERVED5  (JDL_STAT_ADR_REJ_RESERVED4 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   328, CADR:   166 */
    #define JDL_STAT_ADR_REJ_RESERVED6  (JDL_STAT_ADR_REJ_RESERVED5 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   330, CADR:   168 */
    #define JDL_STAT_ADR_REJ_BAR_NC     (JDL_STAT_ADR_REJ_RESERVED6 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   332, CADR:   170 */
    #define JDL_STAT_ADR_REJ_BAR_UN     (JDL_STAT_ADR_REJ_BAR_NC + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   334, CADR:   172 */
    #define JDL_STAT_ADR_REJ_BAR_SH     (JDL_STAT_ADR_REJ_BAR_UN + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   336, CADR:   174 */
    #define JDL_STAT_ADR_REJ_BAR_ST     (JDL_STAT_ADR_REJ_BAR_SH + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   338, CADR:   176 */
    #define JDL_STAT_ADR_REJ_BAR_SP     (JDL_STAT_ADR_REJ_BAR_ST + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   340, CADR:   178 */
    #define JDL_STAT_ADR_REJ_BAR_TP     (JDL_STAT_ADR_REJ_BAR_SP + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   342, CADR:   180 */
    #define JDL_STAT_ADR_REJ_BAR_XR     (JDL_STAT_ADR_REJ_BAR_TP + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   344, CADR:   182 */
    #define JDL_STAT_ADR_REJ_BAR_PHV    (JDL_STAT_ADR_REJ_BAR_XR + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   346, CADR:   184 */
    #define JDL_STAT_ADR_REJ_BAR_DIN    (JDL_STAT_ADR_REJ_BAR_PHV + JDL_STAT_SIZE_REJ_CNTR)     /* BADR:   348, CADR:   186 */
    #define JDL_STAT_ADR_REJ_BAR_LG     (JDL_STAT_ADR_REJ_BAR_DIN + JDL_STAT_SIZE_REJ_CNTR)     /* BADR:   350, CADR:   188 */
    #define JDL_STAT_ADR_REJ_BAR_NG     (JDL_STAT_ADR_REJ_BAR_LG + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   352, CADR:   190 */
    #define JDL_STAT_ADR_REJ_BAR_MC     (JDL_STAT_ADR_REJ_BAR_NG + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   354, CADR:   192 */
    #define JDL_STAT_ADR_REJ_RESERVED7  (JDL_STAT_ADR_REJ_BAR_MC + JDL_STAT_SIZE_REJ_CNTR)      /* BADR:   356, CADR:   194 */
    #define JDL_STAT_ADR_REJ_RESERVED8  (JDL_STAT_ADR_REJ_RESERVED7 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   358, CADR:   196 */
    #define JDL_STAT_ADR_REJ_CHECKSUM   (JDL_STAT_ADR_REJ_RESERVED8 + JDL_STAT_SIZE_REJ_CNTR)   /* BADR:   360, CADR:   198 */

    /* Statistics-異常Totalカウンター*/
    #define JDL_STAT_NEXTADR_FROM_REJ   (JDL_STAT_ADR_REJ_CHECKSUM + JDL_SIZE_CHECKSUM) /* [Next Item Address] BADR:   362, CADR:   200 */
#else  /* _JDL_STAT_USE_REJ_CNT */
    #define JDL_STAT_NEXTADR_FROM_REJ   JDL_STAT_NEXTADR_FROM_ACC
#endif /* _JDL_STAT_USE_REJ_CNT */

/* Counters of Error related *//* Statistics-異常Totalカウンター*/
#define _JDL_STAT_USE_ERR_CNT
#if defined(_JDL_STAT_USE_ERR_CNT)
    #define JDL_STAT_ADR_ERR_BASE JDL_STAT_NEXTADR_FROM_REJ /* [REJ counter BASE Address:362] */
    
        /* Counter settings */
        #define JDL_STAT_SIZE_ERR_CNTR JDL_DATA_TYPE_SIZE_WORD  /* [SIZE:2] */
    
    #define JDL_STAT_ADR_ERR_MEMORY         JDL_STAT_ADR_ERR_BASE                                      /* BADR:   362, CADR:   200 */
    #define JDL_STAT_ADR_ERR_PERIPHERA      (JDL_STAT_ADR_ERR_MEMORY + JDL_STAT_SIZE_ERR_CNTR)         /* BADR:   364, CADR:   202 */
    #define JDL_STAT_ADR_ERR_SK_FULL        (JDL_STAT_ADR_ERR_PERIPHERA + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   366, CADR:   204 */
    #define JDL_STAT_ADR_ERR_POSI_SK        (JDL_STAT_ADR_ERR_SK_FULL + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:   368, CADR:   206 */
    #define JDL_STAT_ADR_ERR_SLIP_SK        (JDL_STAT_ADR_ERR_POSI_SK + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:   370, CADR:   208 */
    #define JDL_STAT_ADR_ERR_F_TOUT_SK      (JDL_STAT_ADR_ERR_SLIP_SK + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:   372, CADR:   210 */
    #define JDL_STAT_ADR_ERR_LOST_BILL      (JDL_STAT_ADR_ERR_F_TOUT_SK + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   374, CADR:   212 */
    #define JDL_STAT_ADR_ERR_F_LOCK_SK      (JDL_STAT_ADR_ERR_LOST_BILL + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   376, CADR:   214 *///not use
    #define JDL_STAT_ADR_ERR_F_SPEED_L      (JDL_STAT_ADR_ERR_F_LOCK_SK + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   378, CADR:   216 */
    #define JDL_STAT_ADR_ERR_F_SPEED_H      (JDL_STAT_ADR_ERR_F_SPEED_L + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   380, CADR:   218 */
    #define JDL_STAT_ADR_ERR_FMOT_LOCK      (JDL_STAT_ADR_ERR_F_SPEED_H + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   382, CADR:   220 */

    #define JDL_STAT_ADR_ERR_POSI_DIS       (JDL_STAT_ADR_ERR_FMOT_LOCK + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   384, CADR:   222 *///not use
    #define JDL_STAT_ADR_ERR_F_TOUT_DIS     (JDL_STAT_ADR_ERR_POSI_DIS + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:   386, CADR:   224 *///not use
    #define JDL_STAT_ADR_ERR_LOST_DIS       (JDL_STAT_ADR_ERR_F_TOUT_DIS + JDL_STAT_SIZE_ERR_CNTR)     /* BADR:   388, CADR:   226 *///not use

    #define JDL_STAT_ADR_ERR_POSI_AT        (JDL_STAT_ADR_ERR_LOST_DIS + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:   390, CADR:   228 */
    #define JDL_STAT_ADR_ERR_SLIP_AT        (JDL_STAT_ADR_ERR_POSI_AT + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:   392, CADR:   230 */
    #define JDL_STAT_ADR_ERR_F_TOUT_AT      (JDL_STAT_ADR_ERR_SLIP_AT + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:   394, CADR:   232 */
    #define JDL_STAT_ADR_ERR_F_LOCK_AT      (JDL_STAT_ADR_ERR_F_TOUT_AT + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   396, CADR:   234 */
    
    #define JDL_STAT_ADR_ERR_APB_TOUT       (JDL_STAT_ADR_ERR_F_LOCK_AT + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   398, CADR:   236 */
    #define JDL_STAT_ADR_ERR_APB_HOME       (JDL_STAT_ADR_ERR_APB_TOUT + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:   400, CADR:   238 */
    #define JDL_STAT_ADR_ERR_APB_O_RUN      (JDL_STAT_ADR_ERR_APB_HOME + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:   402, CADR:   240 *///not use
    #define JDL_STAT_ADR_ERR_CHEAT          (JDL_STAT_ADR_ERR_APB_O_RUN + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   404, CADR:   242 */
    #define JDL_STAT_ADR_ERR_CENT_TOUT      (JDL_STAT_ADR_ERR_CHEAT + JDL_STAT_SIZE_ERR_CNTR)          /* BADR:   406, CADR:   244 */
    #define JDL_STAT_ADR_ERR_CENT_HOME      (JDL_STAT_ADR_ERR_CENT_TOUT + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   408, CADR:   246 */
    #define JDL_STAT_ADR_ERR_CENT_O_RUN     (JDL_STAT_ADR_ERR_CENT_HOME + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   410, CADR:   248 *///not use

    #define JDL_STAT_ADR_ERR_POSI_AT_DIS    (JDL_STAT_ADR_ERR_CENT_O_RUN + JDL_STAT_SIZE_ERR_CNTR)     /* BADR:   412, CADR:   250 *///not use
    #define JDL_STAT_ADR_ERR_SLIP_AT_DIS    (JDL_STAT_ADR_ERR_POSI_AT_DIS + JDL_STAT_SIZE_ERR_CNTR)    /* BADR:   414, CADR:   252 *///not use
    #define JDL_STAT_ADR_ERR_F_TOUT_AT_DIS  (JDL_STAT_ADR_ERR_SLIP_AT_DIS + JDL_STAT_SIZE_ERR_CNTR)    /* BADR:   416, CADR:   254 *///not use

    #define JDL_STAT_ADR_ERR_COMM_LOSS      (JDL_STAT_ADR_ERR_F_TOUT_AT_DIS + JDL_STAT_SIZE_ERR_CNTR)  /* BADR:   418, CADR:   256 *///not use

    /* [ICB] */
    #define JDL_STAT_ADR_ERR_ICB_NO_RESP    (JDL_STAT_ADR_ERR_COMM_LOSS + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:    420, CADR:  258 */
    #define JDL_STAT_ADR_ERR_ICB_COMMU      (JDL_STAT_ADR_ERR_ICB_NO_RESP + JDL_STAT_SIZE_ERR_CNTR)     /* BADR:    422, CADR:  260 */
    #define JDL_STAT_ADR_ERR_ICB_DATA       (JDL_STAT_ADR_ERR_ICB_COMMU + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:    424, CADR:  262 */
#if 1
	#define JDL_STAT_ADR_ERR_ICB_RESERVED1   (JDL_STAT_ADR_ERR_ICB_DATA + JDL_STAT_SIZE_ERR_CNTR)
	#define JDL_STAT_ADR_ERR_ICB_RESERVED2   (JDL_STAT_ADR_ERR_ICB_RESERVED1 + JDL_STAT_SIZE_ERR_CNTR)
	#define JDL_STAT_ADR_ERR_ICB_RESERVED3   (JDL_STAT_ADR_ERR_ICB_RESERVED2 + JDL_STAT_SIZE_ERR_CNTR)
	#define JDL_STAT_ADR_ERR_ICB_RESERVED4   (JDL_STAT_ADR_ERR_ICB_RESERVED3 + JDL_STAT_SIZE_ERR_CNTR)
	#define JDL_STAT_ADR_ERR_ICB_RESERVED5   (JDL_STAT_ADR_ERR_ICB_RESERVED4 + JDL_STAT_SIZE_ERR_CNTR)
	#define JDL_STAT_ADR_ERR_ICB_RESERVED6   (JDL_STAT_ADR_ERR_ICB_RESERVED5 + JDL_STAT_SIZE_ERR_CNTR)
	#define JDL_STAT_ADR_ERR_ICB_RESERVED7   (JDL_STAT_ADR_ERR_ICB_RESERVED6 + JDL_STAT_SIZE_ERR_CNTR)
	#define JDL_STAT_ADR_ERR_ICB_RESERVED8   (JDL_STAT_ADR_ERR_ICB_RESERVED7 + JDL_STAT_SIZE_ERR_CNTR)
    /* [RC] */
    #define JDL_STAT_ADR_ERR_RC_ERR         (JDL_STAT_ADR_ERR_ICB_RESERVED8 + JDL_STAT_SIZE_ERR_CNTR)    /* BADR:    442, CADR:  280 */
#else
    #define JDL_STAT_ADR_ERR_ICB_RESERVED   (JDL_STAT_ADR_ERR_ICB_DATA + JDL_STAT_SIZE_ERR_CNTR*8)      /* BADR:    426, CADR:  264 */
    /* [RC] */
    #define JDL_STAT_ADR_ERR_RC_ERR         (JDL_STAT_ADR_ERR_ICB_RESERVED + JDL_STAT_SIZE_ERR_CNTR)    /* BADR:    442, CADR:  280 */
#endif
    #define JDL_STAT_ADR_ERR_RC_ROM         (JDL_STAT_ADR_ERR_RC_ERR + JDL_STAT_SIZE_ERR_CNTR)          /* BADR:    444, CADR:  282 */
    #define JDL_STAT_ADR_ERR_RC_REMOVED     (JDL_STAT_ADR_ERR_RC_ROM + JDL_STAT_SIZE_ERR_CNTR)          /* BADR:    446, CADR:  284 */
    #define JDL_STAT_ADR_ERR_RC_COMMU       (JDL_STAT_ADR_ERR_RC_REMOVED + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:    448, CADR:  286 */
    #define JDL_STAT_ADR_ERR_RC_DL_ERR      (JDL_STAT_ADR_ERR_RC_COMMU + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:    450, CADR:  288 */
    #define JDL_STAT_ADR_ERR_RC_POS         (JDL_STAT_ADR_ERR_RC_DL_ERR + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:    452, CADR:  290 */
    #define JDL_STAT_ADR_ERR_RC_TRANS       (JDL_STAT_ADR_ERR_RC_POS + JDL_STAT_SIZE_ERR_CNTR)          /* BADR:    454, CADR:  292 */
    #define JDL_STAT_ADR_ERR_RC_TIMEOUT     (JDL_STAT_ADR_ERR_RC_TRANS + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:    456, CADR:  294 */
    #define JDL_STAT_ADR_ERR_RC_DENOMI      (JDL_STAT_ADR_ERR_RC_TIMEOUT + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:    458, CADR:  296 */
    #define JDL_STAT_ADR_ERR_RC_EMPTY       (JDL_STAT_ADR_ERR_RC_DENOMI + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:    460, CADR:  298 */
    #define JDL_STAT_ADR_ERR_RC_DOUBLE      (JDL_STAT_ADR_ERR_RC_EMPTY + JDL_STAT_SIZE_ERR_CNTR)        /* BADR:    462, CADR:  300 */
    #define JDL_STAT_ADR_ERR_RC_FULL        (JDL_STAT_ADR_ERR_RC_DOUBLE + JDL_STAT_SIZE_ERR_CNTR)       /* BADR:    464, CADR:  302 */
    #define JDL_STAT_ADR_ERR_RC_EXCHANGE    (JDL_STAT_ADR_ERR_RC_FULL + JDL_STAT_SIZE_ERR_CNTR)         /* BADR:    466, CADR:  304 */
    #define JDL_STAT_ADR_ERR_RC_FORCE_QUIT  (JDL_STAT_ADR_ERR_RC_EXCHANGE + JDL_STAT_SIZE_ERR_CNTR)     /* BADR:    466, CADR:  306 */

    #define JDL_STAT_ADR_ERR_RESERVED1      (JDL_STAT_ADR_ERR_RC_FORCE_QUIT + JDL_STAT_SIZE_ERR_CNTR)   /* BADR:   470, CADR:   308 */

    
    #define JDL_STAT_ADR_ERR_RESERVED2      (JDL_STAT_ADR_ERR_RESERVED1 + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   472, CADR:   310 */
    #define JDL_STAT_ADR_ERR_RESERVED3      (JDL_STAT_ADR_ERR_RESERVED2 + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   474, CADR:   312 */
    #define JDL_STAT_ADR_ERR_RESERVED4      (JDL_STAT_ADR_ERR_RESERVED3 + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   476, CADR:   314 */
    
    #define JDL_STAT_ADR_ERR_CHECKSUM       (JDL_STAT_ADR_ERR_RESERVED4 + JDL_STAT_SIZE_ERR_CNTR)      /* BADR:   478, CADR:   316 */
    
    #define JDL_STAT_NEXTADR_FROM_ERR   (JDL_STAT_ADR_ERR_CHECKSUM + JDL_SIZE_CHECKSUM) /* [Next Item Address] BADR:   480, CADR:   318] */
#else  /* _JDL_STAT_USE_ERR_CNT */
    #define JDL_STAT_NEXTADR_FROM_ERR   JDL_STAT_NEXTADR_FROM_REJ
#endif /* _JDL_STAT_USE_ERR_CNT */


/*----------------------------------------------------------*/
/* Statistics per Insertion Address  (Statistics)(統計Block)                       */
/*----------------------------------------------------------*/
/* Block information */
#define JDL_STAT_ADR_STAT_BLK_SIZE JDL_STAT_NEXTADR_FROM_ERR                                   /* BADR:   480, CADR:   318 *//* Statistics-統計Blockサイズ*/
#define JDL_STAT_ADR_STAT_BLK_NUM  (JDL_STAT_ADR_STAT_BLK_SIZE + JDL_DATA_TYPE_SIZE_WORD)      /* BADR:   482, CADR:   320 *//* Statistics-統計Block数*/

#define JDL_STAT_ADR_STAT_BASE     (JDL_STAT_ADR_STAT_BLK_NUM + JDL_DATA_TYPE_SIZE_WORD)       /* BADR:   484, CADR:   322 */

#define _JDL_STAT_USE_STAT_PER_INST
#if defined(_JDL_STAT_USE_STAT_PER_INST) && defined(_JDL_STAT_USE_ACC_CNT)
        /* Counter settings */
        #define JDL_STAT_SIZE_STAT_A_CNTR JDL_DATA_TYPE_SIZE_WORD  /* [SIZE:2] */
        #define JDL_STAT_SIZE_STAT_R_CNTR JDL_DATA_TYPE_SIZE_WORD  /* [SIZE:2] */
        #define JDL_STAT_SIZE_STAT_E_CNTR JDL_DATA_TYPE_SIZE_WORD  /* [SIZE:2] */
    
    /* Offset of items in Block *//* Acceptance Block info */
    #define JDL_STAT_OFS_STAT_ACCEPT       0                                                            /* [OFS: 0] *//* Statistics-受け取り枚数 */
    #define JDL_STAT_OFS_STAT_ACC_BAR      (JDL_STAT_OFS_STAT_ACCEPT + JDL_STAT_SIZE_STAT_A_CNTR)       /* [OFS: 2] *//* Statistics-BAR受け取り枚数 */
    
    #define JDL_STAT_OFS_STAT_REJ_OPT      (JDL_STAT_OFS_STAT_ACC_BAR + JDL_STAT_SIZE_STAT_A_CNTR)      /* [OFS: 4] *//* Statistics-統計Block-返却 */
    #define JDL_STAT_OFS_STAT_REJ_MAG      (JDL_STAT_OFS_STAT_REJ_OPT + JDL_STAT_SIZE_STAT_R_CNTR)      /* [OFS: 6] */
    #define JDL_STAT_OFS_STAT_REJ_UV       (JDL_STAT_OFS_STAT_REJ_MAG + JDL_STAT_SIZE_STAT_R_CNTR)      /* [OFS: 8] */
    #define JDL_STAT_OFS_STAT_REJ_CF       (JDL_STAT_OFS_STAT_REJ_UV + JDL_STAT_SIZE_STAT_R_CNTR)       /* [OFS:10] */
    #define JDL_STAT_OFS_STAT_REJ_FEED     (JDL_STAT_OFS_STAT_REJ_CF + JDL_STAT_SIZE_STAT_R_CNTR)       /* [OFS:12] */
    #define JDL_STAT_OFS_STAT_REJ_RET      (JDL_STAT_OFS_STAT_REJ_FEED + JDL_STAT_SIZE_STAT_R_CNTR)     /* [OFS:14] */
    #define JDL_STAT_OFS_STAT_REJ_BAR      (JDL_STAT_OFS_STAT_REJ_RET + JDL_STAT_SIZE_STAT_R_CNTR)      /* [OFS:16] */
    
    #define JDL_STAT_OFS_STAT_ERR_MEM      (JDL_STAT_OFS_STAT_REJ_BAR + JDL_STAT_SIZE_STAT_R_CNTR)      /* [OFS:18] *//* Statistics-統計Block-異常 */
    #define JDL_STAT_OFS_STAT_ERR_FULL     (JDL_STAT_OFS_STAT_ERR_MEM + JDL_STAT_SIZE_STAT_E_CNTR)      /* [OFS:20] */
    #define JDL_STAT_OFS_STAT_ERR_BOX      (JDL_STAT_OFS_STAT_ERR_FULL + JDL_STAT_SIZE_STAT_E_CNTR)     /* [OFS:22] */
    #define JDL_STAT_OFS_STAT_ERR_JAMS     (JDL_STAT_OFS_STAT_ERR_BOX + JDL_STAT_SIZE_STAT_E_CNTR)      /* [OFS:24] */
    #define JDL_STAT_OFS_STAT_ERR_JAMA     (JDL_STAT_OFS_STAT_ERR_JAMS + JDL_STAT_SIZE_STAT_E_CNTR)     /* [OFS:26] */
    #define JDL_STAT_OFS_STAT_ERR_FEED     (JDL_STAT_OFS_STAT_ERR_JAMA + JDL_STAT_SIZE_STAT_E_CNTR)     /* [OFS:28] */
    #define JDL_STAT_OFS_STAT_ERR_STAK     (JDL_STAT_OFS_STAT_ERR_FEED + JDL_STAT_SIZE_STAT_E_CNTR)     /* [OFS:30] */
    #define JDL_STAT_OFS_STAT_ERR_CENT     (JDL_STAT_OFS_STAT_ERR_STAK + JDL_STAT_SIZE_STAT_E_CNTR)     /* [OFS:32] */
    #define JDL_STAT_OFS_STAT_ERR_APB      (JDL_STAT_OFS_STAT_ERR_CENT + JDL_STAT_SIZE_STAT_E_CNTR)     /* [OFS:34] */
    #define JDL_STAT_OFS_STAT_ERR_CHEA     (JDL_STAT_OFS_STAT_ERR_APB + JDL_STAT_SIZE_STAT_E_CNTR)      /* [OFS:36] */
    #define JDL_STAT_OFS_STAT_ERR_COMM_WDT (JDL_STAT_OFS_STAT_ERR_CHEA + JDL_STAT_SIZE_STAT_E_CNTR)     /* [OFS:38] */
    /* [ICB] */
    #define JDL_STAT_OFS_STAT_ERR_ICB_NO_RESP    (JDL_STAT_OFS_STAT_ERR_COMM_WDT + JDL_STAT_SIZE_STAT_E_CNTR)       /* [OFS:40] */
    #define JDL_STAT_OFS_STAT_ERR_ICB_COMMU      (JDL_STAT_OFS_STAT_ERR_ICB_NO_RESP + JDL_STAT_SIZE_STAT_E_CNTR)    /* [OFS:42] */
    #define JDL_STAT_OFS_STAT_ERR_ICB_DATA       (JDL_STAT_OFS_STAT_ERR_ICB_COMMU + JDL_STAT_SIZE_STAT_E_CNTR)      /* [OFS:44] */
#if 1
	#define JDL_STAT_OFS_STAT_ERR_ICB_REV1            (JDL_STAT_OFS_STAT_ERR_ICB_DATA + JDL_STAT_SIZE_STAT_E_CNTR) /* [OFS:46] */
	#define JDL_STAT_OFS_STAT_ERR_ICB_REV2            (JDL_STAT_OFS_STAT_ERR_ICB_REV1 + JDL_STAT_SIZE_STAT_E_CNTR) /* [OFS:48] */	
	#define JDL_STAT_OFS_STAT_ERR_ICB_REV3            (JDL_STAT_OFS_STAT_ERR_ICB_REV2 + JDL_STAT_SIZE_STAT_E_CNTR) /* [OFS:50] */	
	#define JDL_STAT_OFS_STAT_ERR_ICB_REV4            (JDL_STAT_OFS_STAT_ERR_ICB_REV3 + JDL_STAT_SIZE_STAT_E_CNTR) /* [OFS:52] */	
	#define JDL_STAT_OFS_STAT_ERR_ICB_REV5            (JDL_STAT_OFS_STAT_ERR_ICB_REV4 + JDL_STAT_SIZE_STAT_E_CNTR) /* [OFS:54] */	
	#define JDL_STAT_OFS_STAT_ERR_RC_ERR         (JDL_STAT_OFS_STAT_ERR_ICB_REV5 + JDL_STAT_SIZE_STAT_E_CNTR)        /* [OFS:56] */
#else
	#define JDL_STAT_OFS_STAT_ERR_ICB_REV            (JDL_STAT_OFS_STAT_ERR_ICB_DATA + JDL_STAT_SIZE_STAT_E_CNTR*5) /* [OFS:54] */
	/* [RC] */
	#define JDL_STAT_OFS_STAT_ERR_RC_ERR         (JDL_STAT_OFS_STAT_ERR_ICB_REV + JDL_STAT_SIZE_STAT_E_CNTR)        /* [OFS:56] */
#endif
    #define JDL_STAT_OFS_STAT_ERR_RC_ROM_DL      (JDL_STAT_OFS_STAT_ERR_RC_ERR + JDL_STAT_SIZE_STAT_E_CNTR)         /* [OFS:58] */
    #define JDL_STAT_OFS_STAT_ERR_RC_RM_COM      (JDL_STAT_OFS_STAT_ERR_RC_ROM_DL + JDL_STAT_SIZE_STAT_E_CNTR)      /* [OFS:60] */
    #define JDL_STAT_OFS_STAT_ERR_RC_OPT         (JDL_STAT_OFS_STAT_ERR_RC_RM_COM + JDL_STAT_SIZE_STAT_E_CNTR)      /* [OFS:62] */
    #define JDL_STAT_OFS_STAT_ERR_RC_DENOMI      (JDL_STAT_OFS_STAT_ERR_RC_OPT + JDL_STAT_SIZE_STAT_E_CNTR)         /* [OFS:64] */
    #define JDL_STAT_OFS_STAT_ERR_RC_EMPTY       (JDL_STAT_OFS_STAT_ERR_RC_DENOMI + JDL_STAT_SIZE_STAT_E_CNTR)      /* [OFS:66] */
    #define JDL_STAT_OFS_STAT_ERR_RC_REV         (JDL_STAT_OFS_STAT_ERR_RC_EMPTY + JDL_STAT_SIZE_STAT_E_CNTR*2)     /* [OFS:70] */
    
    #define JDL_STAT_OFS_STAT_CHECKSUM     (JDL_STAT_OFS_STAT_ERR_RC_REV + JDL_STAT_SIZE_STAT_E_CNTR)   /* [OFS:72] */  

    /* Block setting */
    #define JDL_STAT_PER_INST_NUM_BLK  50000  /* Insertion number of each statistics block */
    #define JDL_STAT_PER_INST_BLK_SIZE (JDL_STAT_OFS_STAT_CHECKSUM + JDL_SIZE_CHECKSUM) /* [Each Statistics block size:74] */
    #define JDL_STAT_PER_INST_BLK_NUM  10     /* Number of Statistics block */
#else  /* _JDL_STAT_USE_STAT_PER_INST && _JDL_STAT_USE_ACC_CNT */
    /* Block setting */
    #define JDL_STAT_PER_INST_NUM_BLK  0  /* Insertion number of each statistics block */
    #define JDL_STAT_PER_INST_BLK_SIZE 0  /* Each Statistics block size */
    #define JDL_STAT_PER_INST_BLK_NUM  0  /* Number of Statistics block */
#endif /* _JDL_STAT_USE_STAT_PER_INST && _JDL_STAT_USE_ACC_CNT */

#define JDL_STAT_PER_INST_TOTAL (JDL_STAT_PER_INST_BLK_SIZE * JDL_STAT_PER_INST_BLK_NUM) /* [PER INR SIZE:740] 74 * 10 */


/*----------------------------------------------------------*/
/* Address for the reserved                                 */
/*----------------------------------------------------------*/
#define JDL_STAT_ADR_FOR_RESERVED (JDL_STAT_ADR_STAT_BASE + JDL_STAT_PER_INST_TOTAL) /* [Address for the reserved] BADR:   1224, CADR:   1062 */


/*----------------------------------------------------------*/
/* Address for the next category                            */
/*----------------------------------------------------------*/
#define JDL_STAT_ADR_FOR_NEXT_CATEGORY (JDL_STAT_ADR_FOR_RESERVED + 0) /* [Address for the next category] BADR:   1224, CADR:   1062 */


/*----------------------------------------------------------*/
/* Statistics total size                                    */
/*----------------------------------------------------------*/
/* Total size of Statistics Category data to send */
#define JDL_STAT_SEND_TOTAL (JDL_STAT_ADR_FOR_RESERVED - JDL_STAT_ADR_SEND_BASE)      /* [Statistics Send Data Total: 1060] Without Reserved and Revision */
/* Total size of Statistics Category buffer without reserved area */
#define JDL_STAT_BUFF_TOTAL (JDL_STAT_ADR_FOR_NEXT_CATEGORY - JDL_STAT_ADR_BUFF_BASE) /* [Statistics Buffer Total: 1062]  1224 - 162(Next Addr - Base Addr) */



/*==========================================================================*/
/*==========================================================================*/
/* Sensor                                                                   */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/
#define _JDL_USE_SENS
#if defined(_JDL_USE_SENS)
    /*----------------------------------------------------------*/
    /* Revision                                                 */
    /*----------------------------------------------------------*/
    #define JDL_SENS_REV               0x0001
    
    
    /*----------------------------------------------------------*/
    /* Base Address of Sensor Category                          */
    /*----------------------------------------------------------*/
    /* Base Address of Sensor Category buffer */
    #define JDL_SENS_ADR_BUFF_BASE  JDL_STAT_ADR_FOR_NEXT_CATEGORY               /* [Buffer Base: 1224] */
                                                                                     /* BADR:Buffer Address, CADR:Category Data Address */
    #define JDL_SENS_ADR_REV        JDL_SENS_ADR_BUFF_BASE                                   /* BADR:   1224, CADR:     0 */
        
    /* Base Address of Sensor Category data to send */
    #define JDL_SENS_ADR_SEND_BASE  (JDL_SENS_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base: 1226] */
    
    
    /*----------------------------------------------------------*/
    /* Sensor Data setting                                      */
    /*----------------------------------------------------------*/
    #define JDL_SENS_SIZE_APP_VER   4
    #define JDL_SENS_SIZE_ADJ_VAL   10
    #define JDL_SENS_SIZE_COR_VAL   10
    #define JDL_SENS_SIZE_SENS_STAT   10
    
    
    /*----------------------------------------------------------*/
    /* Datas Address                                            */
    /*----------------------------------------------------------*/
    /* !!! Required datas >>>>>>>>>>>>>>>>>> */
    #define JDL_SENS_ADR_NUM_OF_ADJ    JDL_SENS_ADR_SEND_BASE                              /* BADR:   1226, CADR:     2 *//* 調整回数 */
    #define JDL_SENS_ADR_APP_VER       (JDL_SENS_ADR_NUM_OF_ADJ + JDL_DATA_TYPE_SIZE_WORD) /* BADR:   1228, CADR:     4 */
    #define JDL_SENS_ADR_ADJ_TIME      (JDL_SENS_ADR_APP_VER + JDL_SENS_SIZE_APP_VER)      /* BADR:   1232, CADR:     8 *//* 最終センサ調整日時 */
    #define JDL_SENS_ADR_ADJ_VAL       (JDL_SENS_ADR_ADJ_TIME + JDL_SIZE_TIME)             /* BADR:   1240, CADR:    16 : Adjustment value *//* センサ調整値 */
    #define JDL_SENS_ADR_COR_VAL       (JDL_SENS_ADR_ADJ_VAL + JDL_SENS_SIZE_ADJ_VAL)      /* BADR:   1250, CADR:    26 : Correction value *//* センサ補正値 */
    #define JDL_SENS_ADR_COR_TIME      (JDL_SENS_ADR_COR_VAL + JDL_SENS_SIZE_COR_VAL)      /* BADR:   1260, CADR:    36 : Correction time *//* センサ補正日時 */
    #define JDL_SENS_ADR_PRE_COR_VAL   (JDL_SENS_ADR_COR_TIME + JDL_SIZE_TIME)             /* BADR:   1268, CADR:    44 : Previous correction value *//*前回のセンサ補正値 */
    #define JDL_SENS_ADR_PRE_COR_TIME  (JDL_SENS_ADR_PRE_COR_VAL + JDL_SENS_SIZE_COR_VAL)  /* BADR:   1278, CADR:    54 : Previous correction time *//*前回のセンサ補正日時 */
    #define JDL_SENS_ADR_SENS_STAT     (JDL_SENS_ADR_PRE_COR_TIME + JDL_SIZE_TIME)         /* BADR:   1286, CADR:    62 : Current Sensor state *//*センサ状態*/
    /* <<<<<<<<<<<<<<<<<< Required datas !!! */
    
    /*----------------------------------------------------------*/
    /* Address for the reserved                                 */
    /*----------------------------------------------------------*/
    #define JDL_SENS_ADR_FOR_RESERVED  (JDL_SENS_ADR_SENS_STAT + JDL_SIZE_SENS_STAT)       /* [Address for the reserved] BADR: 1291, CADR: 67 */
    
    
    /*----------------------------------------------------------*/
    /* Address for the next category                            */
    /*----------------------------------------------------------*/
    #define JDL_SENS_ADR_FOR_NEXT_CATEGORY (JDL_SENS_ADR_FOR_RESERVED + 0)            /* [Address for the next category] BADR: 1291, CADR: 67  (Reserved area = 0) */
    
    
    /*----------------------------------------------------------*/
    /* Sensor total size                                        */
    /*----------------------------------------------------------*/
    /* Total size of Sensor Category data to send */
    #define JDL_SENS_SEND_TOTAL (JDL_SENS_ADR_FOR_RESERVED - JDL_SENS_ADR_SEND_BASE)      /* [Sensor Send Data Total:65] Without Reserved and Revision */
    /* Total size of Sensor Category buffer without reserved area */
    #define JDL_SENS_BUFF_TOTAL (JDL_SENS_ADR_FOR_NEXT_CATEGORY - JDL_SENS_ADR_BUFF_BASE) /* [Sensor Buffer Total:67] 1291 - 1224(Next Addr - Base Addr) */
    
#endif  /* _JDL_USE_SENS */



/*==========================================================================*/
/*==========================================================================*/
/* Communication                                                            */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/
#define _JDL_USE_COMM
#if defined(_JDL_USE_COMM)
    /*----------------------------------------------------------*/
    /* Revision                                                 */
    /*----------------------------------------------------------*/
    #define JDL_COMM_REV               0x0001
    
    
    /*----------------------------------------------------------*/
    /* Base Address of Communication Category                   */
    /*----------------------------------------------------------*/
    /* Base Address of Communication Category buffer */
    #define JDL_COMM_ADR_BUFF_BASE  JDL_SENS_ADR_FOR_NEXT_CATEGORY               /* [Buffer Base:1291] */
                                                                                     /* BADR:Buffer Address, CADR:Category Data Address */
    #define JDL_COMM_ADR_REV        JDL_COMM_ADR_BUFF_BASE                                   /* BADR:  1291, CADR:     0 */
    
    /* Base Address of Communication Category data to send */
    #define JDL_COMM_ADR_SEND_BASE  (JDL_COMM_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:1293] */
    
    
    /*----------------------------------------------------------*/
    /* Datas Address                                            */
    /*----------------------------------------------------------*/
    /* !!! Required datas >>>>>>>>>>>>>>>>>> */
    #define JDL_COMM_ADR_PROTOCOL_ID  JDL_COMM_ADR_SEND_BASE                              /* BADR: 1293, CADR:     2 */
    #define JDL_COMM_ADR_BUFF_SIZE   (JDL_COMM_ADR_PROTOCOL_ID + JDL_SIZE_PROTOCOL_ID)    /* BADR: 1297, CADR:     6 */
    #define JDL_COMM_ADR_HEAD_INDEX  (JDL_COMM_ADR_BUFF_SIZE + JDL_DATA_TYPE_SIZE_DWORD)  /* BADR: 1301, CADR:    10 */
    #define JDL_COMM_ADR_TAIL_INDEX  (JDL_COMM_ADR_HEAD_INDEX + JDL_DATA_TYPE_SIZE_DWORD) /* BADR: 1305, CADR:    14 */
    #define JDL_COMM_ADR_BUFF_ROUND  (JDL_COMM_ADR_TAIL_INDEX + JDL_DATA_TYPE_SIZE_DWORD) /* BADR: 1309, CADR:    18 */
    
    #define JDL_COMM_ADR_LOG_BASE   (JDL_COMM_ADR_BUFF_ROUND + JDL_SIZE_BUFF_ROUND)       /* BADR: 1210, CADR:    19 */
    /* <<<<<<<<<<<<<<<<<< Required datas !!! */
    
        /* Protocol ID size */
        //#define JDL_COMM_PROTOCOL_ID_SIZE  4
        /* Header size of Save packet */
        #define JDL_COMM_PKT_HEADER_SIZE 2
    
        /* Log Buffer setting */
        #define JDL_COMM_BUFFER_SIZE     4600
    
    
    /*----------------------------------------------------------*/
    /* Address for the reserved                                 */
    /*----------------------------------------------------------*/
    #define JDL_COMM_ADR_FOR_RESERVED  (JDL_COMM_ADR_LOG_BASE + JDL_COMM_BUFFER_SIZE) /* [Address for the reserved] BADR: 5910, CADR:  4619 */
    
    
    /*----------------------------------------------------------*/
    /* Address for the next category                            */
    /*----------------------------------------------------------*/
    #define JDL_COMM_ADR_FOR_NEXT_CATEGORY (JDL_COMM_ADR_FOR_RESERVED + 0)       /* [Address for the next category] BADR: 5910, CADR:  4619  (Reserved area = 0) */
    
    
    /*----------------------------------------------------------*/
    /* Communication total size                                 */
    /*----------------------------------------------------------*/
    /* Total size of Communication Category data to send */
    #define JDL_COMM_SEND_TOTAL (JDL_COMM_ADR_FOR_RESERVED - JDL_COMM_ADR_SEND_BASE)      /* [Communication Send Data Total:4647] Without Reserved and Revision */
    /* Total size of Communication Category buffer without reserved area */
    #define JDL_COMM_BUFF_TOTAL (JDL_COMM_ADR_FOR_NEXT_CATEGORY - JDL_COMM_ADR_BUFF_BASE) /* [Communication Buffer Total:4619] 5910 - 1291 (Next Addr - Base Addr) */
    
#endif  /* _JDL_USE_COMM */



/*==========================================================================*/
/*==========================================================================*/
/* Event                                                                    */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/

/*----------------------------------------------------------*/
/* Revision                                                 */
/*----------------------------------------------------------*/
#define JDL_EVEN_REV             0x0001


/*----------------------------------------------------------*/
/* Base Address of Event Category                          */
/*----------------------------------------------------------*/
/* Base Address of Event Category buffer */
#define JDL_EVEN_ADR_BUFF_BASE  JDL_COMM_ADR_FOR_NEXT_CATEGORY               /* [Buffer Base:5910] */
                                                                                 /* BADR:Buffer Address, CADR:Category Data Address */
#define JDL_EVEN_ADR_REV        JDL_EVEN_ADR_BUFF_BASE                                   /* BADR: 5910, CADR:     0 */

/* Base Address of Event Category data to send */
#define JDL_EVEN_ADR_SEND_BASE  (JDL_EVEN_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:5912] */


/*----------------------------------------------------------*/
/* Required Datas Address                                   */
/*----------------------------------------------------------*/
/* !!! Required datas >>>>>>>>>>>>>>>>>> */
#define JDL_EVEN_ADR_RCD_SIZE    JDL_EVEN_ADR_SEND_BASE                            /* BADR: 5912, CADR:     2 */
#define JDL_EVEN_ADR_RCD_NUM     (JDL_EVEN_ADR_RCD_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 5914, CADR:     4 */
#define JDL_EVEN_ADR_CUR_RCD     (JDL_EVEN_ADR_RCD_NUM + JDL_DATA_TYPE_SIZE_WORD)  /* BADR: 5916, CADR:     6 */
#define JDL_EVEN_ADR_RCD_RND     (JDL_EVEN_ADR_CUR_RCD + JDL_DATA_TYPE_SIZE_WORD)  /* BADR: 5918, CADR:     8 */
/* <<<<<<<<<<<<<<<<<< Required datas !!! */


/*----------------------------------------------------------*/
/* Event Record Datas Address                               */
/*----------------------------------------------------------*/
#define JDL_EVEN_ADR_RCD_BASE    (JDL_EVEN_ADR_RCD_RND + JDL_SIZE_BUFF_ROUND)      /* BADR: 5919, CADR:     9 */

    /* Record item size */
    #define JDL_EVEN_SIZE_DATA    6
    #define JDL_EVEN_SIZE_TIME    4
    
    /* Offset of items in Record */
    #define JDL_EVEN_OFS_CORD     0                                                  /* [OFS: 0] */
    #define JDL_EVEN_OFS_DATA     (JDL_EVEN_OFS_CORD + JDL_SIZE_EVEN_CORD)           /* [OFS: 1] */
    #define JDL_EVEN_OFS_MODE     (JDL_EVEN_OFS_DATA + JDL_EVEN_SIZE_DATA)           /* [OFS: 7] */
    #define JDL_EVEN_OFS_SENS     (JDL_EVEN_OFS_MODE + JDL_SIZE_TASK_MODE)           /* [OFS: 9] */
    #define JDL_EVEN_OFS_TIME     (JDL_EVEN_OFS_SENS + JDL_SIZE_SENS_STAT)           /* [OFS:14] */
    #define JDL_EVEN_OFS_COMM_IDX (JDL_EVEN_OFS_TIME + JDL_EVEN_SIZE_TIME)           /* [OFS:18] */
    #define JDL_EVEN_OFS_COMM_RND (JDL_EVEN_OFS_COMM_IDX + JDL_DATA_TYPE_SIZE_DWORD) /* [OFS:22] */
    
    /* Record setting */
    #define JDL_EVEN_RCD_SIZE     (JDL_EVEN_OFS_COMM_RND + JDL_SIZE_BUFF_ROUND) /* [SIZE:23] */
    #define JDL_EVEN_RCD_NUM      500
    #define JDL_EVEN_RCD_TOTAL    (JDL_EVEN_RCD_SIZE * JDL_EVEN_RCD_NUM)   /* [Recrd Total Size: 11500] 23 * 500 */


/*----------------------------------------------------------*/
/* Address for the reserved                                 */
/*----------------------------------------------------------*/
#define JDL_EVEN_ADR_FOR_RESERVED  (JDL_EVEN_ADR_RCD_BASE + JDL_EVEN_RCD_TOTAL) /* [Address for the reserved] BADR: 17419, CADR:  11509 */


/*----------------------------------------------------------*/
/* Address for the next category                            */
/*----------------------------------------------------------*/
#define JDL_EVEN_ADR_FOR_NEXT_CATEGORY (JDL_EVEN_ADR_FOR_RESERVED + 0)     /* [Address for the next category] BADR: 17419, CADR:  11509  (Reserved area = 0) */


/*----------------------------------------------------------*/
/* Event total size                                         */
/*----------------------------------------------------------*/
/* Total size of Event Category data to send */
#define JDL_EVEN_SEND_TOTAL (JDL_EVEN_ADR_FOR_RESERVED - JDL_EVEN_ADR_SEND_BASE)      /* [Event Send Data Total:11507] Without Reserved and Revision */
/* Total size of Event Category buffer without reserved area */
#define JDL_EVEN_BUFF_TOTAL (JDL_EVEN_ADR_FOR_NEXT_CATEGORY - JDL_EVEN_ADR_BUFF_BASE) /* [Event Buffer Total:11509] 17419 - 5912(Next Addr - Base Addr) */



/*==========================================================================*/
/*==========================================================================*/
/* Error                                                                    */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/

/*----------------------------------------------------------*/
/* Revision                                                 */
/*----------------------------------------------------------*/
#define JDL_EER_REV               0x0001


/*----------------------------------------------------------*/
/* Base Address of Error Category                          */
/*----------------------------------------------------------*/
/* Base Address of Error Category buffer */
#define JDL_ERR_ADR_BUFF_BASE  JDL_EVEN_ADR_FOR_NEXT_CATEGORY              /* [Buffer Base:17419] */
                                                                                /* BADR:Buffer Address, CADR:Category Data Address */
#define JDL_ERR_ADR_REV        JDL_ERR_ADR_BUFF_BASE                                    /* BADR: 17419, CADR:     0 */

/* Base Address of Error Category data to send */
#define JDL_ERR_ADR_SEND_BASE  (JDL_ERR_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:17421] */


/*----------------------------------------------------------*/
/* Required Datas Address                                   */
/*----------------------------------------------------------*/
/* !!! Required datas >>>>>>>>>>>>>>>>>> */
#define JDL_ERR_ADR_BLK_SIZE       JDL_ERR_ADR_SEND_BASE                            /* BADR: 17421, CADR:     2 */
#define JDL_ERR_ADR_BLK_NUM       (JDL_ERR_ADR_BLK_SIZE + JDL_DATA_TYPE_SIZE_WORD)  /* BADR: 17423, CADR:     4 */
#define JDL_ERR_ADR_TRCD_SIZE     (JDL_ERR_ADR_BLK_NUM + JDL_DATA_TYPE_SIZE_WORD)   /* BADR: 17425, CADR:     6 */
#define JDL_ERR_ADR_TRCD_NUM      (JDL_ERR_ADR_TRCD_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 17427, CADR:     8 */
#define JDL_ERR_ADR_CUR_BLK       (JDL_ERR_ADR_TRCD_NUM + JDL_DATA_TYPE_SIZE_WORD)  /* BADR: 17429, CADR:    10 */
#define JDL_ERR_ADR_BLK_RND       (JDL_ERR_ADR_CUR_BLK + JDL_DATA_TYPE_SIZE_WORD)   /* BADR: 17431, CADR:    12 */
/* <<<<<<<<<<<<<<<<<< Required datas !!! */


/*----------------------------------------------------------*/
/* Error Block Datas Address                                */
/*----------------------------------------------------------*/
#define JDL_ERR_ADR_BLK_BASE      (JDL_ERR_ADR_BLK_RND + JDL_SIZE_BUFF_ROUND)       /* BADR: 17432, CADR:    13 */

/* Block item size */
#define JDL_ERR_SIZE_SAME_CNT 1
#define JDL_ERR_SIZE_INIT_CNT 1
#define JDL_ERR_SIZE_INFO 54

/* Offset of items in Block */
#define JDL_ERR_OFS_BLK_CORD      0                                                    /* [BLK OFS: 0] */
#define JDL_ERR_OFS_BLK_TID       (JDL_ERR_OFS_BLK_CORD + JDL_SIZE_ERR_CODE)           /* [BLK OFS: 2] */
#define JDL_ERR_OFS_BLK_MODE      (JDL_ERR_OFS_BLK_TID + JDL_SIZE_TID)                 /* [BLK OFS: 3] */
#define JDL_ERR_OFS_BLK_SAME      (JDL_ERR_OFS_BLK_MODE + JDL_SIZE_TASK_MODE)          /* [BLK OFS: 5] */
#define JDL_ERR_OFS_BLK_INIT      (JDL_ERR_OFS_BLK_SAME + JDL_ERR_SIZE_SAME_CNT)       /* [BLK OFS: 6] */
#define JDL_ERR_OFS_BLK_INFO      (JDL_ERR_OFS_BLK_INIT + JDL_ERR_SIZE_INIT_CNT)       /* [BLK OFS: 7] */
#define JDL_ERR_OFS_BLK_EVEN_IDX  (JDL_ERR_OFS_BLK_INFO + JDL_ERR_SIZE_INFO)           /* [BLK OFS:61] */
#define JDL_ERR_OFS_BLK_EVEN_RND  (JDL_ERR_OFS_BLK_EVEN_IDX + JDL_DATA_TYPE_SIZE_WORD) /* [BLK OFS:63] */

/*--- Trace Record Datas ---*/
#define JDL_ERR_OFS_BLK_TRCD_BASE (JDL_ERR_OFS_BLK_EVEN_RND + JDL_SIZE_BUFF_ROUND) /* [BLK OFS TRCD BASE:64] */
    
    /* Record item size*/
    #define JDL_ERR_SIZE_TRCD_DATA 3
    
    /* Offset of items in Trace record */
    #define JDL_ERR_OFS_TRCD_ID   0                                                /* [TRCD OFS: 0] */
    #define JDL_ERR_OFS_TRCD_MODE (JDL_ERR_OFS_TRCD_ID + JDL_SIZE_TID)             /* [TRCD OFS: 1] */
    #define JDL_ERR_OFS_TRCD_DATA (JDL_ERR_OFS_TRCD_MODE + JDL_SIZE_TASK_MODE)     /* [TRCD OFS: 3] */
    #define JDL_ERR_OFS_TRCD_SENS (JDL_ERR_OFS_TRCD_DATA + JDL_ERR_SIZE_TRCD_DATA) /* [TRCD OFS: 6] */
    
    /* Trace Record setting */
    #define JDL_ERR_TRCD_SIZE     (JDL_ERR_OFS_TRCD_SENS + JDL_SIZE_SENS_STAT) /* [Trace Record SIZE: 11] */
    #define JDL_ERR_TRCD_NUM      64
    #define JDL_ERR_TRCD_TOTAL    (JDL_ERR_TRCD_SIZE * JDL_ERR_TRCD_NUM)  /* [Trace Record Total SIZE:704] */

/* Block setting */
#define JDL_ERR_BLK_SIZE          (JDL_ERR_OFS_BLK_TRCD_BASE + JDL_ERR_TRCD_TOTAL) /* [Error Block SIZE:768] */
#define JDL_ERR_BLK_NUM           16
#define JDL_ERR_BLK_TOTAL         (JDL_ERR_BLK_SIZE * JDL_ERR_BLK_NUM) /* [Error Block Total SIZE: 12288] */


/*----------------------------------------------------------*/
/* Address for the reserved                                 */
/*----------------------------------------------------------*/
#define JDL_ERR_ADR_FOR_RESERVED  (JDL_ERR_ADR_BLK_BASE + JDL_ERR_BLK_TOTAL) /* [Address for the reserved] BADR: 29720, CADR:  12301 */


/*----------------------------------------------------------*/
/* Address for the next category                            */
/*----------------------------------------------------------*/
#define JDL_ERR_ADR_FOR_NEXT_CATEGORY (JDL_ERR_ADR_FOR_RESERVED + 0)    /* [Address for the next category] BADR: 29720, CADR:  12301  (Reserved area = 0) */


/*----------------------------------------------------------*/
/* Error total size                                         */
/*----------------------------------------------------------*/
/* Total size of Error Category data to send */
#define JDL_ERR_SEND_TOTAL (JDL_ERR_ADR_FOR_RESERVED - JDL_ERR_ADR_SEND_BASE)      /* [Error Send Data Total:12299] Without Reserved and Revision */
/* Total size of Error Category buffer without reserved area */
#define JDL_ERR_BUFF_TOTAL (JDL_ERR_ADR_FOR_NEXT_CATEGORY - JDL_ERR_ADR_BUFF_BASE) /* [Error Buffer Total:12301] 29720 - 17419(Next Addr - Base Addr) */



/*==========================================================================*/
/*==========================================================================*/
/* Acceptance                                                               */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/
#define _JDL_USE_ACC
#if defined(_JDL_USE_ACC)
    /*----------------------------------------------------------*/
    /* Revision                                                 */
    /*----------------------------------------------------------*/
    #define JDL_ACC_REV         0x0001
    
    
    /*----------------------------------------------------------*/
    /* Base Address of Acceptance Category                          */
    /*----------------------------------------------------------*/
    /* Base Address of Acceptance Category buffer */
    #define JDL_ACC_ADR_BUFF_BASE  JDL_ERR_ADR_FOR_NEXT_CATEGORY               /* [Buffer Base:29720] */
                                                                                    /* BADR:Buffer Address, CADR:Category Data Address */
    #define JDL_ACC_ADR_REV        JDL_ACC_ADR_BUFF_BASE                                    /* BADR: 29720, CADR:     0 */
    
    /* Base Address of Acceptance Category data to send */
    #define JDL_ACC_ADR_SEND_BASE  (JDL_ACC_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:29720] */
    
    
    /*----------------------------------------------------------*/
    /* Currency Infomation                                      */
    /*----------------------------------------------------------*/
    #define JDL_ACC_SIZE_DENOMI_NUM  2
    #define JDL_ACC_DENOMI_NUM       120
    #define JDL_ACC_CURRENCY_EACH    6
    #define JDL_ACC_CURRENCY_SIZE    (JDL_ACC_DENOMI_NUM * JDL_ACC_CURRENCY_EACH)
    
    #define JDL_ACC_UNKNOWN          1
    #define JDL_ACC_TICKET_NUM       1
    
    /*----------------------------------------------------------*/
    /* Counter settings                                         */
    /*----------------------------------------------------------*/
    #define JDL_ACC_SIZE_ACC_CNTR    JDL_DATA_TYPE_SIZE_DWORD /* [SIZE:4] size of insertion number. (total, bill, ticket, each denomination) */
    #define JDL_ACC_SIZE_REJ_CNTR    JDL_DATA_TYPE_SIZE_WORD  /* [SIZE:2] size of rejected number. */
    
    #define JDL_ACC_SIZE_STAT_A_CNTR JDL_DATA_TYPE_SIZE_WORD  /* [SIZE:2] size of accepted number for statistics  */
    #define JDL_ACC_SIZE_STAT_R_CNTR JDL_DATA_TYPE_SIZE_WORD  /* [SIZE:2] size of rejected number for statistics  */
    
    /*----------------------------------------------------------*/
    /* Datas Address of Acceptance Information                  */
    /*----------------------------------------------------------*/
    /*-- Acceptance Rate ----------------------------------------*//* Acceptance */
    #define JDL_ACC_ADR_STA_DATE    JDL_ACC_ADR_SEND_BASE                            /* BADR: 29720, CADR:     2 */
    #define JDL_ACC_ADR_TOTAL_INS  (JDL_ACC_ADR_STA_DATE + JDL_SIZE_TIME)            /* BADR: 29730, CADR:    10 */
    #define JDL_ACC_ADR_BILL_INS   (JDL_ACC_ADR_TOTAL_INS + JDL_ACC_SIZE_ACC_CNTR)   /* BADR: 29734, CADR:    14 */
    #define JDL_ACC_ADR_TICKET_INS (JDL_ACC_ADR_BILL_INS + JDL_ACC_SIZE_ACC_CNTR)    /* BADR: 29738, CADR:    18 */
    #define JDL_ACC_ADR_OTHER_NUM  (JDL_ACC_ADR_TICKET_INS + JDL_ACC_SIZE_ACC_CNTR)  /* BADR: 29742, CADR:    22 */
    #define JDL_ACC_ADR_INFO_SIZE  (JDL_ACC_ADR_OTHER_NUM + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 29744, CADR:    24 */
    
    #define JDL_ACC_ADR_ACC_BASE   (JDL_ACC_ADR_INFO_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 29746, CADR:    26 */
    
        /* Offset of items in each denomination *//* Acceptance */
        #define JDL_ACC_OFS_ACCEPT         0                                                   /* [OFS: 0] */
        #define JDL_ACC_OFS_REJ_SKEW       (JDL_ACC_OFS_ACCEPT + JDL_ACC_SIZE_ACC_CNTR)        /* [OFS: 4] *//* Acceptance-返却枚数コード別　*/
        #define JDL_ACC_OFS_REJ_MAG_PATTN  (JDL_ACC_OFS_REJ_SKEW + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS: 6] */
        #define JDL_ACC_OFS_REJ_MAG_AMOUN  (JDL_ACC_OFS_REJ_MAG_PATTN + JDL_ACC_SIZE_REJ_CNTR) /* [OFS: 8] */
        #define JDL_ACC_OFS_REJ_POSI_AT    (JDL_ACC_OFS_REJ_MAG_AMOUN + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:10] */
        #define JDL_ACC_OFS_REJ_XRATE      (JDL_ACC_OFS_REJ_POSI_AT + JDL_ACC_SIZE_REJ_CNTR)   /* [OFS:12] */
        #define JDL_ACC_OFS_REJ_CANCEL     (JDL_ACC_OFS_REJ_XRATE + JDL_ACC_SIZE_REJ_CNTR)     /* [OFS:14] */
        #define JDL_ACC_OFS_REJ_SLIP       (JDL_ACC_OFS_REJ_CANCEL + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:16] */
        #define JDL_ACC_OFS_REJ_FMOT_LOCK  (JDL_ACC_OFS_REJ_SLIP + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS:18] */
        #define JDL_ACC_OFS_REJ_FEED_TOUT  (JDL_ACC_OFS_REJ_FMOT_LOCK + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:20] */
        #define JDL_ACC_OFS_REJ_RESERVED1  (JDL_ACC_OFS_REJ_FEED_TOUT + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:22] */
        #define JDL_ACC_OFS_REJ_RESERVED2  (JDL_ACC_OFS_REJ_RESERVED1 + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:24] */
        #define JDL_ACC_OFS_REJ_APB_HOME   (JDL_ACC_OFS_REJ_RESERVED2 + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:26] */
        #define JDL_ACC_OFS_REJ_CENT_HOME  (JDL_ACC_OFS_REJ_APB_HOME + JDL_ACC_SIZE_REJ_CNTR)  /* [OFS:28] */
        #define JDL_ACC_OFS_REJ_PRECOMP    (JDL_ACC_OFS_REJ_CENT_HOME + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:30] */
        #define JDL_ACC_OFS_REJ_PHOT_PATT  (JDL_ACC_OFS_REJ_PRECOMP + JDL_ACC_SIZE_REJ_CNTR)   /* [OFS:32] */
        #define JDL_ACC_OFS_REJ_PHOT_LEVE  (JDL_ACC_OFS_REJ_PHOT_PATT + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:34] */
        #define JDL_ACC_OFS_REJ_INHIBIT    (JDL_ACC_OFS_REJ_PHOT_LEVE + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:36] */
        #define JDL_ACC_OFS_REJ_ESCR_TOUT  (JDL_ACC_OFS_REJ_INHIBIT + JDL_ACC_SIZE_REJ_CNTR)   /* [OFS:38] *///not use
        #define JDL_ACC_OFS_REJ_RETURN     (JDL_ACC_OFS_REJ_ESCR_TOUT + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:40] */
        #define JDL_ACC_OFS_REJ_OPERATION  (JDL_ACC_OFS_REJ_RETURN + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:42] *///not use
        #define JDL_ACC_OFS_REJ_LENGTH     (JDL_ACC_OFS_REJ_OPERATION + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:44] */
        #define JDL_ACC_OFS_REJ_SHORT      (JDL_ACC_OFS_REJ_LENGTH + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:46] */
        #define JDL_ACC_OFS_REJ_LONG       (JDL_ACC_OFS_REJ_SHORT + JDL_ACC_SIZE_REJ_CNTR)     /* [OFS:48] */
        #define JDL_ACC_OFS_REJ_SYNC       (JDL_ACC_OFS_REJ_LONG + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS:50] */
        #define JDL_ACC_OFS_REJ_DYE        (JDL_ACC_OFS_REJ_SYNC + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS:52] */
        #define JDL_ACC_OFS_REJ_HOLE       (JDL_ACC_OFS_REJ_DYE + JDL_ACC_SIZE_REJ_CNTR)       /* [OFS:54] */
        #define JDL_ACC_OFS_REJ_TEAR       (JDL_ACC_OFS_REJ_HOLE + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS:56] */
        #define JDL_ACC_OFS_REJ_DOG_EAR    (JDL_ACC_OFS_REJ_TEAR + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS:58] */
        #define JDL_ACC_OFS_REJ_CF         (JDL_ACC_OFS_REJ_DOG_EAR + JDL_ACC_SIZE_REJ_CNTR)   /* [OFS:60] */
        #define JDL_ACC_OFS_REJ_MCIR       (JDL_ACC_OFS_REJ_CF + JDL_ACC_SIZE_REJ_CNTR)        /* [OFS:62] */
        #define JDL_ACC_OFS_REJ_M3C        (JDL_ACC_OFS_REJ_MCIR + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS:64] */
        #define JDL_ACC_OFS_REJ_M4C        (JDL_ACC_OFS_REJ_M3C + JDL_ACC_SIZE_REJ_CNTR)       /* [OFS:66] */
        #define JDL_ACC_OFS_REJ_IR         (JDL_ACC_OFS_REJ_M4C + JDL_ACC_SIZE_REJ_CNTR)       /* [OFS:68] */
        #define JDL_ACC_OFS_REJ_THREAD     (JDL_ACC_OFS_REJ_IR + JDL_ACC_SIZE_REJ_CNTR)        /* [OFS:70] */
        #define JDL_ACC_OFS_REJ_LOST       (JDL_ACC_OFS_REJ_THREAD + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:72] */
        #define JDL_ACC_OFS_REJ_RESERVED3  (JDL_ACC_OFS_REJ_LOST + JDL_ACC_SIZE_REJ_CNTR)      /* [OFS:74] */
        #define JDL_ACC_OFS_REJ_RESERVED4  (JDL_ACC_OFS_REJ_RESERVED3 + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:76] */
        #define JDL_ACC_OFS_REJ_RESERVED5  (JDL_ACC_OFS_REJ_RESERVED4 + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:78] */
        #define JDL_ACC_OFS_REJ_RESERVED6  (JDL_ACC_OFS_REJ_RESERVED5 + JDL_ACC_SIZE_REJ_CNTR) /* [OFS:80] */
        
#if (JDL_ACC_TICKET_NUM != 0)
        /* The number of items must be less than or equal to the above denomination information. */
        #define JDL_ACC_OFS_REJ_BAR_NC     JDL_ACC_SIZE_ACC_CNTR                               /* [OFS: 4] */
        #define JDL_ACC_OFS_REJ_BAR_UN     (JDL_ACC_OFS_REJ_BAR_NC + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS: 6] */
        #define JDL_ACC_OFS_REJ_BAR_SH     (JDL_ACC_OFS_REJ_BAR_UN + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS: 8] */
        #define JDL_ACC_OFS_REJ_BAR_ST     (JDL_ACC_OFS_REJ_BAR_SH + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:10] */
        #define JDL_ACC_OFS_REJ_BAR_SP     (JDL_ACC_OFS_REJ_BAR_ST + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:12] */
        #define JDL_ACC_OFS_REJ_BAR_TP     (JDL_ACC_OFS_REJ_BAR_SP + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:14] */
        #define JDL_ACC_OFS_REJ_BAR_XR     (JDL_ACC_OFS_REJ_BAR_TP + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:16] */
        #define JDL_ACC_OFS_REJ_BAR_PHV    (JDL_ACC_OFS_REJ_BAR_XR + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:18] */
        #define JDL_ACC_OFS_REJ_BAR_DIN    (JDL_ACC_OFS_REJ_BAR_PHV + JDL_ACC_SIZE_REJ_CNTR)   /* [OFS:20] */
        #define JDL_ACC_OFS_REJ_BAR_LG     (JDL_ACC_OFS_REJ_BAR_DIN + JDL_ACC_SIZE_REJ_CNTR)   /* [OFS:22] */
        #define JDL_ACC_OFS_REJ_BAR_NG     (JDL_ACC_OFS_REJ_BAR_LG + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:24] */
        #define JDL_ACC_OFS_REJ_BAR_MC     (JDL_ACC_OFS_REJ_BAR_NG + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:26] */
        #define JDL_ACC_OFS_REJ_BAR_RESERV (JDL_ACC_OFS_REJ_BAR_MC + JDL_ACC_SIZE_REJ_CNTR)    /* [OFS:28] */
#endif /* JDL_ACC_TICKET_NUM != 0 */

        /* Acceptance Infomation setting */
        #define JDL_ACC_DINFO_SIZE         (JDL_ACC_OFS_REJ_RESERVED6 + JDL_ACC_SIZE_REJ_CNTR) /* [SIZE:82] */
        #define JDL_ACC_DINFO_NUM          (JDL_ACC_DENOMI_NUM + JDL_ACC_UNKNOWN + JDL_ACC_TICKET_NUM) /* [Denominations:122] 120 + 1(Unknown) + 1(Bar Ticket) */
        #define JDL_ACC_DINFO_TOTAL        (JDL_ACC_DINFO_SIZE * JDL_ACC_DINFO_NUM) /* [DINFO Total Size:10004] 82 * 122 */
        
        #define JDL_ACC_DINFO_UNKNOWN_IDX  (JDL_ACC_DENOMI_NUM) /* [Index:120] */
        #define JDL_ACC_DINFO_TICKET_IDX   (JDL_ACC_DENOMI_NUM + JDL_ACC_UNKNOWN) /* [Index:121] */
        
        
    #define JDL_ACC_ADR_ACC_CHECKSUM       (JDL_ACC_ADR_ACC_BASE + JDL_ACC_DINFO_TOTAL)             /* BADR: 39750, CADR: 10030 */
    
    /*-- Statistics 1  ------------------------------------------*/
    #define JDL_ACC_ADR_STAT1_BLK_SIZE     (JDL_ACC_ADR_ACC_CHECKSUM + JDL_SIZE_CHECKSUM)           /* BADR: 39752, CADR: 10032 */
    #define JDL_ACC_ADR_STAT1_BLK_NUM      (JDL_ACC_ADR_STAT1_BLK_SIZE + JDL_DATA_TYPE_SIZE_WORD)   /* BADR: 39754, CADR: 10034 */
    #define JDL_ACC_ADR_STAT1_DINFO_SIZE   (JDL_ACC_ADR_STAT1_BLK_NUM + JDL_DATA_TYPE_SIZE_WORD)    /* BADR: 39756, CADR: 10036 */
    #define JDL_ACC_ADR_STAT1_CUR_BLK      (JDL_ACC_ADR_STAT1_DINFO_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 39758, CADR: 10038 */
    #define JDL_ACC_ADR_STAT1_BLK_RND      (JDL_ACC_ADR_STAT1_CUR_BLK + JDL_DATA_TYPE_SIZE_WORD)    /* BADR: 39760, CADR: 10040 */
    
    #define JDL_ACC_ADR_STAT1_BLK_BASE     (JDL_ACC_ADR_STAT1_BLK_RND + JDL_SIZE_BUFF_ROUND)        /* BADR: 39761, CADR: 10041 */
        
        /* Offset of items in Block of Statistics 1 */
        #define JDL_ACC_OFS_S1_BLK_DINFO       0
         
            /* Offset of items in each denomination of Statistics 1 Block */
            #define JDL_ACC_OFS_S1_DINFO_ACC   0                                                         /* [OFS: 0] */
            #define JDL_ACC_OFS_S1_DINFO_OPT   JDL_ACC_SIZE_STAT_A_CNTR                                  /* [OFS: 2] */
            #define JDL_ACC_OFS_S1_DINFO_MAG   (JDL_ACC_OFS_S1_DINFO_OPT + JDL_ACC_SIZE_STAT_R_CNTR)     /* [OFS: 4] */
            #define JDL_ACC_OFS_S1_DINFO_UV    (JDL_ACC_OFS_S1_DINFO_MAG + JDL_ACC_SIZE_STAT_R_CNTR)     /* [OFS: 6] */
            #define JDL_ACC_OFS_S1_DINFO_CF    (JDL_ACC_OFS_S1_DINFO_UV + JDL_ACC_SIZE_STAT_R_CNTR)      /* [OFS: 8] */
            #define JDL_ACC_OFS_S1_DINFO_FEED  (JDL_ACC_OFS_S1_DINFO_CF + JDL_ACC_SIZE_STAT_R_CNTR)      /* [OFS:10] */
            #define JDL_ACC_OFS_S1_DINFO_RET   (JDL_ACC_OFS_S1_DINFO_FEED + JDL_ACC_SIZE_STAT_R_CNTR)    /* [OFS:12] */
            
#if (JDL_ACC_TICKET_NUM != 0)
            /* The number of items must be less than or equal to the above denomination information. */
            #define JDL_ACC_OFS_S1_DINFO_BAR_NC JDL_ACC_SIZE_STAT_A_CNTR                                 /* [OFS: 2] */
            #define JDL_ACC_OFS_S1_DINFO_BAR_UN (JDL_ACC_OFS_S1_DINFO_BAR_NC + JDL_ACC_SIZE_STAT_A_CNTR) /* [OFS: 4] */
            #define JDL_ACC_OFS_S1_DINFO_BAR_ST (JDL_ACC_OFS_S1_DINFO_BAR_UN + JDL_ACC_SIZE_STAT_A_CNTR) /* [OFS: 6] */
            #define JDL_ACC_OFS_S1_DINFO_BAR_XR (JDL_ACC_OFS_S1_DINFO_BAR_ST + JDL_ACC_SIZE_STAT_A_CNTR) /* [OFS: 8] */
            #define JDL_ACC_OFS_S1_DINFO_BAR_LG (JDL_ACC_OFS_S1_DINFO_BAR_XR + JDL_ACC_SIZE_STAT_A_CNTR) /* [OFS:10] */
            #define JDL_ACC_OFS_S1_DINFO_BAR_MC (JDL_ACC_OFS_S1_DINFO_BAR_LG + JDL_ACC_SIZE_STAT_A_CNTR) /* [OFS:12] */
#endif /* JDL_ACC_TICKET_NUM != 0 */
            
            /* Each Denomination setting */
            #define JDL_ACC_S1_DINFO_SIZE      (JDL_ACC_OFS_S1_DINFO_RET + JDL_ACC_SIZE_STAT_R_CNTR)     /* [SIZE:14] */
            #define JDL_ACC_S1_DINFO_TOTAL     (JDL_ACC_S1_DINFO_SIZE * JDL_ACC_DINFO_NUM) /* [Total Size:1708] 14 * 122 */
            
        #define JDL_ACC_OFS_S1_BLK_STIME   JDL_ACC_S1_DINFO_TOTAL                            /* [OFS:1708] */
        #define JDL_ACC_OFS_S1_BLK_CSUM    (JDL_ACC_OFS_S1_BLK_STIME + JDL_SIZE_TIME)        /* [OFS:1716] */

        /* Block setting */
        #define JDL_ACC_STAT1_PINS_NUM_BLK 1000  /* Insertion number of each statistics 1 block */
        #define JDL_ACC_STAT1_BLK_SIZE     (JDL_ACC_OFS_S1_BLK_CSUM + JDL_SIZE_CHECKSUM) /* [SIZE:1718] */
        #define JDL_ACC_STAT1_BLK_NUM      10
        
        #define JDL_ACC_STAT1_BLK_TOTAL    (JDL_ACC_STAT1_BLK_SIZE * JDL_ACC_STAT1_BLK_NUM) /* [STAT1 Block Total Size:17180] */
    
    /*-- Statistics 2  ------------------------------------------*/
    #define JDL_ACC_ADR_STAT2_BLK_SIZE     (JDL_ACC_ADR_STAT1_BLK_BASE + JDL_ACC_STAT1_BLK_TOTAL)   /* BADR: 56941, CADR: 27221 */
    #define JDL_ACC_ADR_STAT2_BLK_NUM      (JDL_ACC_ADR_STAT2_BLK_SIZE + JDL_DATA_TYPE_SIZE_WORD)   /* BADR: 56943, CADR: 27223 */
    #define JDL_ACC_ADR_STAT2_DINFO_SIZE   (JDL_ACC_ADR_STAT2_BLK_NUM + JDL_DATA_TYPE_SIZE_WORD)    /* BADR: 56945, CADR: 27225 */
    #define JDL_ACC_ADR_STAT2_CUR_BLK      (JDL_ACC_ADR_STAT2_DINFO_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 56947, CADR: 27227 */
    #define JDL_ACC_ADR_STAT2_BLK_RND      (JDL_ACC_ADR_STAT2_CUR_BLK + JDL_DATA_TYPE_SIZE_WORD)    /* BADR: 56949, CADR: 27229 */
    #define JDL_ACC_ADR_STAT2_REJ_SIZE     (JDL_ACC_ADR_STAT2_BLK_RND + JDL_SIZE_BUFF_ROUND)        /* BADR: 56950, CADR: 27230 */
    
    #define JDL_ACC_ADR_STAT2_BLK_BASE     (JDL_ACC_ADR_STAT2_REJ_SIZE + JDL_DATA_TYPE_SIZE_WORD)   /* BADR: 56952, CADR: 27221 */
    
        /* Offset of items in Block of Statistics 2 */
        #define JDL_ACC_OFS_S2_BLK_DINFO   0
         
            /* Offset of items in each denomination of Statistics 2 Block */
            #define JDL_ACC_OFS_S2_DINFO_ACC   0                                                     /* [OFS: 0] */
            #define JDL_ACC_OFS_S2_DINFO_REJ   JDL_ACC_SIZE_STAT_A_CNTR                              /* [OFS: 2] */
            
            /* Each Denomination setting */
            #define JDL_ACC_S2_DINFO_SIZE      (JDL_ACC_OFS_S2_DINFO_REJ + JDL_ACC_SIZE_STAT_R_CNTR) /* [SIZE:4] */
            #define JDL_ACC_S2_DINFO_TOTAL     (JDL_ACC_S2_DINFO_SIZE * JDL_ACC_DINFO_NUM)
            
        #define JDL_ACC_OFS_S2_BLK_OPT     JDL_ACC_S2_DINFO_TOTAL                                    /* [OFS:488] */
        #define JDL_ACC_OFS_S2_BLK_MAG     (JDL_ACC_OFS_S2_BLK_OPT + JDL_ACC_SIZE_STAT_R_CNTR)       /* [OFS:490] */
        #define JDL_ACC_OFS_S2_BLK_UV      (JDL_ACC_OFS_S2_BLK_MAG + JDL_ACC_SIZE_STAT_R_CNTR)       /* [OFS:492] */
        #define JDL_ACC_OFS_S2_BLK_CF      (JDL_ACC_OFS_S2_BLK_UV + JDL_ACC_SIZE_STAT_R_CNTR)        /* [OFS:494] */
        #define JDL_ACC_OFS_S2_BLK_FEED    (JDL_ACC_OFS_S2_BLK_CF + JDL_ACC_SIZE_STAT_R_CNTR)        /* [OFS:496] */
        #define JDL_ACC_OFS_S2_BLK_RET     (JDL_ACC_OFS_S2_BLK_FEED + JDL_ACC_SIZE_STAT_R_CNTR)      /* [OFS:498] */

#if (JDL_ACC_TICKET_NUM != 0)
        /* The number of items must be less than or equal to the above denomination information. */
        #define JDL_ACC_OFS_S2_BLK_BAR_NC  (JDL_ACC_OFS_S2_BLK_RET + JDL_ACC_SIZE_STAT_A_CNTR)       /* [OFS:500] */
        #define JDL_ACC_OFS_S2_BLK_BAR_UN  (JDL_ACC_OFS_S2_BLK_BAR_NC + JDL_ACC_SIZE_STAT_A_CNTR)    /* [OFS:502] */
        #define JDL_ACC_OFS_S2_BLK_BAR_ST  (JDL_ACC_OFS_S2_BLK_BAR_UN + JDL_ACC_SIZE_STAT_A_CNTR)    /* [OFS:504] */
        #define JDL_ACC_OFS_S2_BLK_BAR_XR  (JDL_ACC_OFS_S2_BLK_BAR_ST + JDL_ACC_SIZE_STAT_A_CNTR)    /* [OFS:506] */
        #define JDL_ACC_OFS_S2_BLK_BAR_LG  (JDL_ACC_OFS_S2_BLK_BAR_XR + JDL_ACC_SIZE_STAT_A_CNTR)    /* [OFS:508] */
        #define JDL_ACC_OFS_S2_BLK_BAR_MC  (JDL_ACC_OFS_S2_BLK_BAR_LG + JDL_ACC_SIZE_STAT_A_CNTR)    /* [OFS:510] */
        
        #define JDL_ACC_OFS_S2_BLK_STIME   (JDL_ACC_OFS_S2_BLK_BAR_MC + JDL_ACC_SIZE_STAT_R_CNTR)    /* [OFS:512] */
#else  /* JDL_ACC_TICKET_NUM == 0 */
        #define JDL_ACC_OFS_S2_BLK_STIME   (JDL_ACC_OFS_S2_BLK_RET + JDL_ACC_SIZE_STAT_R_CNTR)       /* [OFS:502] */
#endif /* JDL_ACC_TICKET_NUM != 0 */
        #define JDL_ACC_OFS_S2_BLK_CSUM    (JDL_ACC_OFS_S2_BLK_STIME + JDL_SIZE_TIME)                /* [OFS:520] */
        
        /* Block setting */
        #define JDL_ACC_STAT2_PINS_NUM_BLK 10000  /* Insertion number of each statistics 1 block */
        #define JDL_ACC_STAT2_BLK_SIZE     (JDL_ACC_OFS_S2_BLK_CSUM + JDL_SIZE_CHECKSUM)    /* [SIZE:522] */
        #define JDL_ACC_STAT2_BLK_NUM      10
        #define JDL_ACC_STAT2_BLK_REJ_SIZE 12  /* [SIZE:12] 6 * 2 */

        #define JDL_ACC_STAT2_BLK_TOTAL    (JDL_ACC_STAT2_BLK_SIZE * JDL_ACC_STAT2_BLK_NUM) /* [STAT2 Total Size:5220] 522 * 10 */
    
    
    /*-- Previous Version --------------------------------------*/
    #define JDL_ACC_ADR_PREV_VER_INS       (JDL_ACC_ADR_STAT2_BLK_BASE + JDL_ACC_STAT2_BLK_TOTAL) /* BADR: 62172, CADR: 32452 */
    
    
    /*----------------------------------------------------------*/
    /* Address for the reserved                                 */
    /*----------------------------------------------------------*/
    #define JDL_ACC_ADR_FOR_RESERVED  (JDL_ACC_ADR_PREV_VER_INS + JDL_DATA_TYPE_SIZE_DWORD) /* [Address for the reserved] BADR: 62176, CADR: 32456 */
    
    
    /*----------------------------------------------------------*/
    /* Address for the next category                            */
    /*----------------------------------------------------------*/
    #define JDL_ACC_ADR_FOR_NEXT_CATEGORY (JDL_ACC_ADR_FOR_RESERVED + 0)    /* [Address for the next category] BADR: 62176, CADR: 32456  (Reserved area = 0) */
    
    
    /*----------------------------------------------------------*/
    /* Acceptance total size                                    */
    /*----------------------------------------------------------*/
    /* Total size of Acceptance Category data to send */
    #define JDL_ACC_SEND_TOTAL (JDL_ACC_ADR_FOR_RESERVED - JDL_ACC_ADR_SEND_BASE)      /* [Acceptance Send Data Total:33206] Without Reserved and Revision */
    /* Total size of Acceptance Category buffer without reserved area */
    #define JDL_ACC_BUFF_TOTAL (JDL_ACC_ADR_FOR_NEXT_CATEGORY - JDL_ACC_ADR_BUFF_BASE) /* [Acceptance Buffer Total:33208] 62176 - 32456(Next Addr - Base Addr) */
    
#endif  /* _JDL_USE_ACC */



/*==========================================================================*/
/*==========================================================================*/
/* Position Sensor Analysis                                                 */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/
#define _JDL_USE_POSIANA
#if defined(_JDL_USE_POSIANA)
    /*----------------------------------------------------------*/
    /* Revision                                                 */
    /*----------------------------------------------------------*/
    #define JDL_PANA_REV            0x0002
    
    
    /*----------------------------------------------------------*/
    /* Base Address of Position Sensor Analysis Category        */
    /*----------------------------------------------------------*/
    /* Base Address of Position Sensor Analysis Category buffer */
    #define JDL_PANA_ADR_BUFF_BASE  JDL_ACC_ADR_FOR_NEXT_CATEGORY                /* [Buffer Base:62176] */
                                                                                     /* BADR:Buffer Address, CADR:Category Data Address */
    #define JDL_PANA_ADR_REV        JDL_PANA_ADR_BUFF_BASE                                   /* BADR: 62176, CADR:     0 */
    
    /* Base Address of Position Sensor Analysis Category data to send */
    #define JDL_PANA_ADR_SEND_BASE  (JDL_PANA_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:62178] */
    
    
    /*----------------------------------------------------------*/
    /* Datas Address                                            */
    /*----------------------------------------------------------*/
    #define JDL_PANA_ADR_BLK_SIZE    JDL_PANA_ADR_SEND_BASE                             /* BADR: 62178, CADR:     2 */
    #define JDL_PANA_ADR_BLK_NUM    (JDL_PANA_ADR_BLK_SIZE + JDL_DATA_TYPE_SIZE_WORD)   /* BADR: 62180, CADR:     4 */
    #define JDL_PANA_ADR_SDATA_SIZE (JDL_PANA_ADR_BLK_NUM + JDL_DATA_TYPE_SIZE_WORD)    /* BADR: 62182, CADR:     6 */
    #define JDL_PANA_ADR_SDATA_NUM  (JDL_PANA_ADR_SDATA_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 62184, CADR:     8 */
    #define JDL_PANA_ADR_CUR_BLK    (JDL_PANA_ADR_SDATA_NUM + JDL_DATA_TYPE_SIZE_WORD)  /* BADR: 62186, CADR:    10 */
    #define JDL_PANA_ADR_BLK_RND    (JDL_PANA_ADR_CUR_BLK + JDL_DATA_TYPE_SIZE_WORD)    /* BADR: 62188, CADR:    12 */
    
    /*----------------------------------------------------------*/
    /* Block Datas Address                                      */
    /*----------------------------------------------------------*/
    #define JDL_PANA_ADR_BLK_BASE   (JDL_PANA_ADR_BLK_RND + JDL_SIZE_BUFF_ROUND)        /* BADR: 62189, CADR:    13 */
    
        /* Offset of items in Block */
        #define JDL_PANA_OFS_BLK_ERR_IDX  0                                                    /* [OFS:0] */
        #define JDL_PANA_OFS_BLK_ERR_RND  (JDL_PANA_OFS_BLK_ERR_IDX + JDL_DATA_TYPE_SIZE_WORD) /* [OFS:2] */
        #define JDL_PANA_OFS_BLK_SDATA    (JDL_PANA_OFS_BLK_ERR_RND + JDL_SIZE_BUFF_ROUND)     /* [OFS:3] */
        
            /* Record setting */
            #define JDL_PANA_RCD_SENS_SIZE 5     /* 24-11-05 */

            #define JDL_PANA_RCD_PULSE_SIZE 4
            
            #define JDL_PANA_RCD_SDATE_SIZE (JDL_PANA_RCD_SENS_SIZE + JDL_PANA_RCD_PULSE_SIZE)
            #define JDL_PANA_RCD_ETIME_SIZE 2
            
            #define JDL_PANA_RCD_SIZE   (JDL_PANA_RCD_SDATE_SIZE + JDL_PANA_RCD_ETIME_SIZE)
            #define JDL_PANA_RCD_NUM    140
            #define JDL_PANA_RCD_TOTAL  (JDL_PANA_RCD_SIZE * JDL_PANA_RCD_NUM)
        
        /* Block Setting */
        #define JDL_PANA_BLK_SIZE         (JDL_PANA_OFS_BLK_SDATA + JDL_PANA_RCD_TOTAL)
        #define JDL_PANA_BLK_NUM          16
        #define JDL_PANA_BLK_TOTAL        (JDL_PANA_BLK_SIZE * JDL_PANA_BLK_NUM) /* [BLK Total Size:24688] */
    
    
    /*----------------------------------------------------------*/
    /* Address for the reserved                                 */
    /*----------------------------------------------------------*/
    #define JDL_PANA_ADR_FOR_RESERVED  (JDL_PANA_ADR_BLK_BASE + JDL_PANA_BLK_TOTAL) /* [Address for the reserved] BADR: 86877, CADR: 14171 */
    
    
    /*----------------------------------------------------------*/
    /* Address for the next category                            */
    /*----------------------------------------------------------*/
    #define JDL_PANA_ADR_FOR_NEXT_CATEGORY (JDL_PANA_ADR_FOR_RESERVED + 0)          /* [Address for the next category] BADR: 86877, CADR: 14171  (Reserved area = 0) */
    
    
    /*----------------------------------------------------------*/
    /* Positoin Sensor Analysi total size                       */
    /*----------------------------------------------------------*/
    /* Total size of Positoin Sensor Analysi Category data to send */
    #define JDL_PANA_SEND_TOTAL (JDL_PANA_ADR_FOR_RESERVED - JDL_PANA_ADR_SEND_BASE)      /* [Positoin Sensor Analysi Send Data Total:14169] Without Reserved and Revision */
    /* Total size of Positoin Sensor Analysi Category buffer without reserved area */
    #define JDL_PANA_BUFF_TOTAL (JDL_PANA_ADR_FOR_NEXT_CATEGORY - JDL_PANA_ADR_BUFF_BASE) /* [Positoin Sensor Analysi Buffer Total:14201] 86877 - 82163(Next Addr - Base Addr) */
    
#endif  /* _JDL_USE_POSIANA */


/*==========================================================================*/
/*==========================================================================*/
/* Additional RC                                                            */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/
#if defined(UBA_RTQ)
#define _JDL_USE_RC
#endif 
//#define _JDL_USE_RC
#if defined(_JDL_USE_RC)
    /*----------------------------------------------------------*/
    /* Revision                                                 */
    /*----------------------------------------------------------*/
    #define JDL_RC_REV             0x0001
    
    
    /*----------------------------------------------------------*/
    /* Base Address of Additional RC Category                   */
    /*----------------------------------------------------------*/
    /* Base Address of Additional RC Category buffer */
    #define JDL_RC_ADR_BUFF_BASE  JDL_PANA_ADR_FOR_NEXT_CATEGORY             /* [Buffer Base:86877] */
                                                                                  /* BADR:Buffer Address, CADR:Category Data Address */
    #define JDL_RC_ADR_REV        JDL_RC_ADR_BUFF_BASE                                    /* BADR: 86877, CADR:     0 */
    
    /* Base Address of Additional RC Category data to send */
    #define JDL_RC_ADR_SEND_BASE  (JDL_RC_ADR_REV + JDL_DATA_TYPE_SIZE_WORD) /* [Send Data Base:86879] */
    
    
    /*----------------------------------------------------------*/
    /* RC Data settings                                         */
    /*----------------------------------------------------------*/
    /* Item Size */
    #define JDL_RC_SIZE_UNIT_STAT  2
    #define JDL_RC_SIZE_SENS_STAT  2
    #define JDL_RC_SIZE_ERR        2
    #define JDL_RC_SIZE_NOTES_LEN  1
    #define JDL_RC_SIZE_CURRENY    2
    
    /* Number of Units */
    #define JDL_RC_UNIT_NUM        2
    
    /* Number of Drums */
    #define JDL_RC_DRUM_NUM        2
    
    /* Number of Tracking */
    #define JDL_RC_TRACKING_NUM    32
    
    /*----------------------------------------------------------*/
    /* Datas Address                                            */
    /*----------------------------------------------------------*/
    #define JDL_RC_ADR_UNIT_NUM        JDL_RC_ADR_SEND_BASE                               /* BADR: 86879, CADR:     2 */
    #define JDL_RC_ADR_UNIT_INFO_SIZE (JDL_RC_ADR_UNIT_NUM + JDL_DATA_TYPE_SIZE_WORD)     /* BADR: 86881, CADR:     4 */
    
    /*----------------------------------------------------------*/
    /* Unit Info Address                                        */
    /*----------------------------------------------------------*/
    #define JDL_RC_ADR_UINFO_BASE   (JDL_RC_ADR_UNIT_INFO_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* BADR: 86883, CADR:     6 */
    
        /* Unit Infor Offset */
        #define JDL_RC_OFS_UINFO_BOOT_VER   0                                                       /* [OFS:  0] */
        #define JDL_RC_OFS_UINFO_FIRM_VER   (JDL_RC_OFS_UINFO_BOOT_VER + JDL_SIZE_BOOT_VER)         /* [OFS: 16] */
        #define JDL_RC_OFS_UINFO_FIRM_CRC   (JDL_RC_OFS_UINFO_FIRM_VER + JDL_SIZE_FIRM_VER)         /* [OFS: 80] */
        #define JDL_RC_OFS_UINFO_UNIT_STAT  (JDL_RC_OFS_UINFO_FIRM_CRC + JDL_SIZE_CRC16)            /* [OFS: 82] */
        #define JDL_RC_OFS_UINFO_SENS_STAT  (JDL_RC_OFS_UINFO_UNIT_STAT + JDL_RC_SIZE_UNIT_STAT)    /* [OFS: 84] */
        #define JDL_RC_OFS_UINFO_ERR        (JDL_RC_OFS_UINFO_SENS_STAT + JDL_RC_SIZE_SENS_STAT)    /* [OFS: 86] */
        #define JDL_RC_OFS_UINFO_IN_SPEED   (JDL_RC_OFS_UINFO_ERR + JDL_RC_SIZE_ERR)                /* [OFS: 88] */
        #define JDL_RC_OFS_UINFO_OUT_SPEED  (JDL_RC_OFS_UINFO_IN_SPEED + JDL_DATA_TYPE_SIZE_WORD)   /* [OFS: 90] */
        #define JDL_RC_OFS_UINFO_DRUM_NUM   (JDL_RC_OFS_UINFO_OUT_SPEED + JDL_DATA_TYPE_SIZE_WORD)  /* [OFS: 92] */
        #define JDL_RC_OFS_UINFO_DINFO_SIZE (JDL_RC_OFS_UINFO_DRUM_NUM + JDL_DATA_TYPE_SIZE_WORD)   /* [OFS: 94] */
        
        #define JDL_RC_OFS_UINFO_DINFO_BASE (JDL_RC_OFS_UINFO_DINFO_SIZE + JDL_DATA_TYPE_SIZE_WORD) /* [BASE: 96] */
        
            /* Unit Information Offset */
            #define JDL_RC_OFS_DINFO_STACK      0                                                      /* [DINFO OFS: 0] */
            #define JDL_RC_OFS_DINFO_PAYOUT     (JDL_RC_OFS_DINFO_STACK + JDL_DATA_TYPE_SIZE_DWORD)    /* [DINFO OFS: 4] */
            #define JDL_RC_OFS_DINFO_COLLECT    (JDL_RC_OFS_DINFO_PAYOUT + JDL_DATA_TYPE_SIZE_DWORD)   /* [DINFO OFS: 8] */
            #define JDL_RC_OFS_DINFO_P_REJECT   (JDL_RC_OFS_DINFO_COLLECT + JDL_DATA_TYPE_SIZE_DWORD)  /* [DINFO OFS:12] */
            #define JDL_RC_OFS_DINFO_CHECKSUM   (JDL_RC_OFS_DINFO_P_REJECT + JDL_DATA_TYPE_SIZE_WORD)  /* [DINFO OFS:14] */
            
            #define JDL_RC_OFS_DINFO_SENS_STAT  (JDL_RC_OFS_DINFO_CHECKSUM + JDL_SIZE_CHECKSUM)        /* [DINFO OFS:16] */
            #define JDL_RC_OFS_DINFO_CURRENCY   (JDL_RC_OFS_DINFO_SENS_STAT + JDL_RC_SIZE_SENS_STAT)   /* [DINFO OFS:18] */
            #define JDL_RC_OFS_DINFO_LENGTH     (JDL_RC_OFS_DINFO_CURRENCY + JDL_RC_SIZE_CURRENY)      /* [DINFO OFS:20] */
            #define JDL_RC_OFS_DINFO_MAX_LEN    (JDL_RC_OFS_DINFO_LENGTH + JDL_RC_SIZE_NOTES_LEN)      /* [DINFO OFS:21] */
            #define JDL_RC_OFS_DINFO_MIN_LEN    (JDL_RC_OFS_DINFO_MAX_LEN + JDL_RC_SIZE_NOTES_LEN)     /* [DINFO OFS:22] */
            #define JDL_RC_OFS_DINFO_MAX_NOTES  (JDL_RC_OFS_DINFO_MIN_LEN + JDL_RC_SIZE_NOTES_LEN)     /* [DINFO OFS:23] */
            #define JDL_RC_OFS_DINFO_TRACK_NUM  (JDL_RC_OFS_DINFO_MAX_NOTES + JDL_DATA_TYPE_SIZE_WORD) /* [DINFO OFS:25] */
            
            #define JDL_RC_OFS_DINFO_TRACK_BASE (JDL_RC_OFS_DINFO_TRACK_NUM + JDL_DATA_TYPE_SIZE_WORD) /* [BASE:27] */
            
                #define JDL_RC_OFS_TRACK_CURRENT  0 /* [TRACK OFS: 0] */
                #define JDL_RC_OFS_TRACK_BASE     (JDL_RC_OFS_TRACK_CURRENT + JDL_DATA_TYPE_SIZE_WORD) /* [TRACK OFS: 2] */
                #define JDL_RC_OFS_TRACK_CHECKSUM (JDL_RC_OFS_TRACK_BASE + JDL_RC_TRACKING_NUM)        /* [TRACK OFS:34] */
                
                #define JDL_RC_TRACK_SIZE (JDL_RC_OFS_TRACK_CHECKSUM + JDL_SIZE_CHECKSUM)              /* [TRACK SIZE:36] */
            
            #define JDL_RC_DINFO_SIZE (JDL_RC_OFS_DINFO_TRACK_BASE + JDL_RC_TRACK_SIZE)   /* [DINFO SIZE:63] */
            #define JDL_RC_DINFO_TOTAL_SIZE (JDL_RC_DINFO_SIZE * JDL_RC_DRUM_NUM)         /* [DINFO TOTAL:126] */
        
        #define JDL_RC_UNIFO_SIZE (JDL_RC_OFS_UINFO_DINFO_BASE + JDL_RC_DINFO_TOTAL_SIZE) /* [UNIFO SIZE:222]*/
        #define JDL_RC_UNIFO_TOTAL_SIZE (JDL_RC_UNIFO_SIZE * JDL_RC_UNIT_NUM)             /* [UNIFO TOTAL:444] */
    
    
    /*----------------------------------------------------------*/
    /* Address for the reserved                                 */
    /*----------------------------------------------------------*/
    #define JDL_RC_ADR_FOR_RESERVED  (JDL_RC_ADR_UINFO_BASE + JDL_RC_UNIFO_TOTAL_SIZE) /* [Address for the reserved] BADR: 87327, CADR:   450 */
    
    
    /*----------------------------------------------------------*/
    /* Address for the next category                            */
    /*----------------------------------------------------------*/
    #define JDL_RC_ADR_FOR_NEXT_CATEGORY (JDL_RC_ADR_FOR_RESERVED + 0)                 /* [Address for the next category] BADR: 87327, CADR:   450  (Reserved area = 0) */
    
    
    /*----------------------------------------------------------*/
    /* Additional RC total size                                 */
    /*----------------------------------------------------------*/
    /* Total size of Additional RC Category data to send */
    #define JDL_RC_SEND_TOTAL (JDL_RC_ADR_FOR_RESERVED - JDL_RC_ADR_SEND_BASE)      /* [Additional RC Send Data Total:448] Without Reserved and Revision */
    /* Total size of Additional RC Category buffer without reserved area */
    #define JDL_RC_BUFF_TOTAL (JDL_RC_ADR_FOR_NEXT_CATEGORY - JDL_RC_ADR_BUFF_BASE) /* [Additional RC Buffer Total:450] 87327 - 86877(Next Addr - Base Addr) */
    
#endif  /* _JDL_USE_RC */

/*==========================================================================*/
/*==========================================================================*/
/* Option  RC                                                               */
/*    - Revision, Set Value, Address of each items and total size           */
/*==========================================================================*/
#if defined(UBA_RC) || defined(UBA_RTQ)		/* '19-07-10 */
#define _JDL_USE_OP_RC
// #define _JDL_OPRC_EXCLUDE_YOBI
#if defined(_JDL_OPRC_EXCLUDE_YOBI)
// 予備
#define MRAM_ADDRESS_YOBI				1656
#define MRAM_SIZE_YOBI					14342
#endif

#if defined(_JDL_USE_OP_RC)
    /*----------------------------------------------------------*/
    /* Revision                                                 */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_REV             0x0001
    
    
    /*----------------------------------------------------------*/
    /* Base Address of Option RC Category                       */
    /*----------------------------------------------------------*/
    /* Base Address of Option RC Category buffer */
    #define JDL_OPRC_ADR_BUFF_BASE  JDL_RC_ADR_FOR_NEXT_CATEGORY                                    /* [Buffer Base:93515] */
                                                                                                    /* BADR:Buffer Address, CADR:Category Data Address */
    #define JDL_OPRC_ADR_REV        JDL_OPRC_ADR_BUFF_BASE                                          /* BADR: 93515, CADR:     0 */
    
    /* Base Address of Additional RC Category data to send */
    #define JDL_OPRC_ADR_SEND_BASE  (JDL_OPRC_ADR_REV + JDL_DATA_TYPE_SIZE_WORD)                       /* [Send Data Base:93517] */
    
     /*----------------------------------------------------------*/
    /* RC Data settings                                         */
    /*----------------------------------------------------------*/
    /* Item Size */
    #define JDL_SIZE_UNIT_INFO 16
    #define JDL_SIZE_LOG_FORMAT 2
    #define JDL_SIZE_SERIAL_NO 12
    #define JDL_SIZE_OP_BOOT_VER 28
    #define JDL_SIZE_MAIN_VER 28
    #define JDL_SIZE_ADJ_SOFT_VER 2
    #define JDL_SIZE_ADJ_DATE 8
    #define JDL_SIZE_UI_RESERVE 15
    
    #define JDL_SIZE_RC__RESERVE 9
    #define JDL_SIZE_DOPE_RESERVE 7
    #define JDL_SIZE_REJ_RESERVE 7
    #define JDL_SIZE_DSENS_RESERVE 5
    #define JDL_SIZE_OPE_RESERVE 15
    #define JDL_SIZE_DERR_RESERVE 15
    
    #define JDL_SIZE_MOPE_RESERVE 27
    #define JDL_SIZE_MERR_RESERVE 31
    #define JDL_SIZE_MSENS_RESERVE 0
    #define JDL_SIZE_DL_RESERVE 14
    #define JDL_SIZE_MTRSP_RESERVE 22
    #define JDL_SIZE_LOPE_RESERVE 20
    #define JDL_SIZE_CMTRSP_RESERVE 23
    #define JDL_SIZE_BSUB_RESERVE 14
    #define JDL_SIZE_OPRC_RESERVE 14342
    #define JDL_SIZE_OLOG_DATA 32
    
    /*----------------------------------------------------------*/
    /* Main Unit Info Address                                   */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_MUINFO_BASE   (JDL_OPRC_ADR_SEND_BASE)                                              /* BADR: 93517, CADR:     2 */
    
        /* Unit Info Offset */
        #define JDL_OPRC_OFS_UINFO_UNIT_INFO       0                                                         /* [OFS:   0] */
        #define JDL_OPRC_OFS_UINFO_LOG_FORMAT      (JDL_OPRC_OFS_UINFO_UNIT_INFO + JDL_SIZE_UNIT_INFO)       /* [OFS:  16] */
        #define JDL_OPRC_OFS_UINFO_SERIAL_NO       (JDL_OPRC_OFS_UINFO_LOG_FORMAT + JDL_SIZE_LOG_FORMAT)     /* [OFS:  18] */
        #define JDL_OPRC_OFS_UINFO_BOOT_VER        (JDL_OPRC_OFS_UINFO_SERIAL_NO + JDL_SIZE_SERIAL_NO)       /* [OFS:  30] */
        #define JDL_OPRC_OFS_UINFO_MAIN_VER        (JDL_OPRC_OFS_UINFO_BOOT_VER + JDL_SIZE_OP_BOOT_VER)      /* [OFS:  58] */
        #define JDL_OPRC_OFS_UINFO_ADJ_SOFT_VER    (JDL_OPRC_OFS_UINFO_MAIN_VER + JDL_SIZE_MAIN_VER)         /* [OFS:  86] */
        #define JDL_OPRC_OFS_UINFO_ADJ_DATE        (JDL_OPRC_OFS_UINFO_ADJ_SOFT_VER + JDL_SIZE_ADJ_SOFT_VER) /* [OFS:  88] */
        #define JDL_OPRC_OFS_UINFO_RESERVE         (JDL_OPRC_OFS_UINFO_ADJ_DATE + JDL_SIZE_ADJ_DATE)         /* [OFS:  96] */
        #define JDL_OPRC_OFS_UINFO_CHECKSUM        (JDL_OPRC_OFS_UINFO_RESERVE + JDL_SIZE_UI_RESERVE)        /* [OFS: 111] */
        
        #define JDL_OPRC_OFS_UINFO_BASE            (JDL_OPRC_OFS_UINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)   /* [BASE: 112] */
    /*----------------------------------------------------------*/
    /* Factory Main Unit Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_FMUINFO_BASE              (JDL_OPRC_ADR_MUINFO_BASE + JDL_OPRC_OFS_UINFO_BASE)      /* BADR: 93629, CADR:     114 */
    /*----------------------------------------------------------*/
    /* RC-Twin Factory Main Unit Info Address                   */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_TFMUINFO_BASE             (JDL_OPRC_ADR_FMUINFO_BASE + JDL_OPRC_OFS_UINFO_BASE)     /* BADR: 93741, CADR:     226 */
    /*----------------------------------------------------------*/
    /* RC-Twin Drum Unit Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DUINFO_BASE               (JDL_OPRC_ADR_TFMUINFO_BASE + JDL_OPRC_OFS_UINFO_BASE)    /* BADR: 93853, CADR:     338 */
     /*----------------------------------------------------------*/
    /* RC-Twin RC Denomi Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_RCINFO_BASE               (JDL_OPRC_ADR_DUINFO_BASE + JDL_OPRC_OFS_UINFO_BASE)      /* BADR: 93965, CADR:     450 */
    
    	/* RC Denomi Info Offset */
        #define JDL_OPRC_OFS_RCINFO_DENOMI         0                                                         /* [OFS:  0] */
        #define JDL_OPRC_OFS_RCINFO_LENGH          (JDL_OPRC_OFS_RCINFO_DENOMI + JDL_DATA_TYPE_SIZE_WORD)    /* [OFS:  2] */
        #define JDL_OPRC_OFS_RCINFO_MAX_LEN        (JDL_OPRC_OFS_RCINFO_LENGH + JDL_DATA_TYPE_SIZE_BYTE)     /* [OFS:  3] */
        #define JDL_OPRC_OFS_RCINFO_MIN_LEN        (JDL_OPRC_OFS_RCINFO_MAX_LEN + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  4] */
        #define JDL_OPRC_OFS_RCINFO_MAX_BILL       (JDL_OPRC_OFS_RCINFO_MIN_LEN + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  5] */
        #define JDL_OPRC_OFS_RCINFO_BILL           (JDL_OPRC_OFS_RCINFO_MAX_BILL + JDL_DATA_TYPE_SIZE_BYTE)  /* [OFS:  6] */
        #define JDL_OPRC_OFS_RCINFO_CODE           (JDL_OPRC_OFS_RCINFO_BILL + JDL_DATA_TYPE_SIZE_BYTE)      /* [OFS:  7] */
        
        #define JDL_OPRC_OFS_RCINFO_BASE           (JDL_OPRC_OFS_RCINFO_CODE + JDL_DATA_TYPE_SIZE_BYTE)      /* [BASE: 8] */
        
        #define JDL_OPRC_SIZE_RCINFO               (JDL_OPRC_OFS_RCINFO_BASE * 2)                            /* [SIZE: 16] */
        
        /* RC Denomi Speed Info Offset */
        #define JDL_OPRC_OFS_RCSINFO_D1_STACK      0                                                           /* [OFS:   0] */
        #define JDL_OPRC_OFS_RCSINFO_D1_PAYOUT     (JDL_OPRC_OFS_RCSINFO_D1_STACK + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:   1] */
        #define JDL_OPRC_OFS_RCSINFO_D1_COLLECT    (JDL_OPRC_OFS_RCSINFO_D1_PAYOUT + JDL_DATA_TYPE_SIZE_BYTE)  /* [OFS:   2] */
        #define JDL_OPRC_OFS_RCSINFO_D2_STACK      (JDL_OPRC_OFS_RCSINFO_D1_COLLECT + JDL_DATA_TYPE_SIZE_BYTE) /* [OFS:   3] */
        #define JDL_OPRC_OFS_RCSINFO_D2_PAYOUT     (JDL_OPRC_OFS_RCSINFO_D2_STACK + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:   4] */
        #define JDL_OPRC_OFS_RCSINFO_D2_COLLECT    (JDL_OPRC_OFS_RCSINFO_D2_PAYOUT + JDL_DATA_TYPE_SIZE_BYTE)  /* [OFS:   5] */
        
        #define JDL_OPRC_OFS_RCSINFO_BASE          (JDL_OPRC_OFS_RCSINFO_D2_COLLECT + JDL_DATA_TYPE_SIZE_BYTE) /* [BASE:  6] */
        
        #define JDL_OPRC_OFS_RCINFO_RESERVE        (JDL_OPRC_SIZE_RCINFO + JDL_OPRC_OFS_RCSINFO_BASE)          /* [OFS:  22] */
        #define JDL_OPRC_OFS_RCINFO_CHECKSUM       (JDL_OPRC_OFS_RCINFO_RESERVE + JDL_SIZE_RC__RESERVE)        /* [OFS:  31] */
        #define JDL_OPRC_TOTAL_SIZE_RCINFO         (JDL_OPRC_OFS_RCINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)    /* [SIZE: 32] */
    
    /*----------------------------------------------------------*/
    /* RC-Twin Factory Drum Unit Info Address                   */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_FDUINFO_BASE              (JDL_OPRC_ADR_RCINFO_BASE + JDL_OPRC_TOTAL_SIZE_RCINFO)     /* BADR: 93997, CADR:     482 */
    
    /*----------------------------------------------------------*/
    /* RC-Twin Drum operation Info Address                      */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DOPEINFO_BASE             (JDL_OPRC_ADR_FDUINFO_BASE + JDL_OPRC_OFS_UINFO_BASE)       /* BADR: 94109, CADR:     594 */
    
    	/* Drum operation Info Offset */
        #define JDL_OPRC_OFS_DOPEINFO_FWD          0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_DOPEINFO_REV          (JDL_OPRC_OFS_DOPEINFO_FWD + JDL_DATA_TYPE_SIZE_DWORD)      /* [OFS:  4] */
        
        #define JDL_OPRC_OFS_DOPEINFO_BASE         (JDL_OPRC_OFS_DOPEINFO_REV + JDL_DATA_TYPE_SIZE_DWORD)      /* [BASE: 8] */
        
        #define JDL_OPRC_SIZE_DOPEINFO             (JDL_OPRC_OFS_DOPEINFO_BASE * 2)                            /* [SIZE: 16] */
        
        #define JDL_OPRC_OFS_DOPEINFO_RESERVE      (JDL_OPRC_SIZE_DOPEINFO)                                    /* [OFS:  16] */
        #define JDL_OPRC_OFS_DOPEINFO_CHECKSUM     (JDL_OPRC_OFS_DOPEINFO_RESERVE + JDL_SIZE_DOPE_RESERVE)     /* [OFS:  23] */
        #define JDL_OPRC_TOTAL_SIZE_DOPEINFO       (JDL_OPRC_OFS_DOPEINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)  /* [SIZE: 24] */
    
    /*----------------------------------------------------------*/
    /* RC-Twin Reject Info Address                              */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_REJINFO_BASE              (JDL_OPRC_ADR_DOPEINFO_BASE + JDL_OPRC_TOTAL_SIZE_DOPEINFO) /* BADR: 94133, CADR:     618 */
    
    	/* Drum operation Info Offset */
        #define JDL_OPRC_OFS_REJINFO_D1            0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_REJINFO_D2            (JDL_OPRC_OFS_REJINFO_D1 + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS:  4] */
        
        #define JDL_OPRC_OFS_REJINFO_BASE          (JDL_OPRC_OFS_REJINFO_D2 + JDL_DATA_TYPE_SIZE_DWORD)        /* [BASE: 8] */
        
        #define JDL_OPRC_SIZE_REJINFO_BASE         (JDL_OPRC_OFS_REJINFO_BASE * 1)                             /* [SIZE: 8] */
        
        #define JDL_OPRC_OFS_REJINFO_RESERVE       (JDL_OPRC_SIZE_REJINFO_BASE)                                /* [OFS:  8] */
        #define JDL_OPRC_OFS_REJINFO_CHECKSUM      (JDL_OPRC_OFS_REJINFO_RESERVE + JDL_SIZE_REJ_RESERVE)       /* [OFS:  15] */
        #define JDL_OPRC_TOTAL_SIZE_REJINFO        (JDL_OPRC_OFS_REJINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)   /* [SIZE: 16] */
    
    /*----------------------------------------------------------*/
    /* RC-Twin Drum Sensor Adj Info Address                     */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DSENS_ADJINFO_BASE        (JDL_OPRC_ADR_REJINFO_BASE + JDL_OPRC_TOTAL_SIZE_REJINFO)    /* BADR: 94149, CADR:     634 */
    
    	/* Drum operation Info Offset */
        #define JDL_OPRC_OFS_FAC_AD600DA           0                                                           /* [OFS:   0] */
        #define JDL_OPRC_OFS_FAC_AD800DA           (JDL_OPRC_OFS_FAC_AD600DA + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   1] */
        #define JDL_OPRC_OFS_FAC_DA                (JDL_OPRC_OFS_FAC_AD800DA + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   2] */
        #define JDL_OPRC_OFS_MEN_AD600DA           (JDL_OPRC_OFS_FAC_DA      + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   3] */
        #define JDL_OPRC_OFS_MEN_AD800DA           (JDL_OPRC_OFS_MEN_AD600DA + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   4] */
        #define JDL_OPRC_OFS_MEN_DA                (JDL_OPRC_OFS_MEN_AD800DA + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   5] */
        #define JDL_OPRC_OFS_CUR_AD600DA           (JDL_OPRC_OFS_MEN_DA      + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   6] */
        #define JDL_OPRC_OFS_CUR_AD800DA           (JDL_OPRC_OFS_CUR_AD600DA + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   7] */
        #define JDL_OPRC_OFS_CUR_DA                (JDL_OPRC_OFS_CUR_AD800DA + JDL_DATA_TYPE_SIZE_BYTE)        /* [OFS:   8] */
        
        #define JDL_OPRC_OFS_SENS_ADJ_BASE         (JDL_OPRC_OFS_CUR_DA + JDL_DATA_TYPE_SIZE_BYTE)             /* [BASE:  9] */
        
        #define JDL_OPRC_SIZE_DSENS_ADJINFO_BASE   (JDL_OPRC_OFS_SENS_ADJ_BASE * 2)                            /* [SIZE: 18] */
        
        #define JDL_OPRC_OFS_DSENS_ADJINFO_RESERVE (JDL_OPRC_SIZE_DSENS_ADJINFO_BASE)                          /* [OFS:  18] */
        #define JDL_OPRC_OFS_DSENS_ADJINFO_CHECKSUM (JDL_OPRC_OFS_DSENS_ADJINFO_RESERVE + JDL_SIZE_DSENS_RESERVE)  /* [OFS:  23] */
        #define JDL_OPRC_TOTAL_SIZE_DSENS_ADJINFO  (JDL_OPRC_OFS_DSENS_ADJINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE) /* [SIZE: 24] */
    
    /*----------------------------------------------------------*/
    /* RC-Twin Operation Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_OPEINFO_BASE              (JDL_OPRC_ADR_DSENS_ADJINFO_BASE + JDL_OPRC_TOTAL_SIZE_DSENS_ADJINFO)    /* BADR: 94173, CADR:     658 */
    
    	/* Drum operation Info Offset */
        #define JDL_OPRC_OFS_OPEINFO_STACK         0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_OPEINFO_PAYOUT        (JDL_OPRC_OFS_OPEINFO_STACK + JDL_DATA_TYPE_SIZE_DWORD)     /* [OFS:  4] */
        
        #define JDL_OPRC_OFS_OPEINFO_BASE          (JDL_OPRC_OFS_OPEINFO_PAYOUT + JDL_DATA_TYPE_SIZE_DWORD)    /* [BASE: 8] */
        
        #define JDL_OPRC_SIZE_OPEINFO_BASE         (JDL_OPRC_OFS_OPEINFO_BASE * 2)                             /* [SIZE: 16] */
        
        #define JDL_OPRC_OFS_OPEINFO_RESERVE       (JDL_OPRC_SIZE_OPEINFO_BASE)                                /* [OFS:  16] */
        #define JDL_OPRC_OFS_OPEINFO_CHECKSUM      (JDL_OPRC_OFS_OPEINFO_RESERVE + JDL_SIZE_OPE_RESERVE)       /* [OFS:  31] */
        #define JDL_OPRC_TOTAL_SIZE_OPEINFO        (JDL_OPRC_OFS_OPEINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)   /* [SIZE: 32] */
        
    /*----------------------------------------------------------*/
    /* RC-Twin Drum Error Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DERRINFO_BASE             (JDL_OPRC_ADR_OPEINFO_BASE + JDL_OPRC_TOTAL_SIZE_OPEINFO)   /* BADR: 94205, CADR:     690*/
    
    	/* Drum operation Info Offset */
        #define JDL_OPRC_OFS_DERRINFO_1            0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_DERRINFO_2            (JDL_OPRC_OFS_DERRINFO_1  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS:  2] */
        #define JDL_OPRC_OFS_DERRINFO_3            (JDL_OPRC_OFS_DERRINFO_2  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS:  4] */
        #define JDL_OPRC_OFS_DERRINFO_4            (JDL_OPRC_OFS_DERRINFO_3  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS:  6] */
        #define JDL_OPRC_OFS_DERRINFO_5            (JDL_OPRC_OFS_DERRINFO_4  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS:  8] */
        #define JDL_OPRC_OFS_DERRINFO_6            (JDL_OPRC_OFS_DERRINFO_5  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 10] */
        #define JDL_OPRC_OFS_DERRINFO_7            (JDL_OPRC_OFS_DERRINFO_6  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 12] */
        #define JDL_OPRC_OFS_DERRINFO_8            (JDL_OPRC_OFS_DERRINFO_7  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 14] */
        #define JDL_OPRC_OFS_DERRINFO_9            (JDL_OPRC_OFS_DERRINFO_8  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 16] */
        #define JDL_OPRC_OFS_DERRINFO_10           (JDL_OPRC_OFS_DERRINFO_9  + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 18] */
        #define JDL_OPRC_OFS_DERRINFO_11           (JDL_OPRC_OFS_DERRINFO_10 + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 20] */
        #define JDL_OPRC_OFS_DERRINFO_12           (JDL_OPRC_OFS_DERRINFO_11 + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 22] */
        #define JDL_OPRC_OFS_DERRINFO_13           (JDL_OPRC_OFS_DERRINFO_12 + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 24] */
        #define JDL_OPRC_OFS_DERRINFO_14           (JDL_OPRC_OFS_DERRINFO_13 + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 26] */
        #define JDL_OPRC_OFS_DERRINFO_15           (JDL_OPRC_OFS_DERRINFO_14 + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 28] */
        #define JDL_OPRC_OFS_DERRINFO_16           (JDL_OPRC_OFS_DERRINFO_15 + JDL_DATA_TYPE_SIZE_WORD)        /* [OFS: 30] */
        
        #define JDL_OPRC_OFS_DERRINFO_BASE         (JDL_OPRC_OFS_DERRINFO_16 + JDL_DATA_TYPE_SIZE_WORD)        /* [BASE:32] */
        
        #define JDL_OPRC_SIZE_DERRINFO_BASE        (JDL_OPRC_OFS_DERRINFO_BASE * 1)                            /* [SIZE: 32] */
        
        #define JDL_OPRC_OFS_DERRINFO_RESERVE      (JDL_OPRC_SIZE_DERRINFO_BASE)                               /* [OFS:  32] */
        #define JDL_OPRC_OFS_DERRINFO_CHECKSUM     (JDL_OPRC_OFS_DERRINFO_RESERVE + JDL_SIZE_DERR_RESERVE)     /* [OFS:  47] */
        #define JDL_OPRC_TOTAL_SIZE_DERRINFO       (JDL_OPRC_OFS_DERRINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)  /* [SIZE: 48] */
        
    /*----------------------------------------------------------*/
    /* RC-Quad Factory Main Unit Info Address                   */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_TFMUINFO_Q_BASE           (JDL_OPRC_ADR_DERRINFO_BASE + JDL_OPRC_TOTAL_SIZE_DERRINFO) /* BADR: 94253, CADR:     738 */
    /*----------------------------------------------------------*/
    /* RC-Quad Drum Unit Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DUINFO_Q_BASE             (JDL_OPRC_ADR_TFMUINFO_Q_BASE + JDL_OPRC_OFS_UINFO_BASE)    /* BADR: 94365, CADR:     850 */
     /*----------------------------------------------------------*/
    /* RC-Quad RC Denomi Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_RCINFO_Q_BASE             (JDL_OPRC_ADR_DUINFO_Q_BASE + JDL_OPRC_OFS_UINFO_BASE)      /* BADR: 94477, CADR:     962 */
     
    /*----------------------------------------------------------*/
    /* RC-Quad Factory Drum Unit Info Address                   */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_FDUINFO_Q_BASE            (JDL_OPRC_ADR_RCINFO_Q_BASE + JDL_OPRC_TOTAL_SIZE_RCINFO)   /* BADR: 94509, CADR:     994 */
    
    /*----------------------------------------------------------*/
    /* RC-Quad Drum operation Info Address                      */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DOPEINFO_Q_BASE           (JDL_OPRC_ADR_FDUINFO_Q_BASE + JDL_OPRC_OFS_UINFO_BASE)     /* BADR: 94621, CADR:     1106 */
    
    /*----------------------------------------------------------*/
    /* RC-Quad Reject Info Address                              */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_REJINFO_Q_BASE            (JDL_OPRC_ADR_DOPEINFO_Q_BASE + JDL_OPRC_TOTAL_SIZE_DOPEINFO) /* BADR: 94645, CADR:    1130 */
    
    /*----------------------------------------------------------*/
    /* RC-Quad Drum Sensor Adj Info Address                     */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DSENS_ADJINFO_Q_BASE      (JDL_OPRC_ADR_REJINFO_Q_BASE + JDL_OPRC_TOTAL_SIZE_REJINFO)   /* BADR: 94661, CADR:    1146 */
    
    /*----------------------------------------------------------*/
    /* RC-Quad Operation Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_OPEINFO_Q_BASE            (JDL_OPRC_ADR_DSENS_ADJINFO_Q_BASE + JDL_OPRC_TOTAL_SIZE_DSENS_ADJINFO)    /* BADR: 94685, CADR:    1170 */
        
    /*----------------------------------------------------------*/
    /* RC-Quad Drum Error Info Address                           */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DERRINFO_Q_BASE           (JDL_OPRC_ADR_OPEINFO_Q_BASE + JDL_OPRC_TOTAL_SIZE_OPEINFO) /* BADR: 94717, CADR:    1202*/
    
    /*----------------------------------------------------------*/
    /* Main Operation Info Address                              */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_OPEINFO_M_BASE            (JDL_OPRC_ADR_DERRINFO_Q_BASE + JDL_OPRC_TOTAL_SIZE_DERRINFO)    /* BADR: 94765, CADR:    1250*/
    
    	/* Drum operation Info Offset */
        #define JDL_OPRC_OFS_FOPEINFO_FWD          0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_FOPEINFO_REV          (JDL_OPRC_OFS_FOPEINFO_FWD + JDL_DATA_TYPE_SIZE_DWORD)      /* [OFS:  4] */
        
        #define JDL_OPRC_OFS_FOPEINFO_BASE         (JDL_OPRC_OFS_FOPEINFO_REV + JDL_DATA_TYPE_SIZE_DWORD)      /* [BASE: 8] */
        
        #define JDL_OPRC_SIZE_FOPEINFO             (JDL_OPRC_OFS_FOPEINFO_BASE * 2)                            /* [SIZE: 16] */
        
        #define JDL_OPRC_OFS_OPEINFO_FLAP1         (JDL_OPRC_SIZE_FOPEINFO)                                     /* [OFS: 16] */
        #define JDL_OPRC_OFS_OPEINFO_FLAP2         (JDL_OPRC_OFS_OPEINFO_FLAP1 + JDL_DATA_TYPE_SIZE_DWORD)      /* [OFS: 20] */
        #define JDL_OPRC_OFS_OPEINFO_SOL1          (JDL_OPRC_OFS_OPEINFO_FLAP2 + JDL_DATA_TYPE_SIZE_DWORD)      /* [OFS: 24] */
        #define JDL_OPRC_OFS_OPEINFO_SOL2          (JDL_OPRC_OFS_OPEINFO_SOL1  + JDL_DATA_TYPE_SIZE_DWORD)      /* [OFS: 28] */
        #define JDL_OPRC_OFS_OPEINFO_SOL3          (JDL_OPRC_OFS_OPEINFO_SOL2  + JDL_DATA_TYPE_SIZE_DWORD)      /* [OFS: 32] */
        
        #define JDL_OPRC_OFS_OPEINFO_M_RESERVE     (JDL_OPRC_OFS_OPEINFO_SOL3 + JDL_DATA_TYPE_SIZE_DWORD)       /* [OFS:  36] */
        #define JDL_OPRC_OFS_OPEINFO_M_CHECKSUM    (JDL_OPRC_OFS_OPEINFO_M_RESERVE + JDL_SIZE_MOPE_RESERVE)     /* [OFS:  63] */
        #define JDL_OPRC_TOTAL_SIZE_M_OPEINFO      (JDL_OPRC_OFS_OPEINFO_M_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)  /* [SIZE: 64] */
        
    /*----------------------------------------------------------*/
    /* Main Error Info Address                                  */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_ERRINFO_M_BASE            (JDL_OPRC_ADR_OPEINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_M_OPEINFO) /* BADR: 94829, CADR:    1314*/
    
    	/* Drum operation Info Offset */
        #define JDL_OPRC_OFS_MERRINFO_1            0                                                            /* [OFS:  0] */
        #define JDL_OPRC_OFS_MERRINFO_2            (JDL_OPRC_OFS_MERRINFO_1  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS:  4] */
        #define JDL_OPRC_OFS_MERRINFO_3            (JDL_OPRC_OFS_MERRINFO_2  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS:  8] */
        #define JDL_OPRC_OFS_MERRINFO_4            (JDL_OPRC_OFS_MERRINFO_3  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 12] */
        #define JDL_OPRC_OFS_MERRINFO_5            (JDL_OPRC_OFS_MERRINFO_4  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 16] */
        #define JDL_OPRC_OFS_MERRINFO_6            (JDL_OPRC_OFS_MERRINFO_5  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 20] */
        #define JDL_OPRC_OFS_MERRINFO_7            (JDL_OPRC_OFS_MERRINFO_6  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 24] */
        #define JDL_OPRC_OFS_MERRINFO_8            (JDL_OPRC_OFS_MERRINFO_7  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 28] */
        #define JDL_OPRC_OFS_MERRINFO_9            (JDL_OPRC_OFS_MERRINFO_8  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 32] */
        #define JDL_OPRC_OFS_MERRINFO_10           (JDL_OPRC_OFS_MERRINFO_9  + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 36] */
        #define JDL_OPRC_OFS_MERRINFO_11           (JDL_OPRC_OFS_MERRINFO_10 + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 40] */
        #define JDL_OPRC_OFS_MERRINFO_12           (JDL_OPRC_OFS_MERRINFO_11 + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 44] */
        #define JDL_OPRC_OFS_MERRINFO_13           (JDL_OPRC_OFS_MERRINFO_12 + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 48] */
        #define JDL_OPRC_OFS_MERRINFO_14           (JDL_OPRC_OFS_MERRINFO_13 + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 52] */
        #define JDL_OPRC_OFS_MERRINFO_15           (JDL_OPRC_OFS_MERRINFO_14 + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 56] */
        #define JDL_OPRC_OFS_MERRINFO_16           (JDL_OPRC_OFS_MERRINFO_15 + JDL_DATA_TYPE_SIZE_DWORD)        /* [OFS: 60] */
        
        #define JDL_OPRC_OFS_MERRINFO_BASE         (JDL_OPRC_OFS_MERRINFO_16 + JDL_DATA_TYPE_SIZE_DWORD)        /* [BASE:64] */
        
        #define JDL_OPRC_SIZE_ERRINFO_M_BASE       (JDL_OPRC_OFS_MERRINFO_BASE * 1)                             /* [SIZE: 64] */
        
        #define JDL_OPRC_OFS_ERRINFO_M_RESERVE     (JDL_OPRC_SIZE_ERRINFO_M_BASE)                               /* [OFS:  64] */
        #define JDL_OPRC_OFS_ERRINFO_M_CHECKSUM    (JDL_OPRC_OFS_ERRINFO_M_RESERVE + JDL_SIZE_MERR_RESERVE)     /* [OFS:  95] */
        #define JDL_OPRC_TOTAL_SIZE_M_ERRINFO      (JDL_OPRC_OFS_ERRINFO_M_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)  /* [SIZE: 96] */
        
    /*----------------------------------------------------------*/
    /* Main Twin Sensor Adj Info Address                        */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_SENS_ADJINFO_M_BASE       (JDL_OPRC_ADR_ERRINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_M_ERRINFO)    /* BADR: 94925, CADR:    1410 */
    
        /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_SENSOR_1              0                                                           /* [OFS:   0] */
        #define JDL_OPRC_OFS_SENSOR_2              (JDL_OPRC_OFS_SENSOR_1 + JDL_OPRC_OFS_SENS_ADJ_BASE)        /* [OFS:   9] */
        #define JDL_OPRC_OFS_SENSOR_3              (JDL_OPRC_OFS_SENSOR_2 + JDL_OPRC_OFS_SENS_ADJ_BASE)        /* [OFS:   18] */
        #define JDL_OPRC_OFS_SENSOR_A              (JDL_OPRC_OFS_SENSOR_3 + JDL_OPRC_OFS_SENS_ADJ_BASE)        /* [OFS:   27] */
        #define JDL_OPRC_OFS_SENSOR_B              (JDL_OPRC_OFS_SENSOR_A + JDL_OPRC_OFS_SENS_ADJ_BASE)        /* [OFS:   36] */
        #define JDL_OPRC_OFS_SENSOR_C              (JDL_OPRC_OFS_SENSOR_B + JDL_OPRC_OFS_SENS_ADJ_BASE)        /* [OFS:   45] */
        #define JDL_OPRC_OFS_SENSOR_EX             (JDL_OPRC_OFS_SENSOR_C + JDL_OPRC_OFS_SENS_ADJ_BASE)        /* [OFS:   54] */
        
        #define JDL_OPRC_OFS_SENS_ADJ_M_BASE       (JDL_OPRC_OFS_SENSOR_EX + JDL_OPRC_OFS_SENS_ADJ_BASE)       /* [BASE:  63] */
        
        #define JDL_OPRC_SIZE_SENS_ADJINFO_M_BASE  (JDL_OPRC_OFS_SENS_ADJ_M_BASE * 1)                          /* [SIZE:  63] */
        
        #define JDL_OPRC_OFS_SENS_ADJINFO_M_RESERVE   (JDL_OPRC_SIZE_SENS_ADJINFO_M_BASE)                      /* [OFS:  63] */
        #define JDL_OPRC_OFS_SENS_ADJINFO_M_CHECKSUM  (JDL_OPRC_OFS_SENS_ADJINFO_M_RESERVE + JDL_SIZE_MSENS_RESERVE)     /* [OFS:  63] */
        #define JDL_OPRC_TOTAL_SIZE_M_SENS_ADJINFO    (JDL_OPRC_OFS_SENS_ADJINFO_M_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)   /* [SIZE: 64] */
        
    /*----------------------------------------------------------*/
    /* Main Quad Sensor Adj Info Address                        */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_SENS_ADJ2INFO_M_BASE      (JDL_OPRC_ADR_SENS_ADJINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_M_SENS_ADJINFO)    /* BADR: 94989, CADR:    1474 */
    
    /*----------------------------------------------------------*/
    /* Main Download Info Address                               */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_DLINFO_M_BASE             (JDL_OPRC_ADR_SENS_ADJ2INFO_M_BASE + JDL_OPRC_TOTAL_SIZE_M_SENS_ADJINFO)    /* BADR: 95053, CADR:    1538 */
    
        /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_DLINFO                0                                                           /* [OFS:  0] */
        
        #define JDL_OPRC_OFS_DLINFO_BASE           (JDL_OPRC_OFS_DLINFO + JDL_DATA_TYPE_SIZE_BYTE)             /* [BASE: 1] */
        
        #define JDL_OPRC_SIZE_DLINFO_BASE          (JDL_OPRC_OFS_DLINFO_BASE * 1)                              /* [SIZE: 1] */
        
        #define JDL_OPRC_OFS_DLINFO_RESERVE        (JDL_OPRC_SIZE_DLINFO_BASE)                                 /* [OFS:  1] */
        #define JDL_OPRC_OFS_DLINFO_CHECKSUM       (JDL_OPRC_OFS_DLINFO_RESERVE + JDL_SIZE_DL_RESERVE)         /* [OFS: 15] */
        #define JDL_OPRC_TOTAL_SIZE_DLINFO         (JDL_OPRC_OFS_DLINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)    /* [SIZE:16] */
    
    /*----------------------------------------------------------*/
    /* Main Motor Speed Info Address                            */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_MTRSPINFO_M_BASE          (JDL_OPRC_ADR_DLINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_DLINFO)   /* BADR: 95069, CADR:    1554 */
    
        /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_MTRSPINFO_RCFSTK      0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_MTRSPINFO_RCFPAY      (JDL_OPRC_OFS_MTRSPINFO_RCFSTK + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  1] */
        #define JDL_OPRC_OFS_MTRSPINFO_RCFCOL      (JDL_OPRC_OFS_MTRSPINFO_RCFPAY + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  2] */
        #define JDL_OPRC_OFS_MTRSPINFO_RCRSTK      (JDL_OPRC_OFS_MTRSPINFO_RCFCOL + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  3] */
        #define JDL_OPRC_OFS_MTRSPINFO_RCRPAY      (JDL_OPRC_OFS_MTRSPINFO_RCRSTK + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  4] */
        #define JDL_OPRC_OFS_MTRSPINFO_RCRCOL      (JDL_OPRC_OFS_MTRSPINFO_RCRPAY + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  5] */
        
        #define JDL_OPRC_OFS_MTRSPINFO_BOXSTK      (JDL_OPRC_OFS_MTRSPINFO_RCRCOL + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  6] */
        #define JDL_OPRC_OFS_MTRSPINFO_BOXPAY      (JDL_OPRC_OFS_MTRSPINFO_BOXSTK + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  7] */
        #define JDL_OPRC_OFS_MTRSPINFO_BOXCOL      (JDL_OPRC_OFS_MTRSPINFO_BOXPAY + JDL_DATA_TYPE_SIZE_BYTE)   /* [OFS:  8] */
        
        #define JDL_OPRC_OFS_MTRSPINFO_BASE        (JDL_OPRC_OFS_MTRSPINFO_BOXCOL + JDL_DATA_TYPE_SIZE_BYTE)   /* [BASE: 9] */
        
        #define JDL_OPRC_SIZE_MTRSPINFO_BASE       (JDL_OPRC_OFS_MTRSPINFO_BASE * 1)                           /* [SIZE: 9] */
        
        #define JDL_OPRC_OFS_MTRSPINFO_RESERVE     (JDL_OPRC_SIZE_MTRSPINFO_BASE)                              /* [OFS:  9] */
        #define JDL_OPRC_OFS_MTRSPINFO_CHECKSUM    (JDL_OPRC_OFS_MTRSPINFO_RESERVE + JDL_SIZE_MTRSP_RESERVE)   /* [OFS: 31] */
        #define JDL_OPRC_TOTAL_SIZE_MTRSPINFO      (JDL_OPRC_OFS_MTRSPINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE) /* [SIZE:32] */
    
    /*----------------------------------------------------------*/
    /* Main Last Operation Info Address                         */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_LOPEINFO_M_BASE           (JDL_OPRC_ADR_MTRSPINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_MTRSPINFO)   /* BADR: 95101, CADR:    1586 */
    
        /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_LOPEINFO_OPE          0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_LOPEINFO_PLACE        (JDL_OPRC_OFS_LOPEINFO_OPE + JDL_DATA_TYPE_SIZE_BYTE)       /* [OFS:  1] */
        #define JDL_OPRC_OFS_LOPEINFO_PROG         (JDL_OPRC_OFS_LOPEINFO_PLACE + JDL_DATA_TYPE_SIZE_BYTE)     /* [OFS:  2] */
        
        #define JDL_OPRC_OFS_LOPEINFO_BASE         (JDL_OPRC_OFS_LOPEINFO_PROG + JDL_DATA_TYPE_SIZE_BYTE)      /* [BASE: 3] */
        
        #define JDL_OPRC_SIZE_LOPEINFO_BASE        (JDL_OPRC_OFS_LOPEINFO_BASE * 1)                            /* [SIZE: 3] */
        
        #define JDL_OPRC_OFS_LOPEINFO_RESERVE      (JDL_OPRC_SIZE_LOPEINFO_BASE)                               /* [OFS:  3] */
        #define JDL_OPRC_OFS_LOPEINFO_CHECKSUM     (JDL_OPRC_OFS_LOPEINFO_RESERVE + JDL_SIZE_LOPE_RESERVE)     /* [OFS: 23] */
        #define JDL_OPRC_TOTAL_SIZE_LOPEINFO       (JDL_OPRC_OFS_LOPEINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)  /* [SIZE:24] */
        
    /*----------------------------------------------------------*/
    /* Main Current Motor Speed Info Address                    */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_CMTRSPINFO_M_BASE         (JDL_OPRC_ADR_LOPEINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_LOPEINFO)   /* BADR: 95125, CADR:    1610 */
    
       /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_CMTRSPINFO_HEAD       0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_CMTRSPINFO_TWIN       (JDL_OPRC_OFS_CMTRSPINFO_HEAD + JDL_DATA_TYPE_SIZE_WORD)    /* [OFS:  2] */
        #define JDL_OPRC_OFS_CMTRSPINFO_QUAD       (JDL_OPRC_OFS_CMTRSPINFO_TWIN + JDL_DATA_TYPE_SIZE_WORD)    /* [OFS:  4] */
        #define JDL_OPRC_OFS_CMTRSPINFO_DRUM       (JDL_OPRC_OFS_CMTRSPINFO_QUAD + JDL_DATA_TYPE_SIZE_WORD)    /* [OFS:  6] */
        
        #define JDL_OPRC_OFS_CMTRSPINFO_BASE       (JDL_OPRC_OFS_CMTRSPINFO_DRUM + JDL_DATA_TYPE_SIZE_WORD)    /* [BASE: 8] */
        
        #define JDL_OPRC_SIZE_CMTRSPINFO_BASE      (JDL_OPRC_OFS_CMTRSPINFO_BASE * 1)                          /* [SIZE: 8] */
        
        #define JDL_OPRC_OFS_CMTRSPINFO_RESERVE    (JDL_OPRC_SIZE_CMTRSPINFO_BASE)                             /* [OFS:  8] */
        #define JDL_OPRC_OFS_CMTRSPINFO_CHECKSUM   (JDL_OPRC_OFS_CMTRSPINFO_RESERVE + JDL_SIZE_CMTRSP_RESERVE) /* [OFS: 31] */
        #define JDL_OPRC_TOTAL_SIZE_CMTRSPINFO     (JDL_OPRC_OFS_CMTRSPINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)/* [SIZE:32] */
        
    /*----------------------------------------------------------*/
    /* Main bill sub Info Address                               */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_BSUBINFO_M_BASE           (JDL_OPRC_ADR_CMTRSPINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_CMTRSPINFO)    /* BADR: 95157, CADR:    1642 */
    
        /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_BSUBINFO              0                                                           /* [OFS:  0] */
        
        #define JDL_OPRC_OFS_BSUBINFO_BASE         (JDL_OPRC_OFS_BSUBINFO + JDL_DATA_TYPE_SIZE_BYTE)           /* [BASE: 1] */
        
        #define JDL_OPRC_SIZE_BSUBINFO_BASE        (JDL_OPRC_OFS_BSUBINFO_BASE * 1)                            /* [SIZE: 1] */
        
        #define JDL_OPRC_OFS_BSUBINFO_RESERVE      (JDL_OPRC_SIZE_BSUBINFO_BASE)                               /* [OFS:  1] */
        #define JDL_OPRC_OFS_BSUBINFO_CHECKSUM     (JDL_OPRC_OFS_BSUBINFO_RESERVE + JDL_SIZE_BSUB_RESERVE)     /* [OFS: 14] */
        #define JDL_OPRC_TOTAL_SIZE_BSUBINFO       (JDL_OPRC_OFS_BSUBINFO_CHECKSUM + JDL_DATA_TYPE_SIZE_BYTE)  /* [SIZE:16] */
        
    /*----------------------------------------------------------*/
    /* Main Reserve Address                                     */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_RESERVE_M_BASE            (JDL_OPRC_ADR_BSUBINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_BSUBINFO)    /* BADR: 95173, CADR:    1658 */
    
        /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_RESERVE               0                                                           /* [OFS:  0] */
        
        #define JDL_OPRC_OFS_RESERVE_BASE          (JDL_OPRC_OFS_RESERVE + JDL_SIZE_OPRC_RESERVE)              /* [BASE: 14342] */
        
        #define JDL_OPRC_SIZE_RESERVEO_BASE        (JDL_OPRC_OFS_RESERVE_BASE * 1)                             /* [SIZE: 14342] */

        #define JDL_OPRC_TOTAL_SIZE_RESERVE        (JDL_OPRC_SIZE_RESERVEO_BASE)                               /* [SIZE: 14342] */
    
    /*----------------------------------------------------------*/
    /* Main Operation Log Address                               */
    /*----------------------------------------------------------*/
#if defined (_JDL_OPRC_EXCLUDE_YOBI)
	#define JDL_OPRC_ADR_OLOGINFO_M_BASE           (JDL_OPRC_ADR_BSUBINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_BSUBINFO)    /* BADR: 95173, CADR:    1658 */
#else
    #define JDL_OPRC_ADR_OLOGINFO_M_BASE           (JDL_OPRC_ADR_RESERVE_M_BASE + JDL_OPRC_TOTAL_SIZE_RESERVE)    /* BADR:109515, CADR:   16000 */
#endif
    
        /* Drum operation Info Offset */
        #define JDL_OPRC_OFS_OLOGINFO_IDEX         0                                                           /* [OFS:  0] */
        #define JDL_OPRC_OFS_OLOGINFO_DATA         (JDL_OPRC_OFS_OLOGINFO_IDEX + JDL_DATA_TYPE_SIZE_WORD)      /* [OFS:  2] */
        
        #define JDL_OPRC_SIZE_OLOGINFO_DATA        (JDL_SIZE_OLOG_DATA * 1500)                                 /* [SIZE: 48000] */
        
        #define JDL_OPRC_OFS_OLOGINFO_BASE         (JDL_OPRC_OFS_OLOGINFO_DATA + JDL_OPRC_SIZE_OLOGINFO_DATA)  /* [BASE: 48002] */
        
        #define JDL_OPRC_SIZE_OLOGINFO_BASE        (JDL_OPRC_OFS_OLOGINFO_BASE * 1)                          /* [SIZE: 48002] */
        
        #define JDL_OPRC_TOTAL_SIZE_OLOGINFO       (JDL_OPRC_SIZE_OLOGINFO_BASE)                               /* [SIZE: 48002] */
    
    /*----------------------------------------------------------*/
    /* Address for the reserved                                 */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_FOR_RESERVED  (JDL_OPRC_ADR_OLOGINFO_M_BASE + JDL_OPRC_TOTAL_SIZE_OLOGINFO)           /* [Address for the reserved] BADR: 157517, CADR: 64002 */
    
    
    /*----------------------------------------------------------*/
    /* Address for the next category                            */
    /*----------------------------------------------------------*/
    #define JDL_OPRC_ADR_FOR_NEXT_CATEGORY (JDL_OPRC_ADR_FOR_RESERVED + 0)                 /* [Address for the next category] BADR: 157517, CADR:   64002  (Reserved area = 0) */
    
    
    /*----------------------------------------------------------*/
    /* Additional OPRC total size                                 */
    /*----------------------------------------------------------*/
    /* Total size of Additional RC Category data to send */
    #define JDL_OPRC_SEND_TOTAL (JDL_OPRC_ADR_FOR_RESERVED - JDL_OPRC_ADR_SEND_BASE)      /* [Additional RC Send Data Total:64000] Without Reserved and Revision */
    /* Total size of Additional RC Category buffer without reserved area */
    #define JDL_OPRC_BUFF_TOTAL (JDL_OPRC_ADR_FOR_NEXT_CATEGORY - JDL_OPRC_ADR_BUFF_BASE) /* [Additional RC Buffer Total:64002] 157517 - 93515(Next Addr - Base Addr) */
    
#endif  /* _JDL_USE_OP_RC */
#endif  /* UBA_RC */



/*==========================================================================*/
/*==========================================================================*/
/* JDL Common                                                               */
/*    - JDL Buffer total size                                               */
/*==========================================================================*/
// OPRCはSRAMに入れるないため１つ前にカテゴリーのADRとする
#if defined(UBA_RTQ)
#define JDL_BUFF_TOTAL  JDL_RC_ADR_FOR_NEXT_CATEGORY  /* [Buffer Total Size] */
#else
#define JDL_BUFF_TOTAL  JDL_PANA_ADR_FOR_NEXT_CATEGORY  /* [Buffer Total Size] */
#endif // UBA_RTQ


/*==========================================================================*/
/*==========================================================================*/
/* JDL functions                                                            */
/*==========================================================================*/
extern void jdl_init(u8 clear);
extern void jdl_set_jdl_time(u8 *time);
extern void jdl_powerup(void);
extern void jdl_dev_reset(void);
extern void jdl_move_entry(u16 run_time);
extern void jdl_move_feed(u16 run_time);
extern void jdl_move_stack(u16 run_time);
extern void jdl_move_centering(u16 run_time);
extern void jdl_move_apb(u16 run_time);
extern void jdl_write_eep(void);
extern void jdl_ener_time(void);


extern void jdl_insert(void);
extern void jdl_accept(u16 denomi);
extern void jdl_reject(u16 rej_code, u8 vali_sta, u16 denomi, u32 seq, u32 m_mode1, u32 m_mode2, u32 sens);
extern void jdl_error(u32 err_code, u32 seq, u32 m_mode1, u32 m_mode2, u32 sens);

extern void jdl_add_trace(u8 tid, u8 mode1, u8 mode2, u8 val1, u8 val2, u8 val3);

extern void jdl_comm_init(u8 pid, u8 sst_skip, u8 sst_cmd);
extern void jdl_comm_rx_pkt(u8 *data, u8 size);
extern void jdl_comm_tx_pkt(u8 *data, u8 size);

extern void jdl_sens_update_cor_val(void);

extern void jdl_posiana_update(void);

extern void jdl_rc_set_version(void);
extern void jdl_rc_set_speed(void);
extern void jdl_rc_set_rc_setting(void);
extern void jdl_rc_stack(u16 unit_no, u16 drum_no, u16 denomi, u16 bill_len);
extern void jdl_rc_payout(u16 unit_no, u16 drum_no, u8 pre_feed);
extern void jdl_rc_collect(u16 unit_no, u16 drum_no, u8 pre_feed);

extern u8 jdl_get_sta(u32 *total);
extern u8 jdl_req_data(u32 offset, u32 buff_size);
extern u8 jdl_get_data(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);
extern u8 jdl_get_end(void);
extern u8 jdl_category_clear(u16 cate_no);


extern u8 _jdl_load(u32 offset, u32 size, u8 *data);
extern u8 _jdl_load_word(u32 offset, u16 *data);
extern u8 _jdl_load_dword(u32 offset, u32 *data);

extern u8 _jdl_save(u32 offset, u32 size, u8 *data);
extern u8 _jdl_save_byte(u32 offset, u8 data);
extern u8 _jdl_save_word(u32 offset, u16 data);
extern u8 _jdl_save_dword(u32 offset, u32 data);

extern u8 _jdl_clear(u32 offset, u32 size);

extern u8 _jdl_copy(u32 d_offset, u32 s_offset, u32 size);

extern u8 _jdl_calc_checksum(u32 sta_adr, u32 end_adr, u16 *sum);
extern u8 _jdl_renew_checksum(u32 sta_adr, u32 sum_adr);

extern u8 _jdl_save_data_checksum(u32 offset, u8 *data, u8 type, u32 sta_adr, u32 sum_adr);
extern u8 _jdl_save_data_2_checksum(u32 offset1, u8 *inc_data1, u8 type1, u32 offset2, u8 *inc_data2, u8 type2, u32 sta_adr, u32 sum_adr);

#ifdef _DEBUG_JDL
extern void jdl_debug(void);
#endif /* _DEBUG_JDL */
















