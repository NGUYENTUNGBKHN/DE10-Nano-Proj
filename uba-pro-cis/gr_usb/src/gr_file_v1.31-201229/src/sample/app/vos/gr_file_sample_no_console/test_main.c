/************************************************************************************************/
/*                                                                                              */
/*                           Copyright(C) 2010-2016 Grape Systems, Inc.                         */
/*                                       All Rights Reserved                                    */
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
/*      test_main.c                                                             0.04            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      USB�}�X�X�g���[�W�����L�b�g�p�̃e�X�g���ł��B                                         */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2010/01/28  V0.01                                                           */
/*                              Created initial version                                         */
/*   K.Takagi       2012/01/25  V0.02                                                           */
/*                              Changes for the sample of GR-FILE.                              */
/*   K.Kaneko       2015/03/04  V0.03                                                           */
/*                              Changes calling of VOS API.                                     */
/*   K.Kaneko       2016/03/17  V0.04                                                           */
/*                              Added comment about the buffer address when using direct-io.    */
/*                              Fixed type to be using in VOS1.xx.                              */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "test_main.h"
#include "gr_vos.h"
#include "grp_std_tools.h"                                                      /* for memset   */
#include "gr_bus.h"
#include "grusb.h"

#include "fat.h"
#include "grp_mem_vl_pool.h"
#include "grp_fat_format.h"
#include "grp_stdio.h"
#include "fsif.h"

#define GRDBG_PRINT                     //

/**** INTERNAL DATA DEFINES *********************************************************************/
/* GR-FILE�̏������Ɋւ����` -----------------------------------------------------------------*/
/* �v�[���T�C�Y */
#define FILE_POOL_SIZE                  (1024 * 100)
UINT32  g_aulGrFileBuf[FILE_POOL_SIZE/4];

/* �}�������p�^�X�N */
GRVOS_tTask         *g_ptConDconTask;
#define _CONDCON_TASK_NAME              ((UINT8 *)"tConDcon")
#define _CONDCON_TASK_STACK             (1024)
#define _CONDCON_TASK_PRI               GRVOS_PRI_LOWEST
#define _CONDCON_TASK_STAT              GRVOS_READY

GRVOS_tTask         *g_ptAppTask;
#define _APP_TASK_NAME                  ((UINT8 *)"tApp")
#define _APP_TASK_STACK                 (1024*4)
#define _APP_TASK_PRI                   GRVOS_PRI_LOWEST
#define _APP_TASK_STAT                  GRVOS_READY

/* FSIF����̒ʒm�p�L���[ */
extern GRVOS_tQueue             *g_ptXferQue;

#define _TEST_DATA_BUF_SIZE             (4*1024)
UINT32                          g_aulTestWrBuf[_TEST_DATA_BUF_SIZE/4];
UINT32                          g_aulTestRdBuf[_TEST_DATA_BUF_SIZE/4];
UINT8 *                         g_pucTestWrBuf = (UINT8 *)g_aulTestWrBuf;
UINT8 *                         g_pucTestRdBuf = (UINT8 *)g_aulTestRdBuf;


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL int           _usb_test_GrFileInit(void);
LOCAL int           _usb_test_AppInit(void);
LOCAL INT8*         _usb_test_GetMemoryArea(void);
LOCAL void          _usb_test_ConDconTask( UINT32 ulArg);
LOCAL void          _usb_test_AppTask( UINT32 ulArg);

/************************************************************************************************/
/* FUNCTION   : _usb_test_GetMemoryArea                                                         */
/*                                                                                              */
/* DESCRIPTION: GR-FILE�����v�[���p�������A�h���X�̕ϊ�                                         */
/*              �I�I�I �g�p������ɕ����ă|�[�e�B���O���Ă������� �I�I�I                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : vpMemAddress                    �������A�h���X                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : �ϊ���̃A�h���X                Success                                         */
/*              USB_TEST_NULL                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT8* _usb_test_GetMemoryArea(void)
{
    /* GR-FILE�Ŏg�p���郁�����v�[�����L���b�V���̈��芄�蓖�Ă܂� */
    return (INT8*)g_aulGrFileBuf;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_Test_Stack_Init                                                           */
