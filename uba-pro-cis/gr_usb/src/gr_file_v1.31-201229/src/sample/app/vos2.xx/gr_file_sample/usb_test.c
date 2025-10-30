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
/*      usb_test.c                                         2.04          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      GR-USB/Host# Sample program                                      */
/*                                                                       */
/* FUNCTIONS:                                                            */
/*                                                                       */
/*      GRUSB_Test_Stack_Init       Initialization of usb stack.         */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      gr_vos.h                                                         */
/*      grusb.h                                                          */
/*      usb_test.h                                                       */
/*      usb_dev.h                                                        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*   NAME         DATE        REMARKS                                    */
/*                                                                       */
/*   Y.Hasegawa   2003/09/12  Created initial version 0.00               */
/*   Y.Hasegawa   2003/09/18  Version 0.01                               */
/*   K.Kaneko     2008/06/23  Version 0.02                               */
/*   K.Takagi     2009/03/24  Version 2.00                               */
/*                            不要なヘッダファイルを削除                 */
/*                            GR-USB/HOST#用のGR-VOSのAPIへの変更        */
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
#include "grp_std_tools.h"                                                      /* for memset   */
#include "grp_bus.h"
#include "grp_usbc.h"
#include "grp_fsif.h"

#include "fat.h"
#include "grp_fs_proc_event.h"
#include "grp_mem_vl_pool.h"
#include "grp_stdio.h"
#include "grp_time.h"

#include "grp_sio.h"

#include "cmd.h"
#include "usb_test.h"


/*************************************************************************/
/* defines                                                               */
/*************************************************************************/
#define POOL_SIZE           0x24800
#define STACK_SIZE          10240
#define TASK_PRI            GRP_VOS_PRI_NORMAL
#define TASK_STATUS         GRP_VOS_READY

/*************************************************************************/
/* locals                                                                */
/*************************************************************************/
DLOCAL grp_vos_t_partition_pool     *l_ptPartPool;

/*************************************************************************/
/* grp_fsif_dev_io.cにて定義                                             */
/*************************************************************************/
extern grp_vos_t_queue              *g_ptXferQue;

/*************************************************************************/
/* prototypes                                                            */
/*************************************************************************/
int             GRUSB_Test_Stack_Init(void);
int             GRUSB_Test_Init(void);

#if(GRP_FS_MINIMIZE_LEVEL < 1)
grp_isize_t     test_stdout(FILE *, grp_uchar_t *, grp_isize_t);
grp_isize_t     test_stdin(FILE *, grp_uchar_t *, grp_isize_t);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */

void            test_main_task( grp_u32 param );
void            test_event_task( grp_u32 param );
int             test_cmd_loop( grp_u32 id );


/*************************************************************************/
/* FUNCTION   : GRUSB_Test_Stack_Init                                    */
/*                                                                       */
/* DESCRIPTION: USB Stack Initialize                                     */
/*-----------------------------------------------------------------------*/
/* INPUT      : none                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : GRUSB_OK                Success                          */
/*              GRUSB_ERROR             Error                            */
/*                                                                       */
/*************************************************************************/
grp_si GRUSB_Test_Stack_Init(void)
{
    /* serial modules initialize */
    if (grp_sio_Init() != GRP_SIO_OK) {
        /* error */
        return GRUSB_TEST_ERROR;
    }


    grp_fs_printf("--> GRUSB_Test_Stack_Init  Start -->\r\n");

    /*--- Initialize several modules ---*/
    /* bus module */
    if (grp_bus_Init() != GRP_BUS_OK) {
        /* error */
        grp_fs_printf("!!! ERROR !!! -grp_bus_Init()-\r\n");
        return GRUSB_TEST_ERROR;
    }
    /* GR-VOS */
    if (grp_vos_Init() != GRP_VOS_POS_RESULT) {
        /* error */
        grp_fs_printf("!!! ERROR !!! -grp_vos_Init()-\r\n");
        return GRUSB_TEST_ERROR;
    }
    /* GR-USB/HOST# */
    if (grp_usbc_HostInit() != GRP_USBC_OK) {
        /* error */
        grp_fs_printf("!!! ERROR !!! -grp_usbc_HostInit()-\r\n");
        return GRUSB_TEST_ERROR;
    }
    /* From the device io module to MSC driver */
    if (grp_fsif_dev_io_init() != 0) {
        /* error */
        grp_fs_printf("!!! ERROR !!! -grp_fsif_dev_io_init()-\r\n");
        return GRUSB_TEST_ERROR;
    }
    /* Initialize test application */
    if (GRUSB_Test_Init() != GRUSB_TEST_OK) {
        return GRUSB_TEST_ERROR;
    }

    /*--- Enable interrupt ---*/
    if (grp_usbc_Enable() != GRP_USBC_OK) {
        /* error */
        grp_fs_printf("!!! ERROR !!! -grp_usbc_Enable()-\r\n");
        return GRUSB_TEST_ERROR;
    }

    grp_fs_printf("<-- GRUSB_Test_Stack_Init  Complete <--\r\n");

    return GRUSB_TEST_OK;

} /* GRUSB_Test_USBStack_Init */

