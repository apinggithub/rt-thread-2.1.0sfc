/**
  ******************************************************************************
  * @file    drv_spi_msd.c
  * @author  mousie-yu
  * @version V1.1.1
  * @date    2011.11.22
  * @brief   SD����������, ʹ��SPIģʽ. ֧��MMC��(δ��), SD��, SDHC��
  *          V1.1.1 �޸��˵ȴ�Ӧ�����ʱ, �Ӷ������˴�����SD_V2��
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "drv_spi_msd.h"

/** @addtogroup Drivers
  * @{
  */
/** @defgroup DRV_SD
  * @{
  */



/**
  ******************************************************************************
  * @addtogroup DRV_SD_Configure
  ******************************************************************************
  * @{
  */
#define SdSpiInitLowSpeed()             SpiInit(SD_SPI_NAME, SPI_TYPE_HIGH_EDGE2_MSB, SPI_BaudRatePrescaler_128)   ///< SD��SPI��ʼ������, ������. ����С��400KHz, ��дSD��ָ����.
#define SdSpiInitHighSpeed()            SpiInit(SD_SPI_NAME, SPI_TYPE_HIGH_EDGE2_MSB, SPI_BaudRatePrescaler_2)     ///< SD��SPI��ʼ������, ������. ��дSD��������.
#define SdSpiDeInit()                   SpiDeInit(SD_SPI_NAME)                            ///< SD��SPI���ú����궨��
#define SdSpiTxRxByte(data)             SpiTxRxByte(SD_SPI_NAME, data)                    ///< SD��SPI�շ������궨��

/// SD Ƭѡ�ź����ų�ʼ��, IO������Ϊ�������. SD ���ڼ���źų�ʼ��, IO������Ϊ��������
static __INLINE void SdPortInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  __SPI3_CLK_ENABLE();
  
    /**SPI3 GPIO Configuration    
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/// SD Ƭѡ�ź����źʹ��ڼ���źŽ���
static __INLINE void SdPortDeInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);

  /*Configure GPIO pins : PD2 PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;//GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/// SD Ƭѡ�ź�����ʹ��, ��IO�ڵ�ƽ�õ�
static __INLINE void SdCsEnable(void)
{
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

/// SD Ƭѡ�ź����Ž�ֹ, ��IO�ڵ�ƽ�ø�
static __INLINE void SdCsDisable(void)
{
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
}

/// ��ȡSD�����IO�ڵ�ǰ��ƽֵ. 0, �͵�ƽ. !0, �ߵ�ƽ
static __INLINE uint32_t SD_DETECT_PORT_LEVEL(void)
{
  return (GPIOD->IDR & GPIO_PIN_3);
}
/**
  * @}
  */



/** @defgroup DRV_SD_Private_TypeDefine
  * @brief    ˽�����Ͷ���
  * @{
  */

/**
  * @}
  */

/** @defgroup DRV_SD_Private_MacroDefine
  * @brief    ˽�к궨��
  * @{
  */
#define CMD0                            0                                       ///< ����0 , ����λ
#define CMD1                            1                                       ///< ����1 , CMD_SEND_OP_COND
#define CMD8                            8                                       ///< ����8 , CMD_SEND_IF_COND
#define CMD9                            9                                       ///< ����9 , ��CSD����
#define CMD10                           10                                      ///< ����10, ��CID����
#define CMD12                           12                                      ///< ����12, ֹͣ���ݴ���
#define CMD16                           16                                      ///< ����16, ����SectorSize Ӧ����0x00
#define CMD17                           17                                      ///< ����17, ��sector
#define CMD18                           18                                      ///< ����18, ��Multi sector
#define ACMD23                          23                                      ///< ����23, ���ö�sectorд��ǰԤ�Ȳ���N��block
#define CMD24                           24                                      ///< ����24, дsector
#define CMD25                           25                                      ///< ����25, дMulti sector
#define ACMD41                          41                                      ///< ����41, Ӧ����0x00
#define CMD55                           55                                      ///< ����55, Ӧ����0x01
#define CMD58                           58                                      ///< ����58, ��OCR��Ϣ
/**
  * @}
  */

