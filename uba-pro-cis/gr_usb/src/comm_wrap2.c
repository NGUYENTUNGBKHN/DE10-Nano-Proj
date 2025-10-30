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
LOCAL GRUSB_COMD_WRAP_INITINFO   l_tInitInfo2;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
LOCAL BOOLEAN _GRUSB_COMD_CbRcvCmd2( INT, UINT8*, UINT32, VOID*, INT);
LOCAL VOID _GRUSB_COMD_CbSndSts2( INT, UINT8*, UINT32, VOID*, INT);
LOCAL BOOLEAN _GRUSB_COMD_CbRcvData2( INT, UINT8*, UINT32, VOID*, INT);
LOCAL VOID _GRUSB_COMD_CbSndData2( INT, UINT8*, UINT32, VOID*, INT);
//LOCAL VOID _GRUSB_COMD_CbSndSts2( INT, UINT8*, UINT32, VOID*, INT);
LOCAL VOID _GRUSB_COMD_CbSuspend2( VOID );
LOCAL VOID _GRUSB_COMD_CbResume2( VOID );

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_Init2                                       */
/*                                                                          */
/* DESCRIPTION: Register the callback functions.                            */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptInitInfo          Initial setting information structure   */
/* OUTPUT     : none                                                        */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_COMD_WRAP_Init2( GRUSB_COMD_WRAP_INITINFO*   ptInitInfo )
{
    /* Initialization of a Callback Function */
    l_tInitInfo2.pfnRecvCommand = GRUSB_NULL;
    l_tInitInfo2.pfnSendStatus  = GRUSB_NULL;
    l_tInitInfo2.pfnRecvData    = GRUSB_NULL;
    l_tInitInfo2.pfnSendData    = GRUSB_NULL;
    l_tInitInfo2.pfnSuspend     = GRUSB_NULL;
    l_tInitInfo2.pfnResume      = GRUSB_NULL;

    /* set of Callback Functions */
    if( ptInitInfo->pfnRecvCommand ) {
        l_tInitInfo2.pfnRecvCommand = ptInitInfo->pfnRecvCommand;
    }
    if( ptInitInfo->pfnSendStatus ) {
        l_tInitInfo2.pfnSendStatus = ptInitInfo->pfnSendStatus;
    }
    if( ptInitInfo->pfnRecvData ) {
        l_tInitInfo2.pfnRecvData = ptInitInfo->pfnRecvData;
    }
    if( ptInitInfo->pfnSendData ) {
        l_tInitInfo2.pfnSendData = ptInitInfo->pfnSendData;
    }
    if( ptInitInfo->pfnSuspend ) {
        l_tInitInfo2.pfnSuspend = ptInitInfo->pfnSuspend;
    }
    if( ptInitInfo->pfnResume ) {
        l_tInitInfo2.pfnResume = ptInitInfo->pfnResume;
    }

    GRUSB_DEV_HALCallbackSuspend2( _GRUSB_COMD_CbSuspend2 );
    GRUSB_DEV_HALCallbackResume2( _GRUSB_COMD_CbResume2 );
    
    return;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_RecvData2                                   */
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
INT GRUSB_COMD_WRAP_RecvData2( UINT32  ulSize,
                                 UINT8*   pucData,
                                 VOID*    pAplInfo )
{
    INT     iStat;

    /* Interrupt reception is started */
    iStat = GRUSB_DEV_ApTransferRecv2(GRCOMD_BULKOUT_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     pAplInfo,
                                     _GRUSB_COMD_CbRcvData2);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbRcvData2                                      */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_RecvData2          */
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
LOCAL BOOLEAN _GRUSB_COMD_CbRcvData2( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo2.pfnRecvData)
        (*l_tInitInfo2.pfnRecvData)(ulSize, pucBuf, pAplInfo);
    
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_SendData2                                   */
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
INT GRUSB_COMD_WRAP_SendData2( UINT32  ulSize,
                                 UINT8*   pucData,
                                 VOID*    pAplInfo )
{
    INT     iStat;

    /* Interrupt reception is started */
    iStat = GRUSB_DEV_ApTransferSend2(GRCOMD_BULKIN_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     GRUSB_ADD_0LENPKT,
                                     pAplInfo,
                                     _GRUSB_COMD_CbSndData2);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSndData2                                      */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_SendData2          */
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
LOCAL VOID _GRUSB_COMD_CbSndData2( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo2.pfnSendData)
        (*l_tInitInfo2.pfnSendData)(ulSize, pucBuf, pAplInfo);
}


/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_RecvCommand2                                 */
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
INT GRUSB_COMD_WRAP_RecvCommand2( UINT32  ulSize,
                                 UINT8*   pucData,
                                 VOID*    pAplInfo )
{
    INT     iStat;

    /* Interrupt reception is started */
    iStat = GRUSB_DEV_ApTransferRecv2(GRCOMD_BULKOUT_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     pAplInfo,
                                     _GRUSB_COMD_CbRcvCmd2);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbRcvCmd2                                        */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_RecvCommand2.       */
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
LOCAL BOOLEAN _GRUSB_COMD_CbRcvCmd2( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo2.pfnRecvCommand) {
        (*l_tInitInfo2.pfnRecvCommand)(ulSize, pucBuf, pAplInfo);
    }
    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_SendStatus2                                 */
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
INT GRUSB_COMD_WRAP_SendStatus2( UINT32   ulSize,
                           UINT8*   pucData,
                           VOID*    pAplInfo )
{
    INT     iStat;

    /* Bulk reception is started */
    iStat = GRUSB_DEV_ApTransferSend2(GRCOMD_BULKIN_EP_NUMBER,
                                     pucData,
                                     ulSize,
                                     GRUSB_ADD_0LENPKT,
                                     pAplInfo,
                                     _GRUSB_COMD_CbSndSts2);
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSndSts2                                       */
/*                                                                          */
/* DESCRIPTION: The callback function of GRUSB_COMD_WRAP_SendStatus2        */
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
LOCAL VOID _GRUSB_COMD_CbSndSts2( INT        iEpNo,
                                 UINT8*     pucBuf,
                                 UINT32     ulSize,
                                 VOID*      pAplInfo,
                                 INT        iStat )
{
    /* Callback function is called */
    if (l_tInitInfo2.pfnSendStatus) {
        (*l_tInitInfo2.pfnSendStatus)(ulSize, pucBuf, pAplInfo);
    }
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbSuspend2                                      */
/*                                                                          */
/* DESCRIPTION: Notice processing of Suspend.                               */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbSuspend2( VOID )
{
    /* Callback function is called */
    if( l_tInitInfo2.pfnSuspend) {
        (*l_tInitInfo2.pfnSuspend)();
    }
}

/****************************************************************************/
/* FUNCTION   : _GRUSB_COMD_CbResume2                                       */
/*                                                                          */
/* DESCRIPTION: Notice processing of Resume.                                */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
LOCAL VOID _GRUSB_COMD_CbResume2( VOID )
{
    /* Callback function is called */
    if( l_tInitInfo2.pfnResume) {
        (*l_tInitInfo2.pfnResume)();
    }
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_RecvAbort2                                  */
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
INT GRUSB_COMD_WRAP_RecvAbort2( VOID )
{
    INT     iStat;

    iStat = GRUSB_DEV_ApAbort2( GRCOMD_BULKOUT_EP_NUMBER );
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_WRAP_SendAbort2                                  */
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
INT GRUSB_COMD_WRAP_SendAbort2( VOID )
{
    INT     iStat;

    iStat = GRUSB_DEV_ApAbort2( GRCOMD_BULKIN_EP_NUMBER );
    if (GRUSB_DEV_SUCCESS != iStat) {
        return GRUSB_COMD_ERROR;
    }

    return GRUSB_COMD_SUCCESS;
}

