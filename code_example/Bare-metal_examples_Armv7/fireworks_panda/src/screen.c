/* Bare-metal fireworks example for Pandaboard */

/* Screen driver */

/* Copyright (c) 2011 Arm Limited (or its affiliates). All rights reserved. */
/* Use, modification and redistribution of this file is subject to your possession of a     */
/* valid End User License Agreement for the Arm Product of which these examples are part of */
/* and your compliance with all applicable terms and conditions of such licence agreement.  */


#define LCD_W 1024
#define LCD_H 768

// Clock Manager

#define CM_CLKSEL_MPU_M3_ISS_ROOT 0x4A008100
#define CM_CLKMODE_DPLL_PER      (volatile unsigned int *)(CM_CLKSEL_MPU_M3_ISS_ROOT + 0x40)
#define CM_CLKSEL_DPLL_PER       (volatile unsigned int *)(CM_CLKSEL_MPU_M3_ISS_ROOT + 0x4C)
#define CM_DIV_M5_DPLL_PER       (volatile unsigned int *)(CM_CLKSEL_MPU_M3_ISS_ROOT + 0x5C)

// Display Subsystem
#define DSS_BASE 0x48040000
#define DSS_SYSSTATUS            (volatile unsigned int *)(DSS_BASE + 0x14)
#define DSS_CONTROL              (volatile unsigned int *)(DSS_BASE + 0x40)
#define DSS_STATUS               (volatile unsigned int *)(DSS_BASE + 0x5C)

// Display Controller
#define DISPC_BASE 0x48041000
#define DISPC_SYSCONFIG          (volatile unsigned int *)(DISPC_BASE + 0x10)
#define DISPC_IRQSTATUS          (volatile unsigned int *)(DISPC_BASE + 0x18)
#define DISPC_IRQENABLE          (volatile unsigned int *)(DISPC_BASE + 0x1C)
#define DISPC_CONTROL1           (volatile unsigned int *)(DISPC_BASE + 0x40)
#define DISPC_CONTROL2           (volatile unsigned int *)(DISPC_BASE + 0x238)
#define DISPC_CONFIG1            (volatile unsigned int *)(DISPC_BASE + 0x44)
#define DISPC_CONFIG2            (volatile unsigned int *)(DISPC_BASE + 0x620)
#define DISPC_DEFAULT_COLOR_2    (volatile unsigned int *)(DISPC_BASE + 0x3AC)
#define DISPC_TRANS_COLOR_2      (volatile unsigned int *)(DISPC_BASE + 0x3B0)
#define DISPC_TIMING_H1          (volatile unsigned int *)(DISPC_BASE + 0x64)
#define DISPC_TIMING_V1          (volatile unsigned int *)(DISPC_BASE + 0x68)
#define DISPC_TIMING_H2          (volatile unsigned int *)(DISPC_BASE + 0x400)
#define DISPC_TIMING_V2          (volatile unsigned int *)(DISPC_BASE + 0x404)
#define DISPC_POL_FREQ1          (volatile unsigned int *)(DISPC_BASE + 0x6c)
#define DISPC_DIVISOR1           (volatile unsigned int *)(DISPC_BASE + 0x70)
#define DISPC_POL_FREQ2          (volatile unsigned int *)(DISPC_BASE + 0x408)
#define DISPC_DIVISOR2           (volatile unsigned int *)(DISPC_BASE + 0x40C)
#define DISPC_DIVISOR            (volatile unsigned int *)(DISPC_BASE + 0x804)
#define DISPC_SIZE_DIG           (volatile unsigned int *)(DISPC_BASE + 0x78)
#define DISPC_SIZE_LCD1          (volatile unsigned int *)(DISPC_BASE + 0x7c)
#define DISPC_SIZE_LCD2          (volatile unsigned int *)(DISPC_BASE + 0x3CC)
#define DISPC_GFX_BA0            (volatile unsigned int *)(DISPC_BASE + 0x80)
#define DISPC_GFX_BA1            (volatile unsigned int *)(DISPC_BASE + 0x84)
#define DISPC_GFX_POSITION       (volatile unsigned int *)(DISPC_BASE + 0x88)
#define DISPC_GFX_SIZE           (volatile unsigned int *)(DISPC_BASE + 0x8C)
#define DISPC_GFX_ATTRIBUTES     (volatile unsigned int *)(DISPC_BASE + 0xA0)
#define DISPC_GFX_FIFO_THRESHOLD (volatile unsigned int *)(DISPC_BASE + 0xA4)
#define DISPC_GFX_ROW_INC        (volatile unsigned int *)(DISPC_BASE + 0xAC)
#define DISPC_GFX_PIXEL_INC      (volatile unsigned int *)(DISPC_BASE + 0xB0)
#define DISPC_GFX_WINDOW_SKIP    (volatile unsigned int *)(DISPC_BASE + 0xB4)
#define DISPC_GFX_TABLE_BA       (volatile unsigned int *)(DISPC_BASE + 0xB8)


