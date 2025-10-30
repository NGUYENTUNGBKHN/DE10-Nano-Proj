/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      Flash_api.h                                                             1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      紙幣識別機アプリケーション用QSPI Flashメモリインターフェースヘッダファイル              */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kanamaru     2018/03/06  V1.00                                                           */
/*                              Created initial version                                         */
/*                                                                                              */
/************************************************************************************************/
#ifndef _FLASH_API_H_
#define _FLASH_API_H_

/**** INTERNAL DEFINE ***************************************************************************/
#define FLASH_API_SUCCESS           (0x0000)
#define FLASH_API_INIT_ERROR        (0xD060)
#define FLASH_API_OPEN_ERROR        (0xD061)
#define FLASH_API_ERASE_ERROR       (0xD062)
#define FLASH_API_READ_ERROR        (0xD063)
#define FLASH_API_WRITE_ERROR       (0xD064)
#define FLASH_API_FAIL              (-1)

/* フラッシュメモリ設定 */
// 64Kセクタ
#define FLASH_SECTOR_SIZE           (65536)        /* セクタサイズ */
#define FLASH_SECTOR_NUMBER         (512)          /* セクタ数 */
#define FLASH_ERASE_CODE			QSPI_FLASH_ERASE_SEC
#define FLASH_MAX_SIZE              (FLASH_SECTOR_SIZE*FLASH_SECTOR_NUMBER)

/**** FUNCTION PROTOTYPES ***********************************************************************/
extern int  Flash_Drv_Initialize( void );
extern void Flash_Drv_Deinitialize( void );
extern int  Flash_Drv_Erase( unsigned long ulRomAddr, unsigned long ulSize );
extern int  Flash_Drv_Write( unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize );
extern int  Flash_Drv_EraseAndWrite( unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize );
extern int  Flash_Drv_Read(unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize);



#endif

