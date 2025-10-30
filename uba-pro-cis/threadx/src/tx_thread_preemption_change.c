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
#include "tx_initialize.h"
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_preemption_change                       PORTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes preemption-threshold change requests.  The  */ 
/*    previous preemption is returned to the caller.  If the new request  */ 
/*    allows a higher priority thread to execute, preemption takes place  */ 
/*    inside of this function.                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to thread             */ 
/*    new_threshold                         New preemption-threshold      */ 
/*    old_threshold                         Old preemption-threshold      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Service return status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_smp_rebalance_execute_list Rebalance the execution list  */ 
/*    _tx_thread_system_preempt_check       Check for preemption          */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-15-2012     William E. Lamie         Initial Version 5.5           */ 
/*  07-15-2012     William E. Lamie         Modified comment(s), fixed    */ 
/*                                            problem with preemption-    */ 
/*                                            threshold disabled, and     */ 
/*                                            added SMP optimizations,    */ 
/*                                            resulting in version 5.6    */ 
/*  12-12-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.1  */ 
/*  05-01-2014     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.2  */ 
/*  09-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            added optimization to avoid */ 
/*                                            calling rebalance algorithm,*/ 
/*                                            resulting in version 5.7    */ 
/*  04-01-2016     William E. Lamie         Modified comment(s), and      */ 
/*                                            corrected problem disabling */ 
/*                                            preemption-threshold,       */ 
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
UINT  _tx_thread_preemption_change(TX_THREAD *thread_ptr, UINT new_threshold, UINT *old_threshold)
{

TX_INTERRUPT_SAVE_AREA

UINT        core_index;

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD
ULONG       priority_bit;
UINT        base_priority;
UINT        priority_bit_set;
ULONG       priority_map;                  
UINT        next_preempted;
TX_THREAD   *preempted_thread;
#if TX_MAX_PRIORITIES > 32
UINT        map_index;
#endif
#endif
UINT        status;


#ifdef TX_DISABLE_PREEMPTION_THRESHOLD

    /* Only allow 0 (disable all preemption) and returning preemption-threshold to the 
       current thread priority if preemption-threshold is disabled. All other threshold
       values are converted to 0.  */
    if (new_threshold < thread_ptr -> tx_thread_user_priority)
    {
    
        /* Is the new threshold zero?  */
        if (new_threshold != ((UINT) 0))
        {
        
            /* Convert the new threshold to disable all preemption, since preemption-threshold is
               not supported.  */
            new_threshold =  ((UINT) 0);
        }
    }
#endif

    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_PREEMPTION_CHANGE, thread_ptr, new_threshold, thread_ptr -> tx_thread_preempt_threshold, thread_ptr -> tx_thread_state, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_PREEMPTION_CHANGE_INSERT

    /* Determine if the new threshold is greater than the current user priority.  */
    if (new_threshold > thread_ptr -> tx_thread_user_priority)
    {
    
        /* Restore interrupts.  */
        TX_RESTORE
            
        /* Return error.  */
        status =  TX_THRESH_ERROR;
    }
    else
    {

        /* Return the user's preemption-threshold.   */
        *old_threshold =  thread_ptr -> tx_thread_user_preempt_threshold;

        /* Setup the new threshold.  */
        thread_ptr -> tx_thread_user_preempt_threshold =  new_threshold;

        /* Determine if the new threshold represents a higher priority than the priority inheritance threshold.  */
        if (new_threshold < thread_ptr -> tx_thread_inherit_priority)
        {
    
            /* Update the actual preemption-threshold with the new threshold.  */
            thread_ptr -> tx_thread_preempt_threshold =  new_threshold;
        }
        else
        {
    
            /* Update the actual preemption-threshold with the priority inheritance.  */
            thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_inherit_priority;
        }

        /* Determine if the thread is ready and scheduled.  */
        if (thread_ptr -> tx_thread_state == TX_READY)
        {
        
#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

            /* Determine if the new threshold is the same as the priority.  */
            if (thread_ptr -> tx_thread_user_priority == new_threshold)
            {

                /* Yes, preemption-threshold is being disabled.  */

                /* Determine if this thread was scheduled with preemption-threshold in force.  */
                if (_tx_thread_preemption_threshold_list[thread_ptr -> tx_thread_user_priority] == thread_ptr)
                {

                    /* Clear the entry in the preempted list.  */
                    _tx_thread_preemption_threshold_list[thread_ptr -> tx_thread_user_priority] =  TX_NULL;

#if TX_MAX_PRIORITIES > 32

                    /* Calculate the index into the bit map array.  */
                    map_index =  (thread_ptr -> tx_thread_user_priority)/((UINT) 32);
#endif

                    /* Yes, this thread is at the front of the list.  Make sure
                       the preempted bit is cleared for this thread.  */
                    TX_MOD32_BIT_SET(thread_ptr -> tx_thread_user_priority, priority_bit)
                    _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] & (~(priority_bit));

#if TX_MAX_PRIORITIES > 32

                    /* Determine if there are any other bits set in this preempt map.  */
                    if (_tx_thread_preempted_maps[MAP_INDEX] == ((ULONG) 0))
                    {

                        /* No, clear the active bit to signify this preempt map has nothing set.  */
                        TX_DIV32_BIT_SET(thread_ptr -> tx_thread_user_priority, priority_bit)
                        _tx_thread_preempted_map_active =  _tx_thread_preempted_map_active & (~(priority_bit));
                    }
#endif
                }
            }
#endif

            /* Determine if this thread has global preemption disabled.  */
            if (thread_ptr == _tx_thread_preemption__threshold_scheduled)
            {
      
                /* Clear the global preemption disable flag.  */
                _tx_thread_preemption__threshold_scheduled =  TX_NULL;

#ifndef TX_DISABLE_PREEMPTION_THRESHOLD

                /* Calculate the first thread with preemption-threshold active.  */
#if TX_MAX_PRIORITIES > 32
                if (_tx_thread_preempted_map_active != ((ULONG) 0))
#else
                if (_tx_thread_preempted_maps[0] != ((ULONG) 0))
#endif    
                {
#if TX_MAX_PRIORITIES > 32

                    /* Calculate the index to find the next highest priority thread ready for execution.  */
                    priority_map =    _tx_thread_preempted_map_active;

                    /* Calculate the lowest bit set in the priority map. */
                    TX_LOWEST_SET_BIT_CALCULATE(priority_map, map_index)

                    /* Calculate the base priority as well.  */
                    base_priority =  map_index * ((UINT) 32);
#else

                    /* Setup the base priority to zero.  */
                    base_priority =   ((UINT) 0);
#endif

                    /* Setup temporary preempted map.  */
                    priority_map =  _tx_thread_preempted_maps[MAP_INDEX];

                    /* Calculate the lowest bit set in the priority map. */
                    TX_LOWEST_SET_BIT_CALCULATE(priority_map, priority_bit_set)

                    /* Move priority bit set into priority bit.  */
                    priority_bit =  (ULONG) priority_bit_set;

                    /* Setup the highest priority preempted thread.  */
                    next_preempted =  base_priority + priority_bit;       

                    /* Pickup the previously preempted thread.  */
                    preempted_thread =  _tx_thread_preemption_threshold_list[next_preempted];
                
                    /* Pickup the preempted thread.  */
                    _tx_thread_preemption__threshold_scheduled =  preempted_thread;
                }
#endif
            }

            /* See if preemption needs to take place.  */

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(12, 0, thread_ptr);
#endif
        
            /* Pickup the index.  */
            core_index =  TX_SMP_CORE_ID;

            /* Call the rebalance routine. This routine maps cores and ready threads.  */
            _tx_thread_smp_rebalance_execute_list(core_index);

#ifdef TX_THREAD_SMP_DEBUG_ENABLE

            /* Debug entry.  */
            _tx_thread_smp_debug_entry_insert(13, 0, thread_ptr);
#endif
        
            /* Restore interrupts.  */
            TX_RESTORE

            /* Check for preemption.  */
            _tx_thread_system_preempt_check();
        }
        else
        {
    
            /* Restore interrupts.  */
            TX_RESTORE
        }
    }

    /* Return completion status.  */
    return(status);
}

