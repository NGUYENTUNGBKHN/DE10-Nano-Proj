/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_main.h
* Contents: main validation processing
*
*
*******************************************************************************/
#ifndef TEM_MAIN_H
#define TEM_MAIN_H

void set_bill_information(P_BILL_INFORMATION const bill_info_in);

void run_init_edge_routine(void);
void reset_watchdog_timer(void);

#endif
