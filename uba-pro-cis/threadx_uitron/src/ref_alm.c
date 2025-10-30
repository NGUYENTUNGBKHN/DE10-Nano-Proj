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
/*  ref_alm.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Reference alarm handler                                                     */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      ref_alm                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Reference alarm handler.                                                */
/*                                                                              */
/* INPUT                                                                        */
/*      almid               ID number of the alarm handler.                     */
/*      pk_ralm             Pointer to the packet returning the alarm handler   */
/*                          state.                                              */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_NOEXS             Non-existent object                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Task context                                                            */
/*      Non-task context                                                        */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s),           */
/*                                  The time until the start-up time has been   */
/*                                  changed to return 1 tick less,              */
/*                                  Fixed a problem that the status of stop is  */
/*                                  returned while the alarm handler is running.*/
/*                                                                              */
/********************************************************************************/
ER ref_alm(ID almid, T_RALM* pk_ralm)
{
    ER rtn = E_OK;
    _KERNEL_UITRON_ALM* pAlm;
    TX_TIMER_INTERNAL *internal_ptr;
    TX_TIMER_INTERNAL **list_head;
    UINT active_timer_list;
TX_INTERRUPT_SAVE_AREA

    CHECK_ID(1, _kernel_alm_maxid, almid)
    CHECK_PARAM(pk_ralm!=NULL)
    CHECK_TSK_CONTEXT()

    TX_DISABLE  /* Lockout interrupts. */
    pAlm = (_KERNEL_UITRON_ALM*)ID_LIST(_kernel_alm_id, almid);
    if(pAlm!=NULL)
    {
        /* Pickup address of internal timer structure.  */
        internal_ptr =  &(pAlm->timer.tx_timer_internal);

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */
        pk_ralm->almstat = TALM_STP;

        /* Default the ticks left to the remaining ticks.  */
        pk_ralm->lefttim = internal_ptr -> tx_timer_internal_remaining_ticks;

        /* Determine if the timer is still active.  */
        
        if (internal_ptr -> tx_timer_internal_list_head!=NULL)
        {
            /* Indicate this timer is active.  */
            pk_ralm->almstat = TALM_STA;

            /* Default the active timer list flag to false.  */
            active_timer_list = FALSE;

            /* Determine if the timer is still active.  */
            if ((internal_ptr->tx_timer_internal_list_head>=_tx_timer_list_start) &&
                (internal_ptr->tx_timer_internal_list_head<_tx_timer_list_end))
            {
                /* This timer is active and has not yet expired.  */
                active_timer_list = TRUE;
            }

            /* Determine if the timer is on the active timer list.  */
            if (active_timer_list==TRUE)
            {
                /* Calculate the amount of time that has elapsed since the timer
                   was activated.  */

                /* Setup the list head pointer.  */
                list_head = internal_ptr -> tx_timer_internal_list_head;

                /* Is this timer's entry after the current timer pointer?  */
                if (internal_ptr -> tx_timer_internal_list_head>=_tx_timer_current_ptr)
                {
                    /* Calculate ticks left to expiration - just the difference between this 
                       timer's entry and the current timer pointer.  */
                    pk_ralm->lefttim =  (((ULONG)(((TX_TIMER_INTERNAL **) (list_head)) - \
                                                  ((TX_TIMER_INTERNAL **) (_tx_timer_current_ptr)))) + ((ULONG) 1));
                }
                else
                {
                    /* Calculate the ticks left with a wrapped list condition.  */
                    pk_ralm->lefttim = ((ULONG)(((TX_TIMER_INTERNAL **) (list_head)) - ((TX_TIMER_INTERNAL **) (_tx_timer_list_start))));

                    pk_ralm->lefttim = pk_ralm->lefttim + ((((ULONG)(((TX_TIMER_INTERNAL **) (_tx_timer_list_end)) - \
                                                                     ((TX_TIMER_INTERNAL **) (_tx_timer_current_ptr))))) + ((ULONG) 1));
                }

                /* Adjust the remaining ticks accordingly.  */
                if(internal_ptr->tx_timer_internal_remaining_ticks>TX_TIMER_ENTRIES)
                {
                    /* Subtract off the last full pass throught the timer list and add the time left.  */
                    pk_ralm->lefttim += (internal_ptr->tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES);
                }
            }
            else
            {
                /* The timer is not on the actual timer list so it must either be being processed
                   or on a temporary list to be processed.   */

                /* Calculate the remaining ticks for a timer in the process of expiring.  */
                if (pk_ralm->lefttim>TX_TIMER_ENTRIES)
                {
                    
                    /* Calculate the number of ticks remaining.  */
                    pk_ralm->lefttim = (internal_ptr -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES);
                }
                else
                {
                
                    /* Timer dispatch routine is waiting to execute, no more remaining ticks for this expiration.  */
                    pk_ralm->lefttim = 0;
                }
            }

            pk_ralm->lefttim = TIC2TIM(pk_ralm->lefttim);
        }
    }
    else
    {
        rtn = E_NOEXS;  /* The object is not registered. */
    }
    TX_RESTORE  /* Restore interrupts. */

    return rtn;
}

