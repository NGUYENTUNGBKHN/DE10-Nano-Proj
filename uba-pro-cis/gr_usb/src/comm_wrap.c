/****************************************************************************/
/*                                                                          */
/* FILE NAME                                                    VERSION     */
/*                                                                          */
/*      comm_wrap.c                                               1.00      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      It is a layer for making it compatible with the previous project.   */
/*                                                                          */
/* FANCTIONS:                                                               */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME        DATE       REMARKS                                         */
/*                                                                          */
/*   K.Jinnouchi 2017/11/06 V1.00                                           */
/*                          Created initial version                         */
/*                                                                          */
/****************************************************************************/
#include    "comm_wrap.h"
#include    "comm_def.h"
#include    "peri_hal.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* Descriptor Information */

/* APPLICATION CALLBACK FUNCTION PROTOTYPE */
LOCAL GRUSB_COMD_WRAP_INITINFO   l_tInitInfo;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
LOCAL BOOLEAN _GRUSB_COMD_CbRcvCmd( INT, UINT8*, UINT32, VOID*, INT);
LOCAL VOID _GRUSB_COMD_CbSndSts( INT, UINT8*, UINT32, VOID*, INT);
LOCAL BOOLEAN _GRUSB_COMD_CbRcvData( INT, UINT8*, UINT32, VOID*, INT);
LOCAL VOID _GRUSB_COMD_CbSndData( INT, UINT8*, UINT32, VOID*, INT);
LOCAL VOID _GRUSB_COMD_CbSndSts( INT, UINT8*, UINT32, VOID*, INT);
LOCAL VOID _GRUSB_COMD_CbSuspend( VOID );
LOCAL VOID _GRUSB_COMD_CbResume( VOID );