/*************************************************************************/
/* FUNCTION   : GRUSB_Test_Init                                          */
/*                                                                       */
/* DESCRIPTION: Test Application Initialize                              */
/*-----------------------------------------------------------------------*/
/* INPUT      : none                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : GRUSB_TEST_OK           Success                          */
/*              GRUSB_TEST_ERROR        Error                            */
/*                                                                       */
/*************************************************************************/
int
GRUSB_Test_Init( void )
{
    int             iStatus;

    void            *pvMem;
    grp_vos_t_task  *ptTask;

    /* Set stdio */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_stdio_io_stdout = test_stdout;
    grp_stdio_io_stdin = test_stdin;
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */

    grp_fs_printf("File test start!\n");

    /* Make memory for gr_file */
    iStatus = grp_vos_CreatePartitionPool( &l_ptPartPool, (grp_u8*)"FT_PP", POOL_SIZE, 1 );
    if( iStatus != GRP_VOS_POS_RESULT ){
        return GRUSB_TEST_ERROR;
    }
    iStatus = grp_vos_GetPartitionPool( l_ptPartPool, &pvMem, GRP_VOS_INFINITE );
    if( iStatus != GRP_VOS_POS_RESULT ){
        return GRUSB_TEST_ERROR;
    }
    if( grp_mem_vl_init( pvMem, POOL_SIZE ) != 0 ){
        return GRUSB_TEST_ERROR;
    }

    if( grp_fs_init() != 0 ){
        return GRUSB_TEST_ERROR;
    }

    if( test_cmd_init( 0, 0 ) != 0 ){
        return GRUSB_TEST_ERROR;
    }

    iStatus = grp_vos_CreateTask(
                            &ptTask,
                            (grp_uchar_t *)"FT_TEV",
                            test_event_task,
                            STACK_SIZE,
                            TASK_PRI,
                            TASK_STATUS,
                            0);
    if (iStatus != GRP_VOS_POS_RESULT) {
        grp_fs_printf("task create error %x\n", iStatus);
        return GRUSB_TEST_ERROR;
    }

    iStatus = grp_vos_CreateTask(
                            &ptTask,
                            (grp_uchar_t *)"FT_TM0",
                            test_main_task,
                            STACK_SIZE,
                            TASK_PRI,
                            TASK_STATUS,
                            0);
    if (iStatus != GRP_VOS_POS_RESULT) {
        grp_fs_printf("task create error %x\n", iStatus);
        return GRUSB_TEST_ERROR;
    }

    return GRUSB_TEST_OK;
} /* GRUSB_Test_Init */

/*************************************************************************/
/* FUNCTION   : test_main_task                                           */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*-----------------------------------------------------------------------*/
/* INPUT      : none                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
void    test_main_task( grp_u32 param )
{
    grp_fs_printf("start main%d task\n", param);

    test_cmd_loop( param );     /* execute test command */
}

/*************************************************************************/
/* FUNCTION   : test_event_task                                          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*-----------------------------------------------------------------------*/
/* INPUT      : none                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
void    test_event_task( grp_u32 param )
{
    int     iStatus;
    int     iPtn;
    char    acDevName[] = "usb0";
#if(GRP_FS_MINIMIZE_LEVEL >= 1)
	int	sprintf(char *pcBuf, const char *pcFormat, ...);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */

    grp_fs_printf("start event task\n");

    while (1) {
        iStatus = grp_vos_ReceiveQueue( g_ptXferQue, (void *)&iPtn, GRP_VOS_INFINITE );
        if( iStatus != GRP_VOS_POS_RESULT ){
            break;
        }

        sprintf(acDevName, "usb0");
        acDevName[3] += (char)(iPtn & 0x000000ff);

        if (iPtn & GRP_FSIF_ATTACHED_MEDIA) {
            grp_fs_printf( "Attache %c\n", acDevName[3] );
#if(GRP_FS_MINIMIZE_LEVEL < 1)
            grp_fs_proc_event( acDevName, GRP_FS_EVENT_INSERT);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
        }
        else {
            grp_fs_printf( "Detache %c\n", acDevName[3] );
#if(GRP_FS_MINIMIZE_LEVEL < 1)
            grp_fs_proc_event( acDevName, GRP_FS_EVENT_EJECT);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
        }
    }
}

