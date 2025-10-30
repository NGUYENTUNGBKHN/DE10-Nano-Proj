/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: INTC Register														*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_INTC_REG__
#define __J_INTC_REG__


#define	INTC_DIST_BASE			0x1000
#define	INTC_CPU_BASE			0x0100

#define INTC_ICDDCR				(0x000+INTC_DIST_BASE)
#define INTC_ICDICTR			(0x004+INTC_DIST_BASE)
#define INTC_ICDIIDR			(0x008+INTC_DIST_BASE)
#define INTC_ICDISR				(0x080+INTC_DIST_BASE)
#define INTC_ICDISER(n)			((0x100+(n*4))+INTC_DIST_BASE)
#define INTC_ICDICER(n)			((0x180+(n*4))+INTC_DIST_BASE)
#define INTC_ICDISPR(n)			((0x200+(n*4))+INTC_DIST_BASE)
#define INTC_ICDICPR(n)			((0x280+(n*4))+INTC_DIST_BASE)
#define INTC_ICDIABR(n)			((0x300+(n*4))+INTC_DIST_BASE)
#define INTC_ICDIPR(n)			((0x400+n)+INTC_DIST_BASE)
#define INTC_ICDIPTR(n)			((0x800+n)+INTC_DIST_BASE)
#define INTC_ICDICFR(n)			((0xC00+(n*4))+INTC_DIST_BASE)
#define INTC_ICDPPISR			(0xD00+INTC_DIST_BASE)
#define INTC_ICDSPISR(n)		((0xD04+(n*4))+INTC_DIST_BASE)
#define INTC_ICDSGIR			(0xF00+INTC_DIST_BASE)
#define INTC_ICPIDR(n)			((0xFD0+(n*4))+INTC_DIST_BASE)
#define INTC_ICCIDR(n)			((0xFF0+(n*4))+INTC_DIST_BASE)

#define INTC_ICCICR				(0x000+INTC_CPU_BASE)
#define INTC_ICCPMR 			(0x004+INTC_CPU_BASE)
#define INTC_ICCBPR 			(0x008+INTC_CPU_BASE)
#define INTC_ICCIAR				(0x00C+INTC_CPU_BASE)
#define INTC_ICCEOIR 			(0x010+INTC_CPU_BASE)
#define INTC_ICCRPR				(0x014+INTC_CPU_BASE)
#define INTC_ICCHPIR	 		(0x018+INTC_CPU_BASE)
#define INTC_ICCABPR			(0x01C+INTC_CPU_BASE)
#define INTC_ICCIIDR	 		(0x0FC+INTC_CPU_BASE)


#endif /* __J_INTC_REG__ */




