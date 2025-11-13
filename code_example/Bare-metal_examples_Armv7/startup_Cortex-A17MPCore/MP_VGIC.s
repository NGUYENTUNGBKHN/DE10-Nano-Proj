; ------------------------------------------------------------
; VGIC functions, for Cortex-A17
; Cortex-A17 has no private peripherals accessed via CBAR, so use write/read FILASTARTR instead to access GIC
; Copyright (c) 2011-2018 Arm Limited (or its affiliates). All rights reserved.
; Use, modification and redistribution of this file is subject to your possession of a
; valid End User License Agreement for the Arm Product of which these examples are part of 
; and your compliance with all applicable terms and conditions of such licence agreement.
; ------------------------------------------------------------

  

  AREA  MP_VGIC, CODE, READONLY

; ------------------------------------------------------------
; Distributor
; ------------------------------------------------------------

  ; CPU Interface offset from base of peripheral space --> 0x2000
  ; Interrupt Distributor offset from base of peripheral space --> 0x1000


  EXPORT  enableVGIC
  ; void enableVGIC(void)
  ; Global enable of the Interrupt Distributor
enableVGIC PROC

  ; Get base address of peripheral space
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x1000         ; Add the VGIC offset

  LDR     r1, [r0]                ; Read the VGIC's Enable Register (GICD_CTLR)
  ORR     r1, r1, #0x01           ; Set bit 0, the enable bit
  STR     r1, [r0]                ; Write the VGIC's Enable Register (GICD_CTLR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT disableVGIC
  ; void disableVGIC(void)
  ; Global disable of the Interrupt Distributor
disableVGIC PROC

  ; Get base address of peripheral space
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x1000         ; Add the VGIC offset

  LDR     r1, [r0]                ; Read the VGIC's Enable Register (GICD_CTLR)
  BIC     r1, r1, #0x01           ; Clear bit 0, the enable bit
  STR     r1, [r0]                ; Write the VGIC's Enable Register (GICD_CTLR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT  enableIntID
  ; void enableID(unsigned int ID)
  ; Enables the interrupt source number ID
enableIntID PROC

  ; Get base address of peripheral space
  MOV     r1, r0                  ; Back up passed in ID value
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has an enable bit in the VGIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r1                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r1, r1, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move enable value into r3
  MOV     r3, r3, LSL r1          ; Shift it left to position of ID

  ADD     r2, r2, #0x1100         ; Add the base offset of the Enable Set (GICD_ISERn) registers to the offset for the ID
  STR     r3, [r0, r2]            ; Store out

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT  disableIntID
  ; void disableIntID(unsigned int ID)
  ; Disables the interrupt source number ID
disableIntID PROC

  ; Get base address of peripheral space
  MOV     r1, r0                  ; Back up passed in ID value
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address

  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r1                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r1, r1, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move enable value into r3
  MOV     r3, r3, LSL r1          ; Shift it left to position of ID in 32-bit block

  ADD     r2, r2, #0x1180         ; Add the base offset of the Enable Clear (GICD_ICERn) registers to the offset for the ID
  STR     r3, [r0, r2]            ; Store out

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT setIntPriority
  ; void setIntPriority(unsigned int ID, unsigned int priority)
  ; Sets the priority of the specified ID
  ; r0 = ID
  ; r1 = priority