/** @defgroup DRV_SD_Variables
  * @brief    ����ȫ�ֱ���(˽��/����)
  * @{
  */
static SdType_t sdType;                                                         ///< SD������
/**
  * @}
  */

/** @defgroup DRV_SD_Private_Function
  * @brief    ����˽�к���
  * @{
  */
static uint8_t SdSendCmd(uint8_t Cmd, uint32_t Arg);                            // SDָ��ͺ���
/**
  * @}
  */



/** @defgroup DRV_SD_Function
  * @brief    ����ԭ�ļ�
  * @{
  */

/**
  ******************************************************************************
  * @brief  SD����SPIͨѶ����Ӧ�Ķ˿ڳ�ʼ������
  * @param  None
  * @retval None
  ******************************************************************************
  */
void SdSpiPortInit(void)
{
  SdSpiInitLowSpeed();                                                          // �����ȳ�ʼ��SS����Ϊ�������
  SdPortInit();                                                                 // �ٳ�ʼ��SS��GPIO�ڲ���Ч
  SdCsDisable();
}

/**
  ******************************************************************************
  * @brief  ����SD��, SPI����. ������Ϊ��������
  * @param  None
  * @retval None
  ******************************************************************************
  */
void SdSpiPortDeInit(void)
{
  SdSpiDeInit();
  SdPortDeInit();
}

/**
  ******************************************************************************
  * @brief  SD����ʼ��
  * @param  None
  * @retval ����ֵ, 0, �ɹ�. 1, ʧ��
  ******************************************************************************
  */
uint8_t SdInit(void)
{
  uint16_t i;
  uint8_t response;
  uint8_t rvalue = 1;

  sdType = UNKNOW;
  if (!SdPresent()) return 1;                                                   // SD��������, ֱ���˳�

  SdSpiInitLowSpeed();                                                          // ʹ��SPI, ��Ϊ����ģʽ�����SD��ʶ����
  for (i=10; i>0; i--) SdSpiTxRxByte(0xFF);                                     // �Ȳ���>74�����壬��SD���Լ���ʼ�����

  SdCsEnable();                                                                 // SD��Ƭѡ�ź�ʹ��
  for (i=10000; i>0; i--)
  {
    if (SdSendCmd(CMD0, 0) == 0x01) break;                                      // CMD0, ��SD���������ģʽ
  }
  if (i)                                                                        // SD���ɹ��������ģʽ
  {
    // CMD8ָ����Ӧ��ʶ��SD1.x��SD2.0�淶�Ĺؼ�
    response = SdSendCmd(8, 0x1AA);

    switch (response)
    {
      case 0x05:                                                                // CMD8Ӧ��0x05, ����֧��CMD8ָ��, ��SD1.x����MMC��
      {
        SdSpiTxRxByte(0xFF);                                                    // ��8��CLK, ��SD����������

        // ����CMD55��ACMD41ָ��, ���ڼ����SD������MMC��, MMC����ACMD41��û��Ӧ���
        for (i=4000; i>0; i--)
        {
          response = SdSendCmd(CMD55, 0);
          if (response != 0x01) goto EXIT;                                      // Ӧ�����, ֱ���˳�
          response = SdSendCmd(ACMD41, 0);
          if (response == 0x00) break;                                          // ��ȷӦ��, ����ѭ��
        }
        if (i)
        {
          sdType = SD_V1;                                                       // ��ȷӦ��, ��SD��V1.x
          rvalue = 0;                                                           // SD����ʼ���ɹ����
        }
        else                                                                    // ��Ӧ��, ������MMC��
        {
          for (i=4000; i>0; i--)
          {
            response = SdSendCmd(CMD1, 0);
            if (response == 0x00) break;
          }
          if (i)                                                                // MMC����ʼ���ɹ�
          {
            sdType = MMC;                                                       // ��MMC��
            rvalue = 0;                                                         // SD����ʼ���ɹ����
          }
        }

        if(SdSendCmd(CMD16, 512) != 0x00)                                       // ����SD�����С
        {
          sdType = UNKNOW;
          rvalue = 1;
        }
      } break;

      case 0x01:                                                                // Ӧ��0x01, ��V2.0��SD��
      {
        SdSpiTxRxByte(0xFF);                                                    // V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ�Ӧ������
        SdSpiTxRxByte(0xFF);
        i   = SdSpiTxRxByte(0xFF);
        i <<= 8;
        i  |= SdSpiTxRxByte(0xFF);                                              // 4���ֽ�Ӧ����0x000001AA, ��ʾ�ÿ��Ƿ�֧��2.7V-3.6V�ĵ�ѹ��Χ

        if(i == 0x1AA)                                                          // SD��֧��2.7V-3.6V, ���Բ���
        {
          for (i=4000; i>0; i--)
          {
            response = SdSendCmd(CMD55, 0);
            if (response != 0x01) goto EXIT;                                    // Ӧ�����, ֱ���˳�
            response = SdSendCmd(ACMD41, 0x40000000);                           // SD V2.0Ҫʹ�� 0x40000000
            if (response == 0x00) break;                                        // ��ȷӦ��, ����ѭ��
          }
          if (i)                                                                // CMD41��ȷӦ��
          {
            // ͨ����ȡOCR��Ϣ, ʶ������ͨ�� SD V2.0 ���� SDHC ��
            if (SdSendCmd(CMD58, 0) == 0x00)
            {
              i = SdSpiTxRxByte(0xFF);                                          // Ӧ���4�ֽ�OCR��Ϣ
              SdSpiTxRxByte(0xFF);
              SdSpiTxRxByte(0xFF);
              SdSpiTxRxByte(0xFF);

              if (i & 0x40) sdType = SDHC;                                      // ͨ�����CCSλȷ��SD������
              else          sdType = SD_V2;
              rvalue = 0;                                                       // SD����ʼ���ɹ����
            }
          }
        }
      } break;
    }
  }

EXIT:
  SdCsDisable();
  SdSpiTxRxByte(0xFF);                                                          // 8��CLK, ��SD������������
  SdSpiInitHighSpeed();

  return rvalue;
}

