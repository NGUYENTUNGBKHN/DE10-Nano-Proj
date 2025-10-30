// ------------------------------------------------------------
// Cortex-A MPCore - Interrupt Controller functions
// Header File
//
// Copyright ARM Ltd 2009. All rights reserved.
// ------------------------------------------------------------

#ifndef _CORTEXA_GIC_
#define _CORTEXA_GIC_

// ------------------------------------------------------------
// GIC
// ------------------------------------------------------------

// Typical calls to enable interrupt ID X:
// enable_irq_id(X)                 <-- Enable that ID
// set_irq_priority(X, 0)           <-- Set the priority of X to 0 (the max priority)
// set_priority_mask(0x1F)          <-- Set Core's priority mask to 0x1F (the lowest priority)
// enable_GIC()                     <-- Enable the GIC (global)
// enable_gic_processor_interface() <-- Enable the CPU interface (local to the core)
//
// OR
//
// Use init_GIC() which is a simple switch everything on function! :-)
//

//  Global enable of the Interrupt Distributor
void enable_GIC(void);

// Global disable of the Interrupt Distributor
void disable_GIC(void);

// Enables the interrupt source number ID
void enable_irq_id(unsigned int ID);

// Disables the interrupt source number ID
void disable_irq_id(unsigned int ID);

// Sets the priority of the specifed ID
void set_irq_priority(unsigned int ID, unsigned int priority);

// Enables the processor interface
// Must been done one each core seperately
void enable_gic_processor_interface(void);  

// Disables the processor interface
void disable_gic_processor_interface(void);

// Sets the Priority mask register for the core run on
// The reset value masks ALL interrupts!
void set_priority_mask(unsigned int priority);

// Sets the Binary Point Register for the core run on
void set_binary_port(unsigned int priority);

//  Returns the value of the Interrupt Acknowledge Register
unsigned int read_irq_ack(void);

// Writes ID to the End Of Interrupt register
void write_end_of_irq(unsigned int ID);

// Lazy Init function, a quick way of enabling interrupts
// * Enables the GIC (global) and CPU Interface (just for this core)
// * Enables interrupt sources 0->31, and sets their priority to 0x0
// * Sets the CPU's Priority mask to 0x1F
// * Clears the CPSR I bit
void init_GIC(void);

// ------------------------------------------------------------
// SGI
// ------------------------------------------------------------

// Send a software generate interrupt
void send_sgi(unsigned int ID, unsigned int core_list, unsigned int filter_list);

// �����݃��x���ݒ�, 18/11/06
#define IPL_KERNEL_HIGHEST (0x20)		// �J�[�l�������ݗD��x�ō����x��
#define IPL_KERNEL_HIGHER (0x30)
#define IPL_KERNEL_HIGH (0x40)
#define IPL_KERNEL_NORMAL	(0x50)		// �J�[�l�������ݗD��x�W�����x��
#define IPL_KERNEL_LOW (0x60)
#define IPL_KERNEL_LOWER (0x70)
#define IPL_KERNEL_LOWEST (0x80)
#define IPL_USER_HIGHEST (0x90)		// ���[�U�����ݗD��x�ō����x��
#define IPL_USER_HIGHER (0xa0)
#define IPL_USER_HIGH (0xb0)
#define IPL_USER_NORMAL	(0xc0)		// ���[�U�����ݗD��x�W�����x��
#define IPL_USER_LOW (0xd0)
#define IPL_USER_LOWER (0xe0)
#define IPL_USER_LOWEST (0xf0)
#endif

// ------------------------------------------------------------
// End of MP_GIC.h
// ------------------------------------------------------------
