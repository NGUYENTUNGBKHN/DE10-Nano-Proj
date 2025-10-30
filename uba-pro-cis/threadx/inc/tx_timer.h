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


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    tx_timer.h                                         PORTABLE SMP     */ 
/*                                                           5.9          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the ThreadX timer management component, including */ 
/*    data types and external references.  It is assumed that tx_api.h    */
/*    and tx_port.h have already been included.                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-15-2012     William E. Lamie         Initial Version 5.5           */ 
/*  07-15-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  12-12-2012     William E. Lamie         Modified comment(s), and      */ 
/*                                            added count to detect when  */ 
/*                                            timer interrupt is active,  */ 
/*                                            resulting in version 5.6.1  */ 
/*  05-01-2014     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6.2  */ 
/*  09-01-2015     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.7    */ 
/*  04-01-2016     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.7.1  */ 
/*  08-03-2016     William E. Lamie         Modified comment(s), and      */ 
/*                                            added the global variable   */ 
/*                                            _tx_timer_expired_ptr which */ 
/*                                            is needed by timer info get,*/ 
/*                                            resulting in version 5.7.2  */ 
/*  06-01-2017     William E. Lamie         Modified comment(s), modified */ 
/*                                            code for MISRA compliance,  */ 
/*                                            added macro for extending   */ 
/*                                            timer delete, and removed   */ 
/*                                            unnecessary defines,        */ 
/*                                            resulting in version 5.8    */
/*  02-01-2019     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.9    */ 
/*                                                                        */ 
/**************************************************************************/ 

#ifndef TX_TIMER_H
#define TX_TIMER_H


/* Define timer management specific data definitions.  */

#define TX_TIMER_ID                             ((ULONG) 0x4154494D)
#define TX_TIMER_ENTRIES                        ((ULONG) 32)


/* Define internal timer management function prototypes.  */

VOID        _tx_timer_expiration_process(VOID);
VOID        _tx_timer_initialize(VOID);
VOID        _tx_timer_system_activate(TX_TIMER_INTERNAL *timer_ptr);
VOID        _tx_timer_system_deactivate(TX_TIMER_INTERNAL *timer_ptr);
VOID        _tx_timer_thread_entry(ULONG timer_thread_input);


/* Timer management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#define TIMER_DECLARE extern


/* Define the system clock value that is continually incremented by the 
   periodic timer interrupt processing.  */

TIMER_DECLARE volatile ULONG    _tx_timer_system_clock;


/* Define the current time slice value.  If non-zero, a time-slice is active.
   Otherwise, the time_slice is not active.  There is one of these entries 
   per core.  */

TIMER_DECLARE ULONG             _tx_timer_time_slice[TX_THREAD_SMP_MAX_CORES];


/* Define count to detect when timer interrupt is active.  */

TIMER_DECLARE ULONG             _tx_timer_interrupt_active;


/* Define the time-slice expiration flag.  This is used to indicate that a time-slice
   has happened.  */

TIMER_DECLARE UINT              _tx_timer_expired_time_slice;


/* Define the thread and application timer entry list.  This list provides a direct access
   method for insertion of times less than TX_TIMER_ENTRIES.  */

TIMER_DECLARE TX_TIMER_INTERNAL *_tx_timer_list[TX_TIMER_ENTRIES];


/* Define the boundary pointers to the list.  These are setup to easily manage
   wrapping the list.  */

TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_list_start;
TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_list_end;


/* Define the current timer pointer in the list.  This pointer is moved sequentially
   through the timer list by the timer interrupt handler.  */

TIMER_DECLARE TX_TIMER_INTERNAL **_tx_timer_current_ptr;


/* Define the timer expiration flag.  This is used to indicate that a timer 
   has expired.  */

TIMER_DECLARE UINT              _tx_timer_expired;


/* Define the created timer list head pointer.  */

TIMER_DECLARE TX_TIMER          *_tx_timer_created_ptr;


/* Define the created timer count.  */

TIMER_DECLARE ULONG             _tx_timer_created_count;


/* Define the pointer to the timer that has expired and is being processed.  */

TIMER_DECLARE TX_TIMER_INTERNAL *_tx_timer_expired_timer_ptr;


#ifndef TX_TIMER_PROCESS_IN_ISR

/* Define the timer thread's control block.  */

TIMER_DECLARE TX_THREAD         _tx_timer_thread;


/* Define the variable that holds the timer thread's starting stack address.  */

TIMER_DECLARE VOID              *_tx_timer_stack_start;


/* Define the variable that holds the timer thread's stack size.  */

TIMER_DECLARE ULONG             _tx_timer_stack_size;


/* Define the variable that holds the timer thread's priority.  */

TIMER_DECLARE UINT              _tx_timer_priority;

/* Define the system timer thread's stack.   The default size is defined
   in tx_port.h.  */

TIMER_DECLARE ULONG             _tx_timer_thread_stack_area[(((UINT) TX_TIMER_THREAD_STACK_SIZE)+((sizeof(ULONG)) - ((UINT) 1)))/sizeof(ULONG)];

#else


/* Define the busy flag that will prevent nested timer ISR processing.  */

TIMER_DECLARE UINT              _tx_timer_processing_active;

#endif

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

/* Define the total number of timer activations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_activate_count;


/* Define the total number of timer reactivations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_reactivate_count;


/* Define the total number of timer deactivations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_deactivate_count;


/* Define the total number of timer expirations.  */

TIMER_DECLARE  ULONG            _tx_timer_performance_expiration_count;


/* Define the total number of timer expiration adjustments. These are required
   if the expiration time is greater than the size of the timer list. In such 
   cases, the timer is placed at the end of the list and then reactivated 
   as many times as necessary to finally achieve the resulting timeout. */

TIMER_DECLARE  ULONG            _tx_timer_performance__expiration_adjust_count;

#endif

/* Define default post timer delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#ifndef TX_TIMER_DELETE_PORT_COMPLETION
#define TX_TIMER_DELETE_PORT_COMPLETION(t)
#endif


#endif

