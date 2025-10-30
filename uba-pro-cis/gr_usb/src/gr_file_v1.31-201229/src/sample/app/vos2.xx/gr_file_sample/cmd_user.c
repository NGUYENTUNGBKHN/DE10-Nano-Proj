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
/*      cmd_user.c                                         2.04          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      GR-FILE USER I/F Sample program for GR-USB/HOST#                 */
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
/*   K.Takagi     2009/03/24  Version 2.00                               */
/*                            不要なヘッダファイルを削除                 */
/*   K.Kaneko     2010/11/17  Version 2.03                               */
/*                            Added GRP_FS_MINIMIZE_LEVEL option for     */
/*                            GR-FILE minimize level                     */
/*   K.Kaneko     2016/03/17  Version 2.04                               */
/*                            Supported compile option                   */
/*                            GRP_FS_ENABLE_OVER_2G                      */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/* includes                                                              */
/*************************************************************************/
#include <stdarg.h>
#include <string.h>

#include "grp_vos.h"

#include "fat.h"
#include "grp_fs_if.h"
#include "grp_fs_conv.h"
#include "grp_time.h"
#include "grp_fs_disk_part.h"
#include "grp_fat_format.h"
#include "grp_fs_proc_event.h"
#include "grp_fs_readdir.h"
#include "grp_stdio.h"

#include "cmd.h"
#include "usb_test.h"


/*************************************************************************/
/* defines                                                               */
/*************************************************************************/
#define IO_BUF_SZ       1024            /* I/O buffer size */


/*************************************************************************/
/* globals                                                               */
/*************************************************************************/
grp_fs_dk_part_t    part_tbl[GRP_FS_PART_CNT];

