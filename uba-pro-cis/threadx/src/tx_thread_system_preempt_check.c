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
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_system_preempt_check                    PORTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for preemption that could have occurred as a   */ 
/*    result scheduling activities occurring while the preempt disable    */ 
/*    flag was set.                                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_return              Return to the system          */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Other ThreadX Components                                            */
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
/*  06-01-2017     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.8                 */ 
/*  02-01-2019     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.9    */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  _tx_thread_system_preempt_check(VOID)
{

TX_INTERRUPT_SAVE_AREA

UINT            core_index;
UINT            restore_needed;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Set the restore needed flag.  */
    restore_needed =  TX_TRUE;

    /* Pickup the index.  */
    core_index =  TX_SMP_CORE_ID;

    /* Determine if the call is from initialization, an ISR or if the preempt disable flag is set.  */
    if (_tx_thread_system_state[core_index] == ((ULONG) 0))
    {
    
        /* Ensure the preempt disable flag is not set.  */
        if (_tx_thread_preempt_disable == ((UINT) 0))
        {
    
            /* Thread execution - now determine if preemption should take place.  */
            if (_tx_thread_current_ptr[core_index] != _tx_thread_execute_ptr[core_index])
            {

                /* Yes, thread preemption should take place.  */

#ifdef TX_ENABLE_STACK_CHECKING 
            TX_THREAD   *thread_ptr;

                /* Pickup the next execute pointer.  */
                thread_ptr =  _tx_thread_execute_ptr[core_index];

                /* Determine if there is a thread pointer.  */
                if (thread_ptr != TX_NULL)
                {

                    /* Check this thread's stack.  */
                    TX_THREAD_STACK_CHECK(thread_ptr)
                }
#endif

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

                /* Determine if an idle system return is present.  */
                if (_tx_thread_execute_ptr[core_index] == TX_NULL)
                {

                    /* Yes, increment the return to idle return count.  */
                    _tx_thread_performance_idle_return_count++;
                }
                else
                {

                    /* No, there is another thread ready to run and will be scheduled upon return.  */
                    _tx_thread_performance_non_idle_return_count++;
                }
#endif

#ifndef TX_NOT_INTERRUPTABLE

                /* Increment the preempt disable flag in order to keep the protection.  */
                _tx_thread_preempt_disable++;
        
                /* Restore interrupts.  */
                TX_RESTORE
#endif

                /* Return to the system so the higher priority thread can be scheduled.  */
                _tx_thread_system_return();
                
#ifdef TX_NOT_INTERRUPTABLE

                /* Restore interrupts.  */
                TX_RESTORE
#endif

                /* Clear the restore needed flag, since the interrupt poster/protection has been done.  */
                restore_needed =  TX_FALSE;
            }
        }
    }

    /* Determine if the protection still needs to be restored.  */
    if (restore_needed == TX_TRUE)
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }
}

