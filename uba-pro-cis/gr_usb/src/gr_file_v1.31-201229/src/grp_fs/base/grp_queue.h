#ifndef	_GRP_QUEUE_H_
#define	_GRP_QUEUE_H_
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_queue.h													*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Definitions for queue operation										*/
/* FUNCTIONS:																*/
/*		grp_enque_head				insert at top of the list				*/
/*		grp_enque_tail				append at tail of the list				*/
/*		grp_enque_prev				insert before entry						*/
/*		grp_enque_next				insert after entry						*/
/*		grp_deque_head				get from top of the list				*/
/*		grp_deque_tail				get from tail of the list				*/
/*		grp_deque_ent				dequeue entry							*/
/*		grp_enque_shead				insert top of single head list			*/
/*		grp_enque_sprev				insert before for single head list		*/
/*		grp_enque_snext				insert after for single head list		*/
/*		grp_deque_shead				dequeue top entry from single head list	*/
/*		grp_deque_sent				dequeue entry from single head list		*/
/* DEPENDENCIES:															*/
/*		grp_types.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Nisikado		2003/03/07	Created inital version 1.0				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003 Grape Systems, Inc.,  All Rights Reserved.             */
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

#include "grp_types.h"

/****************************************************************************/
/*  double linked list queue field											*/
/****************************************************************************/
#define grp_que_fld(Type, FldPrefix)				/* queue field */		\
	Type		FldPrefix##Fwd;						/* forward chain */		\
	Type		FldPrefix##Bwd						/* backword chain */