setIntPriority PROC

  ; Get base address of peripheral space
  MOV     r2, r0                  ; Back up passed in ID value
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address

  ; r0 = base addr
  ; r1 = priority
  ; r2 = ID
  
  ; Make sure that priority value is only 8 bits
  AND     r1, r1, #0xFF

  ; Find which register this ID lives in
  BIC     r3, r2, #0x03           ; Make a copy of the ID, clearing off the bottom two bits
                                  ; There are four IDs per reg, by clearing the bottom two bits we get an address offset
  ADD     r3, r3, #0x1400         ; Now add the offset of the Priority Level registers from the base of the peripheral space
  ADD     r0, r0, r3              ; Now add in the base address of the peripheral space, giving us the absolute address


  ; Now work out which ID in the register it is
  AND     r2, r2, #0x03           ; Clear all but the bottom two bits, leaves which ID in the reg it is (which byte)
  MOV     r2, r2, LSL #3          ; Multiply by 8, this gives a bit offset

  ; Read -> Modify -> Write
  MOV     r12, #0xFF              ; 8 bit field mask
  MOV     r12, r12, LSL r2        ; Move mask into correct bit position
  MOV     r1, r1, LSL r2          ; Also, move passed in priority value into correct bit position


  LDR     r3, [r0]                ; Read current value of the Priority Level (GICD_IPRn) register
  BIC     r3, r3, r12             ; Clear appropriate field
  ORR     r3, r3, r1              ; Now OR in the priority value
  STR     r3, [r0]                ; And store it back again

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT getIntPriority
  ; unsigned int getIntPriority(void)
  ; Returns the priority of the specified ID
getIntPriority PROC
  ; TBD
  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT setIntTarget
  ; void setIntTarget(unsigned int ID, unsigned int target)
  ; Sets the target CPUs of the specified ID
setIntTarget PROC

  ; Get base address of peripheral space
  MRC     p15, 4, r2, c15, c0, 1  ; Read peripheral base address

  ; r0 = ID
  ; r1 = target
  ; r2 = base addr
  
  ; Clear unused bits (each target field is only 8 bits long)
  AND     r1, r1, #0xFF

  ; Find which register this ID lives in
  BIC     r3, r0, #0x03           ; Make a copy of the ID, clearing the bottom 2 bits
                                  ; There are four IDs per reg, by clearing the bottom two bits we get an address offset
  ADD     r3, r3, #0x1800         ; Now add the offset of the Target registers from the base of the peripheral space
  ADD     r2, r2, r3              ; Now add in the base address of the peripheral space, giving us the absolute address

  ; Now work out which ID in the register it is
  AND     r0, r0, #0x03           ; Clear all but the bottom two bits, leaves which ID in the reg it is (which byte)
  MOV     r0, r0, LSL #3          ; Multiply by 8, this gives a bit offset

  ; Read -> Modify -> Write
  MOV     r12, #0xFF              ; 8 bit field mask
  MOV     r12, r12, LSL r0        ; Move mask into correct bit position
  MOV     r1, r1, LSL r0          ; Also, move passed in target value into correct bit position

  LDR     r3, [r2]                ; Read current value of the Target (GICD_ITPRn) register
  BIC     r3, r3, r12             ; Clear appropriate field
  ORR     r3, r3, r1              ; Now OR in the target value
  STR     r3, [r2]                ; And store it back again

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT getIntTarget
  ; unsigned int getIntTarget(unsigned int ID)
  ; Returns the target CPUs of the specified ID
  ; r0 - ID - ID of interrupt
getIntTarget PROC
  ; Get base address of peripheral space
  MRC     p15, 4, r2, c15, c0, 1  ; Read peripheral base address

  ; r0 = ID
  ; r2 = base addr

  ; Find which register this ID lives in
  BIC     r3, r0, #0x03           ; Make a copy of the ID, clearing the bottom 2 bits
                                  ; There are four IDs per reg, by clearing the bottom two bits we get an address offset
  ADD     r3, r3, #0x1800         ; Now add the offset of the Target registers from the base of the peripheral space
  ADD     r2, r2, r3              ; Now add in the base address of the peripheral space, giving us the absolute address

  ; Now work out which ID in the register it is
  AND     r0, r0, #0x03           ; Clear all but the bottom two bits, leaves which ID in the reg it is (which byte)
  MOV     r0, r0, LSL #3          ; Multiply by 8, this gives a bit offset

  ; Extract current value
  LDR     r3, [r2]                ; Read current value of the Target (GICD_ITPRn) register

  MOV     r12, #0xFF              ; 8 bit field mask
  MOV     r12, r12, LSL r0        ; Move mask into correct bit position
  AND     r3, r12, r12            ; Clear all the bits, except the field we are interested in
  MOV     r0, r2, LSR r0          ; Move field to bottom of register

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT sendSGI
  ; void sendSGI(unsigned int ID, unsigned int target_list, unsigned int filter_list);
  ; Send a software generate interrupt
  ; r0 - ID - SGI ID to send
  ; r1 - target_list - Value to be written into Target List field
  ; r2 - filter_list - Value to be written into Filter List field
