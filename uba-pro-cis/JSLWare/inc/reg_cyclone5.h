/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: Cyclone V Register definission										*/
/* Comment:																		*/
/* 	・本ソースはデバイス毎に準備して下さい。									*/
/*==============================================================================*/

#ifndef __REG_DEVICE__
#define __REG_DEVICE__

/* ============================================================================== */
/* ============================================================================== */

/* レジスタアクセスは以下のマクロを使用する事 */
#define IOREG8(x,y)			(*(volatile UINT8 *)(x+y))
#define IOREG16(x,y)		(*(volatile UINT16 *)(x+y))
#define IOREG32(x,y)		(*(volatile UINT32 *)(x+y))
#define IOREG_ADDR(x,y)		(x+y)

/* ============================================================================== */
/* ============================================================================== */

#define	SYS_BASE			0xFFD08000
#define	CPU_CFG_BASE		0xFFFEC000
#define	INTC_BASE			CPU_CFG_BASE
#define	PTIME_BASE			CPU_CFG_BASE
#define	SCU_BASE			CPU_CFG_BASE
#define	L2C_BASE			0xFFFEF000
#define	RSTMGR_BASE			0xFFD05000
#define	CLKMGR_BASE			0xFFD04000
#define	SDRC_BASE			0xFFC20000
#define	L3REGS_BASE			0xFF800000
#define	SCAN_BASE			0xFFF02000
#define	FPGAMGR_BASE		0xFF706000
#define	LWFPGASLAVES_BASE	0xFF200000
#define	FPGASLAVES_BASE		0xC0000000

#define	MAC_PORT_CNT		2
#define	MAC0_BASE			0xFF700000
#define	MAC1_BASE			0xFF702000

#define	MMC_PORT_CNT		1
#define MMC0_BASE			0xFF704000

#define	QSPI_PORT_CNT		1
#define	QSPI0_BASE			0xFF705000
#define	QSPI0_DATA_BASE		0xFFA00000

#define	GPIO_PORT_CNT		3
#define GPIO0_BASE			0xFF708000
#define GPIO1_BASE			0xFF709000
#define GPIO2_BASE			0xFF70A000

#define	CAN_PORT_CNT		2
#define	CAN0_BASE			0xFFC00000
#define	CAN1_BASE			0xFFC01000

#define	UART_PORT_CNT		2
#define	UART0_BASE			0xFFC02000
#define	UART1_BASE			0xFFC03000

#define	I2C_PORT_CNT		4
#define	I2C0_BASE			0xFFC04000
#define	I2C1_BASE			0xFFC05000
#define	I2C2_BASE			0xFFC06000
#define	I2C3_BASE			0xFFC07000

#define	DTIM_PORT_CNT		4
#define DTIM0_BASE			0xFFC08000	/* SP Timer0 */
#define DTIM1_BASE			0xFFC09000	/* SP Timer1 */
#define DTIM2_BASE			0xFFD00000	/* OSC Timer0 */
#define DTIM3_BASE			0xFFD01000	/* OSC Timer1 */

#define	WDT_PORT_CNT		2
#define WDT0_BASE			0xFFD02000
#define WDT1_BASE			0xFFD03000

#define	SPI_PORT_CNT		4
#define	SPI0_BASE			0xFFF00000	/* Master0 */
#define	SPI1_BASE			0xFFF01000	/* Master1 */
#define	SPI2_BASE			0xFFE02000	/* Slave0 */
#define	SPI3_BASE			0xFFE03000	/* Slave1 */

#define	DMA_PORT_CNT		1
#define	DMA0_NS_BASE		0xFFE00000
#define	DMA0_SC_BASE		0xFFE01000

/* ============================================================================== */
/* ============================================================================== */

