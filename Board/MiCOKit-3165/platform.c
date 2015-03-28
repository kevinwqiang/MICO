/**
******************************************************************************
* @file    platform.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provides all MICO Peripherals mapping table and platform
*          specific funcgtions.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 

#include "stdio.h"
#include "string.h"

#include "MICOPlatform.h"
#include "platform.h"
#include "stm32f4xx_platform.h"
#include "platform_common_config.h"
#include "PlatformLogging.h"

/******************************************************
*                      Macros
******************************************************/

#ifdef __GNUC__
#define WEAK __attribute__ ((weak))
#elif defined ( __IAR_SYSTEMS_ICC__ )
#define WEAK __weak
#endif /* ifdef __GNUC__ */

/******************************************************
*                    Constants
******************************************************/

/******************************************************
*                   Enumerations
******************************************************/

/******************************************************
*                 Type Definitions
******************************************************/

/******************************************************
*                    Structures
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/
extern WEAK void PlatformEasyLinkButtonClickedCallback(void);
extern WEAK void PlatformStandbyButtonClickedCallback(void);
extern WEAK void PlatformEasyLinkButtonLongPressedCallback(void);
extern WEAK void bootloader_start(void);

/******************************************************
*               Variables Definitions
******************************************************/

/* This table maps STM32 pins to GPIO definitions on the schematic
* A full pin definition is provided in <WICED-SDK>/include/platforms/BCM943362WCD4/platform.h
*/

static uint32_t _default_start_time = 0;
static mico_timer_t _button_EL_timer;

const platform_pin_mapping_t gpio_mapping[] =
{
  /* Common GPIOs for internal use */
  [WL_GPIO1]                          = {GPIOA,  0,  RCC_AHB1Periph_GPIOA},
  [WL_RESET]                          = {GPIOB,  14, RCC_AHB1Periph_GPIOB},
  [MICO_SYS_LED]                      = {GPIOB,  10, RCC_AHB1Periph_GPIOB}, 
  [MICO_RF_LED]                       = {GPIOA,  4,  RCC_AHB1Periph_GPIOA}, 
  [BOOT_SEL]                          = {GPIOB,  1,  RCC_AHB1Periph_GPIOB}, 
  [MFG_SEL]                           = {GPIOB,  0,  RCC_AHB1Periph_GPIOB}, 
  [EasyLink_BUTTON]                   = {GPIOA,  1,  RCC_AHB1Periph_GPIOA}, 
  [STDIO_UART_RX]                     = {GPIOA,  3,  RCC_AHB1Periph_GPIOA},  
  [STDIO_UART_TX]                     = {GPIOA,  2,  RCC_AHB1Periph_GPIOA},  

  /* GPIOs for external use */
  [MICO_GPIO_2]  = {GPIOB,  2,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_8]  = {GPIOA , 2,  RCC_AHB1Periph_GPIOA},
  [MICO_GPIO_9]  = {GPIOA,  1,  RCC_AHB1Periph_GPIOA},
  [MICO_GPIO_12] = {GPIOA,  3,  RCC_AHB1Periph_GPIOA},
  [MICO_GPIO_16] = {GPIOC, 13,  RCC_AHB1Periph_GPIOC},
  [MICO_GPIO_17] = {GPIOB, 10,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_18] = {GPIOB,  9,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_19] = {GPIOB, 12,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_27] = {GPIOA, 12,  RCC_AHB1Periph_GPIOA},  
  [MICO_GPIO_29] = {GPIOA, 10,  RCC_AHB1Periph_GPIOA},
  [MICO_GPIO_30] = {GPIOB,  6,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_31] = {GPIOB,  8,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_33] = {GPIOB, 13,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_34] = {GPIOA,  5,  RCC_AHB1Periph_GPIOA},
  [MICO_GPIO_35] = {GPIOA, 10,  RCC_AHB1Periph_GPIOA},
  [MICO_GPIO_36] = {GPIOB,  1,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_37] = {GPIOB,  0,  RCC_AHB1Periph_GPIOB},
  [MICO_GPIO_38] = {GPIOA,  4,  RCC_AHB1Periph_GPIOA},
};

