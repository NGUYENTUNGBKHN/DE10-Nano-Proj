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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_initialize_kernel_setup                        PORTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is called by the compiler's startup code to make      */ 
/*    ThreadX objects accessible to the compiler's library.  If this      */ 
/*    function is not called by the compiler, all ThreadX initialization  */ 
/*    takes place from the kernel enter function defined previously.      */ 
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
/*    _tx_thread_smp_high_level_initialize  SMP initialization            */ 
/*    _tx_thread_smp_current_state_set  Set system state for all cores    */ 
/*    _tx_initialize_low_level          Low-level initialization          */ 
/*    _tx_initialize_high_level         High-level initialization         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    startup code                      Compiler startup code             */ 
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
/*  06-01-2017     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.8    */ 
/*  02-01-2019     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.9    */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  _tx_initialize_kernel_setup(VOID)
{

    /* Ensure that the system state variable is set to indicate 
       initialization is in progress.  Note that this variable is 
       later used to represent interrupt nesting.  */
    _tx_thread_smp_current_state_set(TX_INITIALIZE_IN_PROGRESS);

    /* Call any port specific preprocessing.  */
    TX_PORT_SPECIFIC_PRE_INITIALIZATION

    /* Invoke the low-level initialization to handle all processor specific
       initialization issues.  */
    _tx_initialize_low_level();
    
    /* Call the high-level SMP  Initialization.  */
    _tx_thread_smp_high_level_initialize();

    /* Invoke the high-level initialization to exercise all of the 
       ThreadX components and the application's initialization 
       function.  */
    _tx_initialize_high_level();

    /* Call any port specific post-processing.  */
    TX_PORT_SPECIFIC_POST_INITIALIZATION

    /* Set the system state to indicate initialization is almost done.  */
    _tx_thread_system_state[0] =  TX_INITIALIZE_ALMOST_DONE;
}

