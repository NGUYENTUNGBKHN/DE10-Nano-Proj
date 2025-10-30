/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_mem_vl_pool.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Variable length memory allocation library							*/
/* FUNCTIONS:																*/
/*		grp_mem_init_vl_pool		initialize pool							*/
/*		grp_mem_add_vl_pool			add memory to pool						*/
/*		grp_mem_alloc_from_vl_pool	allocate memory from pool				*/
/*		grp_mem_free_to_vl_pool		free memory to pool						*/
/*		grp_mem_vl_init				init pool								*/
/* DEPENDENCIES:															*/
/*		<string.h>															*/
/*		grp_types.h															*/
/*		grp_mem.h															*/
/*		grp_sem.h															*/
/*		grp_mem_vl_pool.h													*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/05/30	Created inital version 1.0				*/
/*		T.Imashiki		2003/12/19	Deleted debug option code				*/
/*		T.Imashiki		2003/12/19	Added missing new line in a comment		*/
/*		T.Imahsiki		2004/12/07	Fixed bug setting initial semaphore		*/
/*									value with pool id						*/
/*									Fixed bug releasing semaphore failed to	*/
/*									get lock								*/
/*		T.Imashiki		2007/02/20	Added type casts for 16 bit CPU support	*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		T.Imashiki		2010/11/16	Checked iPoolSize before memset in		*/
/*		K.Kaneko					grp_mem_add_vl_pool						*/
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
#include <string.h>
#include "grp_fs_sysdef.h"
#include "grp_types.h"
#include "grp_mem.h"
#include "grp_sem.h"
#include "grp_mem_vl_pool.h"

#ifdef	GRP_MEM_VL_DEBUG
#include <stdlib.h>
#define grp_mem_fl_alloc(x)		malloc(x)
#define grp_mem_fl_free(x)		free(x)
#define grp_sem_create(sem, name, icnt)		0
#define grp_sem_get(sem, timeout)			0
#define grp_sem_release(sem)
#endif

grp_mem_vl_ctl_t	*grp_mem_vl_ctl = NULL;		/* control table */
int					grp_mem_vl_pool_cnt = 0;	/* pool count */

/****************************************************************************/
/* FUNCTION:	grp_mem_init_vl_pool										*/
/*																			*/
/* DESCRIPTION:	Initialize variable length memory pool						*/
/* INPUT:		pcPoolMem:				pool memory address					*/
/*				iPoolSize:				pool memory size					*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL					error								*/
/*				others					pointer to memory control table		*/
/*																			*/
/****************************************************************************/
grp_mem_vl_ctl_t *
grp_mem_init_vl_pool(
	char				*pcPoolMem,				/* [IN]  pool memory address */
	grp_int32_t			iPoolSize)				/* [IN]  pool memory size */
{
	grp_mem_vl_ctl_t	*ptCtl;					/* control data */
	int					iAlignOff;				/* alignment offset */
	int					iAdjust;				/* adjust size */
	int					iCtlSz;					/* control table size */
	char				acSemName[8];			/* semphore name */

	if (grp_mem_vl_pool_cnt >= 10)				/* too many pools */
		return(NULL);							/* return error */
	iCtlSz = GRP_MEM_VL_RND_UP(sizeof(grp_mem_vl_ctl_t));
												/* control table size */
	iAlignOff = GRP_MEM_VL_ALIGN_OFF(pcPoolMem);/* alignment offset */
	if (iAlignOff) {							/* not on alignment */
		iAdjust = GRP_MEM_VL_ALIGN - iAlignOff;	/* adjust size */
		pcPoolMem += iAdjust;					/* adjust address */
		iPoolSize -= iAdjust;					/* adjust size */
	}
	if (iPoolSize < 
		iCtlSz + (int)sizeof(grp_mem_vl_free_t) * 2 + GRP_MEM_VL_ALIGN)
		return(NULL);							/* invalid parameter */
	ptCtl = (grp_mem_vl_ctl_t *) pcPoolMem;		/* control table */
	memset(pcPoolMem, 0, sizeof(grp_mem_vl_ctl_t)); /* init control table */
	strcpy(acSemName, "vl_mem");				/* set semaphore name */
	acSemName[6] = (char)(grp_mem_vl_pool_cnt + '0'); /* pool id */
	acSemName[7] = 0;							/* NULL terminate */
	if (grp_sem_create(&ptCtl->tLock, acSemName, 1) != 0) /* init lock error */
		return(NULL);							/* return error */
	grp_mem_vl_pool_cnt++;						/* increment pool count */
	ptCtl->uiMemAdded = iCtlSz;					/* init added memory size */
	(void)grp_mem_add_vl_pool(ptCtl, pcPoolMem + iCtlSz, iPoolSize - iCtlSz);
												/* add pool memory */
	return(ptCtl);								/* return control table */
}