/*************************************************************************/
/* FUNCTION   : test_cmd_mount                                           */
/*                                                                       */
/* DESCRIPTION: mount command                                            */
/*************************************************************************/
int test_cmd_mount(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int mode = 0;
    char *fs_type = "fat";
    int ret;
    unsigned char *p;
    int arg_start = 1;

    if (ac > 1 && av[1][0] == '-') {
        arg_start++;
        for (p = &av[1][1]; *p; p++) {
            switch(*p) {
            case 'r':   /* read only */
                mode |= GRP_FS_RONLY;
                break;
            case 'u':   /* read only */
                mode |= GRP_FS_NO_UPD_ACCTIME;
                break;
            case 'f':   /* force mount */
                mode |= GRP_FS_FORCE_MOUNT;
                break;
            case 's':   /* sync mode */
                switch(*++p) {
                case '0':
                    mode |= GRP_FS_SYNC_ALL;
                    break;  
                case '1':
                    mode |= GRP_FS_SYNC_FL_CLOSE;
                    break;
                case '2':
                    mode |= GRP_FS_SYNC_FS_CLOSE;
                    break;
                default:
                    goto usage;
                }
                break;
            case 't':
                if (ac != 5)
                    goto usage;
                fs_type = (char *)av[2];
                arg_start++;
                break;
            default:
                goto usage;
            }
        }
    }
    if (ac != arg_start + 2)
        goto usage;
    ret = grp_fs_mount((char *)av[arg_start], av[arg_start + 1], 
                fs_type, mode);
    if (ret != 0)
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("mount error: %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("mount error: %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_umount                                          */
/*                                                                       */
/* DESCRIPTION: umount command                                           */
/*************************************************************************/
int test_cmd_umount(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int mode = 0;
    unsigned char *dev;
    int ret;

    if (ac == 3) {
        if (av[1][0] != '-' || av[1][1] != 'f')
            goto usage;
        mode = GRP_FS_FORCE_UMOUNT;
        dev = av[2];
    } else if (ac == 2) {
        dev = av[1];
    } else
        goto usage;
    ret = grp_fs_unmount((char *)dev, mode);
    if (ret != 0){
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("umount error: %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("umount error: %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    } else {
        grp_fs_printf("umount ok\n");
    }
    return(ret);

usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_ls                                              */
/*                                                                       */
/* DESCRIPTION: list command                                             */
/*************************************************************************/
int test_cmd_ls(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int fhdl;
    int ret;
    int long_list = 0;
    int file_arg = 1;
    grp_fs_dir_ent_t dirent;
    grp_uchar_t     name[GRP_FS_MAX_PATH];

    if (ac > 1) {
        if (strcmp((char *)av[1], "-l") == 0) {
            long_list = 1;
            file_arg++;
        }
    }
    if (ac != file_arg && ac != file_arg + 1) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (ac == file_arg + 1) {
        ret = grp_fs_get_attr(av[file_arg], &dirent);
        if (ret < 0)
            goto out;
        if (dirent.ucType != GRP_FS_FILE_DIR) {
            dirent.pucName = av[file_arg];
            test_print_dir(&dirent, long_list);
            goto out;
        }
        if ((ret = grp_fs_open(av[file_arg], GRP_FS_O_RDONLY, 0)) < 0)
            goto out;
        fhdl = ret;
    } else {
        fhdl = -1;
    }
    dirent.pucName = name;          /* set file name buffer */
    dirent.sNameSize = sizeof(name);/* set name buffer size */
    dirent.uiStart = 0;             /* start offset is 0 */
    dirent.uiEnd = 0;                   /* end offset is 0 */
    while ((ret = grp_fs_get_dirent(fhdl, &dirent)) > 0) {
        test_print_dir(&dirent, long_list);
        dirent.sNameSize = sizeof(name);/* set name buffer size */
        dirent.uiStart = dirent.uiEnd;  /* set next */
    }
    if (fhdl >= 0)
        grp_fs_close(fhdl);
out:
    if (ret != 0)
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("ls error: %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("ls error: %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_cd                                              */
/*                                                                       */
/* DESCRIPTION: change directory command                                 */
/*************************************************************************/
int test_cmd_cd(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret = 0;

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_chdir(av[1]);
    if (ret != 0)
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("cd error: %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("cd error: %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_stat                                            */
/*                                                                       */
/* DESCRIPTION: stat command                                             */
/*************************************************************************/
int test_cmd_stat(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret = 0;
    struct stat st;

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_stat(av[1], &st);
    if (ret != 0){
        grp_fs_printf("stat error: %s\n", grp_fs_err(ret, NULL));
    } else {
#ifdef GRP_FS_ENABLE_OVER_2G
        grp_fs_printf("DevNo.:%d Fid:%d Mode:%08x Size:%u\n", st.st_dev, st.st_ino, st.st_mode, st.st_size);
#else  /* GRP_FS_ENABLE_OVER_2G */
        grp_fs_printf("DevNo.:%d Fid:%d Mode:%08x Size:%d\n", st.st_dev, st.st_ino, st.st_mode, st.st_size);
#endif /* GRP_FS_ENABLE_OVER_2G */
    }
    return(ret);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/*************************************************************************/
/* FUNCTION   : test_cmd_mkdir                                           */
/*                                                                       */
/* DESCRIPTION: create directory command                                 */
/*************************************************************************/
int test_cmd_mkdir(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int prot = GRP_FS_PROT_RWXA;
    int ret;

    if (ac != 2 && ac != 3) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (ac == 3 && test_cmd_eval_arg(av[2], &prot)) {
        grp_fs_printf("%s: invalid prot %s\n", av[0], av[2]);
        return(-1);
    }
    ret = grp_fs_create(av[1], GRP_FS_FILE_DIR, prot, 0);
    if (ret != 0)
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("mkdir error: %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("mkdir error: %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_rm                                              */
/*                                                                       */
/* DESCRIPTION: remove file/directory command                            */
/*************************************************************************/
int test_cmd_rm(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_unlink(av[1]);
    if (ret != 0)
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("rm error: %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("rm error: %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_rename                                          */
/*                                                                       */
/* DESCRIPTION: rename command                                           */
/*************************************************************************/
int test_cmd_rename(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;

    if (ac != 3) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_rename(av[1], av[2]);
    if (ret != 0)
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("rename error: %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("rename error: %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_cp                                              */
/*                                                                       */
/* DESCRIPTION: cp command                                               */
/*************************************************************************/
int test_cmd_cp(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ifd, ofd;
    int buf_size = 0;
    int file_arg = 1;
    int direct_mode = 0;
    grp_uchar_t *buf;
    int r_cnt = 0;
    int w_cnt = 0;
    int ret = -1;
    int n;

    if (ac < 3)
        goto usage;
    if (strcmp((char *)av[1], "-d") == 0) {
        if (ac != 5)
            goto usage;
        if (test_cmd_eval_arg(av[2], &buf_size) || buf_size <= 0) {
            grp_fs_printf("%s: invalid buffer size %s\n", av[0], av[2]);
            return(-1);
        }
        file_arg = 3;
        direct_mode = GRP_FS_O_DIRECT_IO;
    } else {
        buf_size = IO_BUF_SZ;
    }
    buf = (grp_uchar_t *)grp_mem_alloc(buf_size);
    if (buf == NULL) {
        grp_fs_printf("%s: cannot allocate work memory\n", av[0]);
        return(-1);
    }
    if ((ifd = grp_fs_open(av[file_arg], GRP_FS_O_RDONLY|direct_mode, 0)) < 0) {
        grp_fs_printf("cannot open %s\n", av[file_arg]);
        goto error_with_buf;
    }
    if ((ofd = grp_fs_open(av[file_arg + 1], 
                    GRP_FS_O_WRONLY|GRP_FS_O_CREAT|GRP_FS_O_TRUNC|direct_mode,
                    GRP_FS_PROT_RWXA)) < 0) {
        grp_fs_printf("cannot create %s\n", av[file_arg+1]);
        goto error_with_in_file;
    }
    for (n = 0; (r_cnt = grp_fs_read(ifd, buf, buf_size)) > 0; n += w_cnt) {
        w_cnt = grp_fs_write(ofd, buf, r_cnt);
        if (w_cnt != r_cnt) {
            if (w_cnt >= 0)
                n += w_cnt;
            grp_fs_printf("write failed on %d bytes\n", n);
            break;
        }
    }
    if (r_cnt < 0) {
        grp_fs_printf("read error on %d bytes\n", n);
    } else if (r_cnt == 0)
        ret = 0;
    grp_fs_close(ofd);
error_with_in_file:
    grp_fs_close(ifd);
error_with_buf:
    grp_mem_free(buf);
    return(ret);

usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_cat                                             */
/*                                                                       */
/* DESCRIPTION: cat command                                              */
/*************************************************************************/
int test_cmd_cat(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int fd;
    int n;
    int i;
    grp_uchar_t buf[IO_BUF_SZ];

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if ((fd = grp_fs_open(av[1], GRP_FS_O_RDONLY, 0)) < 0) {
        grp_fs_printf("cannot open %s\n", av[1]);
        return(-1);
    }
    while ((n = grp_fs_read(fd, buf, sizeof(buf))) > 0) {
        for (i = 0; i < n; i++)
            grp_fs_printf("%c", buf[i]);
    }
    grp_fs_close(fd);
    return(0);
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_pwd                                             */
/*                                                                       */
/* DESCRIPTION: pwd command                                              */
/*************************************************************************/
int test_cmd_pwd(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    grp_uchar_t path[GRP_FS_MAX_PATH];

    if (ac != 1) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_get_cwd(path, sizeof(path), '/');
    if (ret == 0)
        grp_fs_printf("%s\n", path);
    else
        grp_fs_printf("pwd = %s\n", grp_fs_err(ret, NULL));
    return(0);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_chmod                                           */
/*                                                                       */
/* DESCRIPTION: chmod command                                            */
/*************************************************************************/
int test_cmd_chmod(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int mode;
    int ret;
    struct stat statb;

    if (ac != 3) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &mode)) {
        grp_fs_printf("%s: bad mode %s\n", av[0], av[1]);
        return(-1);
    }
    ret = chmod(av[2], mode);
    grp_fs_printf("chmod = %s\n", grp_fs_err(ret, NULL));
    ret = stat(av[2], &statb);
    grp_fs_printf("stat = %s\n", grp_fs_err(ret, NULL));
    if (ret == 0) {
#ifdef GRP_FS_ENABLE_OVER_2G
        grp_fs_printf("mode = 0%o, size = %u, dev = %d\n", 
#else  /* GRP_FS_ENABLE_OVER_2G */
        grp_fs_printf("mode = 0%o, size = %d, dev = %d\n", 
#endif /* GRP_FS_ENABLE_OVER_2G */
            statb.st_mode, statb.st_size, statb.st_dev);
    }
    return(0);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/*************************************************************************/
/* FUNCTION   : test_cmd_sync                                            */
/*                                                                       */
/* DESCRIPTION: sync I/O command                                         */
/*************************************************************************/
int test_cmd_sync(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int mode;
    int ret;

    if (ac != 1 && ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (ac == 2) {
        if (test_cmd_eval_arg(av[1], &mode)) {
            grp_fs_printf("%s: invalid mode %s\n", av[0], av[1]);
            return(-1);
        }
    } else {
        mode = 0;
    }
    ret = grp_fs_sync(mode);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("sync = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("sync = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(0);
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_inv_task                                        */
/*                                                                       */
/* DESCRIPTION: invalidate task command                                  */
/*************************************************************************/
int test_cmd_inv_task(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    if (ac != 1 && (ac != 2 || strcmp((char *)av[1], "-a") != 0)) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (ac == 2)
        ret = grp_fs_task_free_all_env();
    else
        ret = grp_fs_task_free_env();
    grp_fs_printf("inv = %s\n", grp_fs_err(ret, NULL));
    return(0);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/*************************************************************************/
/* FUNCTION   : test_print_mnt                                           */
/*                                                                       */
/* DESCRIPTION: print mnt info                                           */
/*************************************************************************/
static void test_print_mnt(grp_fs_mnt_info_t *mnt)
{
    grp_fs_printf("%s: dev=0x%x pdev=", mnt->acDevName, mnt->iDev);
    if (mnt->iParentDev == -1)
        grp_fs_printf("-");
    else
        grp_fs_printf("0x%x", mnt->iParentDev);
    grp_fs_printf(" fs=\"%s\" mnt=\"%s\"", mnt->acFsType, mnt->aucPath);
    grp_fs_printf(" Vol(Ser:0x%x)=\"%.*s\"\n", 
                        mnt->uiVolSerNo, mnt->usVolNameLen, mnt->aucVolName);
    grp_fs_printf("    Blk_Size = 0x%08x Blk=%d/%d File=%d/%d\n",
                    mnt->uiFsBlkSize, mnt->uiFsFreeBlk, mnt->uiFsBlkCnt,
                    mnt->uiFsFreeFile, mnt->uiFsFileCnt);
    grp_fs_printf("    FBuf=0x%08x FOff=0x%08x DBuf=0x%08x DOff=0x%08x\n",
                    mnt->uiFBufSize, mnt->uiFBufOff,
                    mnt->uiDBufSize, mnt->uiDBufOff);
    grp_fs_printf("    status=<%s%s%s%s%s%s%s>\n", 
            (mnt->uiStatus & GRP_FS_MSTAT_RONLY)? "r": "rw",
            (mnt->uiStatus & GRP_FS_MSTAT_DAY_ACCTIME)? " day_acctime":"",
            (mnt->uiStatus & GRP_FS_MSTAT_NO_UPD_ACCTIME)? " no_update_acctime":"",
            (mnt->uiStatus & GRP_FS_MSTAT_NO_MNT_FLAG)? " no_mnt_flag":"",
            (mnt->uiStatus & GRP_FS_MSTAT_NO_CRT_ACCTIME)? " no_crt_acctime":"",
            (mnt->uiStatus & GRP_FS_MSTAT_DEV_INV)? " invalid": "",
            (mnt->uiStatus & GRP_FS_MSTAT_SYNC_ALL)? " sync_all":
            (mnt->uiStatus & GRP_FS_MSTAT_SYNC_FL_CLOSE)? " sync_each_close":
            (mnt->uiStatus & GRP_FS_MSTAT_SYNC_FS_CLOSE)? " sync_last_close":
            " sync_at_umount");
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_get_mnt                                         */
/*                                                                       */
/* DESCRIPTION: fstat command                                            */
/*************************************************************************/
int test_cmd_get_mnt(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    int i;
    int dev;
    grp_fs_mnt_info_t   mnt[16];            /* mount info */

    if (ac == 1) {
        ret = grp_fs_get_mnt(16, mnt);
        grp_fs_printf("fstat = %s\n", grp_fs_err(ret, NULL));
        for (i = 0; i < ret; i++)
            test_print_mnt(&mnt[i]);
        return(ret);
    }
    if (ac != 3 
        || (strcmp((char *)av[1], "-i") && strcmp((char *)av[1], "-n"))) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (strcmp((char *)av[1], "-i") == 0) {
        if (test_cmd_eval_arg(av[2], &dev)) {
            grp_fs_printf("%s: invalid dev %s\n", av[0], av[2]);
            return(-1);
        }
        ret = grp_fs_get_mnt_by_dev(dev, mnt);
    } else {
        ret = grp_fs_get_mnt_by_name((const char *)av[2], mnt);
    }
    grp_fs_printf("fstat = %s\n", grp_fs_err(ret, NULL));
    if (ret == 0)
        test_print_mnt(mnt);
    return(0);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_check_vol                                       */
/*                                                                       */
/* DESCRIPTION: chkvol command                                           */
/*************************************************************************/
int test_cmd_check_vol(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    grp_uchar_t vol_name[16];
    grp_uint32_t vol_ser;
    int vol_name_len = sizeof(vol_name);

    if (ac != 3) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_check_volume((const char *)av[1], (const char *)av[2],
                            vol_name, &vol_name_len, &vol_ser);
    grp_fs_printf("chkvol = %s, %.*s(%d) 0x%x\n", grp_fs_err(ret, NULL), 
                    vol_name_len, vol_name, vol_name_len, vol_ser);
    return(ret);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/*************************************************************************/
/* FUNCTION   : test_cmd_date                                            */
/*                                                                       */
/* DESCRIPTION: date command (get/set date and time)                     */
/*************************************************************************/
int test_cmd_date(test_cmd_t *cmd, int ac, unsigned char **av)
{
    grp_int32_t iTime;
    grp_time_tm_t tTime;
    int i;
    int ret;
    int val[6];

    if (ac == 1)
        goto get_date_time;
    if (ac < 2 || av[1][0] != '-' || av[1][2])
        goto usage;
    switch(av[1][1]) {
    case 's': /* set date and time */
        if (ac != 8)
            goto usage;
        for (i = 0; i < 6; i++) {
            if (test_cmd_eval_arg(av[i+2], &val[i])) {
                grp_fs_printf("%s: invalid param %s\n", av[0], av[i+2]);
                return(-1);
            }
        }
        tTime.sYear = (short)val[0];
        tTime.ucMon = (grp_uchar_t)val[1];
        tTime.ucDay = (grp_uchar_t)val[2];
        tTime.ucHour = (grp_uchar_t)val[3];
        tTime.ucMin = (grp_uchar_t)val[4];
        tTime.ucSec = (grp_uchar_t)val[5];
        ret = grp_time_set(&tTime);
        if (ret != 0)
#if(GRP_FS_MINIMIZE_LEVEL < 1)
            grp_fs_printf("Time set error(%s)\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
			grp_fs_printf("Time set error(%d)\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
        break;
    case 'g': /* get current date and time */
    get_date_time:
        ret = grp_time_get(&iTime);
        if (ret != -1) {
            test_cmd_print_time("Current Time: ", iTime);
            grp_fs_printf("\n");
        }
        break;
    default:
        goto usage;
    }
    return(ret);

usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_format                                          */
/*                                                                       */
/* DESCRIPTION: format command (format media)                            */
/*************************************************************************/
int test_cmd_format(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int i;
    grp_fat_format_param_t  param;
    grp_fs_media_info_t     media;
    char    *dev = NULL;
    int val;
    int ret;

    memset(&param, 0, sizeof(param));
    memset(&media, 0, sizeof(media));
    for (i = 1; i < ac; i++) {
        if (av[i][0] != '-' || i + 1 >= ac) {
            if (dev)
                goto usage;
            dev = (char *)av[i];
            continue;
        }
        if (av[i][1] == 'v') {                  /* volume name */
            if(11 == strlen((char *)av[++i])){
                memcpy(param.aucVolLab, av[i], 11);
            } else {
                strcpy((char *)param.aucVolLab, (char *)av[i]);
            }
            continue;
        }
        if (test_cmd_eval_arg(av[i + 1], &val)) {
            grp_fs_printf("%s: invalid param %s %s\n", av[0], av[i], av[i+1]);
            return(-1);
        }
        switch(av[i++][1]) {
        case 'f':                               /* fat type */
            param.ucFatType = (grp_uchar_t)val;
            break;
        case 'c':                               /* sector/cluster */
            param.uiClstSec = val;
            break;
        case 'd':                               /* root directory */
            param.uiRootCnt = val;
            break;
        case 'r':                               /* reserved */
            param.uiRsvSecCnt = val;
            break;
        case 'a':                               /* alignment */
            param.uiAlign = val;
            break;
        case 'o':                               /* option */
            param.uiOption = val;
            break;
        case 'm':
            media.ucMediaType = (grp_uchar_t)val;/* media type */
            break;
        case 's':                               /* total sector */
            media.uiTotalSec = val;
            break;
        case 't':                               /* total size */
            if (media.iSecShift == 0)
                    val >>= 9;
            else
                    val >>= media.iSecShift;
            media.uiTotalSec = val;
            break;
        case 'k':                               /* sector/track */
            media.usTrkSec = (grp_uint16_t)val;
            break;
        case 'h':                               /* head */
            media.usHead = (grp_uint16_t)val;
            break;
        case 'S':                               /* sector shift */
            media.iSecShift = val;
            break;
        default:
            goto usage;
        }
    }
    ret = grp_fat_format(dev, &param, &media);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("format = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("format = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    if (ret >= 0) {
        grp_fs_printf("type:%d vol:\"%.*s\" clstsec:%d root:%d rsv:%d\n",
                        param.ucFatType, sizeof(param.aucVolLab),
                        param.aucVolLab, param.uiClstSec,
                        param.uiRootCnt, param.uiRsvSecCnt);
        grp_fs_printf("clst:%d, fatsec:%d, volser:0x%02x%02x%02x%02x\n",
                        param.uiClst, param.uiFatSec,
                        param.aucVolSer[0], param.aucVolSer[1],
                        param.aucVolSer[2], param.aucVolSer[3]);
        grp_fs_printf("align:%d, option:0x%x, not_used:%d, adjust:%d\n",
                        param.uiAlign, param.uiOption,
                        param.uiNotUsed, param.uiAdjust);
        grp_fs_printf("media:0x%02x total:%d start:%d head:%d trk:%d shift:%d\n",
                        media.ucMediaType, media.uiTotalSec,
                        media.uiStartSec, media.usHead, media.usTrkSec,
                        media.iSecShift);
    }
    return(0);

usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}

/*************************************************************************/
/* FUNCTION   : test_print_part                                          */
/*                                                                       */
/* DESCRIPTION: print partition                                          */
/*************************************************************************/
static void test_print_part(void)
{
    grp_fs_dk_part_t    *p;
    grp_fs_dk_chs_t     *chs;
    int                 i;

    for (i = 0, p = part_tbl; i < GRP_FS_PART_CNT; i++, p++) {
        grp_fs_printf("%d: active:0x%02x type:0x%02x StartSec:%d SecCnt:%d\n",
                        i, p->ucActive, p->ucPartType, p->uiStartSec,
                        p->uiSecCnt);
        chs = &p->tStartCHS;
        grp_fs_printf("   StartCHS:%d/%d/%d ",
                        chs->usCyl, chs->ucHead, chs->ucSec);
        chs = &p->tEndCHS;
        grp_fs_printf("EndCHS:%d/%d/%d\n",
                        chs->usCyl, chs->ucHead, chs->ucSec);
    }
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_part                                            */
/*                                                                       */
/* DESCRIPTION: part command (set/get partition)                         */
/*************************************************************************/
int test_cmd_part(test_cmd_t *cmd, int ac, unsigned char **av)
{
    grp_fs_dk_part_t    *p;
    int                 ret;
    int                 num;
    int                 val;

    if (ac == 1) {
        test_print_part();
        return(0);
    }
    if (ac == 2) {
            if (strcmp((char *)av[1], "-c") != 0)
                goto usage;
            memset(part_tbl, 0, sizeof(part_tbl));
            return(0);
    }
    if (ac == 3) {
        if (av[1][0] != '-' || av[1][2] != 0)
            goto usage;
        switch(av[1][1]) {
        case 'w':                                   /* set partition table */
        case 'W':                                   /* set partition by auto */
            ret = grp_fs_write_part((char *)av[2], (av[1][1] == 'W'), part_tbl);
            break;
        case 'g':
            ret = grp_fs_read_part((char *)av[2], part_tbl);
            break;
        default:
            goto usage;
        }
        grp_fs_printf("part = %s\n", grp_fs_err(ret, NULL));
        if (ret == 0)
            test_print_part();
        return(0);
    }
    if (strcmp((char *)av[1], "-s") != 0 || ac != 13)
        goto usage;
    if (test_cmd_eval_arg(av[2], &num)) {
        grp_fs_printf("%s: invalid num %s\n", av[0], av[2]);
        return(-1);
    }
    p = &part_tbl[num];
    if (test_cmd_eval_arg(av[3], &val)) {
        grp_fs_printf("%s: invalid active param %s\n", av[0], av[3]);
        return(-1);
    }
    p->ucActive = (grp_uchar_t)val;
    if (test_cmd_eval_arg(av[4], &val)) {
        grp_fs_printf("%s: invalid type param %s\n", av[0], av[4]);
        return(-1);
    }
    p->ucPartType = (grp_uchar_t)val;
    if (test_cmd_eval_arg(av[5], &val)) {
        grp_fs_printf("%s: invalid start CHS(C) param %s\n", av[0], av[5]);
        return(-1);
    }
    p->tStartCHS.usCyl = (grp_uint16_t)val;
    if (test_cmd_eval_arg(av[6], &val)) {
        grp_fs_printf("%s: invalid start CHS(H) param %s\n", av[0], av[6]);
        return(-1);
    }
    p->tStartCHS.ucHead = (grp_uchar_t)val;
    if (test_cmd_eval_arg(av[7], &val)) {
        grp_fs_printf("%s: invalid start CHS(S) param %s\n", av[0], av[7]);
        return(-1);
    }
    p->tStartCHS.ucSec = (grp_uchar_t)val;
    if (test_cmd_eval_arg(av[8], &val)) {
        grp_fs_printf("%s: invalid end CHS(C) param %s\n", av[0], av[8]);
        return(-1);
    }
    p->tEndCHS.usCyl = (grp_uint16_t)val;
    if (test_cmd_eval_arg(av[9], &val)) {
        grp_fs_printf("%s: invalid end CHS(H) param %s\n", av[0], av[9]);
        return(-1);
    }
    p->tEndCHS.ucHead = (grp_uchar_t)val;
    if (test_cmd_eval_arg(av[10], &val)) {
        grp_fs_printf("%s: invalid end CHS(S) param %s\n", av[0], av[10]);
        return(-1);
    }
    p->tEndCHS.ucSec = (grp_uchar_t)val;
    if (test_cmd_eval_arg(av[11], &val)) {
        grp_fs_printf("%s: invalid start_sec param %s\n", av[0], av[11]);
        return(-1);
    }
    p->uiStartSec = val;
    if (test_cmd_eval_arg(av[12], &val)) {
        grp_fs_printf("%s: invalid sector_cnt param %s\n", av[0], av[12]);
        return(-1);
    }
    p->uiSecCnt = val;
    test_print_part();
    return(0);

usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

/*************************************************************************/
/* FUNCTION   : test_cmd_ioctl                                           */
/*                                                                       */
/* DESCRIPTION: ioctl command                                            */
/*************************************************************************/
int test_cmd_ioctl(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    int dev;
    int ctl_val;
    grp_fs_media_info_t media;

    if (ac < 3)
        goto usage;
    if (test_cmd_eval_arg(av[1], &dev)) {
        grp_fs_printf("%s: invalid dev %s\n", av[0], av[1]);
        goto usage;
    }
    switch(av[2][0]) {
    case 'm': /* get media info */
        ret = grp_fs_ioctl_dev(dev, GRP_FS_DEV_CTL_GET_MEDIA,
                                &media);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("ioctl GET_MEDIA_INFO = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("ioctl GET_MEDIA_INFO = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
        if (ret == 0) {
            grp_fs_printf("media:0x%02x total:%d start:%d head:%d trk:%d shift:%d\n",
                        media.ucMediaType, media.uiTotalSec,
                        media.uiStartSec, media.usHead, media.usTrkSec,
                        media.iSecShift);
        }
        break;
    case 'e': /* eject control */
        if (ac != 4)
            goto usage;
        if (test_cmd_eval_arg(av[3], &ctl_val)) {
                grp_fs_printf("%s: invalid param %s\n", av[0], av[3]);
                goto usage;
        }
        ret = grp_fs_ioctl_dev(dev, GRP_FS_DEV_CTL_EJECT, &ctl_val);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("ioctl EJECT_CTL = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("ioctl EJECT_CTL = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
        break;
    case 'f': /* format */
        ret = grp_fs_ioctl_dev(dev, GRP_FS_DEV_CTL_FORMAT, NULL);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("ioctl FORMAT = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("ioctl FORMAT = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
        break;
    default:
        goto usage;
    }
    return(ret);

usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_dev_io                                          */
/*                                                                       */
/* DESCRIPTION: dev_io command                                           */
/*************************************************************************/
int test_cmd_dev_io(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    int dev, off, cnt;
    grp_fs_dev_io_info_t io;
	grp_uchar_t buf[IO_BUF_SZ];

    if (ac != 4)
        goto usage;
    if (test_cmd_eval_arg(av[1], &dev)) {
        grp_fs_printf("%s: invalid dev %s\n", av[0], av[1]);
        goto usage;
    }
    if (test_cmd_eval_arg(av[2], &off)) {
        grp_fs_printf("%s: invalid offset %s\n", av[0], av[2]);
        goto usage;
    }
    if (test_cmd_eval_arg(av[3], &cnt)) {
        grp_fs_printf("%s: invalid cnt %s\n", av[0], av[3]);
        goto usage;
    }
    ret = grp_fs_open_dev(dev, 1, &io);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("dev_open = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("dev_open = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    if (ret != 0)
        return(ret);
    grp_fs_printf("handle: 0x%x, off: 0x%x, size: 0x%x, shift:%d\n",
                    io.iHandle, io.uiOff, io.uiSize, io.iSzShift);
	if( cnt > (sizeof( buf ) / (1 << io.iSzShift)) )
		cnt = (sizeof( buf ) / (1 << io.iSzShift));
    ret = grp_fs_read_dev(io.iHandle, dev, off, buf, cnt);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("dev_read = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("dev_read = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    if (ret != cnt)
        return(ret);
    ret = grp_fs_write_dev(io.iHandle, dev, off, buf, cnt);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("dev_write = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("dev_write = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    if (ret != cnt)
        return(ret);
    ret = grp_fs_close_dev(io.iHandle, dev);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("dev_close = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("dev_close = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

    return(ret);

usage:
    grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
    return(-1);
}