/*                                                                                              */
/* DESCRIPTION: Initialize of thie test application.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
int GRUSB_Test_Stack_Init(void)
{
     STATUS  iStatus;

    /*--- Initialize several modules ---*/

    /* initialize VOS */
    GRVOS_Init();
    GRVOS_OsInitialized();

    /* Initialize GR-BUS */
    iStatus = GRBUS_Init();
    if(GRBUS_OK != iStatus)
        return GRUSB_ERROR;

    /* Initialize GR-USB (Class) */
    iStatus = GRUSB_Class_Initialize();
    if(GRUSB_OK != iStatus)
        return GRUSB_ERROR;

    /* Initialize FSIF Driver */
    iStatus = FSCD_init();
    if(FSCD_E_OK != iStatus)
        return GRUSB_ERROR;

    /* Initialize GR-USB */
    iStatus = GRUSB_Initialize();
    if(GRUSB_OK != iStatus)
        return GRUSB_ERROR;

    /* GR-FILE */
    iStatus = _usb_test_GrFileInit();
    if (USB_TEST_OK != iStatus) {
        /* error */
        return GRUSB_ERROR;
    }

    /* Initialize test application */
    iStatus = _usb_test_AppInit();
    if(USB_TEST_OK != iStatus)
        return GRUSB_ERROR;

    /* start USB */
    iStatus = GRUSB_Enable();
    if(GRUSB_OK != iStatus)
        return GRUSB_ERROR;

    return GRUSB_OK;

}

/************************************************************************************************/
/* FUNCTION   : _usb_test_GrFileInit                                                            */
/*                                                                                              */
/* DESCRIPTION: Initialize of GR-FILE.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
int _usb_test_GrFileInit(void)
{
    INT8                            *pcPoolArea;

    /*--- GR-FILE�����������Ǘ��̏����� ---*/
    /* �������G���A�̎擾 */
    pcPoolArea = _usb_test_GetMemoryArea();
    if (pcPoolArea == 0) {
        /* error */
        return USB_TEST_NG;
    }
    /* �������Ǘ��̏����� */
    grp_mem_vl_init( pcPoolArea, FILE_POOL_SIZE);

    /*--- GR-FILE�̏����� ---*/
    /* configuration�p�����[�^(�K�v�Ȃ��) */
        /* �{�A�v���P�[�V�����ł̓f�t�H���g�l�𗘗p���� */
    /* GR-FILE�̏����� */
    if (grp_fs_init() != 0) {
        /* error */
        return USB_TEST_NG;
    }

    /*--- �t�b�N�֐��̏��������� ---*/
    fat_interrupt_lookup = 0;

    /*--- �W�����o�͊֐��o�^(�K�v�Ȃ��) ---*/
        /* �{�A�v���P�[�V�����ł͕s�v�Ȃ̂Ŗ��ݒ� */
        /* grp_stdio_io_stdout = GRP_USB_NULL; */
        /* grp_stdio_io_stdin  = GRP_USB_NULL; */

    /*--- I/O�G���[�֐��o�^(�K�v�Ȃ��) ---*/
        /* �{�A�v���P�[�V�����ł͕s�v�Ȃ̂Ŗ��ݒ� */

    return USB_TEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _usb_test_ConDconTask                                                           */