/****************************************************************************/
/* FUNCTION:	grp_mem_add_vl_pool											*/
/*																			*/
/* DESCRIPTION:	Add memory to the variable length memory pool				*/
/* INPUT:		ptCtl:					pool control table					*/
/* 				pcPoolMem:				pool memory address					*/
/*				iPoolSize:				pool memory size					*/
/*																			*/
/* RESULT:		0:						success								*/
/*				-1:						error(size too small)				*/
/*																			*/
/****************************************************************************/
int
grp_mem_add_vl_pool(
	grp_mem_vl_ctl_t	*ptCtl,					/* [IN]  pool control table */
	char				*pcPoolMem,				/* [IN]  pool memory address */
	grp_int32_t			iPoolSize)				/* [IN]  pool memory size */
{
	grp_mem_vl_free_t	*ptFree;				/* free entry */
	grp_mem_vl_free_t	*ptPrev;				/* previous free entry */
	grp_mem_vl_free_t	*ptNew;					/* new entry */
	int					iAlignOff;				/* alignment offset */
	int					iAdjust;				/* adjust size */

	/****************************************************/
	/* check paramter, and adjust if necesssary			*/
	/****************************************************/
	iAlignOff = GRP_MEM_VL_ALIGN_OFF(pcPoolMem);/* alignment offset */
	if (iAlignOff) {							/* not on alignment */
		iAdjust = GRP_MEM_VL_ALIGN - iAlignOff;	/* adjust size */
		pcPoolMem += iAdjust;					/* adjust address */
		iPoolSize -= iAdjust;					/* adjust size */
	}
	memset(pcPoolMem, 0, (iPoolSize < 0)? 0:iPoolSize);	/* init with 0 */
	iPoolSize -= (int)(sizeof(grp_mem_vl_free_t) * 2); /* subtract list size */
	iPoolSize = GRP_MEM_VL_RND(iPoolSize);		/* round down alignment */
	if (iPoolSize <= 0)							/* not enough memory */
		return(-1);								/* return error */

	/****************************************************/
	/* set 0 size free entry to distinguish boundary	*/
	/****************************************************/
	ptNew = (grp_mem_vl_free_t *)pcPoolMem;		/* free list entries */
	ptNew->uiFreeSize = 0;						/* set size 0 for pool marker */
	ptNew->ptFreeList = &ptNew[1];				/* point to next one */
	ptNew[1].uiFreeSize = iPoolSize;			/* set free area size */
	ptCtl->uiFree += ptNew[1].uiFreeSize;		/* add free size */
	ptCtl->uiMemAdded += iPoolSize;				/* add added size */
	ptCtl->uiEntCnt += 2;						/* add entry count */

	/****************************************************/
	/* find insert point								*/
	/****************************************************/
	ptPrev = NULL;								/* no previous */
	for (ptFree = ptCtl->ptFreeList; ptFree ; ptFree = ptFree->ptFreeList) {
		if (ptFree >= ptNew)					/* find insert point */
			break;								/* break */
		ptPrev = ptFree;						/* remember previous */
	}
	if (ptPrev == NULL) {						/* insert top */
		ptNew[1].ptFreeList = ptCtl->ptFreeList;/* chain top */
		ptCtl->ptFreeList = ptNew;				/* insert top */
	} else {									/* insert after prev */
		ptNew[1].ptFreeList = ptPrev->ptFreeList;/* chain after prev */
		ptPrev->ptFreeList = ptNew;				/* insert after prev */
	}
	return(0);
}