//Configure DVI-D output connected to LCD2.
void init_screen(unsigned int fb, unsigned int gfx_w, unsigned int gfx_h)
{
    //Configure DPLL_PER to generate 96Mhz on DSS_FCLK
	//96Mhz = Ref_clk * [ 2M/((N+1)*M5) ] = 38.4MHz * [2*20/((1+1)* 8)] = 96MHz
	*CM_CLKSEL_DPLL_PER       = 0x1400;     // N = 0 + 1, M = 20
    *CM_DIV_M5_DPLL_PER       = 0x328;      // M5 = 8

    *DSS_CONTROL              = 0x00000000; // Default value on reset

    *DISPC_GFX_ATTRIBUTES     = 0x4000008D; // Secondary LCD output selected, RGB16-565, GFXENABLE

    *DISPC_SYSCONFIG          =     0x2015; // Smart standby, smart idle, wakeup enable, autoidle
    *DISPC_IRQENABLE          =    0x00000; // Mask  all future  DISPC interrupts
    *DISPC_IRQSTATUS          =    0x1FFFF; // Clear any current DISPC interrupts
    *DISPC_CONTROL1           = 0x00000000; // Default value on reset
    *DISPC_CONTROL2           = 0x00000309; // LCD2 Enabled, Active or TFT operation, 24-bit output
    *DISPC_CONFIG1            = 0x00000004; // Frame data only loaded every frame (no Palette/Gamma Table)
    *DISPC_CONFIG2            = 0x00000004; // Frame data only loaded every frame (no Palette/Gamma Table)
    *DISPC_DEFAULT_COLOR_2    =   0x345678; // Pale blue
    *DISPC_TRANS_COLOR_2      =   0x000000; // Default value on reset

    *DISPC_TIMING_H2          = 0x0FF03F31; // HBP =255 , HFP =63 , HSW=49
    *DISPC_TIMING_V2          = 0x01400504; // VBP =20 , VFP =5 , VSW=4
    *DISPC_POL_FREQ2          =   0x007028; // Signal configuration
    *DISPC_DIVISOR            =   0x000001; // DISPC_DIVISOR.LCD bit field is used.
    *DISPC_DIVISOR2           =   0x010002; // Configure secondary LCD divisors
    *DISPC_SIZE_DIG           = ((LCD_H - 1)<<16) | (LCD_W - 1);
    *DISPC_SIZE_LCD2          = ((LCD_H - 1)<<16) | (LCD_W - 1);
    *DISPC_GFX_BA0            = fb;
    *DISPC_GFX_BA1            = fb;
    *DISPC_GFX_POSITION       = (((LCD_H - gfx_h)/2)<<16) | ((LCD_W - gfx_w)/2); // Centred
    *DISPC_GFX_SIZE           = ((gfx_h - 1)<<16) | (gfx_w - 1);
    *DISPC_GFX_ATTRIBUTES     = 0x4000008D; // RGB16-565, GFXENABLE
    *DISPC_GFX_FIFO_THRESHOLD = 0x03FF03C0; // Default value on reset
    *DISPC_GFX_ROW_INC        = 0x00000001; // Default value on reset
    *DISPC_GFX_PIXEL_INC      =     0x0001; // Default value on reset
    *DISPC_GFX_WINDOW_SKIP    = 0x00000000; // Default value on reset
    *DISPC_GFX_TABLE_BA       = 0x00000000; // Default value on reset

    *DISPC_CONTROL2 = *DISPC_CONTROL2 | 0x20; // Set GOLCD
    while (*DISPC_CONTROL2 & 0x20);         // Wait for DISPC_LCD2 to finish updating

}
