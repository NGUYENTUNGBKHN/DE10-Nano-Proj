                        Express Logic's ThreadX SMP 

1. Revision History

For installation and usage please see the readme_threadx.txt file included
in your distribution. For port-specific version information, please refer
to the bottom of readme_threadx.txt. For ThreadX generic code information,
please refer to readme_threadx_generic.txt. The ThreadX SMP generic code 
version information is as follows:

02/01/2019  ThreadX generic code version 5.9. This release includes the following 
            modifications:

            tx_api.h                                        Changed minor version constant, and added a 
                                                            macro to disable warning of parameter not 
                                                            used.
            tx_mutex_priority_change.c                      Removed update of the priority inheritance
                                                            priority level.
            tx_thread_priority_change.c                     Corrected optimization when no change in the 
                                                            effective priority is necessary.
            tx_thread_relinquish.c                          Added logic to evaluate mapped core instead of 
                                                            the current core in evaluating the next thread 
                                                            replacement, added logic to call rebalance when
                                                            the next thread is excluded from running on the 
                                                            mapped core, and added optimization when thread 
                                                            at the end of the priority list is the thread 
                                                            performing the relinquish.
            tx_thread_suspend.c                             Refined error checking for self suspension to 
                                                            ensure thread context.
            tx_thread_time_slice.c                          Added logic to call rebalance when the next 
                                                            thread is excluded from running on the mapped 
                                                            core, and added optimization when thread at the 
                                                            end of the priority list is the thread being 
                                                            time-sliced.

06/01/2017  ThreadX generic code version 5.8. This release includes the following 
            modifications:

            tx_api.h                                        Changed minor version constant, added 
                                                            suspension sequence to verify cleanup 
                                                            is still necessary, modified code for MISRA 
                                                            compliance, added alignment type for memory 
                                                            pools, corrected compiler warnings in macro 
                                                            definitions, added support for optional 
                                                            extensions.
            tx_thread.h                                     Modified code for MISRA compliance, added 
                                                            default macro definition for setting up timeout, 
                                                            added default macro definition for setting up
                                                            thread timeout pointer, and added macros for 
                                                            extending the thread create, delete, and reset 
                                                            processing.
            tx_timer.h                                      Modified code for MISRA compliance, added macro 
                                                            for extending timer delete, and removed 
                                                            unnecessary defines.
            tx_trace.h                                      Modified code for MISRA compliance.
            tx_byte_pool_search.c                           Modified code for MISRA compliance, added use of 
                                                            alignment type for searching byte pool.
            tx_event_flags_set.c                            Removed unnecessary code, and modified code for 
                                                            MISRA compliance.
            tx_initialize_kernel_enter.c                    Added processing extension, and modified code 
                                                            for MISRA compliance.
            tx_mutex_priority_change.c                      Removed unnecessary code, modified code for MISRA 
                                                            compliance, and added processing extension.
            tx_thread_create.c                              Modified code for MISRA compliance, added optional 
                                                            internal thread extension macro, added macro for 
                                                            setting up the thread timeout, and corrected problem 
                                                            restoring preemption-threshold during initialization.
            tx_thread_initialize.c                          Modified code for MISRA compliance.
            tx_thread_preemption_change.c                   Modified code for MISRA compliance.
            tx_thread_priority_change.c                     Added processing extension, and modified code for 
                                                            MISRA compliance.
            tx_thread_relinquish.c                          Modified code for MISRA compliance.
            tx_thread_resume.c                              Modified code for MISRA compliance.
            tx_thread_smp_core_exclude.c                    Modified code for MISRA compliance.
            tx_thread_smp_core_exclude_get.c                Modified code for MISRA compliance.
            tx_thread_smp_current_state_get.c               Modified code for MISRA compliance.
            tx_thread_smp_debug_entry_insert.c              Modified code for MISRA compliance.
            tx_thread_smp_high_level_initialize.c           Modified code for MISRA compliance.
            tx_thread_smp_rebalance_execute_list.c          Modified code for MISRA compliance.
            tx_thread_smp_utilities.c                       Added new file for SMP utilties when in-line is disabled.
            tx_thread_suspend.c                             Modified code for MISRA compliance, and added protection 
                                                            against self suspension with the preempt-disable flag set.
            tx_thread_system_preempt_check.c                Modified code for MISRA compliance.                                             
            tx_thread_system_resume.c                       Removed unnecessary code, corrected issue with resuming a 
                                                            thread that was previously in the middle of suspending with 
                                                            an additional suspension request, corrected issue resuming 
                                                            a thread with no available cores, and modified code for 
                                                            MISRA compliance.
            tx_thread_system_suspend.c                      Corrected issue with self-suspending thread not at the head 
                                                            of its priority list, modified code for MISRA compliance, 
                                                            and corrected problem setting up the trace information 4 
                                                            field.
            tx_thread_time_slice.c                          Modified code for MISRA compliance.
            tx_thread_time_slice_change.c                   Modified code for MISRA compliance.
            tx_thread_timeout.c                             Modified code for MISRA compliance, added suspension 
                                                            sequence to verify cleanup is still necessary, and added 
                                                            macro for setting up the thread pointer.
            tx_timer_create.c                               Modified code for MISRA compliance.
            tx_timer_initialize.c                           Modified code for MISRA compliance, added initialization for
                                                            timer expired pointer, and added check for TX_NO_TIMER  
                                                            option.
            tx_timer_smp_core_exclude.c                     Modified code for MISRA compliance.
            tx_timer_smp_core_exclude_get.c                 Modified code for MISRA compliance.
            tx_timer_thread_entry.c                         Modified code for MISRA compliance.
            