sendSGI PROC

  AND     r3, r0, #0x0F           ; Mask off unused bits of ID, and move to r3
  AND     r1, r1, #0x0F           ; Mask off unused bits of target_filter
  AND     r2, r2, #0x0F           ; Mask off unused bits of filter_list

  ORR     r3, r3, r1, LSL #16     ; Combine ID and target_filter
  ORR     r3, r3, r2, LSL #24     ; and now the filter list

  ; Get the address of the GIC
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x1F00         ; Add offset of the sgi_trigger reg

  STR     r3, [r0]                ; Write to the Software Generated Interrupt Register  (GICD_SGIR)

  BX      lr

  ENDP

; ------------------------------------------------------------

  EXPORT getIntActive
  ; unsigned int getIntActive(unsigned int ID);
  ; Returns the 1 if ID currently active, or 0 if not active
  ; r0 - ID - ID to check
getIntActive PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has an active bit in the VGIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)
  ADD     r2, r2, #0x1300         ; Add the base offset of the Interrupt Set-Active (GICD_ISACTIVERn) registers to the offset for the ID


  ; Now work out which bit within the 32-bit block the ID is
  AND     r3, r0, #0x1F           ; Mask off to give offset within 32-bit block

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Set-Active (GICD_ISACTIVERn) register
  MOV     r0, r0, LSR r3          ; Shift the bit we want into position 0
  AND     r0, r0, #1              ; Clear all other bits

  BX      lr

  ENDP

; ------------------------------------------------------------

  EXPORT setIntActive
  ; void setIntActive(unsigned int ID);
  ; Make specified ID active
  ; r0 - ID - ID to make active
setIntActive PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has an active bit in the VGIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r0, r0, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move mask into r3
  MOV     r3, r3, LSL r0          ; Shift it left to position of ID

  ADD     r2, r2, #0x1300         ; Add the base offset of the Interrupt Set-Active (GICD_ISACTIVERn) registers to the offset for the ID

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Set-Active (GICD_ISACTIVERn) register
  ORR     r0, r0, r3              ; Set bit (don't have to worry about preserving values)
  STR     r0, [r1, r2]            ; Write back

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT clearIntActive
  ; void clearIntActive(unsigned int ID);
  ; Moves ID
  ; r0 - ID - ID to
clearIntActive PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has an active bit in the VGIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r0, r0, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move mask into r3
  MOV     r3, r3, LSL r0          ; Shift it left to position of ID

  ADD     r2, r2, #0x1380         ; Add the base offset of the Interrupt Clear-Active (GICD_ICACTIVERn) registers to the offset for the ID

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Clear-Active (GICD_ICACTIVERn) register
  ORR     r0, r0, r3              ; Set bit (don't have to worry about preserving values)
  STR     r0, [r1, r2]            ; Write back

  BX      lr

  ENDP

; ------------------------------------------------------------

  EXPORT getIntPending
  ; unsigned int getIntActive(unsigned int ID);
  ; Returns the 1 if ID currently pending, or 0 if not active
  ; r0 - ID - ID to check
getIntPending PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has a pending bit in the VGIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)
  ADD     r2, r2, #0x1200         ; Add the base offset of the Interrupt Set-Pending (GICD_ISPRn) registers to the offset for the ID


  ; Now work out which bit within the 32-bit block the ID is
  AND     r3, r0, #0x1F           ; Mask off to give offset within 32-bit block

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Set-Pending (GICD_ISPRn)) register
  MOV     r0, r0, LSR r3          ; Shift the bit we want into position 0
  AND     r0, r0, #1              ; Clear all other bits

  BX      lr

  ENDP