/****************************************************************************/
/* FUNCTION:	_grp_mem_extract_vl_entry									*/
/*																			*/
/* DESCRIPTION:	Extract free memory entry									*/
/* INPUT:		iAllocSize:				allocate size						*/
/*				ptFree:					free entry							*/
/*				ptPrev:					previous entry						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
static void
_grp_mem_extract_vl_entry(
	grp_int32_t				iAllocSize,			/* [IN]  allocate size */
	grp_mem_vl_free_t		*ptFree,			/* [IN]  free entry */
	grp_mem_vl_free_t		*ptPrev)			/* [IN]  previous entry */
{
	grp_mem_vl_ctl_t		*ptCtl = grp_mem_vl_ctl; /* control data */
	grp_mem_vl_free_t		*ptNewFree;			/* new free entry */
	grp_int32_t				iFreeSize;			/* free size */

	iFreeSize = ptFree->uiFreeSize;				/* free size */
	if (iAllocSize + (grp_int32_t)sizeof(grp_mem_vl_free_t) >= iFreeSize) {
												/* use whole */
		ptPrev->ptFreeList = ptFree->ptFreeList; /* extract the entry */
		ptCtl->uiFree -= ptFree->uiFreeSize;	/* decrement free size */
		ptCtl->uiEntCnt--;						/* decrement entry count */
	} else {									/* use partial area */
		ptNewFree = (grp_mem_vl_free_t *)
			((grp_uchar_t *)ptFree + sizeof(grp_mem_vl_free_t) + iAllocSize);
		ptNewFree->ptFreeList = ptFree->ptFreeList;	/* set next free list */
		ptNewFree->uiFreeSize = iFreeSize 
								- (iAllocSize + sizeof(grp_mem_vl_free_t));
		ptPrev->ptFreeList = ptNewFree;			/* chain in free list */
		ptFree->uiFreeSize = iAllocSize;		/* alloc size */
		ptCtl->uiFree -= (iAllocSize + sizeof(grp_mem_vl_free_t));
												/* decrement free size */
	}
	ptFree->ptFreeList = &ptFree[1];			/* point to body */
	ptCtl->uiUsed += ptFree->uiFreeSize;		/* add used count */
	ptCtl->uiUsedCnt++;							/* increment used count */
}

/****************************************************************************/
/* FUNCTION:	grp_mem_alloc_from_vl_pool									*/
/*																			*/
/* DESCRIPTION:	Allocate a requested size area								*/
/* INPUT:		ptCtl							pool control table			*/
/*				iAllocSize:						size to allocate			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:							error						*/
/*				others:							allocated area				*/
/*																			*/
/****************************************************************************/
void *
grp_mem_alloc_from_vl_pool(
	grp_mem_vl_ctl_t	*ptCtl,					/* [IN]  pool control table */
	grp_int32_t			iAllocSize)				/* [IN]  size to allocate */
{
	grp_mem_vl_free_t	*ptFree;				/* free mem entry */
	grp_mem_vl_free_t	*ptPrev;				/* previous mem entry */
	int					iSemRet;				/* return from grp_sem_get */

	/****************************************************/
	/* check alloc size									*/
	/****************************************************/
	if (iAllocSize <= 0)						/* alloc size is not positive */
		return(NULL);							/* return NULL */

	/****************************************************/
	/* lock operation									*/
	/* Note: lock error is ignored to prevent			*/
	/*		 system down								*/
	/****************************************************/
	if (ptCtl == NULL)							/* not initialized */
		return(NULL);							/* return error */
	iSemRet = grp_sem_get(ptCtl->tLock, GRP_MEM_VL_TIMEOUT); /* lock */

	/****************************************************/
	/* check requested size								*/
	/*													*/
	/* 	  If requested size is greater than pool size,	*/
	/* 	allocate by fix length memory allocation.		*/
	/* 	  If requested size is bigger than left in pool,*/
	/*	allocate a new pool, and return from it.		*/
	/*	  Otherwise, allocate it from a current pool.	*/
	/****************************************************/
	ptPrev = NULL;								/* init previous pointer */
	iAllocSize = GRP_MEM_VL_RND_UP(iAllocSize);	/* round up to alignment */
	if (iAllocSize > (grp_int32_t)ptCtl->uiFree) {		/* not enough free */
		ptFree = NULL;							/* no memory */
		goto out;								/* return no memory */
	}
	for (ptFree = ptCtl->ptFreeList; ptFree; 
							ptPrev = ptFree, ptFree = ptFree->ptFreeList) {
		if ((grp_int32_t)ptFree->uiFreeSize >= iAllocSize) /* found match */
			break;								/* stop search */
	}
	if (ptFree == NULL)							/* not found in pool */
		goto out;								/* return no memory */
	_grp_mem_extract_vl_entry(iAllocSize, ptFree, ptPrev);
												/* extract found entry */

out:
	/****************************************************/
	/* unlock operation									*/
	/****************************************************/
	if (iSemRet == 0)							/* lock succeeded */
		grp_sem_release(ptCtl->tLock);			/* release lock */
	if (ptFree == NULL)							/* not allocated */
		return(NULL);							/* return NULL */
	return((void *)ptFree->ptFreeList);			/* return allocated free area */
}

