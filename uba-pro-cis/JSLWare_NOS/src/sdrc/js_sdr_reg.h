/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: SDRAM Register														*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_SDRAM_REG__
#define __J_SDRAM_REG__


#define SDRAM_CTRLCFG				0x5000
#define SDRAM_DRAMTIMING1			0x5004
#define SDRAM_DRAMTIMING2			0x5008
#define SDRAM_DRAMTIMING3			0x500C
#define SDRAM_DRAMTIMING4			0x5010
#define SDRAM_LOWPWRTIMING			0x5014
#define SDRAM_DRAMODT				0x5018
#define SDRAM_DRAMADDRW				0x502C
#define SDRAM_DRAMIFWIDTH			0x5030
#define SDRAM_DRAMDEVWIDTH			0x5034
#define SDRAM_DRAMSTS				0x5038
#define SDRAM_DRAMINTR				0x503C
#define SDRAM_SBECOUNT				0x5040
#define SDRAM_DBECOUNT				0x5044
#define SDRAM_ERRADDR				0x5048
#define SDRAM_DROPCOUNT				0x504C
#define SDRAM_DROPADDR				0x5050
#define SDRAM_LOWPWREQ				0x5054
#define SDRAM_LOWPWRACK				0x5058
#define SDRAM_STATICCFG				0x505C
#define SDRAM_CTRLWIDTH				0x5060
#define SDRAM_PORTCFG				0x507C
#define SDRAM_FPGAPORTRST			0x5080
#define SDRAM_PROTPORTDEFAULT		0x508C
#define SDRAM_PROTRULEADDR			0x5090
#define SDRAM_PROTRULEID			0x5094
#define SDRAM_PROTRULEDATA			0x5098
#define SDRAM_PROTRULERDWR			0x509C
#define SDRAM_QOSLOWPRI				0x50A0
#define SDRAM_QOSHIGHPRI			0x50A4
#define SDRAM_QOSPRIORITYEN			0x50A8
#define SDRAM_MPPRIORITY			0x50AC
#define SDRAM_MPWEIGHT0				0x50B0
#define SDRAM_MPWEIGHT1				0x50B4
#define SDRAM_MPWEIGHT2				0x50B8
#define SDRAM_MPWEIGHT3				0x50BC
#define SDRAM_REMAPPRIORITY			0x50E0

#define	SDRAM_PHYCTRL0				0x5150
#define	SDRAM_PHYCTRL1				0x5154
#define	SDRAM_PHYCTRL2				0x5158


#endif /* __J_SDRAM_REG__ */