/****************************************************************************/
/* FUNCTION   : GRUSB_COMM_WRAP_Init                                        */
/*                                                                          */
/* DESCRIPTION: Register the callback functions.                            */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptInitInfo          Initial setting information structure   */
/* OUTPUT     : none                                                        */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_COMD_WRAP_Init( GRUSB_COMD_WRAP_INITINFO*   ptInitInfo )
{
    /* Initialization of a Callback Function */
    l_tInitInfo.pfnRecvCommand = GRUSB_NULL;
    l_tInitInfo.pfnSendStatus  = GRUSB_NULL;
    l_tInitInfo.pfnRecvData    = GRUSB_NULL;
    l_tInitInfo.pfnSendData    = GRUSB_NULL;
    l_tInitInfo.pfnSuspend     = GRUSB_NULL;
    l_tInitInfo.pfnResume      = GRUSB_NULL;

    /* set of Callback Functions */
    if( ptInitInfo->pfnRecvCommand ) {
        l_tInitInfo.pfnRecvCommand = ptInitInfo->pfnRecvCommand;
    }
    if( ptInitInfo->pfnSendStatus ) {
        l_tInitInfo.pfnSendStatus = ptInitInfo->pfnSendStatus;
    }
    if( ptInitInfo->pfnRecvData ) {
        l_tInitInfo.pfnRecvData = ptInitInfo->pfnRecvData;
    }
    if( ptInitInfo->pfnSendData ) {
        l_tInitInfo.pfnSendData = ptInitInfo->pfnSendData;
    }
    if( ptInitInfo->pfnSuspend ) {
        l_tInitInfo.pfnSuspend = ptInitInfo->pfnSuspend;
    }
    if( ptInitInfo->pfnResume ) {
        l_tInitInfo.pfnResume = ptInitInfo->pfnResume;
    }

    GRUSB_DEV_HALCallbackSuspend( _GRUSB_COMD_CbSuspend );
    GRUSB_DEV_HALCallbackResume( _GRUSB_COMD_CbResume );
    
    return;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_RecvData                                    */
/*                                                                          */
/* DESCRIPTION: The request to receive of data.                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data buffer                     */
/*              pucData             pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS          Successfully end                */
/*              GRUSB_COMD_ERROR            Recieve request error           */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_WRAP_RecvData( UINT32  ulSize,
                                 UINT8*   pucData,
                                 VOID*    pAplInfo )
{
    INT     iStat;

    /* Interrupt reception is started */
    iStat = GRUSB_DEV_ApTransferRecv(GRCOMD_BULKOUT_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     pAplInfo,
                                     _GRUSB_COMD_CbRcvData);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbRcvData                                       */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_RecvData.          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL BOOLEAN _GRUSB_COMD_CbRcvData( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo.pfnRecvData)
        (*l_tInitInfo.pfnRecvData)(ulSize, pucBuf, pAplInfo);
    
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_SendData                                    */
/*                                                                          */
/* DESCRIPTION: The request to send of data.                                */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data buffer                     */
/*              pucData             pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_VNDD_SUCCESS          Successfully end                */
/*              GRUSB_VNDD_RECV_REQ_ERROR   Recieve request error           */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_WRAP_SendData( UINT32  ulSize,
                                 UINT8*   pucData,
                                 VOID*    pAplInfo )
{
    INT     iStat;

    /* Interrupt reception is started */
    iStat = GRUSB_DEV_ApTransferSend(GRCOMD_BULKIN_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     GRUSB_ADD_0LENPKT,
                                     pAplInfo,
                                     _GRUSB_COMD_CbSndData);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSndData                                       */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_SendData.          */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbSndData( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo.pfnSendData)
        (*l_tInitInfo.pfnSendData)(ulSize, pucBuf, pAplInfo);
}


/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_RecvCommand                                 */
/*                                                                          */
/* DESCRIPTION: The request to receive of the command.                      */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data buffer                     */
/*              pucData             pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_VNDD_SUCCESS          Successfully end                */
/*              GRUSB_VNDD_RECV_REQ_ERROR   Recieve request error           */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_WRAP_RecvCommand( UINT32  ulSize,
                                 UINT8*   pucData,
                                 VOID*    pAplInfo )
{
    INT     iStat;

    /* Interrupt reception is started */
    iStat = GRUSB_DEV_ApTransferRecv(GRCOMD_BULKOUT_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     pAplInfo,
                                     _GRUSB_COMD_CbRcvCmd);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbRcvCmd                                        */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_RecvCommand.       */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL BOOLEAN _GRUSB_COMD_CbRcvCmd( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo.pfnRecvCommand) {
        (*l_tInitInfo.pfnRecvCommand)(ulSize, pucBuf, pAplInfo);
    }
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_SendStatus                                  */
/*                                                                          */
/* DESCRIPTION: The request to send of the status.                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              size of data buffer                     */
/*              pucData             pointer of data buffer                  */
/*              pAplInfo            pointer of Application's information    */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS          Successfully end                */
/*              GRUSB_COMD_ERROR            Receive request error           */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_WRAP_SendStatus( UINT32   ulSize,
                           UINT8*   pucData,
                           VOID*    pAplInfo )
{
    INT     iStat;

    /* Bulk reception is started */
    iStat = GRUSB_DEV_ApTransferSend(GRCOMD_BULKIN_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     GRUSB_ADD_0LENPKT,
                                     pAplInfo,
                                     _GRUSB_COMD_CbSndSts);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSndSts                                        */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_SendStatus.        */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pucBuf              pointer of data buffer                  */
/*              ulSize              size of data                            */
/*              pAplInfo            pointer of Application's information    */
/*              iStat               status                                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbSndSts( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo.pfnSendStatus) {
        (*l_tInitInfo.pfnSendStatus)(ulSize, pucBuf, pAplInfo);
    }
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSuspend                                       */
/*                                                                          */
/* DESCRIPTION: Notice processing of Suspend.                               */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbSuspend( VOID )
{
    /* Callback function is called */
    if( l_tInitInfo.pfnSuspend) {
        (*l_tInitInfo.pfnSuspend)();
    }
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbResume                                        */
/*                                                                          */
/* DESCRIPTION: Notice processing of Resume.                                */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbResume( VOID )
{
    /* Callback function is called */
    if( l_tInitInfo.pfnResume) {
        (*l_tInitInfo.pfnResume)();
    }
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_RecvAbort                                   */
/*                                                                          */
/* DESCRIPTION: The request to abort receiving.                             */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS          Successfully end                */
/*              GRUSB_COMD_ERROR            Abort request error             */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_WRAP_RecvAbort( VOID )
{
    INT     iStat;

    iStat = GRUSB_DEV_ApAbort( GRCOMD_BULKOUT_EP_NUMBER );
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_SendAbort                                   */
/*                                                                          */
/* DESCRIPTION: The request to abort sending.                               */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_COMD_SUCCESS          Successfully end                */
/*              GRUSB_COMD_ERROR            Abort request error             */
/*                                                                          */
/****************************************************************************/
INT GRUSB_COMD_WRAP_SendAbort( VOID )
{
    INT     iStat;

    iStat = GRUSB_DEV_ApAbort( GRCOMD_BULKIN_EP_NUMBER );
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

