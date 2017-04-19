/**
  ******************************************************************************
  * @file    drv_sd.h
  * @author  mousie-yu
  * @version V1.1.1
  * @date    2011.11.22
  * @brief   SD����������, ʹ��SPIģʽ. ֧��MMC��(δ��), SD��, SDHC��
  *          V1.1.1 �޸��˵ȴ�Ӧ�����ʱ, �Ӷ������˴�����SD_V2��
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_SD_H
#define __DRV_SD_H

/* Includes ------------------------------------------------------------------*/
//#include "main.h"
#include "stm32f4xx_hal.h"
//#include "stm32f10x_gpio.h"
//#include "stm32f10x_rcc.h"

/** @addtogroup Drivers
  * @{
  */
/** @addtogroup DRV_SD
  * @{
  */



/**
  ******************************************************************************
  * @defgroup DRV_SD_Configure
  * @brief    �û�����
  ******************************************************************************
  * @{
  */

/** @defgroup SD_Pin_Assignment
  * @brief    SD���Ŷ�Ӧ��ϵ, ����ͼ:
  *           +-------------------------------------------------------+
  *           |                     Pin assignment                    |
  *           +-------------------------+---------------+-------------+
  *           | STM32 SD SPI Pins       |     SD        |    Pin      |
  *           +-------------------------+---------------+-------------+
  *           | SD_CS_PIN               |   CD DAT3     |    1        |
  *           | SD_SPI_MOSI_PIN         |   CMD         |    2        |
  *           |                         |   GND         |    3 (0 V)  |
  *           |                         |   VDD         |    4 (3.3 V)|
  *           | SD_SPI_SCK_PIN          |   CLK         |    5        |
  *           |                         |   GND         |    6 (0 V)  |
  *           | SD_SPI_MISO_PIN         |   DAT0        |    7        |
  *           |                         |   DAT1        |    8        |
  *           |                         |   DAT2        |    9        |
  *           | SD_DETECT_PIN           |   CD          |    10       |
  *           |                         |   GND         |    11 (0 V) |
  *           |                         |   WP          |    12       |
  *           +-------------------------+---------------+-------------+
  * @{
  */
//#define SD_SPI_NAME                     SD_SPI_HW                               ///< ��������ʹ��һ��SPI, SPI��SCK, MISO, MOSI����"drv_spi.h"������
//
//#define SD_CS_PIN                       GPIO_Pin_12                             ///< SD��Pin1, Ƭѡ�źŵ�PIN��
//#define SD_CS_GPIO_PORT                 GPIOB                                   ///< SD��Pin1, Ƭѡ�źŵ�PORT��
//#define SD_CS_GPIO_CLK                  RCC_APB2Periph_GPIOB                    ///< SD��Pin1, Ƭѡ�źŵ�ʱ��ģ��
//
//#define SD_DETECT_PIN                   GPIO_Pin_4                              ///< SD��Pin10����, SD�����ڼ��. PIN��
//#define SD_DETECT_GPIO_PORT             GPIOA                                   ///< SD��Pin10����, SD�����ڼ��. PORT��
//#define SD_DETECT_GPIO_CLK              RCC_APB2Periph_GPIOA                    ///< SD��Pin10����, SD�����ڼ��. ʱ��ģ��
//
//#define SD_SPI_MULTI                    1                                       ///< SD��SPI�Ƿ���, 1, ����. 0, ������
/**
  * @}
  */

/**
  * @}
  */



/** @defgroup DRV_SD_Public_TypeDefine
  * @brief    �������Ͷ���
  * @{
  */
/// SD�����Ͷ���
typedef enum
{
  UNKNOW = (0x00),                                                              ///< ��������
  SD_V1  = (0x01),                                                              ///< V1.x�汾��SD��
  SD_V2  = (0x02),                                                              ///< V2.0�汾��SD��
  SDHC   = (0x03),                                                              ///< SDHC��
  MMC    = (0x04),                                                              ///< MMC��
} SdType_t;

/// SD����Ϣ���Ͷ���
typedef struct
{
  SdType_t type;                                                                ///< SD������
  uint8_t  manufacturer;                                                        ///< �������̱��, ��SD����֯����
  uint8_t  oem[3];                                                              ///< OEM��ʾ, ��SD����֯����
  uint8_t  product[6];                                                          ///< ������������
  uint8_t  revision;                                                            ///< SD���汾, ʹ��BCD��. ��0x32��ʾ3.2
  uint32_t serial;                                                              ///< ���к�
  uint8_t  manufacturing_year;                                                  ///< �������, 0��ʾ2000��, ������
  uint8_t  manufacturing_month;                                                 ///< �����·�
  uint64_t capacity;                                                            ///< SD������, ��λΪ�ֽ�
} SdInfo_t;
/**
  * @}
  */

/** @defgroup DRV_SD_Public_MacroDefine
  * @brief    ���к궨��
  * @{
  */
#define SD_BLOCK_SIZE                   512                                     ///< SD�����С, ����SD���ɼ���512�Ŀ��С
/**
  * @}
  */

/** @defgroup DRV_SD_Public_Variable
  * @brief    ��������ȫ�ֱ���
  * @{
  */

/**
  * @}
  */

/** @defgroup DRV_SD_Public_Function
  * @brief    ���幫�к���
  * @{
  */
void SdSpiPortInit(void);                                                       // SD����SPI��IO�ڳ�ʼ������
void SdSpiPortDeInit(void);                                                     // SD����SPI��IO�ڽ��ú���

uint8_t SdInit(void);                                                           // SD����ʼ��. 0, �ɹ�. 1, ʧ��
uint8_t SdPresent(void);                                                        // ���SD���Ƿ����. 0, ������. 1, ����
uint8_t SdGetInfo(SdInfo_t *info);                                              // ��ȡSD����Ϣ. 0, �ɹ�. 1, ʧ��

uint8_t SdReadBlock(uint8_t* pBuffer, uint32_t sector);                                   // SD����������
uint8_t SdReadMultiBlocks(uint8_t* pBuffer, uint32_t sector, uint32_t blockNum);          // SD��д������
uint8_t SdWriteBlock(const uint8_t* pBuffer, uint32_t sector);                            // SD���������
uint8_t SdWriteMultiBlocks(const uint8_t* pBuffer, uint32_t sector, uint32_t blockNum);   // SD��д�����
/**
  * @}
  */



/**
  * @}
  */
/**
  * @}
  */

#endif
/* END OF FILE ---------------------------------------------------------------*/
