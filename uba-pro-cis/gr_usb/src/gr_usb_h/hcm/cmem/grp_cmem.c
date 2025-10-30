/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2013 Grape Systems, Inc.                            */
/*                                     All Rights Reserved.                                     */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      grp_cmem.c                                                              1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      This file performs memory management processing.                                        */
/*      (for flat memory model (boundary support))                                              */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   M.Suzuki       2013/01/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_cmem.h"

#include "grp_usr_mem_tbl.h"
#include "grp_sys_mem_tbl.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
/* Index value of memory area */
#define GRP_CMEM_AREA_NUM               1                       /* Index number of memory area  */
                                                                /* Set up one or more values.   */
                                                                /* GR_USB uses one.             */
/* Check allocate area */
#define GRP_CMEM_NO_CREAT               0                       /* No Create                    */
#define GRP_CMEM_CREAT                  1                       /* Formatted                    */

/* Category of ID */
#define GRP_CMEM_BASE_ID                1                       /* The base ID                  */
#define GRP_CMEM_NEXT_ID                1                       /* Next ID number               */
#define GRP_CMEM_IDMASK                 ((grp_s32)0x80000000)   /* Mask of ID for GR_USB        */

/* Area size is range of memory pool */
#define GRP_CMEM_RANGE_SIZE             (sizeof(grp_cmem_range) * GRP_CMEM_AREA_NUM)

/* number is tatal memory block */
#define GRP_CMEM_BNUM_TOTAL             (GRP_CMEM_BNUM_ALL + GRP_CMEM_BNUM_GU_ALL)

/* The number of memory partition control array */
#define GRP_CMEM_MEMTBL_CNT             2

/************************************************************************************************/
/* typedef struct                                                                               */
/* Memory Patition Control */
typedef struct {
    void                    *pvNextMBC;         /* next MBC                                     */
    void                    *pvStartAd;         /* start address of memory partition            */
    void                    *pvEndAd;           /* end address of memory partition              */
    void                    *pvStartMBC;        /* start MBC                                    */
    grp_u32                 ulBlkCnt;           /* count of memory block in a memory partition  */
    grp_u32                 ulBlkSz;            /* size of memory block in a memory partition   */

} grp_cmem_mpc;

/* Memory Block Control */
typedef struct
{
    void                    *pvNextMBC;         /* next MBC                                     */
    void                    *pvBlk;             /* pointer of memory block                      */

} grp_cmem_mbc;

/* Free Memory Pool */
typedef struct
{
    void                    *pIdxFreeAd;        /* index of free area address                   */
    grp_u32                 ulFreeSize;         /* size of free area                            */

} grp_cmem_fmp;

/************************************************************************************************/
/* Macro                                                                                        */
#define GRP_CMEM_IDOFT(x)               (((x) ^ GRP_CMEM_IDMASK) >> 31)

/************************************************************************************************/
/* extern variable                                                                              */
EXTERN grp_cmem_pf          l_atCmemPfGu[];
EXTERN grp_cmem_pf          l_atCmemPf[];

/************************************************************************************************/
/* Static Variable                                                                              */
/* Memory pool (256k) */
grp_u32                     l_ulMemArea[GRP_CMEM_MEMAREA_SIZE/sizeof(grp_u32)] __attribute__ ((section ("TMP_NONCACHE"), aligned (4)));

/* Memory pool area */
DLOCAL grp_cmem_range       l_atCmemRange[GRP_CMEM_AREA_NUM]  = {
  /* Start address          Memory area address */
  { (void*)&l_ulMemArea[0], GRP_CMEM_MEMAREA_SIZE}
};

/* creation flag */
DLOCAL grp_s32              l_ulCreatFlag = GRP_CMEM_NO_CREAT;

/* the value of maximum ID */
DLOCAL grp_s32              l_alIdMax[GRP_CMEM_MEMTBL_CNT] = {
    GRP_CMEM_ID_GU_MAX,   GRP_CMEM_ID_MAX
};

