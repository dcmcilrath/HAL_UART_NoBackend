/*
 *  Lab00 Stuff Below
 */

#include "main.h"

// Uart HAL structure
UART_HandleTypeDef UartHandle;
__IO ITStatus UartReady = RESET;
__IO uint32_t UserButtonStatus = 0;  // set to 1 after User Button interrupt 


// Private function prototypes ----------------------------------------------
void SystemClock_Config(void);
static void Error_Handler(void);
//static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
static void CPU_CACHE_Enable(void);

void uart_print(char *, uint8_t);
void uart_puts(char *);
char uart_getchar();
int uart_getline(char *, int);

// Main Execution Loop
int main(void)
{
  //input buffer and size
  char input[80];
  int i;
  // Enable the CPU Cache
  CPU_CACHE_Enable();
  /* STM32F7xx HAL library initialization:
       - Configure the Flash ART accelerator
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
  */
    
  HAL_Init();

  // Configure the system clock to 216 MHz
  SystemClock_Config();
  
  // Configure LED1
  BSP_LED_Init(LED1);

  //##-1- Configure the UART peripheral #####################################
  // Put the USART peripheral in the Asynchronous mode (UART Mode)
  /* UART configured as follows:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = None
      - BaudRate = 9600 baud
      - Hardware flow control disabled (RTS and CTS signals)
  */
  UartHandle.Instance        = USARTx;

  UartHandle.Init.BaudRate   = 115200;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;
  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }  
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }

  // Configure User push-button in Interrupt mode
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
  
  // Wait for User push-button press before starting the Communication.
  // In the meantime, LED1 is blinking
  while(UserButtonStatus == 0)
  {
      // Toggle LED
      BSP_LED_Toggle(LED1); 
      HAL_Delay(100);
  }

  // Output overly optimistic greeting:
  uart_puts("Hello World!\r\n");

  // Loop very simple program
  while (1)
  {
    uart_puts("Type something: ");
    i = uart_getline(input, 80);
    uart_puts("You typed: ");
    uart_print(input, i); // use uart_print because input is not null-terminated automatically
    uart_puts("\r\n\r\n");
  }
  
  //##-4- Wait for the end of the transfer ##################################  
  while (UartReady != SET);
  // Reset transmission flag
  UartReady = RESET;
  // Turn off LED when transmission is complete
  BSP_LED_Off(LED1); 

  // Infinite Loop Because Reasons
  while(1);
}

// Collects characters until size limit or an endline is recieved
// Returns number of characters collected
int uart_getline(char * input, int max_size)
{
  char single;
  int i = 0;

  do
  {
    single = uart_getchar(); // get one character

    if (i >= max_size)
      break;

    if (single != '\n')
      input[i] = single;
    else
      break;

    i++;

  } while(1);

  return i;
}

// Get one character
char uart_getchar()
{
  char input[8] = "00000000"; // holdover from previous code
  HAL_StatusTypeDef halstat;

  do
  {
    // Query UART in blocking mode with timeout of 1s
    halstat = HAL_UART_Receive(&UartHandle, (uint8_t *)input, 1, 1000);
  } while(halstat == HAL_TIMEOUT); // Loop if nothing was recieved

  // Status handling
  if (halstat == HAL_OK) // Good to go
  {
    if (input[0] == '\r' || input[0] == '\n') // handling for dumb windows endlines
    {
      uart_puts("\r\n");
      return '\n';
    }
    else
    {
      uart_print(input, 1);
      return (char)input[0];
    }
  }
  else if (halstat == HAL_TIMEOUT) // This code should no longer be reached.  If it is the world is broken.
    uart_puts("\r\n\r\nHAL does not respond.");
  else if(halstat == HAL_BUSY) // Somebody is hogging the channel
    uart_puts("\r\n\r\nHAL Says: This mission is too important for me to allow you to jeopardize it.");
  
  Error_Handler();  // Go to the error handler if any errors occurred.
  return 255;
}

// Print a string over uart channel
void uart_puts(char * string)
{
  // Find location of null terminator character
  uint8_t length = 0;
  while (length < 255 && string[length] != '\0')
    length++;

  if (length == 255)
    uart_print("puts() size limit exceeded\r\n", 28);
  else
    uart_print(string, length);
}

// wrapper for original print command that sets some defaults like UartHandle and a 1s timeout
// useful for strings that are not null terminated but the size is known.
void uart_print(char * string, uint8_t length)
{
    if (HAL_UART_Transmit(&UartHandle, (uint8_t *)string, length, 1000) != HAL_OK)
      Error_Handler();
}

 /*
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            PLL_R                          = 7  
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
 
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  // Enable HSE Oscillator and activate PLL with HSE as source
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  // Activate the OverDrive to reach the 216 MHz Frequency  
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2; 
  
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }  
}

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of DMA Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
 
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  // Set transmission flag: trasfer complet
  UartReady = SET;

  
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  // Set transmission flag: trasfer complet
  UartReady = SET;

  
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
    Error_Handler();
}


/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == USER_BUTTON_PIN)
  {  
    UserButtonStatus = 1;
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
 
static void Error_Handler(void)
{
  // Turn LED1 on
  BSP_LED_On(LED1);

  uart_puts("\r\nHAL Says: I'm sorry Dave I'm afraid I can't do that.\r\n\r\n");

  while(1)
  {
    // Error if LED1 is slowly blinking (1 sec. period)
    BSP_LED_Toggle(LED1); 
    HAL_Delay(1000); 
  }  
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
 
void assert_failed(uint8_t* file, uint32_t line)
{ 
  // User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)

  // Infinite loop
  while (1)
  {
  }
}
#endif

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
 
static void CPU_CACHE_Enable(void)
{
  // Enable I-Cache
  SCB_EnableICache();

  // Enable D-Cache
  SCB_EnableDCache();
}

//*********************** (C) COPYRIGHT STMicroelectronics *****END OF FILE**
