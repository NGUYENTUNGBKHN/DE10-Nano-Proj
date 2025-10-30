/******************************************************************************/
/*! @addtogroup Main
    @file       hal_sdmmc.c
    @brief      SD/MMC制御ドライバファイル。
    @date       2018/02/26
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
/*! @ingroup hal_sensor hal_sensor */
/* @{ */

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "memorymap.h"
#include "common.h"
#include "soc_cv_av/socal/socal.h"
#include "alt_sdmmc.h"
#include "hal_sdmmc.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/************************** Private Definitions *****************************/
#define SIGNATURE_LEN                   4
#define SDMMC_BLOCK_SZ    512
#define SDMMC_BLK_ALING_MSK 0xFFFFFE00
typedef struct {
  char header;
  char cy_h_and_sector;
  char cy_t;
} chs_t;
/* Firmware image file header */
typedef struct img_header {
        unsigned char signature[SIGNATURE_LEN];
        unsigned int startAddr;
        unsigned int burnAddr;
        unsigned int len;
}img_header_t;

#if 0
uint8_t sdmmc_buf[SDMMC_BLOCK_SZ];
#else
uint8_t *sdmmc_buf;
#endif
img_header_t *img_hdr_p;

typedef void (*FXN_PTR) (void);
//static uint32_t image_addr;
/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

ALT_SDMMC_CARD_INFO_t card_info;
ALT_SDMMC_CARD_MISC_t card_misc_cfg;
/*********************************************************************//**
 * @brief		Initialize SD/MMC
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ALT_STATUS_CODE  _hal_sdmmc_init(void)
{
    int i;
    //uint32_t sd_base = 0;
    uint32_t raw = 0;
    sdmmc_buf = (uint8_t *)SDMMC_BUF_BASE;

    //initialize sdmmc
    ALT_STATUS_CODE status = alt_sdmmc_init();
    if (status != ALT_E_SUCCESS) {
        //ALT_PRINTF("SDMMC init ERROR\n\r");
        return status;
    }
    status = alt_sdmmc_card_pwr_on();
    status = alt_sdmmc_card_identify(&card_info);
    if (status != ALT_E_SUCCESS) {
        //ALT_PRINTF("SDMMC id ERROR\n\r");
        return status;
    }
    status = alt_sdmmc_card_bus_width_set(&card_info, ALT_SDMMC_BUS_WIDTH_4);
    if (status != ALT_E_SUCCESS) {
        //ALT_PRINTF("SDMMC bus width set ERROR\n\r");
        return status;
    }
    alt_sdmmc_fifo_param_set((ALT_SDMMC_FIFO_NUM_ENTRIES >> 3) - 1,
                 ALT_SDMMC_FIFO_NUM_ENTRIES >> 3, (ALT_SDMMC_MULT_TRANS_t) 0);
    alt_sdmmc_dma_enable();
    alt_sdmmc_card_misc_get(&card_misc_cfg);
#if 0
    alt_sdmmc_card_speed_set(&card_info, card_info.xfer_speed);
#else
    alt_sdmmc_card_speed_set(&card_info, (card_info.high_speed ? 2 : 1) * card_info.xfer_speed);
#endif
     // read first block check for MBR Table
    status = alt_sdmmc_read(&card_info, sdmmc_buf, (void *) 0, SDMMC_BLOCK_SZ);
    if (status != ALT_E_SUCCESS) {
        //ALT_PRINTF("SDMMC read MBR ERROR\n\r");
        return status;
    }
    if (alt_read_hword(&sdmmc_buf[MBR_SIG_ADDR]) == MBR_SIGNATURE) {
        //ALT_PRINTF("Parsing MBR Table Found\n\r");
        for (i = 0; i < 4; i++) {
            mbr_partition_entry_t *mbr_entry = (mbr_partition_entry_t *)
                &sdmmc_buf[(MBR_P1_ADDR + (MBR_PENTRY_SZ * i))];
            if (0xA2 == mbr_entry->p_type) { //0xA2 is custom - partition type
                raw++;
                //sd_base = (((mbr_entry->lba_hw2) << 16) | mbr_entry->lba_hw1); //get block address
                //ALT_PRINTF("using custom partition %d\n\r", i);
            }
        }
        if (raw == 0) {
            //ALT_PRINTF("No custom partition found, Raw Mode\n\r");
            //sd_base = 0;
        }
    }
    else {
        //ALT_PRINTF("Raw Mode\n\r");
       //sd_base = 0;
    }
    ///read next image header using sd_base found above as the base address
    //start addr is lba needs to be byte address for read func below
    return status;
}


/* EOF */