/*
* Possible compile time inputs:
* - Set which ADC peripheral to use for each ADC. All on one ADC allows sequential conversion on all inputs. All on separate ADCs allows concurrent conversion.
*/
/* TODO : These need fixing */
const platform_adc_mapping_t adc_mapping[] =
{
  [MICO_ADC_1] = NULL,
 // [MICO_ADC_1] = {ADC1, ADC_Channel_1, RCC_APB2Periph_ADC1, 1, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_2]},
 // [MICO_ADC_2] = {ADC1, ADC_Channel_2, RCC_APB2Periph_ADC1, 1, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_4]},
 // [MICO_ADC_3] = {ADC1, ADC_Channel_3, RCC_APB2Periph_ADC1, 1, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_5]},
};


/* PWM mappings */
const platform_pwm_mapping_t pwm_mappings[] =
{
#if ( MICO_WLAN_POWERSAVE_CLOCK_SOURCE == MICO_WLAN_POWERSAVE_CLOCK_IS_PWM )
  /* Extended PWM for internal use */
  [MICO_PWM_WLAN_POWERSAVE_CLOCK] = {TIM1, 1, RCC_APB2Periph_TIM1, GPIO_AF_TIM1, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_WLAN_POWERSAVE_CLOCK] }, /* or TIM2/Ch2                       */
#endif
  [MICO_PWM_1] = NULL,
  //[MICO_PWM_1]  = {TIM4, 3, RCC_APB1Periph_TIM4, GPIO_AF_TIM4, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_10]},    /* or TIM10/Ch1                       */
  //[MICO_PWM_2]  = {TIM12, 1, RCC_APB1Periph_TIM12, GPIO_AF_TIM12, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_13]}, /* or TIM1/Ch2N                       */
  //[MICO_PWM_3]  = {TIM2, 4, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, (platform_pin_mapping_t*)&gpio_mapping[MICO_GPIO_19]},    
  /* TODO: fill in the other options here ... */
};

const platform_spi_mapping_t spi_mapping[] =
{
  [MICO_SPI_1]  = NULL,
  // [MICO_SPI_1]  =
  // {
  //   .spi_regs              = SPI1,
  //   .gpio_af               = GPIO_AF_SPI1,
  //   .peripheral_clock_reg  = RCC_APB2Periph_SPI1,
  //   .peripheral_clock_func = RCC_APB2PeriphClockCmd,
  //   .pin_mosi              = &gpio_mapping[MICO_GPIO_8],
  //   .pin_miso              = &gpio_mapping[MICO_GPIO_7],
  //   .pin_clock             = &gpio_mapping[MICO_GPIO_6],
  //   .tx_dma_stream         = DMA2_Stream5,
  //   .rx_dma_stream         = DMA2_Stream0,
  //   .tx_dma_channel        = DMA_Channel_3,
  //   .rx_dma_channel        = DMA_Channel_3,
  //   .tx_dma_stream_number  = 5,
  //   .rx_dma_stream_number  = 0
  // }
};

