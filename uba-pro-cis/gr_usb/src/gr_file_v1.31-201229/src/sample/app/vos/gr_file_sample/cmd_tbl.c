/*************************************************************************/
/*                                                                       */
/* Copyright(C) 2002-2016 Grape Systems, Inc.                            */
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
/*      usb_test.c                                         0.04          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      test command table                                               */
/*                                                                       */
/* FUNCTIONS:                                                            */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*   NAME         DATE        REMARKS                                    */
/*                                                                       */
/*   K.Kaneko     2008/06/23  Version 0.02                               */
/*   K.Kaneko     2010/11/17  Version 0.03                               */
/*                            Added GRP_FS_MINIMIZE_LEVEL option for     */
/*                            GR-FILE minimize level                     */
/*   K.Kaneko     2016/03/17  Version 0.04                               */
/*                            Supported compile option                   */
/*                            GRP_FS_ENABLE_OVER_2G                      */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/* includes                                                              */
/*************************************************************************/
#include "gr_types.h"
#include "grp_vos.h"
#include "grusbtyp.h"
#include "grp_fs_sysdef.h"
#include "fat.h"
#include "grp_stdio.h"

#include "cmd.h"
#include "usb_test.h"

/*************************************************************************/
/* globals                                                               */
/*************************************************************************/
/* user interface commands */
test_cmd_func_t	test_cmd_mount;				/* mount */
test_cmd_func_t	test_cmd_umount;			/* umount */
test_cmd_func_t	test_cmd_ls;				/* ls */
test_cmd_func_t	test_cmd_cd;				/* cd */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
test_cmd_func_t	test_cmd_stat;				/* get status */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
test_cmd_func_t	test_cmd_mkdir;				/* create directory */
test_cmd_func_t	test_cmd_rm;				/* rm */
test_cmd_func_t	test_cmd_rename;			/* rename */
test_cmd_func_t	test_cmd_cp;				/* copy file */
test_cmd_func_t	test_cmd_cat;				/* cat file */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
test_cmd_func_t	test_cmd_pwd;				/* print current working dir */
test_cmd_func_t	test_cmd_chmod;				/* change file mode */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
test_cmd_func_t	test_cmd_sync;				/* sync */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
test_cmd_func_t	test_cmd_inv_task;			/* invalidat task */
test_cmd_func_t	test_cmd_get_mnt;			/* get mount info */
test_cmd_func_t	test_cmd_check_vol;			/* check volume */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
test_cmd_func_t	test_cmd_date;				/* set/get date and time */
test_cmd_func_t	test_cmd_format;			/* format media */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
test_cmd_func_t	test_cmd_part;				/* get/set partition */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
test_cmd_func_t	test_cmd_ioctl;				/* device ioctl */
test_cmd_func_t	test_cmd_dev_io;			/* device I/O */

