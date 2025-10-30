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
/**   Thread - High Level SMP Support                                     */
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
/*    _tx_thread_smp_core_exclude_get                    PROTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns the current exclusion list.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to the thread         */ 
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
/*  04-01-2016     William E. Lamie         Modified comment(s), and      */ 
/*                                            utilized new cores excluded */ 
/*                                            member in TX_THREAD,        */ 
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
UINT  _tx_thread_smp_core_exclude_get(TX_THREAD *thread_ptr, ULONG *exclusion_map_ptr)
{

UINT    status;


    /* First, make sure the thread pointer is valid.  */
    if (thread_ptr == TX_NULL)
    {
    
        /* Return pointer error.  */
        status =  TX_THREAD_ERROR;
    }

    /* Check for valid ID.  */
    else if (thread_ptr -> tx_thread_id != TX_THREAD_ID)
    {
    
        /* Return pointer error.  */
        status =  TX_THREAD_ERROR;
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
        *exclusion_map_ptr =  thread_ptr -> tx_thread_smp_cores_excluded;
    
        /* Return a successful status.  */
        status =  TX_SUCCESS;
    }
    
    /* Return status.  */
    return(status);
}