const platform_uart_mapping_t uart_mapping[] =
{
  [MICO_UART_1] =
  {
    .usart                        = USART2,
    .gpio_af                      = GPIO_AF_USART2,
    .pin_tx                       = &gpio_mapping[STDIO_UART_TX],
    .pin_rx                       = &gpio_mapping[STDIO_UART_RX],
    .pin_cts                      = NULL,
    .pin_rts                      = NULL,
    .usart_peripheral_clock       = RCC_APB1Periph_USART2,
    .usart_peripheral_clock_func  = RCC_APB1PeriphClockCmd,
    .usart_irq                    = USART2_IRQn,  
    .tx_dma                       = DMA1,
    .tx_dma_stream                = DMA1_Stream6,
    .tx_dma_stream_number         = 6,
    .tx_dma_channel               = DMA_Channel_4,
    .tx_dma_peripheral_clock      = RCC_AHB1Periph_DMA1,
    .tx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
    .tx_dma_irq                   = DMA1_Stream6_IRQn,
    .rx_dma                       = DMA1,
    .rx_dma_stream                = DMA1_Stream5,
    .rx_dma_stream_number         = 5,
    .rx_dma_channel               = DMA_Channel_4,
    .rx_dma_peripheral_clock      = RCC_AHB1Periph_DMA1,
    .rx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
    .rx_dma_irq                   = DMA1_Stream5_IRQn,
  },
  [MICO_UART_2] =
  {
    .usart                        = USART1,
    .gpio_af                      = GPIO_AF_USART1,
    .pin_tx                       = &gpio_mapping[MICO_GPIO_30],
    .pin_rx                       = &gpio_mapping[MICO_GPIO_29],
    .pin_cts                      = NULL,
    .pin_rts                      = NULL,
    .usart_peripheral_clock       = RCC_APB2Periph_USART1,
    .usart_peripheral_clock_func  = RCC_APB2PeriphClockCmd,
    .usart_irq                    = USART1_IRQn,
    .tx_dma                       = DMA2,
    .tx_dma_stream                = DMA2_Stream7,
    .tx_dma_channel               = DMA_Channel_4,
    .tx_dma_peripheral_clock      = RCC_AHB1Periph_DMA2,
    .tx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
    .tx_dma_irq                   = DMA2_Stream7_IRQn,
    .rx_dma                       = DMA2,
    .rx_dma_stream                = DMA2_Stream2,
    .rx_dma_channel               = DMA_Channel_4,
    .rx_dma_peripheral_clock      = RCC_AHB1Periph_DMA2,
    .rx_dma_peripheral_clock_func = RCC_AHB1PeriphClockCmd,
    .rx_dma_irq                   = DMA2_Stream2_IRQn,
  },
};

const platform_i2c_mapping_t i2c_mapping[] =
{
  [MICO_I2C_1] = NULL,
  // [MICO_I2C_1] =
  // {
  //   .i2c = I2C1,
  //   .pin_scl                 = &gpio_mapping[MICO_GPIO_1],
  //   .pin_sda                 = &gpio_mapping[MICO_GPIO_2],
  //   .peripheral_clock_reg    = RCC_APB1Periph_I2C1,
  //   .tx_dma                  = DMA1,
  //   .tx_dma_peripheral_clock = RCC_AHB1Periph_DMA1,
  //   .tx_dma_stream           = DMA1_Stream7,
  //   .rx_dma_stream           = DMA1_Stream5,
  //   .tx_dma_stream_id        = 7,
  //   .rx_dma_stream_id        = 5,
  //   .tx_dma_channel          = DMA_Channel_1,
  //   .rx_dma_channel          = DMA_Channel_1,
  //   .gpio_af                 = GPIO_AF_I2C1
  // },
};

/******************************************************
*               Function Definitions
******************************************************/

static void _button_EL_irq_handler( void* arg )
{
  (void)(arg);
  int interval = -1;
  
  if ( MicoGpioInputGet( (mico_gpio_t)EasyLink_BUTTON ) == 0 ) {
    _default_start_time = mico_get_time()+1;
    mico_start_timer(&_button_EL_timer);
  } else {
    interval = mico_get_time() + 1 - _default_start_time;
    if ( (_default_start_time != 0) && interval > 50 && interval < RestoreDefault_TimeOut){
      /* EasyLink button clicked once */
      PlatformEasyLinkButtonClickedCallback();
    }
    mico_stop_timer(&_button_EL_timer);
    _default_start_time = 0;
  }
}

static void _button_STANDBY_irq_handler( void* arg )
{
  (void)(arg);
  PlatformStandbyButtonClickedCallback();
}

static void _button_EL_Timeout_handler( void* arg )
{
  (void)(arg);
  _default_start_time = 0;
  PlatformEasyLinkButtonLongPressedCallback();
}

bool watchdog_check_last_reset( void )
{
  if ( RCC->CSR & RCC_CSR_WDGRSTF )
  {
    /* Clear the flag and return */
    RCC->CSR |= RCC_CSR_RMVF;
    return true;
  }
  
  return false;
}

OSStatus mico_platform_init( void )
{
  platform_log( "Platform initialised" );
  
  if ( true == watchdog_check_last_reset() )
  {
    platform_log( "WARNING: Watchdog reset occured previously. Please see watchdog.c for debugging instructions." );
  }
  
  return kNoErr;
}