/* program interface commands */
test_cmd_func_t	test_cmd_open;				/* open */
test_cmd_func_t	test_cmd_close;				/* close */
test_cmd_func_t	test_cmd_read;				/* read */
test_cmd_func_t	test_cmd_write;				/* write */
test_cmd_func_t	test_cmd_seek;				/* seek */
#ifdef GRP_FS_ENABLE_OVER_2G
test_cmd_func_t	test_cmd_seek4G;			/* seek4G */
#endif /* GRP_FS_ENABLE_OVER_2G */
test_cmd_func_t	test_cmd_create;			/* create */
test_cmd_func_t	test_cmd_geta;				/* geta */
test_cmd_func_t	test_cmd_seta;				/* seta */
test_cmd_func_t	test_cmd_trunc;				/* trunc */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
test_cmd_func_t	test_cmd_utime;				/* change access/mod time */
test_cmd_func_t	test_cmd_inv_fs;			/* invalidate fs device */
test_cmd_func_t	test_cmd_check_fs;			/* check fs device */
test_cmd_func_t	test_cmd_get_err;			/* get error data */
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/* command table */
test_cmd_t	test_cmd_tbl[] = {
	/*************************************/
	/* user interface commands			 */
	/*************************************/
	/* е1ееее+ееее2ееее+ееее3ееее+ееее4ееее+ееее5ееее+ееее6ееее+ееее7ееее+ееее8 */
	{ "help",	test_cmd_help,				/* help command */
	  "[cmd_name]                                --- help command",
	  "" },
	{ "mount",	test_cmd_mount,				/* mount */
	  "[-rufs012MATt [type]] dev mnt             --- mount",
	  "" },
	{ "umount",	test_cmd_umount,			/* unmount */
	  "[-f] dev                                  --- unmount",
	  "" },
	{ "ls",		test_cmd_ls,				/* list directory */
	  "[-l] [file_name]                          --- list directory",
	  "" },
	{ "cd",		test_cmd_cd,				/* chdir */
	  "dir                                       --- dir file",
	  "" },
#if(GRP_FS_MINIMIZE_LEVEL < 1)
	{ "stat",	test_cmd_stat,				/* get status */
	  "path                                      --- get status",
	  "" },
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
	{ "mkdir",	test_cmd_mkdir,				/* create directory */
	  "dir [prot]                                --- create directory",
	  "" },
	{ "rm",		test_cmd_rm,				/* rm */
	  "file                                      --- delete file",
	  "" },
	{ "rename",	test_cmd_rename,			/* rename */
	  "old new                                   --- rename file",
	  "" },
	{ "cp",		test_cmd_cp,				/* copy file */
	  "[-d buf_size] src dst                     --- copy file",
	  "" },
	{ "cat",	test_cmd_cat,				/* cat file */
	  "file                                      --- cat file",
	  "" },
#if(GRP_FS_MINIMIZE_LEVEL < 1)
	{ "pwd",	test_cmd_pwd,				/* print current dir */
	  "                                          --- print currnet directory",
	  "" },
	{ "chmod",	test_cmd_chmod,				/* change mode */
	  "mode file                                 --- change mode",
	  "" },
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
	{ "sync",	test_cmd_sync,				/* sync */
	  "mode                                      --- sync I/O",
	  "" },
#if(GRP_FS_MINIMIZE_LEVEL < 1)
	{ "inv",	test_cmd_inv_task,			/* invalidate task */
	  "[-a]                                      --- invalidat task",
	  "" },
	{ "fstat", test_cmd_get_mnt,			/* get status */
	  "[-n dev_name] [-i dev_num]                --- get FS status",
	  "" },
	{ "chkvol", test_cmd_check_vol,			/* check volume */
	  "dev_name fs_type                          --- check volume",
	  "" },
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
	{ "date",	test_cmd_date,				/* get/set date and time */
	  "[-g] [-s YYYY DD MM dd mm ss]             --- get/set date and time",
	  "" },
	{ "format",	test_cmd_format,			/* format media */
	  "dev -f type -v vol -c clst -d root -r rsv --- format media\n"
	  "\t-m media -t total_size -s sector -h head \n"
	  "\t-k trk -a align -o option -S sec_shift",
	  "" },
#if(GRP_FS_MINIMIZE_LEVEL < 1)
	{ "part",	test_cmd_part,				/* get/set partition */
	  "[-c] [-wWg dev]                           --- get/set partition\n"
	  "\t[-s num active type s_c s_h s_s e_c e_h e_s start sec_cnt]",
	  "" },
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
	{ "ioctl",	test_cmd_ioctl,				/* device ioctl */
	  "dev cmd param                             --- device ioctl",
	  "" },
	{ "dev_io",	test_cmd_dev_io,			/* device I/O */
	  "dev off cnt                               --- device I/O",
	  "" },
	/* е1ееее+ееее2ееее+ееее3ееее+ееее4ееее+ееее5ееее+ееее6ееее+ееее7ееее+ееее8 */

	/*************************************/
	/* program interface commands		 */
	/*************************************/
	/* е1ееее+ееее2ееее+ееее3ееее+ееее4ееее+ееее5ееее+ееее6ееее+ееее7ееее+ееее8 */
	{ "open",	test_cmd_open,				/* open file */
	  "file mode prot                            --- open file",
	  "" },
	{ "close",	test_cmd_close,				/* close file */
	  "fd                                        --- close file",
	  "" },
	{ "read",	test_cmd_read,				/* read file */
	  "fd size [save]                            --- read file",
	  "" },
	{ "write",	test_cmd_write,				/* write file */
	  "fd size file                              --- write file",
	  "" },
	{ "seek",	test_cmd_seek,				/* seek file */
	  "fd offset mode                            --- seek file",
	  "" },
#ifdef GRP_FS_ENABLE_OVER_2G
	{ "seek4G",	test_cmd_seek4G,			/* seek file */
	  "fd offset mode                            --- seek file",
	  "" },
#endif /* GRP_FS_ENABLE_OVER_2G */
	{ "create",	test_cmd_create,			/* create file */
	  "file type prot attr                       --- create file",
	  "" },
	{ "geta",	test_cmd_geta,				/* get attribute */
	  "file                                      --- get attribute",
	  "" },
	{ "seta",	test_cmd_seta,				/* set attribute */
	  "file prot attr ct mt at                   --- set attribute",
	  "" },
	{ "trunc",	test_cmd_trunc,				/* truncate */
	  "fd off                                    --- truncate file",
	  "" },
#if(GRP_FS_MINIMIZE_LEVEL < 1)
	{ "utime",	test_cmd_utime,				/* change time */
	  "file atime mtime                          --- change access/mod time",
	  "" },
	{ "invfs", test_cmd_inv_fs,             /* invalidate FS device */
	  "dev_name                                  --- invalidate FS device",
	  "" },
	{ "chkfs", test_cmd_check_fs,			/* check FS device */
	  "dev_name                                  --- check FS device",
	  "" },
	{ "geterr", test_cmd_get_err,			/* get error data */
	  "mode dev blk                              --- get error data",
	  "" },
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
	{ NULL }
	/* е1ееее+ееее2ееее+ееее3ееее+ееее4ееее+ееее5ееее+ееее6ееее+ееее7ееее+ееее8 */
};
