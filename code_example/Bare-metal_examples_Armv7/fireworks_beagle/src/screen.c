/* Bare-metal fireworks example for Beagle Board */

/* Screen driver */

/* Copyright (c) 2010 Arm Limited (or its affiliates). All rights reserved. */
/* Use, modification and redistribution of this file is subject to your possession of a     */
/* valid End User License Agreement for the Arm Product of which these examples are part of */
/* and your compliance with all applicable terms and conditions of such licence agreement.  */


#define LCD_W 1024
#define LCD_H 768

// Clock Manager
#define CLK_CM_BASE 0x48004D00
#define CM_CLKEN_PLL            (volatile unsigned int *)(CLK_CM_BASE + 0x00)
#define CM_CLKSEL2_PLL          (volatile unsigned int *)(CLK_CM_BASE + 0x44)

// Display Subsystem Clock Manager
#define DSS_CM_BASE 0x48004E00
#define CM_CLKSEL_DSS           (volatile unsigned int *)(DSS_CM_BASE + 0x40)

// Display Subsystem
#define DSS_BASE 0x48050000
#define DSS_SYSCONFIG            (volatile unsigned int *)(DSS_BASE + 0x10)
#define DSS_CONTROL              (volatile unsigned int *)(DSS_BASE + 0x40)
#define DSS_SDI_CONTROL          (volatile unsigned int *)(DSS_BASE + 0x44)
#define DSS_PLL_CONTROL          (volatile unsigned int *)(DSS_BASE + 0x48)

// Display Controller
#define DISPC_BASE 0x48050400
#define DISPC_SYSCONFIG          (volatile unsigned int *)(DISPC_BASE + 0x10)
#define DISPC_IRQSTATUS          (volatile unsigned int *)(DISPC_BASE + 0x18)
#define DISPC_IRQENABLE          (volatile unsigned int *)(DISPC_BASE + 0x1C)
#define DISPC_CONTROL            (volatile unsigned int *)(DISPC_BASE + 0x40)
#define DISPC_CONFIG             (volatile unsigned int *)(DISPC_BASE + 0x44)
#define DISPC_DEFAULT_COLOR_0    (volatile unsigned int *)(DISPC_BASE + 0x4C)
#define DISPC_DEFAULT_COLOR_1    (volatile unsigned int *)(DISPC_BASE + 0x50)
#define DISPC_TRANS_COLOR_0      (volatile unsigned int *)(DISPC_BASE + 0x54)
#define DISPC_TRANS_COLOR_1      (volatile unsigned int *)(DISPC_BASE + 0x58)
#define DISPC_TIMING_H           (volatile unsigned int *)(DISPC_BASE + 0x64)
#define DISPC_TIMING_V           (volatile unsigned int *)(DISPC_BASE + 0x68)
#define DISPC_POL_FREQ           (volatile unsigned int *)(DISPC_BASE + 0x6C)
#define DISPC_DIVISOR            (volatile unsigned int *)(DISPC_BASE + 0x70)
#define DISPC_SIZE_DIG           (volatile unsigned int *)(DISPC_BASE + 0x78)
#define DISPC_SIZE_LCD           (volatile unsigned int *)(DISPC_BASE + 0x7C)
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


#ifndef DM37_
#define DM37_
#define DM37x_PROD_CODE    0xCAFEB891
#endif


void init_screen(unsigned int fb, unsigned int gfx_w, unsigned int gfx_h, unsigned int processor)
{
    if (processor == DM37x_PROD_CODE) /* BeagleBoard-xM */
    {
	   *CM_CLKSEL2_PLL  = 0x0483600C; // PERIPH_DPLL_MULT=864, PERIPH_DPLL_DIV=12, Lock F = 1GHz - 2GHz, SD_DIV = 4
       *CM_CLKSEL_DSS   = 0x00001006; // CLKSEL_TV div=16, CLKSEL_DSS1 div=6
    }
    else
    {
       *CM_CLKSEL2_PLL  =    0x1B00C; // PERIPH_DPLL_MULT=432, PERIPH_DPLL_DIV=12
       *CM_CLKSEL_DSS   =     0x1006; // CLKSEL_TV div=16, CLKSEL_DSS1 div=6
       *CM_CLKEN_PLL    = 0x00370037; // Lock DPLL3 & DPLL4 to 0.75~1MHz
    }

    *DSS_SYSCONFIG   = 0x00000001; // Autoidle
    *DSS_CONTROL     = 0x00000078;
    *DSS_SDI_CONTROL = 0x00000000; // Default value on reset
    *DSS_PLL_CONTROL = 0x00000000; // Default value on reset

    *DISPC_SYSCONFIG          =     0x2015; // Smart standby, smart idle, wakeup enable, autoidle
    *DISPC_IRQENABLE          =    0x00000; // Mask  all future  DISPC interrupts
    *DISPC_IRQSTATUS          =    0x1FFFF; // Clear any current DISPC interrupts
    *DISPC_CONTROL            = 0x0001830B; // Bypass (RFBI disabled), 24 bit output, enable digital & LCD
    *DISPC_CONFIG             = 0x00000004; // Frame data only loaded every frame (no Palette/Gamma Table)
    *DISPC_DEFAULT_COLOR_0    =   0x345678; // Pale blue
    *DISPC_DEFAULT_COLOR_1    =   0x000000; // Default value on reset
    *DISPC_TRANS_COLOR_0      =   0x000000; // Default value on reset
    *DISPC_TRANS_COLOR_1      =   0x000000; // Default value on reset

    *DISPC_TIMING_H           = 0x0FF03F31;
    *DISPC_TIMING_V           = 0x01400504;
    *DISPC_POL_FREQ           =   0x007028;
    *DISPC_DIVISOR            =   0x010002; // Default value on reset
    *DISPC_SIZE_DIG           = ((LCD_H - 1)<<16) | (LCD_W - 1);
    *DISPC_SIZE_LCD           = ((LCD_H - 1)<<16) | (LCD_W - 1);
    *DISPC_GFX_BA0            = fb;
    *DISPC_GFX_BA1            = fb;
    *DISPC_GFX_POSITION       = (((LCD_H - gfx_h)/2)<<16) | ((LCD_W - gfx_w)/2); // Centred
    *DISPC_GFX_SIZE           = ((gfx_h - 1)<<16) | (gfx_w - 1);
    *DISPC_GFX_ATTRIBUTES     =     0x008d; // RGB16, GFXENABLE
    *DISPC_GFX_FIFO_THRESHOLD = 0x03FF03C0; // Default value on reset
    *DISPC_GFX_ROW_INC        = 0x00000001; // Default value on reset
    *DISPC_GFX_PIXEL_INC      =     0x0001; // Default value on reset
    *DISPC_GFX_WINDOW_SKIP    = 0x00000000; // Default value on reset
    *DISPC_GFX_TABLE_BA       = 0x00000000; // Default value on reset

    *DISPC_CONTROL = *DISPC_CONTROL | 0x60; // Set GOLCD & GO DIGITAL
    while (*DISPC_CONTROL & 0x60);  // Wait for DISPC to finish updating
}
