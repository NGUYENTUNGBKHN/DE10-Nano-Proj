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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_timer.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_timer_info_get                                  PORTABLE C      */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves information from the specified timer.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timer_ptr                         Pointer to timer control block    */ 
/*    name                              Destination for the timer name    */ 
/*    active                            Destination for active flag       */ 
/*    remaining_ticks                   Destination for remaining ticks   */ 
/*                                        before expiration               */ 
/*    reschedule_ticks                  Destination for reschedule ticks  */ 
/*    next_timer                        Destination for next timer on the */ 
/*                                        created list                    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), and      */ 
/*                                            added filter option to      */ 
/*                                            trace insert, resulting     */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, merged   */ 
/*                                            event logging support, and  */ 
/*                                            added ULONG casting,        */ 
/*                                            resulting in version 5.4    */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*  06-01-2017     William E. Lamie         Modified comment(s), and      */ 
/*                                            added logic for accurate    */ 
/*                                            remaining time calculation, */ 
/*                                            resulting in version 5.8    */ 
/*  02-01-2019     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.9    */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_timer_info_get(TX_TIMER *timer_ptr, CHAR **name, UINT *active, ULONG *remaining_ticks, 
                ULONG *reschedule_ticks, TX_TIMER **next_timer)
{

TX_INTERRUPT_SAVE_AREA

TX_TIMER_INTERNAL   *internal_ptr;
TX_TIMER_INTERNAL   **list_head;
ULONG               ticks_left;
UINT                timer_active;
UINT                active_timer_list;


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_INFO_GET, timer_ptr, TX_POINTER_TO_ULONG_CONVERT(&ticks_left), 0, 0, TX_TRACE_TIMER_EVENTS)

    /* Log this kernel call.  */
    TX_EL_TIMER_INFO_GET_INSERT

    /* Retrieve the name of the timer.  */
    if (name != TX_NULL)
    {
    
        *name =  timer_ptr -> tx_timer_name;
    }
    
    /* Pickup address of internal timer structure.  */
    internal_ptr =  &(timer_ptr -> tx_timer_internal);

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Default active to false.  */
    timer_active =  TX_FALSE;
    
    /* Default the ticks left to the remaining ticks.  */
    ticks_left =  internal_ptr -> tx_timer_internal_remaining_ticks;

    /* Determine if the timer is still active.  */
    if (internal_ptr -> tx_timer_internal_list_head != TX_NULL)
    {   

        /* Indicate this timer is active.  */
        timer_active =  TX_TRUE;

        /* Default the active timer list flag to false.  */
        active_timer_list =  TX_FALSE;

        /* Determine if the timer is still active.  */
        if (internal_ptr -> tx_timer_internal_list_head >= _tx_timer_list_start)
        {

            /* Determine if the list head is before the end of the list.  */
            if (internal_ptr -> tx_timer_internal_list_head < _tx_timer_list_end)
            {

                /* This timer is active and has not yet expired.  */
                active_timer_list =  TX_TRUE;
            }
        }

        /* Determine if the timer is on the active timer list.  */
        if (active_timer_list == TX_TRUE)
        {

            /* Calculate the amount of time that has elapsed since the timer
               was activated.  */
          
            /* Setup the list head pointer.  */
            list_head =  internal_ptr -> tx_timer_internal_list_head;          

            /* Is this timer's entry after the current timer pointer?  */
            if (internal_ptr -> tx_timer_internal_list_head >= _tx_timer_current_ptr)
            {

                /* Calculate ticks left to expiration - just the difference between this 
                   timer's entry and the current timer pointer.  */
                ticks_left =  ((TX_TIMER_POINTER_DIF(list_head, _tx_timer_current_ptr)) + ((ULONG) 1));
            }
            else
            {

                /* Calculate the ticks left with a wrapped list condition.  */
                ticks_left =  ((TX_TIMER_POINTER_DIF(list_head, _tx_timer_list_start)));

                ticks_left =  ticks_left + ((TX_TIMER_POINTER_DIF(_tx_timer_list_end, _tx_timer_current_ptr)) + ((ULONG) 1));
            }

            /* Adjust the remaining ticks accordingly.  */
            if (internal_ptr -> tx_timer_internal_remaining_ticks > TX_TIMER_ENTRIES)
            {
            
                /* Subtract off the last full pass through the timer list and add the
                   time left.  */
                ticks_left =  (internal_ptr -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES) + ticks_left;
            }

        }
        else
        {
    
            /* The timer is not on the actual timer list so it must either be being processed
               or on a temporary list to be processed.   */

            /* Check to see if this timer is the timer currently being processed.  */
            if (_tx_timer_expired_timer_ptr == internal_ptr)
            {
            
                /* Timer dispatch routine is executing, waiting to execute, or just finishing. No more remaining ticks for this expiration.  */
                ticks_left =  ((ULONG) 0);               
            }
            else
            {

                /* Timer is not the one being processed, which means it must be on the temporary expiration list
                   waiting to be processed.  */
            
                /* Calculate the remaining ticks for a timer in the process of expiring.  */
                if (ticks_left > TX_TIMER_ENTRIES)
                {
                    
                    /* Calculate the number of ticks remaining.  */
                    ticks_left =  internal_ptr -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES;
                }
                else
                {
                
                    /* Timer dispatch routine is waiting to execute, no more remaining ticks for this expiration.  */
                    ticks_left =  ((ULONG) 0);               
                }
            }
        }
    }

    /* Setup return values for an inactive timer.  */
    if (active != TX_NULL)
    {
        
       /* Setup the timer active indication.  */
       *active =  timer_active;
    }
    if (remaining_ticks != TX_NULL)
    {
        
        /* Setup the default remaining ticks value.  */
        *remaining_ticks =  ticks_left;
    }

    /* Pickup the reschedule ticks value.  */
    if (reschedule_ticks != TX_NULL)
    {
    
        *reschedule_ticks =  internal_ptr -> tx_timer_internal_re_initialize_ticks;
    }
    
    /* Pickup the next created application timer.  */
    if (next_timer != TX_NULL)
    {
    
        *next_timer =  timer_ptr -> tx_timer_created_next;
    }
    
    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