/**
  ******************************************************************************
 * @brief  ���SD���Ƿ����
 * @param  None
 * @retval ����ֵ, 0, ������. 1, ����
 ******************************************************************************
 */
uint8_t SdPresent(void)
{
  if (SD_DETECT_PORT_LEVEL())
  {
    sdType = UNKNOW;
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
  ******************************************************************************
  * @brief  ��ȡSD����Ϣ
  * @param  info,   SD����Ϣָ��
  * @retval ����ֵ, 0, �ɹ�. 1, ʧ��
  ******************************************************************************
  */
uint8_t SdGetInfo(SdInfo_t *info)
{
  uint16_t i;
  uint8_t rvalue = 1;
  uint8_t data;
  uint8_t csd_read_bl_len = 0;
  uint8_t csd_c_size_mult = 0;
  uint32_t csd_c_size = 0;

  assert_param(info != NULL);
  if((info == NULL) || (sdType == UNKNOW)) return rvalue;                       // ��ָ�� �� sd��δ��ʼ��, ֱ�ӷ���

#if (SD_SPI_MULTI == 1)
  SdSpiInitHighSpeed();
#endif

  SdCsEnable();
  if (SdSendCmd(CMD10, 0) == 0x00)                                              // ��ȡCID�Ĵ���
  {
    for (i=4000; i>0; i--)
    {
      if (SdSpiTxRxByte(0xFF) == 0xFE) break;                                   // ׼���ô�������
    }
    if (i)
    {
      rvalue = 0;                                                               // ��ȡCID�Ĵ����ɹ�
      for (i=0; i<18; i++)                                                      // 16������, 2��CRCУ��
      {
        data = SdSpiTxRxByte(0xFF);                                             // ����CID����
        switch(i)
        {
          case 0:
              info->manufacturer = data;
              break;
          case 1:
          case 2:
              info->oem[i - 1] = data;
              break;
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
              info->product[i - 3] = data;
              break;
          case 8:
              info->revision = data;
              break;
          case 9:
          case 10:
          case 11:
          case 12:
              info->serial |= (uint32_t)data << ((12 - i) * 8);
              break;
          case 13:
              info->manufacturing_year = data << 4;
              break;
          case 14:
              info->manufacturing_year |= data >> 4;
              info->manufacturing_month = data & 0x0f;
              break;
        }
      }
    }
  }
  if (rvalue) return rvalue;

  rvalue = 1;
  if (SdSendCmd(CMD9, 0) == 0x00)                                               // ��ȡCSD�Ĵ���
  {
    for (i=4000; i>0; i--)
    {
      if (SdSpiTxRxByte(0xFF) == 0xFE) break;                                   // ׼���ô�������
    }
    if (i)
    {
      rvalue = 0;                                                               // ��ȡCID�Ĵ����ɹ�
      for (i=0; i<18; i++)                                                      // 16������, 2��CRCУ��
      {
        data = SdSpiTxRxByte(0xFF);

        if (sdType == SDHC)                                                     // SDHC��
        {
          switch(i)
          {
            case 7:
              csd_c_size = (data & 0x3f);
              csd_c_size <<= 8;
              break;
            case 8:
              csd_c_size |= data;
              csd_c_size <<= 8;
              break;
            case 9:
              csd_c_size |= data;
              csd_c_size++;
              info->capacity = csd_c_size;
              info->capacity <<= 19;                                            // *= (512 * 1025)
              break;
          }
        }
        else                                                                    // ��ͨSD�� �� MMC ��
        {
          switch(i)
          {
            case 5:
              csd_read_bl_len = data & 0x0f;
              break;
            case 6:
              csd_c_size = data & 0x03;
              csd_c_size <<= 8;
              break;
            case 7:
              csd_c_size |= data;
              csd_c_size <<= 2;
              break;
            case 8:
              csd_c_size |= data >> 6;
              ++csd_c_size;
              break;
            case 9:
              csd_c_size_mult = data & 0x03;
              csd_c_size_mult <<= 1;
              break;
            case 10:
              csd_c_size_mult |= data >> 7;
              info->capacity = csd_c_size;
              info->capacity <<= (csd_c_size_mult + csd_read_bl_len + 2);
              break;
          }
        }
      }
    }
  }

  info->type = sdType;

  return rvalue;
}

/**
  ******************************************************************************
  * @brief  ��ȡSD����һ����,
  * @param  pBuffer, ���ݴ洢ָ��
  * @param  sector,  ��ȡ������, �������ַ. ���СĬ��Ϊ512byte
  * @retval ����ֵ,  0, �ɹ�. 1, ʧ��
  ******************************************************************************
  */
uint8_t SdReadBlock(uint8_t* pBuffer, uint32_t sector)
{
  uint16_t i;
  uint8_t  response;
  uint8_t  rvalue = 1;

  if ((sdType == UNKNOW) || (pBuffer == 0)) return rvalue;
  if (sdType != SDHC) sector <<= 9;                                             // ����SDHC, ��Ҫ�����ַתΪ�ֽڵ�ַ, sector *= 512

#if (SD_SPI_MULTI == 1)
  SdSpiInitHighSpeed();
#endif

  SdCsEnable();
  response = SdSendCmd(CMD17, sector);                                          // ���� CMD17, ��������
  if (response == 0x00)                                                         // Ӧ����ȷ
  {
    for (i=10000; i>0; i--)
    {
      if (SdSpiTxRxByte(0xFF) == 0xFE) break;                                   // ׼���ô�������
    }
    if (i)
    {
      rvalue = 0;
      for (i=512; i>0; i--) *pBuffer++ = SdSpiTxRxByte(0xFF);                   // ��һ����
      SdSpiTxRxByte(0xFF);                                                      // ��CRCУ��
      SdSpiTxRxByte(0xFF);
    }
  }
  SdCsDisable();
  SdSpiTxRxByte(0xFF);

  return rvalue;
}

/**
  ******************************************************************************
  * @brief  ��ȡSD���Ķ����,
  * @param  pBuffer,  ��ȡ��ַָ��
  * @param  sector,   ��ȡ������, �������ַ. ���СĬ��Ϊ512byte
  * @param  blockNum, ��Ҫ��ȡ��SD������
  * @retval ����ֵ,         0, �ɹ�. 1, ʧ��
  ******************************************************************************
  */
uint8_t SdReadMultiBlocks(uint8_t* pBuffer, uint32_t sector, uint32_t blockNum)
{
  uint32_t i;
  uint8_t  response;
  uint8_t  rvalue = 1;

  if ((sdType == UNKNOW) || (pBuffer == 0) || (blockNum == 0)) return rvalue;
  if (sdType != SDHC) sector <<= 9;                                             // ����SDHC, ��Ҫ�����ַתΪ�ֽڵ�ַ, sector *= 512

#if (SD_SPI_MULTI == 1)
  SdSpiInitHighSpeed();
#endif

  SdCsEnable();
  response = SdSendCmd(CMD18, sector);                                          // ���� CMD18, �������
  if (response == 0x00)                                                         // Ӧ����ȷ
  {
    rvalue = 0;
    while (blockNum--)
    {
      for (i=10000; i>0; i--)
      {
        if (SdSpiTxRxByte(0xFF) == 0xFE) break;                                 // ׼���ô�������
      }
      if (i)
      {
        for (i=512; i>0; i--) *pBuffer++ = SdSpiTxRxByte(0xFF);                 // �������
        SdSpiTxRxByte(0xFF);                                                    // ��CRCУ��
        SdSpiTxRxByte(0xFF);
      }
      else
      {
        rvalue = 1;
      }
    }
  }
  SdSendCmd(CMD12, 0);                                                          // ����ֹͣ����
  SdCsDisable();
  SdSpiTxRxByte(0xFF);

  return rvalue;
}

/**
  ******************************************************************************
  * @brief  д��SD����һ����
  * @param  pBuffer, д������ָ��
  * @param  sector,  д�������, �������ַ. ���СĬ��Ϊ512byte
  * @retval ����ֵ,  0, �ɹ�. 1, ʧ��
  ******************************************************************************
  */
uint8_t SdWriteBlock(const uint8_t* pBuffer, uint32_t sector)
{
  uint32_t i;
  uint8_t  response;
  uint8_t  rvalue = 1;

  if ((sdType == UNKNOW) || (pBuffer == 0)) return rvalue;
  if (sdType != SDHC) sector <<= 9;                                             // ����SDHC, ��Ҫ�����ַתΪ�ֽڵ�ַ, sector *= 512

#if (SD_SPI_MULTI == 1)
  SdSpiInitHighSpeed();
#endif

  SdCsEnable();
  response = SdSendCmd(CMD24, sector);                                          // ���� CMD24, д������
  if (response == 0x00)                                                         // Ӧ����ȷ
  {
    SdSpiTxRxByte(0xFF);                                                        // ����CLK, �ȴ�SD��׼����
    SdSpiTxRxByte(0xFE);                                                        // ��ʼ����0xFE
    for (i = 512; i>0 ; i--) SdSpiTxRxByte(*pBuffer++);                         // дһ����
    SdSpiTxRxByte(0xFF);                                                        // ��CRCУ��
    SdSpiTxRxByte(0xFF);

    if ((SdSpiTxRxByte(0xFF) & 0x1F) == 0x05)                                   // ������Ӧ��
    {
      for (i=100000; i>0; i--)                                                  // д����Ҫʱ��, V1������, ��ʱ����̫С.
      {
        if (SdSpiTxRxByte(0xFF) == 0xFF)                                        // �ȴ�����д�����
        {
          rvalue = 0;                                                           // ���Ϊд�ɹ�
          break;
        }
      }
    }
  }
  SdCsDisable();
  SdSpiTxRxByte(0xFF);

  return rvalue;
}

/**
  ******************************************************************************
  * @brief  д��SD���Ķ����
  * @param  pBuffer,  д���ַָ��
  * @param  sector,   д�������, �������ַ. ���СĬ��Ϊ512byte
  * @param  blockNum, ��Ҫд���SD������
  * @retval ����ֵ,  0, �ɹ�. 1, ʧ��
  ******************************************************************************
  */
uint8_t  SdWriteMultiBlocks(const uint8_t* pBuffer, uint32_t sector, uint32_t blockNum)
{
  uint32_t i;
  uint8_t  response;
  uint8_t  rvalue = 1;

  if ((sdType == UNKNOW) || (pBuffer == 0) || (blockNum == 0)) return rvalue;
  if (sdType != SDHC) sector <<= 9;                                             // ����SDHC, ��Ҫ�����ַתΪ�ֽڵ�ַ, sector *= 512

#if (SD_SPI_MULTI == 1)
  SdSpiInitHighSpeed();
#endif

  SdCsEnable();
  if(sdType != MMC) SdSendCmd(ACMD23, sector);                                  // ʹ��ACMD23Ԥ����SD������
  response = SdSendCmd(CMD25, sector);                                          // ���� CMD25, д�����
  if (response == 0x00)                                                         // Ӧ����ȷ
  {
    rvalue = 0;
    for (; blockNum>0; blockNum--)
    {
      SdSpiTxRxByte(0xFF);                                                      // ����CLK, �ȴ�SD��׼����
      SdSpiTxRxByte(0xFC);                                                      // ����ʼ����0xFC, �����Ƕ��д��

      for(i=512; i>0; i--) SdSpiTxRxByte(*pBuffer++);                           // д��һ����
      SdSpiTxRxByte(0xFF);                                                      // ��ȡCRC��Ϣ
      SdSpiTxRxByte(0xFF);

      if ((SdSpiTxRxByte(0xFF) & 0x1F) == 0x05)                                 // ��ȡSD��Ӧ����Ϣ
      {
        for (i=100000; i>0; i--)                                                // д����Ҫʱ��, V1������, ��ʱ����̫С.
        {
          if (SdSpiTxRxByte(0xFF) == 0xFF) break;                               // �ȴ�����д�����
        }
      }
      else
      {
        i = 0;
      }
      if (i == 0) rvalue = 1;                                                   // ��һ����д�����, �ͱ��Ϊ����
    }
  }
  SdSpiTxRxByte(0xFD);                                                          // ����SD�����д
  for (i=100000; i>0; i--)                                                      // �ȴ����д�����
  {
    if (SdSpiTxRxByte(0xFF) == 0xFF) break;
  }
  SdCsDisable();
  SdSpiTxRxByte(0xFF);

  return rvalue;
}

/**
  ******************************************************************************
  * @brief  ����5�ֽڵ�SD��ָ��
  * @param  Cmd, Ҫ���͵�ָ��
  * @param  Arg, ָ�����
  * @retval ����ֵ, ָ��Ӧ��
  ******************************************************************************
  */
uint8_t SdSendCmd(uint8_t cmd, uint32_t arg)
{
  uint8_t response;
  ubase_t i;

  SdSpiTxRxByte(0xFF);

  SdSpiTxRxByte(0x40 | cmd);                                                    // ����ָ��
  SdSpiTxRxByte(arg >> 24);
  SdSpiTxRxByte(arg >> 16);
  SdSpiTxRxByte(arg >> 8);
  SdSpiTxRxByte(arg >> 0);
  switch(cmd)
  {
    case CMD0:
      SdSpiTxRxByte(0x95);                                                      // CMD0��У��Ϊ0x95
      break;
    case CMD8:
      SdSpiTxRxByte(0x87);                                                      // CMD8��У��Ϊ0x87
      break;
    default:
      SdSpiTxRxByte(0xff);                                                      // ����ָ����SPIģʽ������У��
      break;
  }

  for (i=10; i>0; i--)                                                          // ����Ӧ��
  {
    response = SdSpiTxRxByte(0xFF);
    if(response != 0xFF) break;
  }

  return response;
}
/**
  * @}
  */



/**
  * @}
  */
/**
  * @}
  */

/* END OF FILE ---------------------------------------------------------------*/