08/03/2016  ThreadX generic code version 5.7.2. This release includes the following 
            modifications (also includes ThreadX 5.7 SP1 non-SMP source):

            tx_api.h                                        Corrected compiler warnings in macro definitions.
            tx_timer.h                                      Added the global variable _tx_timer_expired_ptr 
                                                            which is needed by timer info get.
            tx_byte_pool_search.c                           Modified logic to ensure the integrity of 
                                                            the search pointer.
            tx_mutex_priority_change.c                      Added logic to reverse preemption-threshold while 
                                                            the thread's priority is changed.
            tx_thread_priority_change.c                     Added logic to reverse preemption-threshold while 
                                                            the thread's priority is changed.
            tx_thread_smp_rebalance_execute_list.c          Corrected the update of possible cores.
            tx_thread_system_resume.c                       Added performance optimization for the simple
                                                            preemption case.
            tx_thread_system_suspend.c                      Corrected the update of possible cores, and 
                                                            added performance optimization by determining
                                                            if nontrivial scheduling is possible before full 
                                                            examination.
            tx_timer_create.c                               Kept protection over timer activation.
            tx_timer_initialize.c                           Added initialization for timer expired pointer, and
                                                            added check for TX_NO_TIMER option.
            tx_timer_thread_entry.c                         Ensure timer is not accessed after timeout unless 
                                                            reactivation is necessary, set pointer to indicate 
                                                            timer being processed, and perform reactivation with
                                                            protection.
            tx*.c                                           Changed comments and copyright header.
            tx*.h                                           Changed comments and copyright header.


04/01/2016  ThreadX generic code version 5.7.1. This release includes the following 
            modifications:

            tx_api.h                                        Added core excluded/allowed members to the 
                                                            TX_THREAD structure, and changed member name 
                                                            control to excluded for internal timer struct.
            tx_event_flags_set.c                            Moved notify function pointer setup before any 
                                                            thread resumption.
            tx_mutex_priority_change.c                      Added logic to not place thread at head of list 
                                                            when lowering priority if the priority level has 
                                                            a thread with preemption-threshold in force, and 
                                                            optimized processing.
            tx_thread.h                                     Added optimized in-line helper functions, and 
                                                            added preemption-threshold list.
            tx_thread_create.c                              Added clearing of the thread preemption-threshold 
                                                            list, changed the name of core control to core 
                                                            excluded in internal timer setup, and added setup 
                                                            for the new TX_THREAD cores excluded and allowed 
                                                            members.
            tx_thread_initialize.c                          Added initialization of the preemption-threshold 
                                                            list.
            tx_thread_preemption_change.c                   Corrected problem disabling preemption-threshold.
            tx_thread_priority_change.c                     Added logic to not place thread at head of list 
                                                            when lowering priority if the priority level has 
                                                            a thread with preemption-threshold in force, and 
                                                            optimized processing.
            tx_thread_relinquish.c                          Corrected preemption-threshold issue, and utilized 
                                                            new cores allowed member in TX_THREAD.
            tx_thread_smp_core_exclude.c                    Added logic to ensure the core mapped information is 
                                                            consistent with the new core exclusion.
            tx_thread_smp_core_exclude_get.c                Utilized new cores excluded member in TX_THREAD.
            tx_thread_smp_rebalance_execute_list.c          Corrected problem with schedule thread not being 
                                                            updated in one optimization case, added setup of 
                                                            the preempted list for preemption-threshold, and 
                                                            simplified code via in-line functions.
            tx_thread_system_resume.c                       Optimized processing for simple case, and simplified 
                                                            code via in-line functions.
            tx_thread_system_suspend.c                      Optimized processing for simple case, corrected problem 
                                                            disabling preemption-threshold, and simplified code via 
                                                            in-line functions.
            tx_thread_time_slice.c                          Corrected preemption-threshold issue, and utilized new 
                                                            cores allowed member in TX_THREAD.
            tx_timer_create.c                               Changed the name of core control to core excluded in 
                                                            internal timer setup.
            tx_timer_info_get.c                             Added check for proper remaining time calculation.
            tx_timer_smp_core_exclude.c                     Changed the name of core control to core excluded in 
                                                            internal timer setup.
            tx_timer_smp_core_exclude_get.c                 Changed the name of core control to core excluded in 
                                                            internal timer setup.
            tx_timer_thread_entry.c                         Changed the name of core control to core excluded in 
                                                            internal timer setup.
            tx_trace.h                                      Modified code to sync with standard ThreadX tx_trace.h.
            tx*.c                                           Changed comments and copyright header.
            tx*.h                                           Changed comments and copyright header.

