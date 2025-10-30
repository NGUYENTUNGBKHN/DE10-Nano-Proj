/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2019 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This software is copyrighted by and is the sole property of Express   */ 
/*  Logic, Inc.  All rights, title, ownership, or other interests         */ 
/*  in the software remain the property of Express Logic, Inc.  This      */ 
/*  software may only be used in accordance with the corresponding        */ 
/*  license agreement.  Any unauthorized use, duplication, transmission,  */ 
/*  distribution, or disclosure of this software is expressly forbidden.  */ 
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */ 
/*  written consent of Express Logic, Inc.                                */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               www.expresslogic.com          */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** ThreadX Component                                                     */ 
/**                                                                       */
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_suspend                                 PORTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles application suspend requests.  If the suspend */ 
/*    requires actual processing, this function calls the actual suspend  */ 
/*    thread routine.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to thread to suspend  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Return completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_suspend         Actual thread suspension          */ 
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-15-2012     William E. Lamie         Initial Version 5.5           */ 
/*  07-15-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  12-12-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.1  */ 
/*  05-01-2014     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.2  */ 
/*  09-01-2015     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.7    */ 
/*  04-01-2016     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.7.1  */ 
/*  08-03-2016     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.7.2  */ 
/*  06-01-2017     William E. Lamie         Modified comment(s), modified */ 
/*                                            code for MISRA compliance,  */ 
/*                                            and added protection against*/ 
/*                                            self suspension with the    */ 
/*                                            preempt-disable flag set,   */ 
/*                                            resulting in version 5.8    */ 
/*  02-01-2019     William E. Lamie         Modified comment(s), and      */ 
/*                                            refined error checking for  */ 
/*                                            self suspension to ensure   */ 
/*                                            thread context, resulting   */ 
/*                                            in version 5.9              */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_thread_suspend(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD  *current_thread;
UINT        status;                         
UINT        core_index;


    /* Lockout interrupts while the thread is being suspended.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SUSPEND_API, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_SUSPEND_INSERT

    /* Check the specified thread's current status.  */
    if (thread_ptr -> tx_thread_state == TX_READY)
    {

        /* Initialize status to success.  */
        status =  TX_SUCCESS;

        /* Pickup the index.  */
        core_index =  TX_SMP_CORE_ID;

        /* Determine if we are in a thread context.  */
        if (_tx_thread_system_state[core_index] == ((ULONG) 0))
        {
        
            /* Yes, we are in a thread context.  */

            /* Determine if the current thread is also the suspending thread.  */
            if (current_thread == thread_ptr)
            {
        
                /* Determine if the preempt disable flag is non-zero.  */
                if (_tx_thread_preempt_disable != ((UINT) 0))
                {

                    /* Thread is terminated or completed.  */
                    status =  TX_SUSPEND_ERROR;
                }
            }
        }

        /* Determine if the status is still successful.  */
        if (status == TX_SUCCESS)
        {           

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;

#ifdef TX_NOT_INTERRUPTABLE

            /* Call actual non-interruptable thread suspension routine.  */
            _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Set the suspending flag. */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Setup for no timeout period.  */
            thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Call actual thread suspension routine.  */
            _tx_thread_system_suspend(thread_ptr);
#endif

#ifdef TX_MISRA_ENABLE

            /* Disable interrupts.  */
            TX_DISABLE
        
            /* Return success.  */
            status =  TX_SUCCESS;
#else

            /* If MISRA is not enabled, return directly.  */
            return(TX_SUCCESS);
#endif            
        }
    }
    else if (thread_ptr -> tx_thread_state == TX_TERMINATED)
    {

        /* Thread is terminated.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_COMPLETED)
    {

        /* Thread is completed.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_SUSPENDED)
    {

        /* Already suspended, just set status to success.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* Just set the delayed suspension flag.  */
        thread_ptr -> tx_thread_delayed_suspend =  TX_TRUE;

        /* Set status to success.  */
        status =  TX_SUCCESS;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Always return success, since this function does not perform error 
       checking.  */
    return(status);
}

