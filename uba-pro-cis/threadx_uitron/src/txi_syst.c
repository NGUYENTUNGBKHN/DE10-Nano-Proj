/********************************************************************************/
/*                Copyright (C) 2002-2017 Grape Systems, Inc.                   */
/*                     All Rights Reserved.                                     */
/*                                                                              */
/*  This software is furnished under a license and may be used and copied only  */
/*  in accordance with the terms of such license and with the inclusion of the  */
/*  above copyright notice. No title to and ownership of the software is        */
/*  transferred.                                                                */
/*  Grape Systems Inc. makes no warranties with respect to the performance of   */
/*  this computer program, and specifically disclaims any responsibility for    */
/*  any damages, special or consequential, connected with the use of this       */
/*  program.                                                                    */
/*                                                                              */
/********************************************************************************/

/********************************************************************************/
/*  txi_syst.c                                                                  */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  System thread functions                                                     */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"
#include "tx_queue.h"

static TX_QUEUE _kernel_system_queue;
static ULONG _kernel_system_queue_data[(SYSTEM_QUEUE_SIZE)*4];

static TX_THREAD _kernel_system_thread;
static char     _kernel_system_thread_stack[SYSTEM_STACK_SIZE];

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_system_thread_entry                                             */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      System data queue thread.                                               */
/*                                                                              */
/* INPUT                                                                        */
/*      entry_input         Thread data                                         */
/*                                                                              */
/* OUTPUT                                                                       */
/*      NONE                                                                    */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_queue_receive           Receive from data queue                     */
/*      Entry function                                                          */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Internal                                                                */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s),           */
/*                                  TX_INTERRUPT_SAVE_AREA and TX_DISABLE which */
/*                                  do not have a meaning in using it were      */
/*                                  deleted.                                    */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
static void _kernel_system_thread_entry(ULONG entry_input)
{
    UINT status;
    TX_QUEUE* system_queue = (TX_QUEUE*)entry_input;
    ULONG rcv_msg[2];
    void (*function)(void*);

    while(system_queue!=NULL)
    {
        status = _tx_queue_receive(system_queue, rcv_msg, TX_WAIT_FOREVER);
        if(status==TX_SUCCESS)
        {
            function = (void (*)(void*))rcv_msg[0];
            (*function)((void*)rcv_msg[1]);
        }
    }
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_system_request                                                  */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Processing is required of system thread.                                */
/*                                                                              */
/* INPUT                                                                        */
/*      function                    Entry function                              */
/*      param                       Parameter                                   */
/*                                                                              */
/* OUTPUT                                                                       */
/*      Send status                                                             */
/*                                                                              */
/* CALLS                                                                        */
/*      _tx_queue_send              Send data queue                             */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Internal                                                                */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
BOOL _kernel_system_request(void (*function)(void*), void* param)
{
    UINT status;
    ULONG snd_msg[2];

    snd_msg[0] = (ULONG)function;
    snd_msg[1] = (ULONG)param;
    status = _tx_queue_send(&_kernel_system_queue, snd_msg, TX_NO_WAIT);

    return (status==TX_SUCCESS);
}

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_system_thread_init                                              */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Create system data queue thread.                                        */
/*                                                                              */
/* INPUT                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* OUTPUT                                                                       */
/*      Create status                                                           */
/*                                                                              */
/* CALLS                                                                        */
/*      tx_queue_create             Create data queue                           */
/*      _tx_thread_create           Create thread                               */
/*      tx_queue_delete             Delete data queue                           */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Internal                                                                */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
BOOL _kernel_system_thread_init(void)
{
    if(tx_queue_create(&_kernel_system_queue, "uiSYSQ", TX_2_ULONG,
                    _kernel_system_queue_data, sizeof(_kernel_system_queue_data))==TX_SUCCESS)
    {
        if(_tx_thread_create(&_kernel_system_thread, "uiSYST",
                        _kernel_system_thread_entry, (ULONG)&_kernel_system_queue,
                        _kernel_system_thread_stack, SYSTEM_STACK_SIZE, 0, 0,
                        TX_NO_TIME_SLICE, TX_AUTO_START)==TX_SUCCESS)
        {
            return TRUE;
        }

        tx_queue_delete(&_kernel_system_queue);
    }

    return FALSE;
}