#define	OSW_INT_NUM							212
#define	OSW_PRIVINT_NUM						32
/* interrupt_id */
/* Private */
#define OSW_INT_SGI0						0
#define OSW_INT_SGI1						1
#define OSW_INT_SGI2						2
#define OSW_INT_SGI3						3
#define OSW_INT_SGI4						4
#define OSW_INT_SGI5						5
#define OSW_INT_SGI6						6
#define OSW_INT_SGI7						7
#define OSW_INT_SGI8						8
#define OSW_INT_SGI9						9
#define OSW_INT_SGI10						10
#define OSW_INT_SGI11						11
#define OSW_INT_SGI12						12
#define OSW_INT_SGI13						13
#define OSW_INT_SGI14						14
#define OSW_INT_SGI15						15
#define OSW_INT_GLOBAL_TIMER				27
#define OSW_INT_FIQ							28
#define OSW_INT_PRIVATE_TIMER				29
#define OSW_INT_PRIVATE_WDT					30
#define OSW_INT_IRQ							31
/* Shared */
#define OSW_INT_CPU0_PARITYFALL				32
#define OSW_INT_CPU0_PARITYFALL_BTAC		33
#define OSW_INT_CPU0_PARITYFALL_GHB			34
#define OSW_INT_CPU0_PARITYFALL_I_TAG		35
#define OSW_INT_CPU0_PARITYFALL_I_DATA		36
#define OSW_INT_CPU0_PARITYFALL_TLB			37
#define OSW_INT_CPU0_PARITYFALL_D_OUTER		38
#define OSW_INT_CPU0_PARITYFALL_D_TAG		39
#define OSW_INT_CPU0_PARITYFALL_D_DATA		40
#define OSW_INT_CPU0_DEFLAGS0				41
#define OSW_INT_CPU0_DEFLAGS1				42
#define OSW_INT_CPU0_DEFLAGS2				43
#define OSW_INT_CPU0_DEFLAGS3				44
#define OSW_INT_CPU0_DEFLAGS4				45
#define OSW_INT_CPU0_DEFLAGS5				46
#define OSW_INT_CPU0_DEFLAGS6				47
#define OSW_INT_CPU1_PARITYFALL				48
#define OSW_INT_CPU1_PARITYFALL_BTAC		49
#define OSW_INT_CPU1_PARITYFALL_GHB			50
#define OSW_INT_CPU1_PARITYFALL_I_TAG		51
#define OSW_INT_CPU1_PARITYFALL_I_DATA		52
#define OSW_INT_CPU1_PARITYFALL_TLB			53
#define OSW_INT_CPU1_PARITYFALL_D_OUTER		54
#define OSW_INT_CPU1_PARITYFALL_D_TAG		55
#define OSW_INT_CPU1_PARITYFALL_D_DATA		56
#define OSW_INT_CPU1_DEFLAGS0				57
#define OSW_INT_CPU1_DEFLAGS1				58
#define OSW_INT_CPU1_DEFLAGS2				59
#define OSW_INT_CPU1_DEFLAGS3				60
#define OSW_INT_CPU1_DEFLAGS4				61
#define OSW_INT_CPU1_DEFLAGS5				62
#define OSW_INT_CPU1_DEFLAGS6				63
#define OSW_INT_SCU_PARITYFALL0				64
#define OSW_INT_SCU_PARITYFALL1				65
#define OSW_INT_SCU_EV_ABORT				66
#define OSW_INT_L2_ECC_BYTE_WR_IRQ			67
#define OSW_INT_L2_COR_IRQ					68
#define OSW_INT_L2_UNCOR_IRQ				69
#define OSW_INT_L2_COMBINED_IRQ				70
#define OSW_INT_DDR_ECC_ERROR_IRQ			71
#define OSW_INT_FPGA_IRQ0					72
#define OSW_INT_FPGA_IRQ1					73
#define OSW_INT_FPGA_IRQ2					74
#define OSW_INT_FPGA_IRQ3					75
#define OSW_INT_FPGA_IRQ4					76
#define OSW_INT_FPGA_IRQ5					77
#define OSW_INT_FPGA_IRQ6					78
#define OSW_INT_FPGA_IRQ7					79
#define OSW_INT_FPGA_IRQ8					80
#define OSW_INT_FPGA_IRQ9					81
#define OSW_INT_FPGA_IRQ10					82
#define OSW_INT_FPGA_IRQ11					83
#define OSW_INT_FPGA_IRQ12					84
#define OSW_INT_FPGA_IRQ13					85
#define OSW_INT_FPGA_IRQ14					86
#define OSW_INT_FPGA_IRQ15					87
#define OSW_INT_FPGA_IRQ16					88
#define OSW_INT_FPGA_IRQ17					89
#define OSW_INT_FPGA_IRQ18					90
#define OSW_INT_FPGA_IRQ19					91
#define OSW_INT_FPGA_IRQ20					92
#define OSW_INT_FPGA_IRQ21					93
#define OSW_INT_FPGA_IRQ22					94
#define OSW_INT_FPGA_IRQ23					95
#define OSW_INT_FPGA_IRQ24					96
#define OSW_INT_FPGA_IRQ25					97
#define OSW_INT_FPGA_IRQ26					98
#define OSW_INT_FPGA_IRQ27					99
#define OSW_INT_FPGA_IRQ28					100
#define OSW_INT_FPGA_IRQ29					101
#define OSW_INT_FPGA_IRQ30					102
#define OSW_INT_FPGA_IRQ31					103
#define OSW_INT_FPGA_IRQ32					104
#define OSW_INT_FPGA_IRQ33					105
#define OSW_INT_FPGA_IRQ34					106
#define OSW_INT_FPGA_IRQ35					107
#define OSW_INT_FPGA_IRQ36					108
#define OSW_INT_FPGA_IRQ37					109
#define OSW_INT_FPGA_IRQ38					110
#define OSW_INT_FPGA_IRQ39					111
#define OSW_INT_FPGA_IRQ40					112
#define OSW_INT_FPGA_IRQ41					113
#define OSW_INT_FPGA_IRQ42					114
#define OSW_INT_FPGA_IRQ43					115
#define OSW_INT_FPGA_IRQ44					116
#define OSW_INT_FPGA_IRQ45					117
#define OSW_INT_FPGA_IRQ46					118
#define OSW_INT_FPGA_IRQ47					119
#define OSW_INT_FPGA_IRQ48					120
#define OSW_INT_FPGA_IRQ49					121
#define OSW_INT_FPGA_IRQ50					122
#define OSW_INT_FPGA_IRQ51					123
#define OSW_INT_FPGA_IRQ52					124
#define OSW_INT_FPGA_IRQ53					125
#define OSW_INT_FPGA_IRQ54					126
#define OSW_INT_FPGA_IRQ55					127
#define OSW_INT_FPGA_IRQ56					128
#define OSW_INT_FPGA_IRQ57					129
#define OSW_INT_FPGA_IRQ58					130
#define OSW_INT_FPGA_IRQ59					131
#define OSW_INT_FPGA_IRQ60					132
#define OSW_INT_FPGA_IRQ61					133
#define OSW_INT_FPGA_IRQ62					134
#define OSW_INT_FPGA_IRQ63					135
#define OSW_INT_DMA_IRQ0					136
#define OSW_INT_DMA_IRQ1					137
#define OSW_INT_DMA_IRQ2					138
#define OSW_INT_DMA_IRQ3					139
#define OSW_INT_DMA_IRQ4					140
#define OSW_INT_DMA_IRQ5					141
#define OSW_INT_DMA_IRQ6					142
#define OSW_INT_DMA_IRQ7					143
#define OSW_INT_DMA_IRQ_ABORT				144
#define OSW_INT_DMA_ECC_COR_IRQ				145
#define OSW_INT_DMA_ECC_UNCOR_IRQ			146
#define OSW_INT_EMAC0_IRQ					147
#define OSW_INT_EMAC0_TX_ECC_COR_IRQ		148
#define OSW_INT_EMAC0_TX_ECC_UNCOR_IRQ 		149
#define OSW_INT_EMAC0_RX_ECC_COR_IRQ		150
#define OSW_INT_EMAC0_RX_ECC_UNCOR_IRQ 		151
#define OSW_INT_EMAC1_IRQ					152
#define OSW_INT_EMAC1_TX_ECC_COR_IRQ		153
#define OSW_INT_EMAC1_TX_ECC_UNCOR_IRQ 		154
#define OSW_INT_EMAC1_RX_ECC_COR_IRQ		155
#define OSW_INT_EMAC1_RX_ECC_UNCOR_IRQ 		156
#define OSW_INT_USB0_IRQ					157
#define OSW_INT_USB0_COR_IRQ				158
#define OSW_INT_USB0_UNCOR_IRQ				159
#define OSW_INT_USB1_IRQ					160
#define OSW_INT_USB1_COR_IRQ				161
#define OSW_INT_USB1_UNCOR_IRQ				162
#define OSW_INT_CAN0_STS_IRQ				163
#define OSW_INT_CAN0_MO_IRQ					164
#define OSW_INT_CAN0_ECC_COR_IRQ			165
#define OSW_INT_CAN0_ECC_UNCOR_IRQ			166
#define OSW_INT_CAN1_STS_IRQ				167
#define OSW_INT_CAN1_MO_IRQ					168
#define OSW_INT_CAN1_ECC_COR_IRQ			169
#define OSW_INT_CAN1_ECC_UNCOR_IRQ			170
#define OSW_INT_SDMMC0_IRQ					171
#define OSW_INT_SDMMC0_PORTA_ECC_COR_IRQ	172
#define OSW_INT_SDMMC0_PORTA_ECC_UNCOR_IRQ	173
#define OSW_INT_SDMMC0_PORTB_ECC_COR_IRQ	174
#define OSW_INT_SDMMC0_PORTB_ECC_UNCOR_IRQ	175
#define OSW_INT_NAND0_IRQ					176
#define OSW_INT_NANDR0_ECC_COR_IRQ			177
#define OSW_INT_NANDR0_ECC_UNCOR_IRQ		178
#define OSW_INT_NANDW0_ECC_COR_IRQ			179
#define OSW_INT_NANDW0_ECC_UNCOR_IRQ		180
#define OSW_INT_NANDE0_ECC_COR_IRQ			181
#define OSW_INT_NANDE0_ECC_UNCOR_IRQ		182
#define OSW_INT_QSPI0_IRQ					183
#define OSW_INT_QSPI0_ECC_COR_IRQ			184
#define OSW_INT_QSPI0_ECC_UNCOR_IRQ			185
#define OSW_INT_SPI0_IRQ					186
#define OSW_INT_SPI1_IRQ					187
#define OSW_INT_SPI2_IRQ					188
#define OSW_INT_SPI3_IRQ					189
#define OSW_INT_I2C0_IRQ					190
#define OSW_INT_I2C1_IRQ					191
#define OSW_INT_I2C2_IRQ					192
#define OSW_INT_I2C3_IRQ					193
#define OSW_INT_UART0_IRQ					194
#define OSW_INT_UART1_IRQ					195
#define OSW_INT_GPIO0_IRQ					196
#define OSW_INT_GPIO1_IRQ					197
#define OSW_INT_GPIO2_IRQ					198
#define OSW_INT_TIM0_IRQ					199
#define OSW_INT_TIM1_IRQ					200
#define OSW_INT_TIM2_IRQ					201
#define OSW_INT_TIM3_IRQ					202
#define OSW_INT_WDT0_IRQ					203
#define OSW_INT_WDT1_IRQ					204
#define OSW_INT_CLKMGR0_IRQ					205
#define OSW_INT_MPUWAKEUP0_IRQ				206
#define OSW_INT_FPGA_MAN0_IRQ				207
#define OSW_INT_NCTI0_IRQ					208
#define OSW_INT_NCTI1_IRQ					209
#define OSW_INT_RAM0_ECC_COR_IRQ			210
#define OSW_INT_RAM0_ECC_UNCOR_IRQ			211