/****************************************************************************/
/*  insert at the top of queue												*/
/****************************************************************************/
#define grp_enque_head(Q_Hd, HdPrefix, Q_Ent, EntPrefix) {					\
	(Q_Ent)->EntPrefix##Bwd = NULL;					/* bwd is NULL */		\
	(Q_Ent)->EntPrefix##Fwd = (Q_Hd)->HdPrefix##Fwd;/* fwd is Hd fwd */		\
	(Q_Hd)->HdPrefix##Fwd = (Q_Ent);				/* set Hd fwd */		\
	if ((Q_Ent)->EntPrefix##Fwd)					/* prev top exists */	\
		(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = (Q_Ent);	/* insert */	\
	else											/* no prev top */		\
		(Q_Hd)->HdPrefix##Bwd = (Q_Ent);			/* set Hd bwd */		\
}

/****************************************************************************/
/*  append at the tail of queue												*/
/****************************************************************************/
#define grp_enque_tail(Q_Hd, HdPrefix, Q_Ent, EntPrefix) {					\
	(Q_Ent)->EntPrefix##Fwd = NULL;					/* fwd is NULL */		\
	(Q_Ent)->EntPrefix##Bwd = (Q_Hd)->HdPrefix##Bwd;/* bwd is tail */		\
	(Q_Hd)->HdPrefix##Bwd = (Q_Ent);				/* set Hd bwd */		\
	if ((Q_Ent)->EntPrefix##Bwd)					/* prev tail exists */	\
		(Q_Ent)->EntPrefix##Bwd->EntPrefix##Fwd = (Q_Ent);	/* insert */	\
	else											/* no prev tail */		\
		(Q_Hd)->HdPrefix##Fwd = (Q_Ent);			/* set Hd fwd */		\
}

/****************************************************************************/
/*  insert before entry														*/
/****************************************************************************/
#define grp_enque_prev(Q_Hd, HdPrefix, Q_At, Q_Ent, EntPrefix) {			\
	(Q_Ent)->EntPrefix##Fwd = (Q_At);				/* fwd is At */			\
	(Q_Ent)->EntPrefix##Bwd = (Q_At)->EntPrefix##Bwd;/* bwd is At bwd */	\
	(Q_At)->EntPrefix##Bwd = (Q_Ent);				/* set At bwd */		\
	if ((Q_Ent)->EntPrefix##Bwd)					/* prev bwd exists */	\
		(Q_Ent)->EntPrefix##Bwd->EntPrefix##Fwd = (Q_Ent);	/* insert */	\
	else											/* prev bwd exits */	\
		(Q_Hd)->HdPrefix##Fwd = (Q_Ent);			/* set Hd fwd */		\
}

/****************************************************************************/
/*  insert after entry														*/
/****************************************************************************/
#define grp_enque_next(Q_Hd, HdPrefix, Q_At, Q_Ent, EntPrefix) {			\
	(Q_Ent)->EntPrefix##Fwd = (Q_At)->EntPrefix##Fwd;/* fwd is At fwd */	\
	(Q_Ent)->EntPrefix##Bwd = (Q_At);				/* bwd is At */			\
	(Q_At)->EntPrefix##Fwd = (Q_Ent);				/* set At fwd */		\
	if ((Q_Ent)->EntPrefix##Fwd)					/* prev fwd exists */	\
		(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = (Q_Ent);	/* insert */	\
	else											/* no prev fwd */		\
		(Q_Hd)->HdPrefix##Bwd = (Q_Ent);			/* set Hd bwd */		\
}

/****************************************************************************/
/*  dequeue from queue head 												*/
/****************************************************************************/
#define grp_deque_head(Q_Hd, HdPrefix, Q_Ent, EntPrefix) {					\
	if ((Q_Hd)->HdPrefix##Fwd) {					/* Ent exists */		\
		(Q_Ent) = (Q_Hd)->HdPrefix##Fwd;			/* Ent is Hd fwd */		\
		(Q_Hd)->HdPrefix##Fwd = (Q_Ent)->EntPrefix##Fwd; /* deque from top */\
		if ((Q_Ent)->EntPrefix##Fwd)				/* next ent exists */	\
			(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = NULL; /* deque */		\
		else										/* no next ent */		\
			(Q_Hd)->HdPrefix##Bwd = NULL;			/* Hd fwd NULL */		\
		(Q_Ent)->EntPrefix##Fwd = (Q_Ent)->EntPrefix##Bwd = NULL; 			\
	} else {										/* no Ent */			\
		(Q_Ent) = NULL;								/* set NULL */			\
	}																		\
}

/****************************************************************************/
/*  dequeue from queue tail 												*/
/****************************************************************************/
#define grp_deque_tail(Q_Hd, HdPrefix, Q_Ent, EntPrefix) {					\
	if ((Q_Hd)->HdPrefix##Bwd) {					/* Ent exists */		\
		(Q_Ent) = (Q_Hd)->HdPrefix##Bwd;			/* Ent is Hd bwd */		\
		(Q_Hd)->HdPrefix##Bwd = (Q_Ent)->EntPrefix##Bwd; /* deque from tail */\
		if ((Q_Ent)->EntPrefix##Bwd)				/* bwd entry exits */	\
			(Q_Ent)->EntPrefix##Bwd->EntPrefix##Fwd = NULL;	 /* deque */	\
		else										/* no bwd entry */		\
			(Q_Hd)->HdPrefix##Fwd = NULL;			/* Hd bwd NULL */		\
		(Q_Ent)->EntPrefix##Fwd = (Q_Ent)->EntPrefix##Bwd = NULL;	 		\
	} else {										/* no Ent */			\
		(Q_Ent) = NULL;								/* set NULL */			\
	}																		\
}

/****************************************************************************/
/*  dequeue entry															*/
/****************************************************************************/
#define grp_deque_ent(Q_Hd, HdPrefix, Q_Ent, EntPrefix)	{					\
	if ((Q_Ent)->EntPrefix##Fwd)					/* fwd entry exists */	\
		(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = 							\
					(Q_Ent)->EntPrefix##Bwd;		/* deque from fwd */	\
	else											/* no fwd entry */		\
		(Q_Hd)->HdPrefix##Bwd = (Q_Ent)->EntPrefix##Bwd; /* set Hd bwd */	\
	if ((Q_Ent)->EntPrefix##Bwd)					/* bwd entry exists */	\
		(Q_Ent)->EntPrefix##Bwd->EntPrefix##Fwd = 							\
					(Q_Ent)->EntPrefix##Fwd;		/* deque from bwd */	\
	else											/* no bwd entry */		\
		(Q_Hd)->HdPrefix##Fwd = (Q_Ent)->EntPrefix##Fwd; /* set Hd fwd */	\
	(Q_Ent)->EntPrefix##Fwd = NULL;					/* clear link */		\
	(Q_Ent)->EntPrefix##Bwd = NULL;					/* clear link */		\
}

/****************************************************************************/
/*  insert at the top of single head queue									*/
/****************************************************************************/
#define grp_enque_shead(Q_Hd, Q_Ent, EntPrefix) {							\
	(Q_Ent)->EntPrefix##Bwd = NULL;					/* bwd is NULL */		\
	(Q_Ent)->EntPrefix##Fwd = *(Q_Hd);				/* fwd is Hd */			\
	*(Q_Hd) = (Q_Ent);								/* set Hd */			\
	if ((Q_Ent)->EntPrefix##Fwd)					/* prev top exists */	\
		(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = (Q_Ent);	/* insert */	\
}

/****************************************************************************/
/*  insert before entry for single head list								*/
/****************************************************************************/
#define grp_enque_sprev(Q_Hd, Q_At, Q_Ent, EntPrefix) {						\
	(Q_Ent)->EntPrefix##Fwd = (Q_At);				/* fwd is At */			\
	(Q_Ent)->EntPrefix##Bwd = (Q_At)->EntPrefix##Bwd;/* bwd is At bwd */	\
	(Q_At)->EntPrefix##Bwd = (Q_Ent);				/* set At bwd */		\
	if ((Q_Ent)->EntPrefix##Bwd)					/* prev bwd exists */	\
		(Q_Ent)->EntPrefix##Bwd->EntPrefix##Fwd = (Q_Ent);	/* insert */	\
	else											/* prev bwd exits */	\
		*(Q_Hd) = (Q_Ent);							/* set Hd */			\
}

/****************************************************************************/
/*  insert after entry for single head list									*/
/****************************************************************************/
#define grp_enque_snext(Q_Hd, Q_At, Q_Ent, EntPrefix) {						\
	(Q_Ent)->EntPrefix##Fwd = (Q_At)->EntPrefix##Fwd;/* fwd is At fwd */	\
	(Q_Ent)->EntPrefix##Bwd = (Q_At);				/* bwd is At */			\
	(Q_At)->EntPrefix##Fwd = (Q_Ent);				/* set At fwd */		\
	if ((Q_Ent)->EntPrefix##Fwd)					/* prev fwd exists */	\
		(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = (Q_Ent);	/* insert */	\
}

/****************************************************************************/
/*  dequeue from single queue head 											*/
/****************************************************************************/
#define grp_deque_shead(Q_Hd, Q_Ent, EntPrefix) {							\
	if (*(Q_Hd)) {									/* Ent exists */		\
		(Q_Ent) = *(Q_Hd);							/* Ent is Hd  */		\
		*(Q_Hd) = (Q_Ent)->EntPrefix##Fwd;			/* deque from top */\
		if ((Q_Ent)->EntPrefix##Fwd)				/* next ent exists */	\
			(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = NULL; /* deque */		\
		(Q_Ent)->EntPrefix##Fwd = (Q_Ent)->EntPrefix##Bwd = NULL; 			\
	} else {										/* no Ent */			\
		(Q_Ent) = NULL;								/* set NULL */			\
	}																		\
}

/****************************************************************************/
/*  dequeue entry from single head list										*/
/****************************************************************************/
#define grp_deque_sent(Q_Hd, Q_Ent, EntPrefix)	{							\
	if ((Q_Ent)->EntPrefix##Fwd)					/* fwd entry exists */	\
		(Q_Ent)->EntPrefix##Fwd->EntPrefix##Bwd = 							\
					(Q_Ent)->EntPrefix##Bwd;		/* deque from fwd */	\
	if ((Q_Ent)->EntPrefix##Bwd)					/* bwd entry exists */	\
		(Q_Ent)->EntPrefix##Bwd->EntPrefix##Fwd = 							\
					(Q_Ent)->EntPrefix##Fwd;		/* deque from bwd */	\
	else											/* no bwd entry */		\
		*(Q_Hd) = (Q_Ent)->EntPrefix##Fwd;			/* set Hd fwd */		\
	(Q_Ent)->EntPrefix##Fwd = NULL;					/* clear link */		\
	(Q_Ent)->EntPrefix##Bwd = NULL;					/* clear link */		\
}
#endif	/* _GRP_QUEUE_H_ */
