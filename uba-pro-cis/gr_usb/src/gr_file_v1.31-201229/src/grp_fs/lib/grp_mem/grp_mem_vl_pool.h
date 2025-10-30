#ifndef	_GRP_MEM_VL_POOL_H_
#define	_GRP_MEM_VL_POOL_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_mem_vl_pool.h											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for variable length memory allocation library			*/
/* FUNCTIONS:																*/
/*		grp_mem_init_vl_pool		initialize pool							*/
/*		grp_mem_add_vl_pool			add memory to pool						*/
/*		grp_mem_alloc_from_vl_pool	allocate memory from pool				*/
/*		grp_mem_free_to_vl_pool		free memory to pool						*/
/*		grp_mem_vl_init				init pool								*/
/*		grp_mem_vl_add				add memory to pool						*/
/*		grp_mem_vl_alloc			allocate memory							*/
/*		grp_mem_vl_free				free memory								*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/*		grp_mem.h															*/
/*		grp_sem.h															*/
/*		grp_mem_vl_pool.h													*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/30	Created inital version 1.0				*/
/*		T.Imashiki		2005/02/10	Changed type cast for 16 bit CPU		*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		K.Kaneko		2010/11/16	Delete include grp_mem_vl_pool.h		*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2010 Grape Systems, Inc.,  All Rights Reserved.        */
/*                                                                          */
/* This software is furnished under a license, and may be used only in      */
/* accordance with the terms of such license and with the inclusion of the  */
/* above copyright notice.  No title to and the ownership of the software   */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* non-infringement of any intellectual property rights and the performance */
/* of this computer program, and specifically disclaims any responsibility  */
/* for any damages, special or consequential, connected  with the use of    */
/* this program.                                                            */
/****************************************************************************/
#include "grp_fs_sysdef.h"
#include "grp_types.h"
#include "grp_mem.h"
#include "grp_sem.h"

/****************************************************************************/
/* parameters																*/
/****************************************************************************/
#define GRP_MEM_VL_TIMEOUT		30000			/* timeout for semaphore */
#define GRP_MEM_VL_ALIGN		8				/* memory alignment */

/****************************************************************************/
/* macro definitions														*/
/****************************************************************************/
#define GRP_MEM_VL_ALIGN_OFF(iOff)				/* alignment offset */		\
	(((int)((char *)(iOff)-(char *)0)) & (GRP_MEM_VL_ALIGN - 1))
#define GRP_MEM_VL_RND(iSize)					/* round down align size */	\
	(((grp_int32_t)(iSize)) & ~(GRP_MEM_VL_ALIGN - 1))
#define GRP_MEM_VL_RND_UP(iSize)				/* round up align size */	\
	GRP_MEM_VL_RND(((grp_int32_t)(iSize)) + (GRP_MEM_VL_ALIGN - 1))

/****************************************************************************/
/* variable length memory free management table								*/
/****************************************************************************/
typedef struct grp_mem_vl_free grp_mem_vl_free_t;
struct grp_mem_vl_free {
	grp_mem_vl_free_t		*ptFreeList;			/* free list */
	grp_uint32_t			uiFreeSize;				/* free size */
};

/****************************************************************************/
/* variable length memory top management table								*/
/****************************************************************************/
typedef struct grp_mem_vl_ctl {
	grp_sem_t				tLock;				/* lock variable */
	grp_uint32_t			uiMemAdded;			/* added memory count */
	grp_uint32_t			uiUsed;				/* used size */
	grp_uint32_t			uiFree;				/* total free size */
	grp_uint32_t			uiUsedCnt;			/* used count */
	grp_uint32_t			uiEntCnt;			/* entry count */
	grp_mem_vl_free_t		*ptFreeList;		/* free list */
} grp_mem_vl_ctl_t;

/****************************************************************************/
/* exported interfaces														*/
/****************************************************************************/
grp_mem_vl_ctl_t *grp_mem_init_vl_pool(			/* initialize pool */
		char				*pcPoolMem,			/* [IN]  pool memory address */
		grp_int32_t			iPoolSize);			/* [IN]  pool memory size */
int grp_mem_add_vl_pool(						/* add memory to pool */
		grp_mem_vl_ctl_t	*ptCtl,				/* [IN]  pool control table */
		char				*pcPoolMem,			/* [IN]  pool memory address */
		grp_int32_t			iPoolSize);			/* [IN]  pool memory size */
void * grp_mem_alloc_from_vl_pool(				/* allocate memory from pool */
		grp_mem_vl_ctl_t	*ptCtl,				/* [IN]  pool control table */
		grp_int32_t			iAllocSize);		/* [IN]  size to allocate */
int grp_mem_free_to_vl_pool(					/* free memory to pool */
		grp_mem_vl_ctl_t	*ptCtl,				/* [IN]  pool control table */
		void				*pvMem);			/* [IN]  memory to free */
#define grp_mem_vl_init(pcPoolMem, iPoolSize)	/* init pool */			\
		(((grp_mem_vl_ctl = grp_mem_init_vl_pool(pcPoolMem, iPoolSize)) \
			!= NULL)? 0: -1)
#define grp_mem_vl_add(pcPoolMem, iPoolSize)	/* add memory to pool */\
		grp_mem_add_vl_pool(grp_mem_vl_ctl, pcPoolMem, iPoolSize)
#define grp_mem_vl_alloc(iAllocSize)			/* allocate memory */	\
		grp_mem_alloc_from_vl_pool(grp_mem_vl_ctl, iAllocSize)
#define grp_mem_vl_free(pvMem)					/* free memory */		\
		grp_mem_free_to_vl_pool(grp_mem_vl_ctl, pvMem)

/****************************************************************************/
/* exported variable														*/
/****************************************************************************/
extern grp_mem_vl_ctl_t		*grp_mem_vl_ctl;	/* memory control table */

#endif	/* _GRP_MEM_VL_POOL_H_ */