/* System Manager */
#define	SYS_SILICONID1						0x00
#define	SYS_SILICONID2						0x04
#define	SYS_WDDBG							0x10
#define	SYS_BOOTINFO						0x14
#define	SYS_HPSINFO							0x18
#define	SYS_PARITYINJ						0x1C
/* FPGAINTFGRP */
#define	SYS_FPGAINT_GBL						0x20
#define	SYS_FPGAINT_INDIV					0x24
#define	SYS_FPGAINT_MODULE					0x28
/* SCANMGRGRP */
#define	SYS_SCAN_CTRL_MODULE				0x30
/* FRZCTRL */
#define	SYS_FRZ_VIOCTRL(n)					(0x40+(4*n))
#define	SYS_FRZ_HIOCTRL						0x50
#define	SYS_FRZ_SRC							0x54
#define	SYS_FRZ_HWCTRL						0x58
/* EMACGRP */
#define	SYS_EMAC_CTRL						0x60
#define	SYS_EMAC_L3MASTER					0x64
/* DMAGRP */
#define	SYS_DMA_CTRL						0x70
#define	SYS_DMA_PERSECURITY					0x74
/* ISWGRP */
#define	SYS_ISW_HANDOFF(n)					(0x80+(n*4))
/* ROMCODEGRP */
#define	SYS_ROMCODE_CTRL					0xC0
#define	SYS_ROMCODE_CPU1STARTADDR			0xC4
#define	SYS_ROMCODE_INITSWSTATE				0xC8
#define	SYS_ROMCODE_INITSWLASTLD			0xCC
#define	SYS_ROMCODE_BOOTROMSWSTATE			0xD0
#define	SYS_ROMCODE_WARMRAMGRP				0xE0
/* ROMHWGRP */
#define	SYS_ROMHW_CTRL						0x100
/* SDMMCGRP */
#define	SYS_SDMMC_CTRL						0x108
#define	SYS_SDMMC_L3MASTER					0x10C
/* NANDGRP */
#define	SYS_NAND_BOOTSTRAP					0x110
#define	SYS_NAND_L3MASTER					0x114
/* USBGRP */
#define	SYS_USB_L3MASTER					0x118
/* ECCGRP */
#define	SYS_ECC_L2							0x140
#define	SYS_ECC_OCRAM						0x144
#define	SYS_ECC_USB0						0x148
#define	SYS_ECC_USB1						0x14C
#define	SYS_ECC_EMAC0						0x150
#define	SYS_ECC_EMAC1						0x154
#define	SYS_ECC_DMA							0x158
#define	SYS_ECC_CAN0						0x15C
#define	SYS_ECC_CAN1						0x160
#define	SYS_ECC_NAND						0x164
#define	SYS_ECC_QSPI						0x168
#define	SYS_ECC_SDMMC						0x16C
/* PINMUXGRP */
#define	SYS_PINMUX_EMACIO(n)				(0x400+(n*4))
#define	SYS_PINMUX_FLASHIO(n)				(0x450+(n*4))
#define	SYS_PINMUX_GENERALIO(n)				(0x480+(n*4))
#define	SYS_PINMUX_MIXED1IO0(n)				(0x500+(n*4))
#define	SYS_PINMUX_MIXED2IO0(n)				(0x558+(n*4))
#define	SYS_PINMUX_GPLINMUX(n)				(0x578+((n-48)*4))
#define	SYS_PINMUX_GPLMUX(n)				(0x5D4+(n*4))
#define	SYS_PINMUX_NANDUSEFPGA				0x6F0
#define	SYS_PINMUX_RGMII1USEFPGA			0x6F8
#define	SYS_PINMUX_I2C0USEFPGA				0x704
#define	SYS_PINMUX_RGMII0USEFPGA			0x714
#define	SYS_PINMUX_I2C3USEFPGA				0x724
#define	SYS_PINMUX_I2C2USEFPGA				0x728
#define	SYS_PINMUX_I2C1USEFPGA				0x72C
#define	SYS_PINMUX_SPIM1USEFPGA				0x730
#define	SYS_PINMUX_SPIM0USEFPGA				0x738




#endif /* __REG_DEVICE__ */