/*************************************************************************/
/* FUNCTION   : test_cmd_loop                                            */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*-----------------------------------------------------------------------*/
/* INPUT      : none                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
int     test_cmd_loop(grp_u32 id)
{
    while ( test_cmd_interpret((int)id) == 0 ); /* interpret commands */

    return(0);
}

/*************************************************************************/
/* FUNCTION   : cons_putchar                                             */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*-----------------------------------------------------------------------*/
/* INPUT      : mode                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
int cons_putchar(int c)
{
    if( grp_sio_PutChar( c ) )
        return(c);

    return( 0 );
}

/*************************************************************************/
/* FUNCTION   : cons_getchar                                             */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*-----------------------------------------------------------------------*/
/* INPUT      : mode                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
int cons_getchar(int mode)
{
    int         retc;
    int         c;
    grp_uchar_t uc;

    retc = 0;
    while(!retc){
        retc = grp_sio_GetChar( &uc );      /* get 1byte */
        if(!retc){                          /* non receive */
            grp_vos_DelayTask(1);           /* delay 1 msec */
        }
    }

    if (0 < retc) {
        c = (int)uc;                        /* character */
        if ('\r' == c) {                    /* carriage return */
            c = '\n';                       /* convert to newline */
        }
    } else {
        c = -1;                             /* return error */
    }

    return(c);
}

/*************************************************************************/
/* FUNCTION   : test_stdout                                              */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*-----------------------------------------------------------------------*/
/* INPUT      : none                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
grp_isize_t
#if(GRP_FS_MINIMIZE_LEVEL < 1)
test_stdout(FILE *fp, grp_uchar_t *buf, grp_isize_t size)
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
test_stdout(grp_uchar_t *buf, grp_isize_t size)
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
{
    int     i;

    if (size <= 0)
        return(0);

    for (i = 0; i < size; i++) {
        if (buf[i] == '\n') {
            cons_putchar((int)'\r');
            cons_putchar((int)'\n');
        } else
            cons_putchar((int)buf[i]);
    }

    return(size);
}

/*************************************************************************/
/* FUNCTION   : test_stdin                                               */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*-----------------------------------------------------------------------*/
/* INPUT      : none                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
grp_isize_t
#if(GRP_FS_MINIMIZE_LEVEL < 1)
test_stdin(FILE *fp, grp_uchar_t *buf, grp_isize_t size)
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
test_stdin(grp_uchar_t *buf, grp_isize_t size)
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
{
    int c;

    if (size <= 0)
        return(0);
    c = cons_getchar(0);                    /* get character */
    if (c < 0)                              /* read failed */
        return(-1);                         /* return error */
    if (c == 0)                             /* 0 */
        return(0);                          /* return 0 */
    buf[0] = (grp_uchar_t)c;                /* put in buffer */

    return(1);                              /* return 1 */
}

/*************************************************************************/
/* FUNCTION   : lookup_cmd                                               */
/*                                                                       */
/* DESCRIPTION: lookup command                                           */
/*-----------------------------------------------------------------------*/
/* INPUT      : cmd                                                      */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
test_cmd_t * lookup_cmd(unsigned char *cmd)
{
    test_cmd_t  *cp;

    for (cp = test_cmd_tbl; cp->cmd_name; cp++) {/* lookup command table */
        if (strcmp(cp->cmd_name, (char *)cmd) == 0) /* match name */
            return(cp);                     /* return it */
    }
    return(NULL);                           /* not found */
}

/*************************************************************************/
/* FUNCTION   : test_cmd_help                                            */
/*                                                                       */
/* DESCRIPTION: help command                                             */
/*-----------------------------------------------------------------------*/
/* INPUT      : cmd                                                      */
/*              ac                                                       */
/*              av                                                       */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
int test_cmd_help(test_cmd_t *cmd, int ac, unsigned char **av)
{
    test_cmd_t  *cp;

    if (ac > 1) {                           /* cmd specified */
        cp = lookup_cmd(av[1]);             /* lookup command */
        if (cp == NULL) {                   /* not found */
            grp_fs_printf("%s: no such command %s\n", 
                                (char *)av[0], (char *)av[1]);
            return(-1);                     /* return error */
        } else {                            /* found */
            grp_fs_printf("%s\t", cp->cmd_name);
											/* print command name */
            grp_fs_printf("%s\n", cp->cmd_desc);
											/* print description */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
            test_stdout(stdout,				/* print description details */
						(grp_uchar_t *)cp->cmd_desc_details,
						strlen(cp->cmd_desc_details));
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
			test_stdout((grp_uchar_t *)cp->cmd_desc_details,	/* print description details */
						strlen(cp->cmd_desc_details));
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
        }
    } else {                                /* no cmd specified */
        for (cp = test_cmd_tbl; cp->cmd_name; cp++) {
            grp_fs_printf("%s\t", cp->cmd_name);
											/* print command name */
            grp_fs_printf("%s\n", cp->cmd_desc);
											/* print description */
            grp_vos_DelayTask(20);
        }
    }
    return(0);                              /* return success */
}

