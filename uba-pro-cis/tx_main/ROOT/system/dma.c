/******************************************************************************/
/*! @addtogroup Group2
    @file       if_main.c
    @brief      Main process for I/F
    @date       2018/02/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/

#include "kernel.h"
#include "kernel_inc.h"
#include "MP_SCU.h"
#include "soc_cv_av/alt_reset_manager.h"
#include "alt_interrupt.h"
#include "alt_address_space.h"
#include "alt_dma.h"
#include "common.h"

#define EXT
#include "com_ram.c"


/************************** External function *****************************/
/************************** Variable declaration *****************************/

/*********************************************************************//**
 * @brief		Initialize DMA Controller (DMA-330)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ALT_STATUS_CODE dmac_init(void)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    // Uninit DMA
    if(status == ALT_E_SUCCESS)
    {
        status = alt_dma_uninit();
    }

    // Configure everything as defaults.
    if (status == ALT_E_SUCCESS)
    {
        ALT_DMA_CFG_t dma_config;
        dma_config.manager_sec = ALT_DMA_SECURITY_DEFAULT;
        for (int i = 0; i < 8; ++i)
        {
            dma_config.irq_sec[i] = ALT_DMA_SECURITY_DEFAULT;
        }
        for (int i = 0; i < 32; ++i)
        {
            dma_config.periph_sec[i] = ALT_DMA_SECURITY_DEFAULT;
        }
        for (int i = 0; i < 4; ++i)
        {
            dma_config.periph_mux[i] = ALT_DMA_PERIPH_MUX_DEFAULT;
        }

        status = alt_dma_init(&dma_config);
    }

    return status;
}

/*********************************************************************//**
 * @brief		Start DMA transfer
 * @param[in]	ALT_STATUS_CODE result
 * @return 		None
 **********************************************************************/
ALT_STATUS_CODE dmac_start(void* src, void* dst, u32 size)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

	//--ACP configuration--//
	const u32 ARUSER = 0x0000001F; //coherent cacheable reads
	const u32 AWUSER = 0x0000001F; //coherent cacheable writes
	if(status == ALT_E_SUCCESS)
    {
		//Set output ID3 for dynamic reads and ID4 for dynamic writes
		status = alt_acp_id_map_dynamic_read_set(ALT_ACP_ID_OUT_DYNAM_ID_3);
		status = alt_acp_id_map_dynamic_write_set(ALT_ACP_ID_OUT_DYNAM_ID_4);
		//Configure the page and user write sideband signal options that are applied
		//to all write transactions that have their input IDs dynamically mapped.
		status = alt_acp_id_map_dynamic_read_options_set(ALT_ACP_ID_MAP_PAGE_0, ARUSER);
		status = alt_acp_id_map_dynamic_write_options_set(ALT_ACP_ID_MAP_PAGE_0, AWUSER);
	}

	if(status != ALT_E_SUCCESS)
	{
		return ALT_E_ERROR;
	}

    //--DMAC initialization--//
    status = dmac_init();
	if(status != ALT_E_SUCCESS)
	{
		return ALT_E_ERROR;
	}
    //--define some variables to do the transfer--//
	ALT_DMA_CHANNEL_t Dma_Channel; // DMA channel to be used

    //--Allocate a DMA channel and do the transfer--//
    status = alt_dma_channel_alloc_any(&Dma_Channel);
    if(status != ALT_E_SUCCESS)
	{
		return ALT_E_ERROR;
	}

    ALT_DMA_PROGRAM_t* program_ptr = (ALT_DMA_PROGRAM_t*) 0xFFFF0000;
    //--Copy source buffer to destination buffer--//
	status = alt_dma_memory_to_memory(
			Dma_Channel,
			program_ptr,
			dst,
			src,
			size,
			false,
			(ALT_DMA_EVENT_t)0);
	// Wait for transfer to complete
	if (status == ALT_E_SUCCESS)
	{
		ALT_DMA_CHANNEL_STATE_t channel_state = ALT_DMA_CHANNEL_STATE_EXECUTING;
		while((status == ALT_E_SUCCESS) && (channel_state != ALT_DMA_CHANNEL_STATE_STOPPED))
		{
			status = alt_dma_channel_state_get(Dma_Channel, &channel_state);
			if(channel_state == ALT_DMA_CHANNEL_STATE_FAULTING)
			{
				return ALT_E_ERROR;
			}
		}
	}

	return status;
}
/* EOF */
