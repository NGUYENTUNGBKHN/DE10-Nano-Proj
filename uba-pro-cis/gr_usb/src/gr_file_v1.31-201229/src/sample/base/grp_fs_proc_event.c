/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_proc_event.c											*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Process insert/eject event											*/
/* FUNCTIONS:																*/
/*		grp_fs_proc_event			process insert/eject event				*/
/* DEPENDENCIES:															*/
/*		grp_stdio.h					if defined(GRP_STDIO)					*/
/*		<stdio.h>					if !defined(GRP_STDIO)					*/
/*		<string.h>															*/
/*		grp_fs.h															*/
/*		grp_fs_proc_event.h													*/
/*		None																*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/09/01	Created inital version 1.0				*/
/*		T.Imashiki		2006/08/28	Fixed missing parameter to _grp_fs_get	*/
/*									_response at _grp_fs_exec_unmount		*/
/*		T.Imashiki		2006/10/18	Make retrieval of error data in ifdef	*/
/*									option GRP_FS_SAVE_ERROR_DATA			*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		K.Kaneko		2010/11/17	Added GRP_FS_MINIMIZE_LEVEL option for	*/
/*									GR-FILE minimize level					*/
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
#if(GRP_FS_MINIMIZE_LEVEL < 1)
#ifdef	GRP_STDIO
#include "grp_stdio.h"
#undef	getchar
#define	getchar()		cons_getchar(1)
extern int cons_getchar(int);
#else	/* GRP_STDIO */
#include <stdio.h>
#endif	/* GRP_STDIO */
#include <string.h>
#include "grp_fs.h"
#include "grp_fs_proc_event.h"

grp_fs_mnt_def_t _grp_fs_mnt_def[] = {		/* mount definition table */
	{ "uda0",	"/",	GRP_FS_SYNC_FL_CLOSE },			/* USB 0 */
	{ "fd0",	"/",	GRP_FS_SYNC_FL_CLOSE },			/* fd 0 */
	{ "usb0",	"/",	GRP_FS_SYNC_FL_CLOSE },			/* USB 0 */
	{ NULL }
};

/****************************************************************************/
/* FUNCTION:	_grp_fs_lookup_mnt_def										*/
/*																			*/
/* DESCRIPTION:	Lookup mount definition										*/
/* INPUT:		pcDev:				device name 							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		NULL:				not found								*/
/*				others:				mount definition						*/
/*																			*/
/****************************************************************************/
static grp_fs_mnt_def_t *
_grp_fs_lookup_mnt_def(
	const char			*pcDev)				/* [IN]  devince name */
{
	grp_fs_mnt_def_t	*ptMntDef;			/* mount definition info */

	for (ptMntDef = _grp_fs_mnt_def; ptMntDef->pcDev; ptMntDef++) {
		if (strcmp(ptMntDef->pcDev, pcDev) == 0) /* found matched entry */
			return(ptMntDef);				/* return the entry */
	}
	return(NULL);							/* return not found */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_output_msg											*/
/*																			*/
/* DESCRIPTION:	Output message												*/
/* INPUT:		pcMsg:				message									*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		length of message											*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_output_msg(
	const char			*pcMsg,				/* message to output */
	...)									/* parameters */
{
	va_list				vap;				/* argument list */
	int					iLen;				/* message length */

	/****************************************************/
	/* output message code here (XXX)					*/
	/****************************************************/
	va_start(vap, pcMsg);					/* set argument list */
	iLen = vfprintf(stdout, pcMsg, vap);	/* output message */
	fflush(stdout);							/* flush output */
	va_end(vap);							/* end argument list */
	return(iLen);							/* return length */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_get_response										*/
/*																			*/
/* DESCRIPTION:	Output message and get response								*/
/* INPUT:		pcMsg:				message									*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					response not 'y'						*/
/*				1:					response 'y'							*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_get_response(
	const char			*pcMsg,				/* message to output */
	...)									/* parameters */
{
	int					iResponse;			/* response */
	int					iChar;				/* character */
	va_list				vap;				/* argument list */

	/****************************************************/
	/* output message code here (XXX)					*/
	/****************************************************/
	va_start(vap, pcMsg);					/* set argument list */
	vfprintf(stdout, pcMsg, vap);			/* output message */
	fflush(stdout);							/* flush output */
	va_end(vap);							/* end argument list */

	/****************************************************/
	/* get response code here (XXX)						*/
	/****************************************************/
	iResponse = getchar();					/* get response */
	for (iChar = iResponse; iChar != '\n' && iChar != EOF; iChar = getchar());
	return((iResponse == (int)'y')? 1: 0);	/* return response */
}