/*************************************************************************/
/* FUNCTION   : test_print_dir                                           */
/*                                                                       */
/* DESCRIPTION: print directory entry                                    */
/*-----------------------------------------------------------------------*/
/* INPUT      : dir                                                      */
/*              long_list                                                */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
void test_print_dir(grp_fs_dir_ent_t *dir, int long_list)
{
    grp_fs_printf("%c%c%c%c%c%c%c%c%c%c",
        (dir->ucType == GRP_FS_FILE_DIR)? 'd':
        (dir->ucType == GRP_FS_FILE_FILE)? 'f':
        (dir->ucType == GRP_FS_FILE_LINK)? 'l':
        (dir->ucType == GRP_FS_FILE_OTHER)? 'o': '-',
        (dir->uiProtect & GRP_FS_PROT_RUSR)? 'r': '-',
        (dir->uiProtect & GRP_FS_PROT_WUSR)? 'w': '-',
        (dir->uiProtect & GRP_FS_PROT_XUSR)? 'x': '-',
        (dir->uiProtect & GRP_FS_PROT_RGRP)? 'r': '-',
        (dir->uiProtect & GRP_FS_PROT_WGRP)? 'w': '-',
        (dir->uiProtect & GRP_FS_PROT_XGRP)? 'x': '-',
        (dir->uiProtect & GRP_FS_PROT_ROTH)? 'r': '-',
        (dir->uiProtect & GRP_FS_PROT_WOTH)? 'w': '-',
        (dir->uiProtect & GRP_FS_PROT_XOTH)? 'x': '-');
    if ((dir->uiAttr & FAT_ATTR_TYPE_MASK) == FAT_ATTR_LONG) 
        grp_fs_printf(" l---- ");
    else
        grp_fs_printf(" %c%c%c%c%c ",
            (dir->uiAttr & FAT_ATTR_DIR)? 'd':
            (dir->uiAttr & FAT_ATTR_VOLID)? 'v': 'f',
            (dir->uiAttr & FAT_ATTR_RONLY)? 'r': 'w',
            (dir->uiAttr & FAT_ATTR_ARCHIVE)? 'a': '-',
            (dir->uiAttr & FAT_ATTR_HIDDEN)? 'h': '-',
            (dir->uiAttr & FAT_ATTR_SYSTEM)? 's': '-');
    grp_fs_printf("%d 0x%06x ", dir->iDev, dir->uiFid);
#ifdef GRP_FS_ENABLE_OVER_2G
    grp_fs_printf("%10u ", dir->uiSize); 
#else  /* GRP_FS_ENABLE_OVER_2G */
    grp_fs_printf("%10d ", dir->iSize); 
#endif /* GRP_FS_ENABLE_OVER_2G */
    test_cmd_print_time("", dir->iMTime);
    grp_fs_printf(" %s\n", dir->pucName);
    if (long_list) {
        test_cmd_print_time("                              ", dir->iCTime);
        test_cmd_print_time(" ", dir->iATime);
        grp_fs_printf(" (%d-%d)\n", dir->uiStart, dir->uiEnd);
    }
}

/*************************************************************************/
/* FUNCTION   : test_cmd_print_time                                      */
/*                                                                       */
/* DESCRIPTION: print time information                                   */
/*-----------------------------------------------------------------------*/
/* INPUT      : name                                                     */
/*              time                                                     */
/* OUTPUT     : none                                                     */
/*                                                                       */
/* RESULTS    : none                                                     */
/*                                                                       */
/*************************************************************************/
void test_cmd_print_time(const char *name, grp_int32_t time)
{
    grp_time_tm_t tm;

    if (time == 0) {                        /* null time */
        grp_fs_printf("%s00/00/00 00:00:00", name);
    } else {                                /* time exists */
        grp_time_localtime(time, &tm);      /* convert time */
        grp_fs_printf("%s%02d/%02d/%02d %02d:%02d:%02d", name,
                            tm.sYear % 100, tm.ucMon, tm.ucDay,
                            tm.ucHour, tm.ucMin, tm.ucSec);
    }
}