void init_platform( void )
{
   MicoGpioInitialize( (mico_gpio_t)MICO_SYS_LED, OUTPUT_PUSH_PULL );
   MicoGpioOutputLow( (mico_gpio_t)MICO_SYS_LED );
   MicoGpioInitialize( (mico_gpio_t)MICO_RF_LED, OUTPUT_OPEN_DRAIN_NO_PULL );
   MicoGpioOutputHigh( (mico_gpio_t)MICO_RF_LED );
  
   //  Initialise EasyLink buttons
   MicoGpioInitialize( (mico_gpio_t)EasyLink_BUTTON, INPUT_PULL_UP );
   mico_init_timer(&_button_EL_timer, RestoreDefault_TimeOut, _button_EL_Timeout_handler, NULL);
   MicoGpioEnableIRQ( (mico_gpio_t)EasyLink_BUTTON, IRQ_TRIGGER_BOTH_EDGES, _button_EL_irq_handler, NULL );
  
   //  Initialise Standby/wakeup switcher
   MicoGpioInitialize( (mico_gpio_t)Standby_SEL, INPUT_PULL_UP );
   MicoGpioEnableIRQ( (mico_gpio_t)Standby_SEL , IRQ_TRIGGER_FALLING_EDGE, _button_STANDBY_irq_handler, NULL);

   MicoFlashInitialize( MICO_SPI_FLASH );
}

#ifdef BOOTLOADER
const unsigned char CRC8Table[]={
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};
#define SizePerRW                   (4096)
static uint8_t data[SizePerRW];

uint8_t CRC8_Table(uint8_t crc8_ori, uint8_t *p, uint32_t counter)
{
    uint8_t crc8 = crc8_ori;
    for( ; counter > 0; counter--){
        crc8 = CRC8Table[crc8^*p];
        p++;
    }
    return(crc8);
}