#ifdef	GRP_FS_SAVE_ERROR_DATA
/****************************************************************************/
/* FUNCTION:	_grp_check_saved_cache										*/
/*																			*/
/* DESCRIPTION:	check saved cache, and restore it if necessary				*/
/* INPUT:		pcDev:				device name 							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				others:				error code								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_check_saved_cache(
	const char			*pcDev)				/* [IN]  devince name */
{
	/****************************************************/
	/* check saved cache code here (XXX)				*/
	/*		check using grp_fs_check_volume(			*/
	/*				pcDev, "fat",			 			*/
	/*				aucVolName, &uiVolLen, &uiVolSer);	*/
	/****************************************************/
	return(0);								/* XXX */
}
#endif	/* GRP_FS_SAVE_ERROR_DATA */

/****************************************************************************/
/* FUNCTION:	_grp_fs_exec_mount											*/
/*																			*/
/* DESCRIPTION:	Execute mount												*/
/* INPUT:		pcDev:				device name 							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				others:				error code								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_exec_mount(
	const char			*pcDev)				/* [IN]  devince name */
{
	int					iRet;				/* return value */
	int					iRes;				/* response */
	grp_fs_mnt_def_t	*ptMntDef;			/* mount definition info */
	char				acMsgBuf[32];		/* message buffer */

#ifdef	GRP_FS_SAVE_ERROR_DATA
	iRet = _grp_fs_check_saved_cache(pcDev); /* check saved cache image */
	if (iRet != 0)							/* error detected */
		return(iRet);						/* return error */
#endif	/* GRP_FS_SAVE_ERROR_DATA */
	ptMntDef = _grp_fs_lookup_mnt_def(pcDev); /* get mount definition */
	if (ptMntDef == NULL) {					/* no mount definition */
		_grp_fs_output_msg("no mount definition for \"%s\"\n", pcDev);
		return(GRP_FS_ERR_BAD_DEV);			/* return error */
	}
	iRet = grp_fs_mount(pcDev, (grp_uchar_t *)ptMntDef->pcMntPoint,
					"fat", ptMntDef->iMntMode); /* mount device */
	if (iRet == GRP_FS_ERR_NEED_CHECK) {	/* need file system check */
		iRes = _grp_fs_get_response(
					"need file system check: force to mount(y/n)? ");
		if (iRes > 0) 						/* requested force mount */
			iRet = grp_fs_mount(pcDev, (grp_uchar_t *)ptMntDef->pcMntPoint,
								"fat", ptMntDef->iMntMode|GRP_FS_FORCE_MOUNT);
											/* force to mount device */
	}
	if (iRet != 0) {						/* error occured */
		_grp_fs_output_msg("failed to mount \"%s\": %s\n", pcDev,
							grp_fs_err(iRet, acMsgBuf)); /* output error msg */
	} else {								/* mount succeeded */
		_grp_fs_output_msg("mounted \"%s\" on \"%s\"\n",
							pcDev, ptMntDef->pcMntPoint);/* output success */
	}
	return(iRet);							/* return status */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_proc_bad_media										*/
/*																			*/
/* DESCRIPTION:	Process insertion of incorrect media						*/
/* INPUT:		ptMntInfo:			mount information						*/
/*				pucVolName:			inserted volume name					*/
/*				uiVolSer:			insreted volume serial number			*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				others:				error code								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_proc_bad_media(
	grp_fs_mnt_info_t	*ptMntInfo,			/* [IN]  mount information */
	grp_uchar_t			*pucVolName,		/* [IN]  inserted volume name */
	grp_uint32_t		uiVolSer)			/* [IN]  inserted serial number */
{
	int					iRes;				/* response */
	int					iRet;				/* return value */
	char				acMsgBuf[32];		/* message buffer */
#ifdef	GRP_FS_SAVE_ERROR_DATA
	grp_int32_t			iSize;				/* size of read data */
	grp_uint32_t		uiNeed;				/* need size */
	grp_uchar_t			aucBuf[GRP_FS_SAVE_BUF_SIZE]; /* save buffer size */
#endif	/* GRP_FS_SAVE_ERROR_DATA */

	_grp_fs_output_msg("inserted incorrect media on \"%s\"\n",
						ptMntInfo->acDevName);
	_grp_fs_output_msg("old one: %s(0x%08x)  new one: %s(0x%08x)\n",
						ptMntInfo->aucVolName, ptMntInfo->uiVolSerNo,
						pucVolName, uiVolSer);
	iRes = _grp_fs_get_response("force mount(y/n)? ");
	if (iRes == 0)							/* not request force mount */
		return(GRP_FS_ERR_NEED_CHECK);		/* return error */

#ifdef	GRP_FS_SAVE_ERROR_DATA
	/****************************************************/
	/* save cache code here (XXX)						*/
	/****************************************************/
	while ((iSize = grp_fs_get_error(
				GRP_FS_GE_CONTENT|GRP_FS_GE_RELEASE
				|GRP_FS_GE_DIRTY|GRP_FS_GE_FBUF|GRP_FS_GE_DBUF,
				ptMntInfo->iDev, GRP_FS_GE_BLK_ANY,
				aucBuf, sizeof(aucBuf), &uiNeed)) > 0) {
		/****************************************************/
		/* save read cache code here (XXX)					*/
		/****************************************************/
		;									/* XXX */
	}
#endif	/* GRP_FS_SAVE_ERROR_DATA */

	/****************************************************/
	/* force to unmount									*/
	/****************************************************/
	iRet = grp_fs_unmount(ptMntInfo->acDevName, GRP_FS_FORCE_UMOUNT);
	if (iRet != 0) {						/* failed to unmount */
		_grp_fs_output_msg("failed to unmount \"%s\": %s\n",
							ptMntInfo->acDevName,
							grp_fs_err(iRet, acMsgBuf)); /* output error msg */
		return(iRet);						/* return error */
	}
	_grp_fs_output_msg("unmounted \"%s\"\n", ptMntInfo->acDevName);
											/* output success */
	return(_grp_fs_exec_mount(ptMntInfo->acDevName));/* execute mount */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_check_ins											*/
/*																			*/
/* DESCRIPTION:	Check inserted media										*/
/* INPUT:		ptMntInfo:			mount information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				others:				error code								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_check_ins(
	grp_fs_mnt_info_t	*ptMntInfo)			/* [IN]  mount information */
{
	int					iRet;				/* return value */
	grp_uchar_t			aucVolName[GRP_FS_VOL_NAME_LEN]; /* volume name */
	int					iVolNameLen = sizeof(aucVolName);/* volume name len */
	grp_uint32_t		uiVolSer;			/* serial number */
	char				acMsgBuf[32];		/* message buffer */

	iRet = grp_fs_check_fs_dev(ptMntInfo->acDevName, 
								aucVolName, &iVolNameLen, &uiVolSer);
											/* check inserted media */
	if (iRet == 0) {						/* inserted correct media */
		_grp_fs_output_msg("restart I/O on \"%s\"\n", ptMntInfo->acDevName);
											/* output restart I/O message */
		return(iRet);						/* return seccess */
	}
	if (iRet == GRP_FS_ERR_NEED_CHECK) {	/* inserted incorrect media */
		return(_grp_fs_proc_bad_media(ptMntInfo, aucVolName, uiVolSer));
											/* process incorrect media */
	}
	_grp_fs_output_msg("failed to check media on \"%s\": %s\n", 
						ptMntInfo->acDevName,
						grp_fs_err(iRet, acMsgBuf)); /* output error msg */
	return(iRet);							/* return error */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_warn_eject											*/
/*																			*/
/* DESCRIPTION:	Output warning message for improper eject operation,		*/
/*				and temporarily invalidate I/O for the device				*/
/* INPUT:		ptMntInfo:			mount information						*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_warn_eject(
	grp_fs_mnt_info_t	*ptMntInfo)			/* [IN]  mount information */
{
	_grp_fs_output_msg(
		"\"%s\"(VolNo:0x%08x) is ejected without unmount; insert again\n",
		ptMntInfo->acDevName, ptMntInfo->uiVolSerNo);/* output warning msg */
	grp_fs_invalidate_fs_dev(ptMntInfo->acDevName);	/* invalidate device */
	return(0);								/* return */
}

/****************************************************************************/
/* FUNCTION:	_grp_fs_exec_unmount										*/
/*																			*/
/* DESCRIPTION:	Execute unmount 											*/
/* INPUT:		pcDev:				device name 							*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				others:				error code								*/
/*																			*/
/****************************************************************************/
static int
_grp_fs_exec_unmount(
	const char			*pcDev)				/* [IN]  devince name */
{
	int					iRet;				/* return value */
	int					iRes;				/* response */
	char				acMsgBuf[32];		/* message buffer */

	iRet = grp_fs_unmount(pcDev, 0);		/* execute unmount */
	if (iRet == GRP_FS_ERR_BUSY) {			/* still busy */
		iRes = _grp_fs_get_response(
					"\"%s\" is busy: force to unmount(y/n)? ", pcDev);
		if (iRes > 0) 						/* requested force unmount */
			iRet = grp_fs_unmount(pcDev, GRP_FS_FORCE_UMOUNT);
											/* force to unmount device */
	}
	if (iRet != 0) {						/* error occured */
		_grp_fs_output_msg("failed to unmount \"%s\": %s\n", pcDev,
							grp_fs_err(iRet, acMsgBuf)); /* output error msg */
	} else {								/* unmount succeeded */
		_grp_fs_output_msg("unmounted \"%s\"\n", pcDev); /* output success */
	}
	return(iRet);							/* return status */
}

/****************************************************************************/
/* FUNCTION:	grp_fs_proc_event											*/
/*																			*/
/* DESCRIPTION:	Process insert/eject event									*/
/* INPUT:		pcDev:				device name 							*/
/*				iEvent:				event type								*/
/*										GRP_FS_EVENT_INSERT:  insert event	*/
/*										GRP_FS_EVENT_EJECT:   eject event	*/
/*										GRP_FS_EVENT_UNMOUNT: unmount req	*/
/* OUTPUT:		None														*/
/*																			*/
/* RESULT:		0:					success									*/
/*				others:				error code								*/
/*																			*/
/****************************************************************************/
int
grp_fs_proc_event(
	const char			*pcDev,				/* [IN]  devince name */
	int					iEvent)				/* [IN]  event type */
{
	int					iDev;				/* device number */
	int					iRet;				/* return value */
	grp_fs_mnt_info_t	tMntInfo;			/* mount information */

	iDev = grp_fs_lookup_dev(pcDev);		/* lookup device */
	if (iDev < 0) 							/* not found */
		return(iDev);						/* return error */
	iRet = grp_fs_get_mnt_by_dev(iDev, &tMntInfo);/* check mounted */
	switch(iEvent) {						/* check event */
	case GRP_FS_EVENT_INSERT:				/* insert event */
		if (iRet == GRP_FS_ERR_BAD_DEV)		/* not mounted */
			return(_grp_fs_exec_mount(pcDev));/* execute mount */
		else if (iRet == 0)					/* already mounted */
			return(_grp_fs_check_ins(&tMntInfo)); /* check inserted media */
		return(iRet);						/* return error */
	case GRP_FS_EVENT_EJECT:				/* eject event */
		if (iRet == 0 						/* mounted device */
			&& (tMntInfo.uiStatus & GRP_FS_MSTAT_DEV_INV) == 0) /* valid */
			return(_grp_fs_warn_eject(&tMntInfo));/* warn bad eject */
		return(0);							/* do nothing for not mounted dev */
	case GRP_FS_EVENT_UNMOUNT:				/* umount request */
		return(_grp_fs_exec_unmount(pcDev));/* execute unmount */
	default:								/* other event */
		return(GRP_FS_ERR_BAD_PARAM);		/* return error */
	}
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
