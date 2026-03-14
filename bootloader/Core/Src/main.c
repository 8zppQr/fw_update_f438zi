/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <tinycrypt/sha256.h>
#include <tinycrypt/constants.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#pragma pack(push, 1)
typedef struct {
    uint32_t ih_magic;            // 0x96f3b83d
    uint32_t ih_load_addr;        // dumpinfoでは0
    uint16_t ih_hdr_size;         // dumpinfoでは0x200
    uint16_t ih_protect_tlv_size; // dumpinfoでは0
    uint32_t ih_img_size;         // dumpinfoのimg_size
    uint32_t ih_flags;            // dumpinfoのflags
    // image_version
    uint8_t  iv_major;
    uint8_t  iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
    uint32_t _pad1;               // MCUboot headerの詰め物（dumpinfoにもある想定）
} image_header_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint16_t magic;      // 0x6907
    uint16_t tlv_tot;    // TLV全体サイズ（このヘッダ4バイトを含む）
} image_tlv_info_t;

typedef struct {
    uint16_t type;       // 0x0010 = SHA256
    uint16_t len;        // 0x0020 = 32 bytes
    // followed by len bytes
} image_tlv_t;
#pragma pack(pop)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SLOT_A_ADDR (0x08040000U)
#define SLOT_B_ADDR (0x080C0000U)
#define HEADER_SIZE (0x200U)
#define IMAGE_MAGIC 0x96F3B83DU
#define TLV_INFO_MAGIC 0x6907U
#define TLV_TYPE_SHA256 0x0010U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
typedef void (*pFunction)(void);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

ETH_TxPacketConfig TxConfig;
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

ETH_HandleTypeDef heth;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
/* USER CODE BEGIN PFP */
void bootimg(uint32_t img_addr);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART3 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);

    return ch;
}

static void print_hex(const uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\r\n");
}

//void test_sha256_abc(void)
//{
//    const uint8_t msg[] = "abc";
//    uint8_t digest[TC_SHA256_DIGEST_SIZE];
//    struct tc_sha256_state_struct s;
//
//    int ret;
//
//    ret = tc_sha256_init(&s);
//    if (ret != TC_CRYPTO_SUCCESS) {
//        printf("tc_sha256_init failed\r\n");
//        return;
//    }
//
//    ret = tc_sha256_update(&s, msg, strlen((const char *)msg));
//    if (ret != TC_CRYPTO_SUCCESS) {
//        printf("tc_sha256_update failed\r\n");
//        return;
//    }
//
//    ret = tc_sha256_final(digest, &s);
//    if (ret != TC_CRYPTO_SUCCESS) {
//        printf("tc_sha256_final failed\r\n");
//        return;
//    }
//
//    printf("SHA256(\"abc\") = ");
//    print_hex(digest, sizeof(digest));
//}

static void PrintSlotBHeader(void)
{
    const image_header_t *hdr = (const image_header_t *)SLOT_B_ADDR;

    printf("=== Slot B image header @ 0x%08lX ===\r\n", (unsigned long)SLOT_B_ADDR);
    printf("magic:              0x%08lX\r\n", (unsigned long)hdr->ih_magic);
    printf("load_addr:          0x%08lX\r\n", (unsigned long)hdr->ih_load_addr);
    printf("hdr_size:           0x%04X\r\n",  (unsigned)hdr->ih_hdr_size);
    printf("protected_tlv_size: 0x%04X\r\n",  (unsigned)hdr->ih_protect_tlv_size);
    printf("img_size:           0x%08lX\r\n", (unsigned long)hdr->ih_img_size);
    printf("flags:              0x%08lX\r\n", (unsigned long)hdr->ih_flags);
    printf("version:            %u.%u.%u+%lu\r\n",
           (unsigned)hdr->iv_major,
           (unsigned)hdr->iv_minor,
           (unsigned)hdr->iv_revision,
           (unsigned long)hdr->iv_build_num);

    if (hdr->ih_magic != IMAGE_MAGIC) {
        printf("Header magic NG\r\n");
        return;
    }

    uint32_t vector = SLOT_B_ADDR + (uint32_t)hdr->ih_hdr_size;
    uint32_t msp    = *(__IO uint32_t *)vector;
    uint32_t rh     = *(__IO uint32_t *)(vector + 4U);

    printf("vector_addr:         0x%08lX\r\n", (unsigned long)vector);
    printf("vector[0] MSP:       0x%08lX\r\n", (unsigned long)msp);
    printf("vector[1] ResetHdlr: 0x%08lX\r\n", (unsigned long)rh);


}