void init_platform_bootloader( void )
{
  OSStatus err = kNoErr;

  MicoGpioInitialize( (mico_gpio_t)MICO_SYS_LED, OUTPUT_PUSH_PULL );
  MicoGpioOutputLow( (mico_gpio_t)MICO_SYS_LED );
  MicoGpioInitialize( (mico_gpio_t)MICO_RF_LED, OUTPUT_OPEN_DRAIN_NO_PULL );
  MicoGpioOutputHigh( (mico_gpio_t)MICO_RF_LED );
  
  MicoGpioInitialize((mico_gpio_t)BOOT_SEL, INPUT_PULL_UP);
  MicoGpioInitialize((mico_gpio_t)MFG_SEL, INPUT_HIGH_IMPEDANCE);

  /* Specific operations used in EMW3165 production */
#define NEED_RF_DRIVER_COPY_BASE    ((uint32_t)0x08008000)
#define TEMP_RF_DRIVER_BASE         ((uint32_t)0x08040000)
#define TEMP_RF_DRIVER_END          ((uint32_t)0x0807FFFF)
  
  const uint8_t isDriverNeedCopy = *(uint8_t *)(NEED_RF_DRIVER_COPY_BASE);
  const uint32_t totalLength = ( DRIVER_FLASH_SIZE < 0x40000)?  DRIVER_FLASH_SIZE:0x40000;
  const uint8_t crcResult = *(uint8_t *)(TEMP_RF_DRIVER_END);
  uint8_t targetCrcResult = 0;
  
  uint32_t copyLength;
  uint32_t destStartAddress_tmp = DRIVER_START_ADDRESS;
  uint32_t sourceStartAddress_tmp = TEMP_RF_DRIVER_BASE;
  uint32_t i;

  if ( isDriverNeedCopy != 0x0 )
    return;

  platform_log( "Bootloader start to copy RF driver..." );
  /* Copy RF driver to SPI flash */
  err = MicoFlashInitialize( (mico_flash_t)MICO_FLASH_FOR_DRIVER );
  require_noerr(err, exit);
  err = MicoFlashInitialize( (mico_flash_t)MICO_INTERNAL_FLASH );
  require_noerr(err, exit);
  err = MicoFlashErase( MICO_FLASH_FOR_DRIVER, DRIVER_START_ADDRESS, DRIVER_END_ADDRESS );
  require_noerr(err, exit);
  platform_log( "Time: %d", mico_get_time_no_os() );
  
  for(i = 0; i <= totalLength/SizePerRW; i++){
    if( i == totalLength/SizePerRW ){
      if(totalLength%SizePerRW)
        copyLength = totalLength%SizePerRW;
      else
        break;
    }else{
      copyLength = SizePerRW;
    }
    printf(".");
    err = MicoFlashRead( MICO_INTERNAL_FLASH, &sourceStartAddress_tmp, data , copyLength );
    require_noerr( err, exit );
    err = MicoFlashWrite( MICO_FLASH_FOR_DRIVER, &destStartAddress_tmp, data, copyLength);
    require_noerr(err, exit);
  }

  printf("\r\n");
  /* Check CRC-8 check-sum */
  platform_log( "Bootloader start to verify RF driver..." );
  sourceStartAddress_tmp = TEMP_RF_DRIVER_BASE;
  destStartAddress_tmp = DRIVER_START_ADDRESS;
  
  for(i = 0; i <= totalLength/SizePerRW; i++){
    if( i == totalLength/SizePerRW ){
      if(totalLength%SizePerRW)
        copyLength = totalLength%SizePerRW;
      else
        break;
    }else{
      copyLength = SizePerRW;
    }
    printf(".");
    err = MicoFlashRead( MICO_FLASH_FOR_DRIVER, &destStartAddress_tmp, data, copyLength );
    require_noerr( err, exit );

    targetCrcResult = CRC8_Table(targetCrcResult, data, copyLength);
  }

  printf("\r\n");
  //require_string( crcResult == targetCrcResult, exit, "Check-sum error" ); 
  if( crcResult != targetCrcResult ){
    platform_log("Check-sum error");
    while(1);
  }
  /* Clear RF driver from temperary storage */
  platform_log("Bootloader start to clear RF driver temporary storage...");

  err = MicoFlashInitialize( (mico_flash_t)MICO_INTERNAL_FLASH );
  require_noerr(err, exit);  
  
  /* Clear copy tag */
  err = MicoFlashErase(MICO_INTERNAL_FLASH, NEED_RF_DRIVER_COPY_BASE, NEED_RF_DRIVER_COPY_BASE);
  require_noerr(err, exit);

exit:
  MicoFlashFinalize( MICO_INTERNAL_FLASH );
  MicoFlashFinalize( MICO_FLASH_FOR_DRIVER );
}

#endif


void host_platform_reset_wifi( bool reset_asserted )
{
  if ( reset_asserted == true )
  {
    MicoGpioOutputLow( (mico_gpio_t)WL_RESET );  
  }
  else
  {
    MicoGpioOutputHigh( (mico_gpio_t)WL_RESET ); 
  }
}

void host_platform_power_wifi( bool power_enabled )
{
  if ( power_enabled == true )
  {
    MicoGpioOutputLow( (mico_gpio_t)WL_REG );  
  }
  else
  {
    MicoGpioOutputHigh( (mico_gpio_t)WL_REG ); 
  }
}

void MicoSysLed(bool onoff)
{
    if (onoff) {
        MicoGpioOutputHigh( (mico_gpio_t)MICO_SYS_LED );
    } else {
        MicoGpioOutputLow( (mico_gpio_t)MICO_SYS_LED );
    }
}

void MicoRfLed(bool onoff)
{
    if (onoff) {
        MicoGpioOutputLow( (mico_gpio_t)MICO_RF_LED );
    } else {
        MicoGpioOutputHigh( (mico_gpio_t)MICO_RF_LED );
    }
}

bool MicoShouldEnterMFGMode(void)
{
  if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==false && MicoGpioInputGet((mico_gpio_t)MFG_SEL)==false)
    return true;
  else
    return false;
}

bool MicoShouldEnterBootloader(void)
{
  if(MicoGpioInputGet((mico_gpio_t)BOOT_SEL)==false && MicoGpioInputGet((mico_gpio_t)MFG_SEL)==true)
    return true;
  else
    return false;
}