/****************************************************************************/
/* FUNCTION:	grp_mem_free_to_vl_pool										*/
/*																			*/
/* DESCRIPTION:	Free area allocated by grp_mem_vl_alloc						*/
/* INPUT:		pvMem:							momory to free				*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		-1:								error						*/
/*				0:								success						*/
/*																			*/
/****************************************************************************/
int
grp_mem_free_to_vl_pool(
	grp_mem_vl_ctl_t	*ptCtl,					/* [IN]  pool control table */
	void				*pvMem)					/* [IN]  memory to free */
{
	grp_mem_vl_free_t	*ptEnt;					/* entry to free */
	grp_mem_vl_free_t	*ptFree;				/* free entry */
	grp_mem_vl_free_t	*ptPrev;				/* pervious entry */
	grp_mem_vl_free_t	*ptNext;				/* next entry */
	int					iRet = -1;				/* return value */
	int					iSemRet;				/* return from grp_sem_get */
	
	/****************************************************/
	/* lock operation									*/
	/* Note: lock error is ignored to prevent			*/
	/*		 system down								*/
	/****************************************************/
	if (ptCtl == NULL)							/* not initialized */
		return(-1);								/* return error */
	iSemRet = grp_sem_get(ptCtl->tLock, GRP_MEM_VL_TIMEOUT); /* lock */

	/****************************************************/
	/* check the area is valid one						*/
	/****************************************************/
	ptEnt = &((grp_mem_vl_free_t *)pvMem)[-1];	/* entry to free */
	if (ptEnt->ptFreeList != (grp_mem_vl_free_t *)pvMem)	/* invalid */
		goto out;								/* return error */
	ptEnt->ptFreeList = NULL;					/* reset to NULL */

	/****************************************************/
	/* find insert point								*/
	/****************************************************/
	ptPrev = NULL;								/* no previous */
	for (ptFree = ptCtl->ptFreeList; ptFree ; ptFree = ptFree->ptFreeList) {
		if (ptFree >= ptEnt)					/* find insert point */
			break;								/* break */
		ptPrev = ptFree;						/* remember previous */
	}
	if (ptPrev == NULL) {						/* insert top */
		/****************************************************/
		/* since pool marker exists at top, this should not	*/
		/* occur for valid entry							*/
		/****************************************************/
		goto out;								/* return error */
	}

	/****************************************************/
	/* merge to previous entry if possible				*/
	/****************************************************/
	ptCtl->uiUsed -= ptEnt->uiFreeSize;			/* decrement used size */
	ptCtl->uiFree += ptEnt->uiFreeSize;			/* increment free size */
	ptCtl->uiUsedCnt--;							/* decrement used count */
	if ((char *)(&ptPrev[1]) + ptPrev->uiFreeSize == (char *)ptEnt 
		&& ptPrev->uiFreeSize != 0) {			/* mergeable */
		/****************************************************/
		/* merge to previous entry							*/
		/****************************************************/
		ptPrev->uiFreeSize += ptEnt->uiFreeSize + sizeof(grp_mem_vl_free_t);
		ptEnt = ptPrev;							/* treat prev as target */
		ptCtl->uiFree += sizeof(grp_mem_vl_free_t); /* increment free size */
	} else {									/* not mergeable */
		/****************************************************/
		/* insert afer previous entry						*/
		/****************************************************/
		ptEnt->ptFreeList = ptPrev->ptFreeList;	/* chain remain list */
		ptPrev->ptFreeList = ptEnt;				/* insert after prev */
		ptCtl->uiEntCnt++;						/* increment entry count */
	}

	/****************************************************/
	/* merge to next entry if possible					*/
	/****************************************************/
	ptNext = ptEnt->ptFreeList;
	if ((char *)(&ptEnt[1]) + ptEnt->uiFreeSize == (char *)ptNext
		&& ptNext->uiFreeSize != 0) {			/* mergeable */
		/****************************************************/
		/* merge to next entry								*/
		/****************************************************/
		ptEnt->uiFreeSize += ptNext->uiFreeSize + sizeof(grp_mem_vl_free_t);
		ptEnt->ptFreeList = ptNext->ptFreeList;	/* skip next entry */
		ptCtl->uiFree += sizeof(grp_mem_vl_free_t); /* increment free size */
		ptCtl->uiEntCnt--;						/* decrement entry count */
	}
	iRet = 0;									/* set success return value */

out:
	/****************************************************/
	/* unlock operation									*/
	/****************************************************/
	if (iSemRet == 0)							/* lock succeeded */
		grp_sem_release(ptCtl->tLock);			/* release lock */
	return(iRet);								/* return */
}