09/01/2015  ThreadX generic code version 5.7. This release includes the following 
            modifications:

            tx_api.h                                        Modified code for ThreadX 5.7 compatibility, added 
                                                            default for port-specific memory synchronization primitive,  
                                                            modified minor version define, added thread start 
                                                            macro for performing port and/or user specified
                                                            processing when a thread starts, and added constant
                                                            TX_TIMER_TICKS_PER_SECOND for use by other middleware
                                                            components as a common time reference.
            tx_thread.h                                     Corrected next priority find for priority levels
                                                            greater than 32.
            tx_byte_pool_search.c                           Added code to assert pool ownership after protection 
                                                            is obtained to ensure no changes to the pool or the 
                                                            pool search pointer are made without ownership.
            tx_initialize_kernel_enter.c                    Removed TX_INITIALIZE_INIT since it is no longer needed, 
                                                            and added memory synchronization macros.
            tx_thread_preemption_change.c                   Added optimization to avoid calling rebalance algorithm.
            tx_thread_smp_rebalance_execute_list.c          Corrected problem scheduling thread on excluded core, 
                                                            and added various optimizations.
            tx_thread_system_resume.c                       Added protection after the call to _tx_thread_system_return
                                                            when the build option TX_NOT_INTERRUPTABLE is used, and added 
                                                            optimization to avoid calling rebalance algorithm.
            tx_thread_system_suspend.c                      Added protection after the call to _tx_thread_system_return
                                                            when the build option TX_NOT_INTERRUPTABLE is used, 
                                                            corrected problem scheduling thread on excluded core, and 
                                                            added optimization to avoid unnecessary priority search.
            tx*.c                                           Changed comments and copyright header.
            tx*.h                                           Changed comments and copyright header.


05/01/2014  ThreadX generic code version 5.6.2. This release includes the following 
            modifications:

            tx_api.h                                        Added timeout sequence information to the 
                                                            thread control block.
            tx_thread.h                                     Made the release cores flag volatile, and 
                                                            removed unnecessary prototypes.
            tx_byte_pool_search.c                           Added SMP-specific version.
            tx_thread_create.c                              Added logic to initialize the thread's 
                                                            executing core.
            tx_thread_system_resume.c                       Added optimization in resuming a thread with
                                                            core(s) excluded.
            tx_thread_system_suspend.c                      Changed code to eliminate a compiler warning, 
                                                            added sequence counters for timeouts and 
                                                            suspension, and and corrected problem 
                                                            suspending the last thread with preemption-
                                                            threshold in force.
            tx_thread_smp_high_level_initialize.c           Updated protection structure member name.
            tx_thread_timeout.c                             Added SMP-specific version.
            tx_thread_time_slice.c                          Added quick check for no time-slice expiration, 
                                                            and removed protection logic since this function 
                                                            is called under protection.
            tx_timer_thread_entry.c                         Released protection over timeout call.
            tx*.c                                           Changed comments and copyright header.
            tx*.h                                           Changed comments and copyright header.


12/12/2012  Initial ThreadX SMP generic code version 5.6.1. 


Copyright(c) 1996-2019 Express Logic, Inc.


Express Logic, Inc.
11423 West Bernardo Court
San Diego, CA  92127

www.expresslogic.com


