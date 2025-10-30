#ifndef _GRUSB_TEST_
#define _GRUSB_TEST_
/*************************************************************************/
/*                                                                       */
/* Copyright(C) 2002-2010 Grape Systems, Inc.                            */
/*                        All Rights Reserved                            */
/*                                                                       */
/* This software is furnished under a license and may be used and copied */
/* only in accordance with the terms of such license and with the        */
/* inclusion of the above copyright notice. No title to and ownership of */
/* the software is transferred.                                          */
/* Grape Systems Inc. makes no representation or warranties with respect */
/* to the performance of this computer program, and specifically         */
/* disclaims any responsibility for any damages, special or              */
/* consequential, connected with the use of this program.                */
/*                                                                       */
/*************************************************************************/
/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      usb_test.h                                         1.03          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      GR-USB/HostII Sample program                                     */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*   NAME         DATE        REMARKS                                    */
/*                                                                       */
/*   S.Tamaki     2003/09/11  Created initial version 1.00               */
/*   K.Kaneko     2008/06/23  Version 1.02                               */
/*   K.Kaneko     2010/11/17  Version 1.03                               */
/*                            Added GRP_FS_MINIMIZE_LEVEL option for     */
/*                            GR-FILE minimize level                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/* defines                                                               */
/*************************************************************************/
/* error code */
#define     GRUSB_TEST_OK           0
#define     GRUSB_TEST_ERROR        -1


/*************************************************************************/
/* exported functions                                                    */
/*************************************************************************/
test_cmd_func_t test_cmd_help;              /* help command */


/*************************************************************************/
/* prototypes                                                            */
/*************************************************************************/
test_cmd_t * 	lookup_cmd(unsigned char *cmd);
void			test_print_dir(grp_fs_dir_ent_t *dir, int long_list);
void			test_cmd_print_time(const char *name, grp_int32_t itime);

#if(GRP_FS_MINIMIZE_LEVEL < 1)
grp_isize_t		test_stdin(FILE *fp, grp_uchar_t *buf, grp_isize_t size);
grp_isize_t		test_stdout(FILE *fp, grp_uchar_t *buf, grp_isize_t size);
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
grp_isize_t 	test_stdout(grp_uchar_t *buf, grp_isize_t size);
grp_isize_t 	test_stdin(grp_uchar_t *buf, grp_isize_t size);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */


#endif	/* _GRUSB_TEST_ */