/* the List of porting information.  */
DLOCAL grp_cmem_pf          *l_paCmemPF[GRP_CMEM_MEMTBL_CNT] = {
    /* use GR-USB/HOST#    use application */
    l_atCmemPfGu,           l_atCmemPf
};

/* Free Memory Pool */
DLOCAL grp_cmem_fmp         l_atCmemFMP[GRP_CMEM_AREA_NUM];

/* memory block control */
DLOCAL grp_cmem_mbc         l_pCmemMBC[GRP_CMEM_BNUM_TOTAL];
DLOCAL grp_cmem_mbc         *l_ptCmemMBC = l_pCmemMBC;

/* memory partitoin control of user  */
DLOCAL grp_cmem_mpc         l_tCmemMPC_Us[GRP_CMEM_ID_MAX];
DLOCAL grp_cmem_mpc         *l_ptCmemMPC_Us = l_tCmemMPC_Us;

/* memory partitoin control of GR-USB/HOST# */
DLOCAL grp_cmem_mpc         l_tCmemMPC_Gu[GRP_CMEM_ID_GU_MAX];
DLOCAL grp_cmem_mpc         *l_ptCmemMPC_Gu = l_tCmemMPC_Gu;

/* the list of MPC */
DLOCAL grp_cmem_mpc*        l_patCmemMPC[GRP_CMEM_MEMTBL_CNT] = {
    l_tCmemMPC_Gu, l_tCmemMPC_Us
};

/* Semaphore */
DLOCAL grp_vos_t_semaphore  *l_ptCemSemaphore;


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_s32   _grp_cmem_InitMemoryBlockControl(grp_cmem_pf*,grp_cmem_mbc*,grp_cmem_fmp*);
LOCAL grp_s32   _grp_cmem_InitPartitonBlockControl(grp_cmem_fmp*,grp_cmem_pf*,grp_cmem_mpc*,grp_cmem_mbc**);
LOCAL grp_s32   _grp_cmem_ChangePartitionStart(grp_cmem_pf*,grp_cmem_fmp*);
LOCAL grp_s32   _grp_cmem_AdjustBoundaryPartitionStart(grp_cmem_pf*,grp_cmem_fmp*);
LOCAL grp_s32   _grp_cmem_InitAllControlObject(grp_cmem_fmp*,grp_cmem_mpc**,grp_cmem_mbc*);
LOCAL void      _grp_cmem_InitFMP(grp_cmem_fmp*);
LOCAL grp_s32   _grp_cmem_AllocateMemoryPool(grp_cmem_range*,grp_u32);