static void VerifySlotBHash(void)
{
    const image_header_t *hdr = (const image_header_t *)SLOT_B_ADDR;

    if (hdr->ih_magic != IMAGE_MAGIC) {
        printf("Header magic NG\r\n");
        return;
    }

    uint32_t vector_addr = SLOT_B_ADDR + (uint32_t)hdr->ih_hdr_size;
    uint32_t tlv_start   = vector_addr + (uint32_t)hdr->ih_img_size;

    const image_tlv_info_t *info = (const image_tlv_info_t *)tlv_start;

    printf("=== Slot B TLV verify ===\r\n");
    printf("TLV start:            0x%08lX\r\n", (unsigned long)tlv_start);
    printf("TLV magic:            0x%04X\r\n", info->magic);
    printf("TLV total size:       0x%04X\r\n", info->tlv_tot);

    if (info->magic != TLV_INFO_MAGIC) {
        printf("TLV magic NG\r\n");
        return;
    }

    uint32_t tlv_end = tlv_start + info->tlv_tot;
    uint32_t p = tlv_start + sizeof(image_tlv_info_t);

    while (p + sizeof(image_tlv_t) <= tlv_end) {
        const image_tlv_t *tlv = (const image_tlv_t *)p;
        uint32_t value_addr = p + sizeof(image_tlv_t);

        printf("TLV type:             0x%04X\r\n", tlv->type);
        printf("TLV len:              0x%04X\r\n", tlv->len);

        if (value_addr + tlv->len > tlv_end) {
            printf("TLV range error\r\n");
            return;
        }

        if (tlv->type == TLV_TYPE_SHA256 && tlv->len == 32U) {
            uint8_t expected[32];
            uint8_t calculated[32];
            struct tc_sha256_state_struct s;

            memcpy(expected, (const void *)value_addr, sizeof(expected));

            printf("Expected SHA256:\r\n");
            print_hex(expected, sizeof(expected));

            uint32_t hash_start = SLOT_B_ADDR;
            uint32_t hash_len   = (uint32_t)hdr->ih_hdr_size + (uint32_t)hdr->ih_img_size;

            if (!tc_sha256_init(&s)) {
                printf("tc_sha256_init failed\r\n");
                return;
            }

            if (!tc_sha256_update(&s, (const uint8_t *)hash_start, hash_len)) {
                printf("tc_sha256_update failed\r\n");
                return;
            }

            if (!tc_sha256_final(calculated, &s)) {
                printf("tc_sha256_final failed\r\n");
                return;
            }

            printf("Calculated SHA256:\r\n");
            print_hex(calculated, sizeof(calculated));

            if (memcmp(expected, calculated, sizeof(expected)) == 0) {
                printf("HASH MATCH\r\n");
            } else {
                printf("HASH MISMATCH\r\n");
            }
            return;
        }

        p = value_addr + tlv->len;
    }

    printf("SHA256 TLV not found\r\n");
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  /* USER CODE BEGIN 2 */
  printf("bootloader started!\r\n");
  PrintSlotBHeader();
  VerifySlotBHash();
  printf("Select number:\r\n");
  printf("1:Boot slot A\r\n");
  printf("2:Boot slot B\r\n");
  uint8_t ch;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    if (HAL_UART_Receive(&huart3, &ch, 1, 10) == HAL_OK)
	    {
	        if (ch == '1')
	        {
	            printf("Booting SLOT A\r\n");
	            bootimg(SLOT_A_ADDR);
	        }
	        else if (ch == '2')
	        {
	            printf("Booting SLOT B\r\n");
	            bootimg(SLOT_B_ADDR + HEADER_SIZE);
	        }
	        else
	        {
	            printf("Invalid key\r\n");
	        }
	    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void bootimg(uint32_t img_addr)
{
	pFunction JumpToImg;
	printf("jumping to 0x%08lX...\r\n", img_addr);

	SCB->VTOR = img_addr;
	__set_MSP(*(uint32_t*)img_addr);

	JumpToImg = (pFunction)(*(uint32_t*)(img_addr + 4));

	JumpToImg();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