/*                                                                                              */
/* DESCRIPTION: MS���f�B�A�̑}�����Ď�����^�X�N                                                */
/*              �{�^�X�N�ł͐ڑ�����ѐؒf�̒ʒm�݂̂��s���܂��B                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulArg                           reserved                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _usb_test_ConDconTask( UINT32 ulArg)
{
    int                             iPtn;

    /* ���[�j���O�΍� */
    ulArg = 0;

    while (1) {
        if (GRVOS_ReceiveQueue(   g_ptXferQue,
                                  (void *)&iPtn,
                                  GRVOS_INFINITE) != GRVOS_POS_RESULT) {
            continue;
        }

        if (iPtn & ATDT_QUE_DETACHE) {
            GRDBG_PRINT("<-- DISCONNECT USB0 \r\n");
        }
        else {
            GRDBG_PRINT("--> CONNECT USB0 \r\n");
        }
    }
}


/************************************************************************************************/
/* FUNCTION   : _usb_test_mount                                                                 */
/*                                                                                              */
/* DESCRIPTION: �}�E���g���������s���܂�                                                        */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_mount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_mount(aucDevName, "/", "fat", GRP_FS_SYNC_FL_CLOSE);
    if (iRet == 0) {
        GRDBG_PRINT("     O mount ok\r\n");
    }
    else if (iRet == GRP_FS_ERR_NEED_CHECK) {
        GRDBG_PRINT("     X mount ng - need force to mount\r\n");
    }
    else {
        GRDBG_PRINT("     X mount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_force_mount                                                           */
/*                                                                                              */
/* DESCRIPTION: �}�E���g���������s���܂�                                                        */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_force_mount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_mount(aucDevName, "/", "fat", (GRP_FS_SYNC_FL_CLOSE | GRP_FS_FORCE_MOUNT));
    if (iRet == 0) {
        GRDBG_PRINT("     O force mount ok\r\n");
    }
    else {
        GRDBG_PRINT("     X force mount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_unmount                                                               */
/*                                                                                              */
/* DESCRIPTION: �A���}�E���g���������s���܂�                                                    */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_unmount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_unmount(aucDevName, 0);
    if (iRet == 0) {
        GRDBG_PRINT("     O unmount ok\r\n");
    }
    else if (iRet == GRP_FS_ERR_BUSY) {
        GRDBG_PRINT("     X unmount ng - need force to unmount\r\n");
    }
    else {
        GRDBG_PRINT("     X unmount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_force_unmount                                                         */
/*                                                                                              */
/* DESCRIPTION: �A���}�E���g���������s���܂�                                                    */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_force_unmount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_unmount(aucDevName, GRP_FS_FORCE_UMOUNT);
    if (iRet == 0) {
        GRDBG_PRINT("     O force unmount ok\r\n");
    }
    else {
        GRDBG_PRINT("     X force unmount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_ls                                                                    */
/*                                                                                              */
/* DESCRIPTION: ���X�g�̎擾���������s���V���A���Ƀt�@�C�������o�͂��܂�                        */
/*----------------------------------------------------------------------------------------------*/
#define _USB_TEST_FILE_NAME_MAX         (16)
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_ls(void)
{
    int                             iHdr    = 0;
    int                             iRet    = 0;
    grp_fs_dir_ent_t                tDirent;
    UINT8                           aucName[_USB_TEST_FILE_NAME_MAX*2+1];

    /* NULL�X�g�b�p�[�̕t���i�j */
    aucName[_USB_TEST_FILE_NAME_MAX*2] = 0;

    /* ���[�g�f�B���N�g���̏�񂵂����Ȃ����� */
    iHdr = -1;

    GRDBG_PRINT("--- File List---\r\n");
    tDirent.pucName     = aucName;                          /* set file aucName buffer      */
    tDirent.sNameSize   = _USB_TEST_FILE_NAME_MAX*2;        /* set aucName buffer size      */
    tDirent.uiStart     = 0;                                /* start offset is 0            */
    tDirent.uiEnd       = 0;                                /* end offset is 0              */
    while ((iRet = grp_fs_get_dirent(iHdr, &tDirent)) > 0) {
        GRDBG_PRINT("%s\r\n", tDirent.pucName);
        tDirent.sNameSize = _USB_TEST_FILE_NAME_MAX*2;      /* set aucName buffer size      */
        tDirent.uiStart   = tDirent.uiEnd;                  /* set next                     */
    }
    GRDBG_PRINT("\r\n");
    return 0;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_format                                                                */
/*                                                                                              */
/* DESCRIPTION: �t�H�[�}�b�g���s�Ȃ��܂�                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_format(void)
{
    grp_fat_format_param_t          param;
    grp_fs_media_info_t             media;
    int                             iRet;
    UINT8                           aucDevName[] = "usb0";

    memset(&param, 0, sizeof(param));
    memset(&media, 0, sizeof(media));
    iRet = grp_fat_format(aucDevName, &param, &media);
    if (iRet >= 0) {    /* ok */
        GRDBG_PRINT("     O format ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X format ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_open_file                                                             */
/*                                                                                              */
/* DESCRIPTION: �t�@�C�����I�[�v�����܂�                                                        */
/*----------------------------------------------------------------------------------------------*/
int                             g_iHandle1    = -1;
UINT8                           g_aucTestFileName[] = "TEST1.TXT";
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_open_file(void)
{
    int                             iMode       = 0;
    int                             iProt       = 0;
    int                             iRet;
    UINT8                           aucFilneName[_USB_TEST_FILE_NAME_MAX*2+1];

    grp_std_strcpy(aucFilneName, g_aucTestFileName);
    iMode = GRP_FS_O_RDWR | GRP_FS_O_CREAT | GRP_FS_O_DIRECT_IO;
    iProt = GRP_FS_PROT_RWXA;                                           /* ���ׂċ���   */
    iRet = grp_fs_open(aucFilneName, iMode, iProt);
    if (iRet >= 0) {    /* ok */
        g_iHandle1 = iRet;
        iRet = 0;
        GRDBG_PRINT("     O file open ok\r\n");
    }
    else {              /* ng */
        g_iHandle1 = -1;
        GRDBG_PRINT("     X file open ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_close_file                                                            */
/*                                                                                              */
/* DESCRIPTION: �t�@�C�����N���[�Y���܂�                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_close_file(void)
{
    int                             iRet;

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file close ng <not open file>\r\n");
    }

    iRet = grp_fs_close(g_iHandle1);
    if (iRet == 0) {    /* ok */
        g_iHandle1 = -1;
        GRDBG_PRINT("     O file close ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file close ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_write_file                                                            */
/*                                                                                              */
/* DESCRIPTION: �t�@�C���Ƀf�[�^���������݂܂�                                                  */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_write_file(void)
{
    int                             iRet;
    UINT8 *                         pucBuf = (UINT8 *)g_pucTestWrBuf;
    UINT32                          ulDataSz = _TEST_DATA_BUF_SIZE;
    int                             i;
    int                             j;

    /* �e�X�g�f�[�^�Ńo�b�t�@�𖄂߂� */
    for (i=0, j=0; i<ulDataSz; i++) {
        *(pucBuf + i) = i;
        if (i%512 == 0) {
            *(pucBuf + i) = j++;
        }
    }

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file write ng <not open file>\r\n");
    }

    /* �t�@�C���I�[�v���� GRP_FS_O_DIRECT_IO ���w�肵�Ă���ꍇ�� */
    /* ��L���b�V���̈�̃o�b�t�@�������ɓn���܂� */
    iRet = grp_fs_write(g_iHandle1, pucBuf, ulDataSz);
    if (iRet >= 0) {    /* ok */
        iRet = 0;
        GRDBG_PRINT("     O file write ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file write ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_read_file                                                             */
/*                                                                                              */
/* DESCRIPTION: �t�@�C������f�[�^��ǂݍ��݂܂�                                                */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_read_file(void)
{
    int                             iRet;
    UINT8 *                         pucBuf = (UINT8 *)g_pucTestRdBuf;
    UINT32                          ulDataSz = _TEST_DATA_BUF_SIZE;

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file read ng <not open file>\r\n");
    }

    /* �o�b�t�@�̏����� */
    grp_std_memset(pucBuf, 0, ulDataSz);

    /* �t�@�C���I�[�v���� GRP_FS_O_DIRECT_IO ���w�肵�Ă���ꍇ�� */
    /* ��L���b�V���̈�̃o�b�t�@�������ɓn���܂� */
    iRet = grp_fs_read(g_iHandle1, pucBuf, ulDataSz);
    if (iRet >= 0) {    /* ok */
        iRet = 0;
        GRDBG_PRINT("     O file read ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file read ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_del_file                                                              */
/*                                                                                              */
/* DESCRIPTION: �t�@�C�����폜���܂�                                                            */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_del_file(void)
{
    int                             iRet;

    if (g_iHandle1 != -1) {
        /* error */
        GRDBG_PRINT("     X file delete ng <opened file>\r\n");
    }

    iRet = grp_fs_unlink(g_aucTestFileName);
    if (iRet == 0) {    /* ok */
        GRDBG_PRINT("     O file delete ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file delete ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_check_file                                                            */
/*                                                                                              */
/* DESCRIPTION: �ǂݍ��񂾃t�@�C�����������񂾃f�[�^�Ɣ�r���܂�                                */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_check_file(void)
{
    int                             iRet;

    iRet = grp_std_memcmp(g_pucTestWrBuf, g_pucTestRdBuf, _TEST_DATA_BUF_SIZE);
    if (iRet == 0) {    /* ok */
        GRDBG_PRINT("     O file check ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file check ng\r\n");
    }

    /* �o�b�t�@�̏����� */
    grp_std_memset(g_pucTestRdBuf, 0, _TEST_DATA_BUF_SIZE);

    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_AppTask                                                               */
/*                                                                                              */
/* DESCRIPTION: �e�X�g�A�v���P�[�V�����p�̃^�X�N                                                */
/*              �{�^�X�N�ł̓O���[�o���ϐ�(g_ulTestNum)��ύX���邱�ƂŐF�X�ȃe�X�g�����{���܂� */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulArg                           reserved                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
UINT32                          g_ulTestNum = 0;
/*----------------------------------------------------------------------------------------------*/
LOCAL void _usb_test_AppTask( UINT32 ulArg)
{
    /* ���[�j���O�΍� */
    ulArg = 0;

    while (1) {
        switch (g_ulTestNum) {
        case 0:     /* �A�C�h�� */
            GRP_VOS_DelayTask(100);                                                 break;
        case 1:     /* �}�E���g */
            _usb_test_mount();                              g_ulTestNum = 0;        break;
        case 2:     /* �A���}�E���g */
            _usb_test_unmount();                            g_ulTestNum = 0;        break;
        case 3:     /* �t�@�C�����X�g */
            _usb_test_ls();                                 g_ulTestNum = 0;        break;
        case 5:     /* �����}�E���g */
            _usb_test_force_mount();                        g_ulTestNum = 0;        break;
        case 6:     /* �����A���}�E���g */
            _usb_test_force_unmount();                      g_ulTestNum = 0;        break;
        case 7:     /* �t�H�[�}�b�g */
            _usb_test_format();                             g_ulTestNum = 0;        break;
        case 10:    /* �t�@�C���I�[�v�� */
            _usb_test_open_file();                          g_ulTestNum = 0;        break;
        case 11:    /* �t�@�C���N���[�Y */
            _usb_test_close_file();                         g_ulTestNum = 0;        break;
        case 12:    /* �t�@�C�����C�g g_pucTestWrBuf �ɐݒ肵�t�@�C���֏������� */
            _usb_test_write_file();                         g_ulTestNum = 0;        break;
        case 13:    /* �t�@�C�����[�h g_pucTestRdBuf �Ƀt�@�C�����ǂ݂��� */
            _usb_test_read_file();                          g_ulTestNum = 0;        break;
        case 14:    /* �t�@�C���폜 */
            _usb_test_del_file();                           g_ulTestNum = 0;        break;
        case 15:    /* �t�@�C����r g_pucTestWrBuf �� g_pucTestRdBuf �̔�r */
            _usb_test_check_file();                         g_ulTestNum = 0;        break;

        case 20:    /* �t�@�C���̃I�[�v���`���C�g�`�N���[�Y */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_write_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            g_ulTestNum = 0;
            break;
        case 21:    /* �t�@�C���̃I�[�v���`���[�h�`�N���[�Y */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_read_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            g_ulTestNum = 0;
            break;

        case 30:     /* �}�E���g-���X�g-�A���}�E���g�̌J��Ԃ� */
            _usb_test_mount();
            _usb_test_ls();
            _usb_test_unmount();
            GRP_VOS_DelayTask(500);
            break;
        case 31:     /* �}�E���g-���X�g-�A���}�E���g�̌J��Ԃ� */
            _usb_test_mount();
            _usb_test_unmount();
            GRP_VOS_DelayTask(500);
            break;
        case 32:    /* �t�@�C����r�A���e�X�g */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_write_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_read_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_check_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_del_file()   != 0) {              g_ulTestNum = 0;        break;}
            GRP_VOS_DelayTask(500);
            break;

        default:    /* �������e�X�g�ԍ� */
            g_ulTestNum = 0;                                                        break;
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _usb_test_AppInit                                                               */
/*                                                                                              */
/* DESCRIPTION: Initialize of thie test application.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL int _usb_test_AppInit(void)
{
    /* �t�@�C���I�[�v���� GRP_FS_O_DIRECT_IO ���w�肵�Ă���ꍇ�� */
    /* ��L���b�V���̈�̃A�h���X��ݒ肵�܂� */
    g_pucTestWrBuf = (UINT8 *)g_aulTestWrBuf;
    g_pucTestRdBuf = (UINT8 *)g_aulTestRdBuf;

    /*--- ���f�B�A�}�������^�X�N�̍쐬 ---*/
    if (GRP_VOS_CreateTask( &g_ptConDconTask,
                            _CONDCON_TASK_NAME,
                            _usb_test_ConDconTask,
                            _CONDCON_TASK_STACK,
                            _CONDCON_TASK_PRI,
                            _CONDCON_TASK_STAT, 0) != GRP_VOS_POS_RESULT) {
        /* error */
        return USB_TEST_NG;
    }

    /*--- �e�X�g�A�v���P�[�V�����p�^�X�N�̍쐬 ---*/
    if (GRP_VOS_CreateTask( &g_ptAppTask,
                            _APP_TASK_NAME,
                            _usb_test_AppTask,
                            _APP_TASK_STACK,
                            _APP_TASK_PRI,
                            _APP_TASK_STAT, 0) != GRP_VOS_POS_RESULT) {
        /* error */
        return USB_TEST_NG;
    }

    return USB_TEST_OK;
}

/*----------------------------------------------------------------------------------------------*/
/*  �ȉ��̊֐���GR-FILE�̃��C�u�����������肽���Ȃ����߁A�_�~�[�̊֐��Ƃ��ėp�ӂ��Ă����B       */
/*  cons_getchar�͒ʏ�proc_event���̃A�v���ł����g���Ă��Ȃ��̂ŁA���̃f���ł͎g�p���Ȃ�����  */
/*  �ɂ���B                                                                                    */
/*----------------------------------------------------------------------------------------------*/

/************************************************************************************************/
/* FUNCTION   : cons_putchar                                                                    */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : mode                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
int cons_putchar(int iChar)
{
    return 0;
}

/************************************************************************************************/
/* FUNCTION   : cons_getchar                                                                    */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : mode                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
int cons_getchar(int iMode)
{
    return 0;
}