; ------------------------------------------------------------

  EXPORT setIntPending
  ; void setIntPending(unsigned int ID);
  ; Make specified ID active
  ; r0 - ID - ID to make active
setIntPending PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has a pending bit in the VGIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r0, r0, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move mask into r3
  MOV     r3, r3, LSL r0          ; Shift it left to position of ID

  ADD     r2, r2, #0x1200         ; Add the base offset of the Interrupt Set-Pending (GICD_ISPRn) registers to the offset for the ID

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Set-Pending (GICD_ISPRn) register
  ORR     r0, r0, r3              ; Set bit (don't have to worry about preserving values)
  STR     r0, [r1, r2]            ; Write back

  BX      lr

  ENDP

; ------------------------------------------------------------

  EXPORT clearIntPending
  ; void clearIntPending(unsigned int ID);
  ; Moves ID
  ; r0 - ID - ID to
clearIntPending PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has a pending bit in the VGIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r0, r0, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move mask into r3
  MOV     r3, r3, LSL r0          ; Shift it left to position of ID

  ADD     r2, r2, #0x1280         ; Add the base offset of the Interrupt Clear-Pending (GICD_ICPRn) registers to the offset for the ID

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Clear-Pending (GICD_ICPRn) register
  ORR     r0, r0, r3              ; Set bit (don't have to worry about preserving values)
  STR     r0, [r1, r2]            ; Write back

  BX      lr

  ENDP

; ------------------------------------------------------------
; Physical CPU Interface
; ------------------------------------------------------------

  EXPORT enableVGICPhysicalCPUInterface
  ; void enableVGICPhysicalCPUInterface(void)
  ; Enables the physical CPU interface
  ; Must been done for each CPU separately
enableVGICPhysicalCPUInterface PROC

  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r1, [r0]                ; Read the CPU Interface Control register (GICC_CTLR)
  ORR     r1, r1, #0x03           ; Bit 0: Enables secure interrupts, Bit 1: Enables Non-Secure interrupts
  BIC     r1, r1, #0x08           ; Bit 3: Ensure Group 0 interrupts are signalled using IRQ, not FIQ
  STR     r1, [r0]                ; Write the CPU Interface Control register (GICC_CTLR)

  BX      lr
  ENDP


; ------------------------------------------------------------

  EXPORT disableVGICPhysicalCPUInterface
  ; void disableVGICPhysicalCPUInterface(void)
  ; Disables the processor interface
  ; Must been done for each CPU separately
disableVGICPhysicalCPUInterface PROC

  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r1, [r0]                ; Read the CPU Interface Control register (GICC_CTLR)
  BIC     r1, r1, #0x03           ; Bit 0: Enables secure interrupts, Bit 1: Enables Non-Secure interrupts
  STR     r1, [r0]                ; Write the CPU Interface Control register (GICC_CTLR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT enableDIR
  ; void enableDIR(void)
  ; Sets the EOImode bit in the GICC_CTLR
  ; When this bit is set the EOI and DIR registers must be written
  ; to complete an interrupt
enableDIR PROC
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r1, [r0]                ; Read the CPU Interface Control register (GICC_CTLR)
  ORR     r1, r1, #(1 << 9)       ; Bit 9: EOIMode bit
  STR     r1, [r0]                ; Write the CPU Interface Control register (GICC_CTLR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT disableDIR
  ; void disableDIR(void)
  ; Clears the EOImode bit in the GICC_CTLR
  ; When this bit is clear on the EOI register must be written
  ; to complete an interrupt.  Writing the DIR is UNPREDICTABLE
disableDIR PROC
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r1, [r0]                ; Read the CPU Interface Control register (GICC_CTLR)
  BIC     r1, r1, #(1 << 9)       ; Bit 9: EOIMode bit
  STR     r1, [r0]                ; Write the CPU Interface Control register (GICC_CTLR)

  BX      lr
  ENDP


; ------------------------------------------------------------

  EXPORT setPriorityMask
  ; void setPriorityMask(unsigned int priority)
  ; Sets the Priority mask register for the CPU run on
  ; The reset value masks ALL interrupts!
setPriorityMask PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  STR     r0, [r1, #0x0004]       ; Write the Priority Mask register (GICC_PMR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT setBinaryPoint
  ; void setBinaryPoint(unsigned int priority)
  ; Sets the Binary Point Register for the CPU run on (for the current world)
setBinaryPoint PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  STR     r0, [r1, #0x0008]       ; Write the Priority Mask register (GICC_BPR_S/N)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT readIntAck
  ; unsigned int readIntAck(void)
  ; Returns the value of the Interrupt Acknowledge Register
readIntAck PROC
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset
  LDR     r0, [r0, #0x000C]       ; Read the Interrupt Acknowledge Register (GICC_IAR)
  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT writeEOI
  ; void writeEOI(unsigned int ID)
  ; Writes ID to the End Of Interrupt register (GICC_EOIR)
writeEOI PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  STR     r0, [r1, #0x0010]       ; Write ID to the End of Interrupt register (GICC_EOIR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT  writeDIR
  ; void writeDIR(unsigned int ID)
  ; Writes ID to the Deactivate Interrupt register (GICC_DIR)
writeDIR PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x3000         ; Add Interrupt Controller Physical CPU interface offset
                                  ; + offset for GICC EOIR

  STR     r0, [r1]                ; Write ID to the End of Interrupt register (GICC_EOIR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT  getHighestPendingInt
  ; unsigned int getHighestPendingInt(void)
  ; Returns the ID of the highest pending interrupt
getHighestPendingInt PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r0, [r1, #0x0018]       ; Read Highest Pending Interrupt Register (GICC_HPIR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT  getRunningPriority
  ; unsigned int getRunningPriority(void)
  ; Returns the priority of the currently being handled IRQ
getRunningPriority PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r0, [r1, #0x0014]       ; Read Running Priority Register (GICC_RPR)

  BX      lr
  ENDP
  


; ------------------------------------------------------------
; Virtual CPU Interface (Hypervisor view)
; ------------------------------------------------------------


; ------------------------------------------------------------
; Virtual CPU Interface (Virtual Machine view)
; ------------------------------------------------------------

; No functions are given for these registers.  It is assumed
; that the Hypervisor will map the VCPUIF to appear where the
; guest VM expects to find the physical interface.  Therefore
; the guest VM would use the functions for the physical 
; interface.
  
; ------------------------------------------------------------
; TrustZone
; ------------------------------------------------------------

  EXPORT enableSecureFIQs
  ; void enableSecureFIQs(void);
  ; Enables the sending of secure interrupts as FIQs
enableSecureFIQs PROC

  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r1, [r0]                ; Read the CPU Interface Control register (GICC_CTLR)
  ORR     r1, r1, #0x08           ; Bit 3: Controls whether secure interrupts are signalled as IRQs or FIQs
  STR     r1, [r0]                ; Write the CPU Interface Control register (GICC_CTLR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT disableSecureFIQs
  ; void disableSecureFIQs(void);
  ; Disables the sending of secure interrupts as FIQs
disableSecureFIQs PROC

  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r1, [r0]                ; Read the CPU Interface Control register (GICC_CTLR)
  BIC     r1, r1, #0x08           ; Bit 3: Controls whether secure interrupts are signalled as IRQs or FIQs
  STR     r1, [r0]                ; Write the CPU Interface Control register (GICC_CTLR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT makeIntSecure
  ; void makeIntSecure(unsigned int ID);
  ; Sets the specified ID as being Secure
  ; r0 - ID - ID of interrupt to be made Secure
makeIntSecure PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has a secutiy bit in the GIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r0, r0, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move enable value into r3
  MOV     r3, r3, LSL r0          ; Shift it left to position of ID

  ADD     r2, r2, #0x1080         ; Add the base offset of the Interrupt Security registers (GICD_ISRn) to the offset for the ID

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Configuration
  BIC     r0, r0, r3              ; Clear bit (0 = secure)
  STR     r0, [r1, r2]            ; Store out

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT makeIntNonSecure
  ; void makeIntNonSecure(unsigned int ID);
  ; Sets the specified ID as being non-secure
  ; r0 - ID - ID of interrupt to be made non-secure
makeIntNonSecure PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has a secutiy bit in the GIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)

  ; Now work out which bit within the 32-bit block the ID is
  AND     r0, r0, #0x1F           ; Mask off to give offset within 32-bit block
  MOV     r3, #1                  ; Move enable value into r3
  MOV     r3, r3, LSL r0          ; Shift it left to position of ID

  ADD     r2, r2, #0x1080         ; Add the base offset of the Interrupt Security registers (GICD_ISRn) to the offset for the ID

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Configuration
  ORR     r0, r0, r3              ; Set bit (1 = secure)
  STR     r0, [r1, r2]            ; Store out

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT getIntSecurity
  ; unsigned int getIntSecurity(unsigned int ID);
  ; Returns 1 is specified interruppt is non-secure, or 0 is secure
  ; r0 - ID - the ID of the interrupt to be queried
getIntSecurity PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address

  ; Each interrupt source has a secutiy bit in the GIC.  These
  ; are grouped into registers, with 32 sources per register
  ; First, we need to identify which 32-bit block the interrupt lives in
  MOV     r2, r0                  ; Make working copy of ID in r2
  MOV     r2, r2, LSR #5          ; LSR by 5 places, affective divide by 32
                                  ; r2 now contains the 32-bit block this ID lives in
  MOV     r2, r2, LSL #2          ; Now multiply by 4, to convert offset into an address offset (four bytes per reg)
  ADD     r2, r2, #0x1080         ; Add the base offset of the Interrupt Security registers (GICD_ISRn) to the offset for the ID

  ; Now work out which bit within the 32-bit block the ID is
  AND     r3, r0, #0x1F           ; Mask off to give offset within 32-bit block

  LDR     r0, [r1, r2]            ; Read appropriate Interrupt Configuration
  MOV     r0, r0, LSR r3          ; Move the bit we want into position 0
  AND     r0, r0, #1              ; Clear all bits but for 0

  BX      lr
  ENDP
  
; ------------------------------------------------------------

  EXPORT  readAliasedIntAck
  ; unsigned int readAliasedIntAck(void)
  ; Returns the value of the Aliased Interrupt Acknowledge Register (GICC_AIAR)
  ; This register is only accessible in the Secure world, and
  ; shows what the Normal world would see.
readAliasedIntAck PROC
  MRC     p15, 4, r0, c15, c0, 1  ; Read peripheral base address
  ADD     r0, r0, #0x2000         ; Add Interrupt Controller Physical CPU interface offset
  LDR     r0, [r0, #0x0020]       ; Read the Aliased Interrupt Acknowledge Register (GICC_AIAR)

  BX      lr
  ENDP

; ------------------------------------------------------------

  EXPORT  writeAliasedEOI
  ; void writeAliasedEOI(unsigned int ID)
  ; Writes ID to the Aliased End Of Interrupt register (GICC_AEOIR)
  ; This register is only accessible in the Secure world, and
  ; shows what the Normal world would see.
writeAliasedEOI PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  STR     r0, [r1, #0x0024]       ; Write ID to the End of Interrupt register  (GICC_AEOIR)

  BX      lr
  ENDP
  
; ------------------------------------------------------------

  EXPORT  getAliasedHighestPendingInt
  ; unsigned int getAliasedHighestPendingInt(unsigned int ID)
  ; Returns the ID of the highest pending interrupt
  ; This register is only accessible in the Secure world, and
  ; shows what the Normal world would see.
getAliasedHighestPendingInt PROC
  MRC     p15, 4, r1, c15, c0, 1  ; Read peripheral base address
  ADD     r1, r1, #0x2000         ; Add Interrupt Controller Physical CPU interface offset

  LDR     r0, [r1, #0x0028]       ; Read Highest Pending Interrupt Register (GICC_AHPPIR)

  BX      lr
  ENDP

  END

; ------------------------------------------------------------
; End of MP_VGIC.s
; ------------------------------------------------------------
