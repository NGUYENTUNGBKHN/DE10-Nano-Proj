// ------------------------------------------------------------
// VGIC Functions, for Cortex-A17
//
// Copyright (c) 2011-2018 Arm Limited (or its affiliates). All rights reserved.
// Use, modification and redistribution of this file is subject to your possession of a
// valid End User License Agreement for the Arm Product of which these examples are part of 
// and your compliance with all applicable terms and conditions of such licence agreement.
// ------------------------------------------------------------

#ifndef _VGIC_H
#define _VGIC_H

// PPI IDs:
#define   GIC_PPI_S_PHYSICAL_TIMER  (29)
#define   GIC_PPI_NS_PHYSICAL_TIMER (30)
#define   GIC_PPI_VIRTUAL_TIMER     (27)
#define   GIC_PPI_HYP_TIMER         (26)

#define   GIC_PPI_VIRT_MAINTENANCE  (25)

#define   GIC_PPI_LEGACY_IRQ        (31)
#define   GIC_PPI_LEGACY_FIQ        (28)


// ------------------------------------------------------------
// Distributor
//------------------------------------------------------------

// Global enable of the Interrupt Distributor
void enableVGIC(void);

// Global disable of the Interrupt Distributor
void disableVGIC(void);

// Enables the interrupt source number ID
void enableID(unsigned int ID);

// Disables the interrupt source number ID
void disableIntID(unsigned int ID);

// Sets the priority of the specified ID
// ID        - ID of interrupt to be set
// priority  - Priority interrupt is to be set to
void setIntPriority(unsigned int ID, unsigned int priority);

// Returns the priority of the specified ID
unsigned int getIntPriority(void);

#define GIC_TARGET_NONE      (0x0)
#define GIC_TARGET_CPU0      (0x1)
#define GIC_TARGET_CPU1      (0x2)
#define GIC_TARGET_CPU2      (0x4)
#define GIC_TARGET_CPU3      (0x8)

// Sets the target CPUs of the specified ID
// ID        - ID of interrupt to be set
// target    - Which CPUs this interrupt should be directed to
void setIntTarget(unsigned int ID, unsigned int target);

// Returns the target CPUs of the specified ID
// ID - ID of interrupt
unsigned int getIntTarget(unsigned int ID);

// Send a software generate interrupt
// ID - SGI ID to send
// target_list - Value to be written into Target List field
// filter_list - Value to be written into Filter List field
void sendSGI(unsigned int ID, unsigned int target_list, unsigned int filter_list);

// Returns the 1 if ID currently active, or 0 if not active
// ID - ID to check
unsigned int getIntActive(unsigned int ID);

// Make specified ID active
// ID - ID to make active
void setIntActive(unsigned int ID);

// Clears the active state of the specified ID
// ID - ID to clear active state of
void clearIntActive(unsigned int ID);

// Returns the 1 if ID currently pending, or 0 if not active
// ID - ID to check
unsigned int getIntActive(unsigned int ID);

// Moves the specified to the Pending state
// ID - ID to make pending
void setIntPending(unsigned int ID);

void clearIntPending(unsigned int ID);

// ------------------------------------------------------------
// Physical CPU Interface
// ------------------------------------------------------------

// Enables the physical CPU interface
// Must been done for each CPU separately
void enableVGICPhysicalCPUInterface(void);

// Disables the processor interface
// Must been done for each CPU separately
void disableVGICPhysicalCPUInterface(void);

//  Sets the EOImode bit in the GICC_CTLR
// When this bit is set the EOI and DIR registers must be written
// to complete an interrupt
void enableDIR(void);

// Clears the EOImode bit in the GICC_CTLR
// When this bit is clear on the EOI register must be written
// to complete an interrupt.  Writing the DIR is UNPREDICTABLE
void disableDIR(void);

// Sets the Priority mask register for the CPU run on
// The reset value masks ALL interrupts!
void setPriorityMask(unsigned int priority);

// Sets the Binary Point Register for the CPU run on (for the current world)
void setBinaryPoint(unsigned int priority);

// Returns the value of the Interrupt Acknowledge Register
unsigned int readIntAck(void);

//  Writes ID to the End Of Interrupt register (GICC_EOIR)
void writeEOI(unsigned int ID);

// Writes ID to the Deactivate Interrupt register (GICC_DIR)
void writeDIR(unsigned int ID);

// Returns the ID of the highest pending interrupt
unsigned int getHighestPendingInt(void);

//  Returns the priority of the currently being handled IRQ
unsigned int getRunningPriority(void);

// ------------------------------------------------------------
// Virtual CPU Interface (Hypervisor view)
// ------------------------------------------------------------


// ------------------------------------------------------------
// Virtual CPU Interface (Virtual Machine view)
// ------------------------------------------------------------

// No functions are given for these registers.  It is assumed
// that the Hypervisor will map the VCPUIF to appear where the
// guest VM expects to find the physical interface.  Therefore
// the guest VM would use the functions for the physical
// interface.

// ------------------------------------------------------------
// TrustZone
// ------------------------------------------------------------

// Enables the sending of secure interrupts as FIQs
void enableSecureFIQs(void);

// Disables the sending of secure interrupts as FIQs
void disableSecureFIQs(void);

// Sets the specified ID as being Secure
// ID - ID of interrupt to be made Secure
void makeIntSecure(unsigned int ID);

// Sets the specified ID as being non-secure
// ID - ID of interrupt to be made non-secure
void makeIntNonSecure(unsigned int ID);

// Returns 1 is specified interrupt is non-secure, or 0 is secure
// ID - the ID of the interrupt to be queried
unsigned int getIntSecurity(unsigned int ID);

// Returns the value of the Aliased Interrupt Acknowledge Register (GICC_AIAR)
// This register is only accessible in the Secure world, and
// shows what the Normal world would see.
unsigned int readAliasedIntAck(void);

// Writes ID to the Aliased End Of Interrupt register (GICC_AEOIR)
// This register is only accessible in the Secure world, and
// shows what the Normal world would see.
void writeAliasedEOI(unsigned int ID);

// Returns the ID of the highest pending interrupt
// This register is only accessible in the Secure world, and
// shows what the Normal world would see.
unsigned int getAliasedHighestPendingInt(unsigned int ID);

#endif

// ------------------------------------------------------------
// End of MP_VGIC.h
// ------------------------------------------------------------