/************************************************************************************************/
/* FUNCTION   : _grp_cmem_InitMemoryBlockControl                                                */
/*                                                                                              */
/* DESCRIPTION: Initialize memory block control                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPF                            porting file                                    */
/*              ptMBC                           memory block control                            */
/*              ptFMP                           free memory pool                                */
/* OUTPUT     : ptMBC                           memory block control                            */
/*              ptFMP                           free memory pool                                */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_INIT                Internal infomation generation goes wrong       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cmem_InitMemoryBlockControl( grp_cmem_pf *ptPF, grp_cmem_mbc *ptMBC, grp_cmem_fmp *ptFMP)
{
grp_s32                         lStatus = GRP_CMEM_OK;
grp_s32                         lLp;
#if (GRP_CMEM_BND_ENABLE == GRP_USB_TRUE)
grp_s32                         lNowBndOft;         /* Offset of current boundary               */
grp_s32                         lNowBndRst;         /* Rest is empty area of current boundary   */
#endif  /* (GRP_CMEM_BND_ENABLE == GRP_USB_TRUE) */

    for (lLp=0; lLp<ptPF->ulBlkCnt; lLp++, ptMBC++) {
#if (GRP_CMEM_BND_ENABLE == GRP_USB_TRUE)
        /* Specification size cannot be taken to a memory boundary */
        lNowBndOft  = (grp_s32)ptFMP->pIdxFreeAd % GRP_CMEM_BND_VALUE;
        lNowBndRst  = GRP_CMEM_BND_VALUE - lNowBndOft;
        if (lNowBndRst < ptPF->ulBlkSz) {
            lStatus = GRP_CMEM_ER_INIT;
            break;
        }
#endif  /* (GRP_CMEM_BND_ENABLE == GRP_USB_TRUE) */

        /* Specification of large size then a memory boundary */
        if (ptFMP->ulFreeSize < ptPF->ulBlkSz) {
            lStatus = GRP_CMEM_ER_INIT;
            break;
        }

        /* Set memory block control */
        ptMBC->pvBlk = ptFMP->pIdxFreeAd;

        if ((lLp + GRP_CMEM_NEXT_ID) == ptPF->ulBlkCnt) {
            ptMBC->pvNextMBC = GRP_USB_NULL;
        }
        else {
            ptMBC->pvNextMBC = ptMBC + GRP_CMEM_NEXT_ID;
        }

        /* Renewal free memory pool information */
        ptFMP->pIdxFreeAd  = (void *)((grp_u32)ptFMP->pIdxFreeAd + ptPF->ulBlkSz);
        ptFMP->ulFreeSize -= ptPF->ulBlkSz;
    }

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cmem_InitPartitonBlockControl                                              */
/*                                                                                              */
/* DESCRIPTION: Initialze partition block control object                                        */
/*              (The start address don't overpass the address is optionary boundary.)           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFMP                           free memory pool                                */
/*              grp_cmem_pf                     porting file                                    */
/*              ptMPC                           memory partition control area                   */
/*              pptMBC                          memory block control area                       */
/* OUTPUT     : ptFMP                           free memory pool(rest)                          */
/*              pptMBC                          memory block control area                       */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_INIT                Internal infomation generation goes wrong       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cmem_InitPartitonBlockControl( grp_cmem_fmp *ptFMP, grp_cmem_pf *ptPF, grp_cmem_mpc *ptMPC, grp_cmem_mbc **pptMBC)
{
grp_s32                         lStatus = GRP_CMEM_OK;
grp_cmem_mbc                    *ptMBC  = *pptMBC;

    if (ptPF->ulBlkCnt == 0) {
        /* Set memory partition control */
        ptMPC->pvNextMBC    = GRP_USB_NULL;
        ptMPC->pvStartMBC   = GRP_USB_NULL;
        ptMPC->pvStartAd    = GRP_USB_NULL;
        ptMPC->pvEndAd      = GRP_USB_NULL;
        ptMPC->ulBlkCnt     = ptPF->ulBlkCnt;
        ptMPC->ulBlkSz      = ptPF->ulBlkSz;
    }
    else {
        /* Set memory partition control(1) */
        ptMPC->pvNextMBC    = ptMBC;
        ptMPC->pvStartMBC   = ptMBC;
        ptMPC->pvStartAd    = ptFMP->pIdxFreeAd;
        ptMPC->ulBlkCnt     = ptPF->ulBlkCnt;
        ptMPC->ulBlkSz      = ptPF->ulBlkSz;
        
        /* Quota processing of memory block */
        lStatus = _grp_cmem_InitMemoryBlockControl( ptPF, ptMBC, ptFMP);

        /* Set memory partition control(2) */
        ptMPC->pvEndAd = ptFMP->pIdxFreeAd;

        /* Change free memory block control address. */
        *pptMBC = ptMBC + ptPF->ulBlkCnt;
    }

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cmem_ChangePartitionStart                                                  */
/*                                                                                              */
/* DESCRIPTION: Change partition start address.                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPF                            porting file information                        */
/* OUTPUT     : ptFMP                           information of free memory pool                 */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_INIT                Internal infomation generation goes wrong       */
/*                                                                                              */
/************************************************************************************************/
#if (GRP_CMEM_BND_ENABLE == GRP_USB_TRUE)
LOCAL grp_s32 _grp_cmem_ChangePartitionStart( grp_cmem_pf *ptPF, grp_cmem_fmp *ptFMP)
{
grp_s32                     lStatus = GRP_CMEM_ER_VAL;
grp_s32                     lNowBndOft;             /* Offset of current boundary               */
grp_s32                     lNowBndRst;             /* Rest is empty area of current boundary   */

    /* Specification of large size then a memory boundary */
    if (ptPF->ulBlkSz >= GRP_CMEM_BND_VALUE) {
        return GRP_CMEM_ER_VAL;
    }

    /* Calculation of the availability to a block boundary */
    lNowBndOft  = (grp_s32)ptFMP->pIdxFreeAd % GRP_CMEM_BND_VALUE;
    lNowBndRst  = GRP_CMEM_BND_VALUE - lNowBndOft;

    /* If can get one block size for lNowBndRst area  */
    if (lNowBndRst >= ptPF->ulBlkSz) {
        lStatus = GRP_CMEM_OK;
    }
    else {
        /* If all empty size minmun than lNowBndRst */
        if (ptFMP->ulFreeSize <= lNowBndRst) {
            lStatus = GRP_CMEM_ER_VAL;
        }
        else {
            /* Renewal of memory block */
            ptFMP->pIdxFreeAd  = (void *)((grp_s32)ptFMP->pIdxFreeAd + lNowBndRst );
            ptFMP->ulFreeSize -= lNowBndRst;

            lStatus = GRP_CMEM_OK;
        }
    }

    return lStatus;
}
#endif /*(GRP_CMEM_BND_ENABLE == GRP_USB_TRUE)*/

/************************************************************************************************/
/* FUNCTION   : _grp_cmem_AdjustBoundaryPartitionStart                                          */
/*                                                                                              */
/* DESCRIPTION: Change partition start address if it's not adjust.                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPF                            porting file information                        */
/* OUTPUT     : ptFMP                           information of free memory pool                 */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_INIT                Internal infomation generation goes wrong       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cmem_AdjustBoundaryPartitionStart( grp_cmem_pf *ptPF, grp_cmem_fmp *ptFMP)
{
grp_s32                         lStatus = GRP_CMEM_ER_VAL;
grp_s32                         lNowBndOft;        /* Offset of current boundary                */
grp_s32                         lNowBndRst;        /* Rest is empty area of current boundary    */

    if (ptPF->ulBnd == 0) {
        return GRP_CMEM_OK;
    }

    /* Calculate surplus variable */
    lNowBndOft = (grp_s32)ptFMP->pIdxFreeAd % ptPF->ulBnd;

    /* No change start address(adjust boundary) */
    if (lNowBndOft == 0) {
        return GRP_CMEM_OK;
    }

    /* Calculate a new start address. */
    lNowBndRst = ptPF->ulBnd - lNowBndOft;

    /* If the all of empty size minmun than lNowBndRst */
    if (ptFMP->ulFreeSize <= lNowBndRst) {
        /* Cannot get all partition area. */
        lStatus = GRP_CMEM_ER_VAL;
    }
    else {
        /* Renewal free memory pool information */
        ptFMP->pIdxFreeAd  = (void *)((grp_s32)ptFMP->pIdxFreeAd + lNowBndRst);
        ptFMP->ulFreeSize -= lNowBndRst;
        
        lStatus = GRP_CMEM_OK;
    }

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cmem_InitAllControlObject                                                  */
/*                                                                                              */
/* DESCRIPTION: Initialize all control object.                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFMP                           array of free memory pool                       */
/* OUTPUT     : patMPC[]                        list of memory patition control                 */
/*              ptMBC                           start of memory block control                   */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_INIT                Internal infomation generation goes wrong       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cmem_InitAllControlObject( grp_cmem_fmp *ptFMP, grp_cmem_mpc *patMPC[GRP_CMEM_MEMTBL_CNT], grp_cmem_mbc *ptMBC)
{
grp_cmem_pf                     *ptPF;                  /* Index of memory pool control block   */
grp_cmem_mpc                    *ptMPC;
grp_s32                         lStatus = GRP_CMEM_OK;
grp_s32                         lLp1;
grp_s32                         lLp2;

    for (lLp1=0; lLp1<GRP_CMEM_MEMTBL_CNT; lLp1++) {
        ptMPC = patMPC[lLp1];                                   /* get MPC area                 */
        ptPF  = l_paCmemPF[lLp1];                               /* get porting format           */

        for (lLp2=0; lLp2<l_alIdMax[lLp1]; lLp2++, ptMPC++, ptPF++) {

#if (GRP_CMEM_BND_ENABLE == GRP_USB_TRUE)
            lStatus = _grp_cmem_ChangePartitionStart(
                                ptPF,                           /* memory pool control block    */
                                &ptFMP[ptPF->ulArea]);          /* free memory pool             */
            if (lStatus == GRP_CMEM_ER_VAL) {
                break;
            }

#else   /* (GRP_CMEM_BND_ENABLE == GRP_USB_FALSE) */
            lStatus = _grp_cmem_AdjustBoundaryPartitionStart(
                                ptPF,                           /* memory pool control block    */
                                &ptFMP[ptPF->ulArea]);          /* free memory pool             */
            if (lStatus == GRP_CMEM_ER_VAL) {
                break;
            }

#endif  /*(GRP_CMEM_BND_ENABLE == GRP_USB_FALSE)*/

            lStatus = _grp_cmem_InitPartitonBlockControl(
                                &ptFMP[ ptPF->ulArea ],
                                ptPF,
                                ptMPC,
                                &ptMBC );
            if (lStatus == GRP_CMEM_ER_INIT) {
                break;
            }
        }

        if (lStatus != GRP_CMEM_OK) {
            break;
        }
    }

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cmem_InitFMP                                                               */
/*                                                                                              */
/* DESCRIPTION: Initialize free memory information.                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptFMP                           pointer of free memory pool                     */
/* OUTPUT     : ptFMP                           free memory pool                                */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_cmem_InitFMP( grp_cmem_fmp *ptFMP)
{
grp_s32                         lLp;

    /* Initialize free memory partition information */
    for (lLp=0; lLp<GRP_CMEM_AREA_NUM; lLp++, ptFMP++) {
        ptFMP->pIdxFreeAd = l_atCmemRange[lLp].pvStartAd;       /* set start address free area  */
        ptFMP->ulFreeSize = l_atCmemRange[lLp].ulSize;          /* set free size of memory poo  */
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cmem_AllocateMemoryPool                                                    */
/*                                                                                              */
/* DESCRIPTION: Allocate memory pool area from memory buffer area.                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulNumMP                         number of memory pool area                      */
/* OUTPUT     : ptRange                         range of memory pool                            */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_INIT                Internal infomation generation goes wrong       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cmem_AllocateMemoryPool( grp_cmem_range *ptRange, grp_u32 ulNumMP)
{
    /* No used */

    ptRange = GRP_USB_NULL; /* Warning measures */
    ulNumMP = 0;            /* Warning measures */

    return GRP_CMEM_OK;
}

/************************************************************************************************/
/* Extern function                                                                              */
/************************************************************************************************/

/************************************************************************************************/
/* FUNCTION   : grp_cmem_PoolInit                                                               */
/*                                                                                              */
/* DESCRIPTION: Memory management I/F is initialized.                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_INIT                Internal infomation generation goes wrong       */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cmem_PoolInit(void)
{
grp_s32                         lStatus     = GRP_CMEM_OK;

    /* Allocate memory buffer area. */
    lStatus = _grp_cmem_AllocateMemoryPool( &l_atCmemRange[0], GRP_CMEM_AREA_NUM);
    if (lStatus != GRP_CMEM_OK) {
        return GRP_CMEM_ER_INIT;
    }

    /* for Solution Engine non-cache */
    l_atCmemRange[0].pvStartAd = (void *)((grp_u32)(l_atCmemRange[0].pvStartAd));

    /* Initialize free memory information */
    _grp_cmem_InitFMP( &l_atCmemFMP[0]);

    /* Initialize all control object */
    lStatus = _grp_cmem_InitAllControlObject( &l_atCmemFMP[0], &l_patCmemMPC[0], l_ptCmemMBC);
    if (lStatus != GRP_CMEM_OK) {
        return GRP_CMEM_ER_INIT;
    }

    /* Create Semaphore */
    if (grp_vos_CreateSemaphore( &l_ptCemSemaphore, (grp_u8 *)"sCMEM", 1) != GRP_VOS_POS_RESULT) {
        return GRP_CMEM_ER_INIT;
    }

    /* Set create flag enable */
    l_ulCreatFlag = GRP_CMEM_CREAT;

    return GRP_CMEM_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cmem_BlkGet                                                                 */
/*                                                                                              */
/* DESCRIPTION: Allocate fixed-length memory block                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulID                            Fixed-length memory pool ID                     */
/*              ppvBlk                          The pointer storing address of fixed-length     */
/*                                              memory block                                    */
/* OUTPUT     : ppvBlk                          The pointer of fixed-length memory block        */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_ID                  The memory pool ID is out of range              */
/*              GRP_CMEM_ER_NOEXS               The memory pool is not generated                */
/*              GRP_CMEM_ER_NOBLK               There is no empty block                         */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cmem_BlkGet( grp_u32 ulID, void **ppvBlk)
{
grp_cmem_mpc                    *ptMPC;                             /* memory partition control */
grp_cmem_mbc                    *ptMBC;                             /* memory block control     */
grp_s32                         lStatus;                            /* Status                   */
grp_s32                         lMask_ID;                           /* Mask ID                  */
    
#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    if (grp_vos_GetSemaphore( l_ptCemSemaphore, GRP_VOS_INFINITE) !=  GRP_CMEM_OK) {
        return GRP_CMEM_ER_BLK;
    }

#else   /* GRP_USB_HOST_NO_PARAM_CHECKING */
    grp_vos_GetSemaphore( l_ptCemSemaphore, GRP_VOS_INFINITE );

#endif  /* GRP_USB_HOST_NO_PARAM_CHECKING */

    lMask_ID = ulID & ~GRP_CMEM_IDMASK;                         /* The mask of the ID section   */

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    if (l_ulCreatFlag == GRP_CMEM_NO_CREAT) {                    /* No create CMEM area         */
        lStatus = GRP_CMEM_ER_NOEXS;
    }
    else if ((0 == lMask_ID)                                        /* ID is zero               */
          || (l_alIdMax[GRP_CMEM_IDOFT(ulID)] < lMask_ID)) {        /* ID is large than ID_MAX  */
        lStatus = GRP_CMEM_ER_ID;
    }
    else
#endif  /* GRP_USB_HOST_NO_PARAM_CHECKING */
    {
        /* Choose between USER or GR-USB */
        if ((ulID & GRP_CMEM_IDMASK ) == 0) {                                       /* User  ?  */
            ptMPC = &l_ptCmemMPC_Us[lMask_ID - GRP_CMEM_BASE_ID];
        }
        else {                                                                      /* GR_USB ? */
            ptMPC = &l_ptCmemMPC_Gu[lMask_ID - GRP_CMEM_BASE_ID];
        }

        /* Get memory block */
        if (ptMPC->pvNextMBC == GRP_USB_NULL) {                         /* Nothing empty block  */
            lStatus = GRP_CMEM_ER_NOBLK;
        }
        else {                                                          /* Exist empty block    */
            /* Get a memory block control infomation is no used */
            ptMBC = (grp_cmem_mbc*)ptMPC->pvNextMBC;

            /* Set address of memory block  */
            *ppvBlk = ptMBC->pvBlk;

            /* Change next MBC */
            ptMPC->pvNextMBC = ptMBC->pvNextMBC;

            lStatus = GRP_CMEM_OK;
        }
    }

#ifndef  GRP_USB_HOST_NO_PARAM_CHECKING
    if (grp_vos_ReleaseSemaphore( l_ptCemSemaphore) != GRP_CMEM_OK) {
        lStatus = GRP_CMEM_ER_BLK;
    }
#else
    grp_vos_ReleaseSemaphore( l_ptCemSemaphore );
#endif

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cmem_BlkRel                                                                 */
/*                                                                                              */
/* DESCRIPTION: Release fixed-length memory block.                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pvBlk                           The pointer of fixed-length memory block        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_CMEM_OK                     Normal termination                              */
/*              GRP_CMEM_ER_BLK                 There is no empty block                         */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cmem_BlkRel( void *pvBlk)
{
grp_cmem_mpc*                   ptMPC;      /* The address of a memory pool management block    */
grp_cmem_mbc*                   ptRelMBC;   /* The address of a memory pool management block    */
grp_u32                         ulDef_Size; /* The defference size from the offset address      */
grp_u32                         ulIdxBlk;                   /* The index ID for which it asks   */
grp_s32                         lLp1;                       /* The loop for memory pool users   */
grp_s32                         lLp2;                       /* The loop for index retrieval     */
grp_s32                         lStatus = GRP_CMEM_ER_BLK;  /* Status                           */

#ifndef GRP_USB_HOST_NO_PARAM_CHECKING
    if (grp_vos_GetSemaphore( l_ptCemSemaphore, GRP_VOS_INFINITE) !=  GRP_CMEM_OK) {
        return GRP_CMEM_ER_BLK;
    }

#else   /* GRP_USB_HOST_NO_PARAM_CHECKING */
    grp_vos_GetSemaphore( l_ptCemSemaphore, GRP_VOS_INFINITE);

#endif  /*GRP_USB_HOST_NO_PARAM_CHECKING */

    /* the loop of memory partition number */
    for (lLp1=0; lLp1<GRP_CMEM_MEMTBL_CNT; lLp1++) {
        /* Get array of memory partition control. */
        ptMPC = l_patCmemMPC[lLp1];

        /*  Search memory partition control.  */
        for (lLp2=0; lLp2<l_alIdMax[lLp1]; lLp2++, ptMPC++) {
            /* Search memory pool involed the release block */
            if ((ptMPC->pvEndAd   >  pvBlk)
             && (ptMPC->pvStartAd <= pvBlk)) {
                /* Calc size of offset memory partition */
                ulDef_Size = (grp_u32)pvBlk - (grp_u32)(((grp_cmem_mbc *)ptMPC->pvStartMBC)->pvBlk);

                if ((ulDef_Size % ptMPC->ulBlkSz) == 0) {
                    /* Get memory block control is released. */
                    ulIdxBlk = ulDef_Size / ptMPC->ulBlkSz;
                    ptRelMBC = (grp_cmem_mbc *)ptMPC->pvStartMBC + ulIdxBlk;

                    /* Push the release memory block to MPC */
                    ptRelMBC->pvNextMBC = ptMPC->pvNextMBC;
                    ptMPC->pvNextMBC    = ptRelMBC;

                    lStatus = GRP_CMEM_OK;
                }
                else {
                    lStatus = GRP_CMEM_ER_BLK;
                }
                break;
            }
        }
        if (lStatus == GRP_CMEM_OK) {
            break;
        }
    }

#ifndef  GRP_USB_HOST_NO_PARAM_CHECKING
    if (grp_vos_ReleaseSemaphore( l_ptCemSemaphore) != GRP_CMEM_OK) {
        lStatus = GRP_CMEM_ER_BLK;
    }

#else
    grp_vos_ReleaseSemaphore( l_ptCemSemaphore );

#endif

    return lStatus;
}

/************************************************************************************************/
/* FUNCTION   : grp_cmem_GetLogical                                                             */
/*                                                                                              */
/* DESCRIPTION: The logical address is acquired                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pvBlk                           The pointer of a physical address               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Logical address                 The pointer of the logical address              */
/*                                                                                              */
/************************************************************************************************/
void* grp_cmem_GetLogical( void *pvPhyAdr)
{
    return pvPhyAdr;
}

/************************************************************************************************/
/* FUNCTION   : grp_cmem_GetPhysical                                                            */
/*                                                                                              */
/* DESCRIPTION: A physical address is acquired                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pvBlk                           The pointer of a logical address                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Physical address                The pointer of the physical address             */
/*                                                                                              */
/************************************************************************************************/
void* grp_cmem_GetPhysical( void *pvLogAdr)
{
    return pvLogAdr;
}