#ifdef	GRP_MEM_VL_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include "../test_cmd/test_cmd.h"

/****************************************************************************/
/* FUNCTION:	grp_mem_vl_print_info										*/
/*																			*/
/* DESCRIPTION:	Print variable length memory pool information				*/
/* INPUT:		None														*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		None														*/
/*																			*/
/****************************************************************************/
void
grp_mem_vl_print_info(
	grp_mem_vl_ctl_t	*ptCtl)					/* [IN]  pool control table */
{
	grp_mem_vl_free_t	*ptEnt;					/* free entry */
	grp_mem_vl_free_t	*ptNext;				/* next entry */
	int					i;						/* loop count */
	int					iHole;					/* hole size */

	if (ptCtl == NULL)
		return;
	test_cmd_printf("Added: 0x%x, Used: 0x%x, Free: 0x%x, ",
				 ptCtl->uiMemAdded, ptCtl->uiUsed, ptCtl->uiFree);
	test_cmd_printf("UsedCnt: %d, EntCnt: %d\n",
				ptCtl->uiUsedCnt, ptCtl->uiEntCnt);
	for (i = 0, ptEnt = ptCtl->ptFreeList; ptEnt; ptEnt = ptNext, i++) {
		ptNext = ptEnt->ptFreeList;
		if (ptNext)
			iHole = (char *)ptNext - (((char *)&ptEnt[1]) + ptEnt->uiFreeSize);
		else
			iHole = 0;
		test_cmd_printf("%d:\t%08x - %08x(%08x:%08x)\n",
					i, (grp_int32_t)ptEnt, 
					(grp_int32_t)(((char *)&ptEnt[1]) + ptEnt->uiFreeSize),
					(grp_int32_t)ptEnt->uiFreeSize, iHole);
	}
	if (i != ptCtl->uiEntCnt)
		test_cmd_printf("*** entry count missmatch %d, %d\n", 
						i, ptCtl->uiEntCnt);
}

#endif
