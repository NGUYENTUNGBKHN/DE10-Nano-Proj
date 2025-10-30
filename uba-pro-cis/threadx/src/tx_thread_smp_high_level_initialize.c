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
/**   Initialization                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_timer.h"
#include "tx_thread.h"



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_smp_high_level_initialize               PORTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the ThreadX SMP data structures and       */ 
/*    CPU registers.                                                      */ 
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
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_initialize_kernel_enter           ThreadX entry                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-15-2012     William E. Lamie         Initial Version 5.5           */ 
/*  07-15-2012     William E. Lamie         Modified comment(s), and      */ 
/*                                            added SMP optimizations,    */ 
/*                                            resulting in version 5.6    */ 
/*  12-12-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.1  */ 
/*  05-01-2014     William E. Lamie         Modified comment(s), and      */ 
/*                                            updated protection structure*/ 
/*                                            member name, resulting in   */ 
/*                                            version 5.6.2               */ 
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
void  _tx_thread_smp_high_level_initialize(void)
{

    /* Clear the system error flag.  */
    _tx_thread_smp_system_error =  TX_FALSE;

    /* Ensure that the system state variable is set to indicate 
       initialization is in progress.  Note that this variable is 
       later used to represent interrupt nesting.  */
    _tx_thread_smp_current_state_set(TX_INITIALIZE_IN_PROGRESS);

    /* Clear the thread protection.  */
    TX_MEMSET(&_tx_thread_smp_protection, 0, sizeof(TX_THREAD_SMP_PROTECT));

    /* Set the field of the protection to all ones to indicate it is invalid.  */
    _tx_thread_smp_protection.tx_thread_smp_protect_core =  ((ULONG) 0xFFFFFFFFUL);

    /* Clear the thread schedule list.  */
    TX_MEMSET(&_tx_thread_smp_schedule_list[0], 0, sizeof(_tx_thread_smp_schedule_list)); 

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    /* Call low-level SMP initialize.  */
    _tx_thread_smp_low_level_initialize(((UINT) TX_THREAD_SMP_MAX_CORES));
#else

    /* Determine if the dynamic maximum number of cores is 0. If so, default it
       to the compile-time maximum.  */
    if (_tx_thread_smp_max_cores == 0)
    {
    
        /* Default to the compile-time maximum.  */
        _tx_thread_smp_max_cores =  TX_THREAD_SMP_MAX_CORES;
    }

    /* Call low-level SMP initialize.  */
    _tx_thread_smp_low_level_initialize(_tx_thread_smp_max_cores);
#endif
}
