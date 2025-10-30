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
/**   Timer - High Level SMP Support                                      */
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
/*    _tx_timer_smp_core_exclude_get                     PROTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns the current exclusion list for the timer.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timer_ptr                             Pointer to the timer          */ 
/*    exclusion_map_ptr                     Destination for the current   */ 
/*                                            exclusion list              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Status                                                              */ 
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
/*  05-15-2012     William E. Lamie         Initial Version 5.5           */ 
/*  07-15-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  12-12-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.1  */ 
/*  05-01-2014     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.2  */ 
/*  09-01-2015     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.7    */ 
/*  04-01-2016     William E. Lamie         Modified comment(s), changed  */ 
/*                                            the name of core control to */ 
/*                                            core excluded in internal   */ 
/*                                            timer setup, resulting      */ 
/*                                            in version 5.7.1            */ 
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
UINT  _tx_timer_smp_core_exclude_get(TX_TIMER *timer_ptr, ULONG *exclusion_map_ptr)
{

UINT    status;  


    /* First, make sure the timer pointer is valid.  */
    if (timer_ptr == TX_NULL)
    {
    
        /* Return pointer error.  */
        status =  TX_TIMER_ERROR;
    }

    /* Check for valid ID.  */
    else if (timer_ptr -> tx_timer_id != TX_TIMER_ID)
    {
    
        /* Return pointer error.  */
        status =  TX_TIMER_ERROR;
    }

    /* Is the destination pointer NULL?  */
    else if (exclusion_map_ptr == TX_NULL)
    {
    
        /* Return pointer error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Save the current exclusion map in the destination.  */
        *exclusion_map_ptr =  (timer_ptr -> tx_timer_internal.tx_timer_internal_smp_cores_excluded) & ((ULONG) TX_THREAD_SMP_CORE_MASK);
    
        /* Set the status to success.  */
        status =  TX_SUCCESS;
    }
    
    /* Return success.  */
    return(status);
}

