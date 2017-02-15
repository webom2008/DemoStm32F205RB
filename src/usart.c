#include "usart.h"
#include "configs.h"
#include <string.h>



#define UART1_BAUDRATE                      256000

#if (CFG_ON == CFG_UART1_TXRX_DMA)
#define UART_TXRX_HANDLE_TIMER_MS           5

#define UART_DMA_RX_BUF_SIZE                64
#define UART_DMA_TX_BUF_SIZE                256
#define UART_TX_BUF_SIZE                    CFG_PRINTF_BUF

#define USARTx_DMAx_CLK                 RCC_AHB1Periph_DMA2

#define USARTx_RX_DMA_CHANNEL           DMA_Channel_4
#define USARTx_RX_DMA_STREAM            DMA2_Stream2
#define USARTx_RX_DMA_FLAG_FEIF         DMA_FLAG_FEIF2
#define USARTx_RX_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF2
#define USARTx_RX_DMA_FLAG_TEIF         DMA_FLAG_TEIF2
#define USARTx_RX_DMA_FLAG_HTIF         DMA_FLAG_HTIF2
#define USARTx_RX_DMA_FLAG_TCIF         DMA_FLAG_TCIF2

#define USARTx_TX_DMA_CHANNEL           DMA_Channel_4
#define USARTx_TX_DMA_STREAM            DMA2_Stream7
#define USARTx_TX_DMA_FLAG_FEIF         DMA_FLAG_FEIF7
#define USARTx_TX_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF7
#define USARTx_TX_DMA_FLAG_TEIF         DMA_FLAG_TEIF7
#define USARTx_TX_DMA_FLAG_HTIF         DMA_FLAG_HTIF7
#define USARTx_TX_DMA_FLAG_TCIF         DMA_FLAG_TCIF7

#define USARTx_DR_ADDRESS                   ((uint32_t)USART1 + 0x04) 

static DMA_InitTypeDef  TxDMA_InitStructure;

static vu8 u8UartDmaRxBuffer[UART_DMA_RX_BUF_SIZE];
static vu8 u8UartDmaTxBuffer[UART_DMA_TX_BUF_SIZE];
static vu8 u8UartTxBuffer[UART_TX_BUF_SIZE];
static vu16 u16UartTxBufferOffset = 0;
static vu16 u16UartDmaTxHead = 0;
static vu16 u16UartDmaRxHead = 0;

typedef enum
{
    DMA_IDLE = 0,
    DMA_BUSY,
    DMA_ERROR
} DMA_WORK_STATE;

static __IO DMA_WORK_STATE eUartTxDmaState = DMA_IDLE;


#if (CFG_ON == CFG_UART1_TIM9_HANDLE)
static void Tim9_Init(const u16 ms);
#endif /* (CFG_ON == CFG_UART1_TIM9_HANDLE) */

#endif /* (CFG_ON == CFG_UART1_TXRX_DMA) */


#if (CFG_ON == CFG_UART1_PRINTF)
#endif /* (CFG_ON == CFG_UART1_PRINTF) */


#if (CFG_ON == CFG_UART1_PRINTF) && (CFG_ON == CFG_LCD_PRINTF)
#error 'Both CFG_UART1_PRINTF and CFG_LCD_PRINTF ON'
#elif (CFG_OFF == CFG_UART1_PRINTF) && (CFG_OFF == CFG_LCD_PRINTF)
#error 'Both CFG_UART1_PRINTF and CFG_LCD_PRINTF OFF'
#else
#endif

#define DRV_LOCK()      do{__disable_irq();}while(0)
#define DRV_UNLOCK()    do{__enable_irq();}while(0)


/*******************************************************************************
* Function Name  : USART_Configuration
* Description    : Configure Open207V_USARTx 
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void USART_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
#if (CFG_ON == CFG_UART1_TXRX_DMA)
    DMA_InitTypeDef  RxDMA_InitStructure;
#endif

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);

#if (CFG_ON == CFG_UART1_TXRX_DMA)
    RCC_AHB1PeriphClockCmd(USARTx_DMAx_CLK, ENABLE); /* Enable the DMA clock */
#endif

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*
    USARTx configured as follow:
    - BaudRate = 115200 baud  
    - Word Length = 8 Bits
    - One Stop Bit
    - No parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Receive and transmit    
    */

    USART_InitStructure.USART_BaudRate = UART1_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

#if (CFG_ON == CFG_UART1_TXRX_DMA)
    RxDMA_InitStructure.DMA_PeripheralBaseAddr = USARTx_DR_ADDRESS;
    RxDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    RxDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    RxDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    RxDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    RxDMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    RxDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    RxDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    RxDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    RxDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    RxDMA_InitStructure.DMA_Channel = USARTx_RX_DMA_CHANNEL;
    RxDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; 
    RxDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)u8UartDmaRxBuffer;
    RxDMA_InitStructure.DMA_BufferSize = (uint16_t)UART_DMA_RX_BUF_SIZE;

    DMA_DeInit(USARTx_RX_DMA_STREAM);
    DMA_Init(USARTx_RX_DMA_STREAM, &RxDMA_InitStructure);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);   /* Enable the USART Rx DMA request */
    DMA_Cmd(USARTx_RX_DMA_STREAM, ENABLE);/* Enable the DMA RX Stream */

    TxDMA_InitStructure.DMA_PeripheralBaseAddr = USARTx_DR_ADDRESS;
    TxDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    TxDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    TxDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    TxDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    TxDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    TxDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    TxDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    TxDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    TxDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    TxDMA_InitStructure.DMA_Channel = USARTx_TX_DMA_CHANNEL;
    TxDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    TxDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)u8UartDmaTxBuffer;

#if (CFG_ON == CFG_UART1_TIM9_HANDLE)
    Tim9_Init(UART_TXRX_HANDLE_TIMER_MS);
#endif /* (CFG_ON == CFG_UART1_TIM9_HANDLE) */
#endif
    USART_Cmd(USART1, ENABLE);
}

#if (CFG_ON == CFG_UART1_TXRX_DMA)
static u16 GetUartRxDMACurrentOffset(void)
{
    return (u16)(UART_DMA_RX_BUF_SIZE - DMA_GetCurrDataCounter(USARTx_RX_DMA_STREAM));
}

static u16 getRxBufferValidLen(void)
{
    u16 nLen = 0;
    u16 offset = GetUartRxDMACurrentOffset();

    if (offset > u16UartDmaRxHead)
    {
        nLen = offset - u16UartDmaRxHead;
    }
    else if (offset < u16UartDmaRxHead)
    {
        nLen = offset + UART_DMA_RX_BUF_SIZE - u16UartDmaRxHead;
    }
    else //(offset == u16UartDmaRxHead)
    {
        nLen = 0;
    }
    return nLen;
}

static u8 getRxBufferOneByte(void)
{
    u8 data = u8UartDmaRxBuffer[u16UartDmaRxHead];
    u16UartDmaRxHead = (u16UartDmaRxHead + 1) % UART_DMA_RX_BUF_SIZE;
    return data;
}

static void UartRxDmaHandle(void)
{
    u16 u16DataLen = getRxBufferValidLen();
    
    while(0 != u16DataLen)
    {
        u16DataLen--;

        // echo function.
        DRV_LOCK();
        u8UartTxBuffer[u16UartTxBufferOffset] = getRxBufferOneByte();
        u16UartTxBufferOffset = (u16UartTxBufferOffset+1) % UART_TX_BUF_SIZE;
        DRV_UNLOCK();
    }
}

static u16 getTxBufferValidLen(void)
{
    u16 nLen = 0;
    u16 offset = 0;

    DRV_LOCK();
    offset = u16UartTxBufferOffset;
    DRV_UNLOCK();
    
    if (offset > u16UartDmaTxHead)
    {
        nLen = offset - u16UartDmaTxHead;
    }
    else if (offset < u16UartDmaTxHead)
    {
        nLen = offset + UART_TX_BUF_SIZE - u16UartDmaTxHead;
    }
    else //(offset == u16UartDmaTxHead)
    {
        nLen = 0;
    }
    return nLen;
}

static int IsUartTxDmaIdle(void)
{
    if (DMA_IDLE == eUartTxDmaState)
    {
        return 1;
    }
    else // eUartTxDmaState = DMA_BUSY
    {
        // Do nothing!
    }
    
    if ((SET == DMA_GetFlagStatus(USARTx_TX_DMA_STREAM, USARTx_TX_DMA_FLAG_TCIF)) \
        &&(SET == (USART_GetFlagStatus(USART1, USART_FLAG_TC))))
    {
        DMA_ClearFlag(USARTx_TX_DMA_STREAM,
                        USARTx_TX_DMA_FLAG_HTIF \
                        | USARTx_TX_DMA_FLAG_TCIF); /* Clear DMA Streams flags */
        DMA_Cmd(USARTx_TX_DMA_STREAM, DISABLE);/* Disable the DMA Streams */
        USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);/* Disable the USART Tx DMA request */
        eUartTxDmaState = DMA_IDLE;
    }

    return 0;
}

static void UartTxDmaSend(void)
{
    u16 nLen = 0;
    u16 nTail = 0;
    u16 nLenToSend = 0;

    nLen = getTxBufferValidLen();
    if (nLen > UART_DMA_TX_BUF_SIZE) // Max. Len = UART_DMA_TX_BUF_SIZE
    {
        nLen = UART_DMA_TX_BUF_SIZE;
    }
    
    nTail = u16UartDmaTxHead+nLen;
    if (nTail > UART_TX_BUF_SIZE)
    {
        nLenToSend = UART_TX_BUF_SIZE - u16UartDmaTxHead;
    }
    else
    {
        nLenToSend = nLen;
    }
    
    DRV_LOCK();
    memcpy((void *)u8UartDmaTxBuffer, (void *)&u8UartTxBuffer[u16UartDmaTxHead], nLenToSend);
    DRV_UNLOCK();
    u16UartDmaTxHead = (u16UartDmaTxHead + nLenToSend) % UART_TX_BUF_SIZE;
    
    DMA_DeInit(USARTx_TX_DMA_STREAM);
    TxDMA_InitStructure.DMA_BufferSize = (uint16_t)nLenToSend; // MAX.= UART_DMA_TX_BUF_SIZE 
    DMA_Init(USARTx_TX_DMA_STREAM, &TxDMA_InitStructure); 
    
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);/* Enable the USART DMA requests */
    USART_ClearFlag(USART1, USART_FLAG_TC);/* Clear the TC bit in the SR register by writing 0 to it */
   /* Enable the DMA TX Stream, USART will start sending the command code (2bytes) */
    DMA_Cmd(USARTx_TX_DMA_STREAM, ENABLE);
    eUartTxDmaState = DMA_BUSY;
}


#if (CFG_ON == CFG_UART1_TIM9_HANDLE)
static void Tim9_Init(const u16 ms)
{
    TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
    uint16_t PrescalerValue = 0;
    u16 u16Period = 0;
    NVIC_InitTypeDef   NVIC_InitStructure;

    // Lowest IRQ Level
    NVIC_InitStructure.NVIC_IRQChannel  = TIM1_BRK_TIM9_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* -----------------------------------------------------------------------
    TIM9 Configuration: TIM_IT_Update Mode:
    
    In this example TIM9 input clock (TIM9CLK) is set to 2 * APB2 clock (PCLK2), 
    since APB2 prescaler is different from 1. 
    
      TIM9CLK = 2 * PCLK2 
          
    To get TIM9 counter clock at 0.1ms (10kHz), the prescaler is computed as follows:
       Prescaler = (TIM9CLK / TIM9 counter clock) - 1
       Prescaler = ((2 * PCLK2 ) /10 KHz) - 1
    */
    PrescalerValue = (u16)((SystemCoreClock / 10000) - 1);
    
    u16Period = ms * 10;
    
    TIM_DeInit(TIM9);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE); 
    TIM_BaseInitStructure.TIM_Period = u16Period;        
    TIM_BaseInitStructure.TIM_Prescaler = PrescalerValue;        
    TIM_BaseInitStructure.TIM_ClockDivision = 0; 
    TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_BaseInitStructure.TIM_RepetitionCounter = 0; 
    TIM_Cmd(TIM9, DISABLE);
    TIM_TimeBaseInit(TIM9, &TIM_BaseInitStructure); 

    TIM_ClearFlag(TIM9, TIM_FLAG_Update); 
    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE); 
    TIM_Cmd(TIM9, ENABLE);
}
#endif /* (CFG_ON == CFG_UART1_TIM9_HANDLE) */

#endif /* (CFG_ON == CFG_UART1_TXRX_DMA) */

void Uart_Server(void)
{
#if (CFG_ON == CFG_UART1_TXRX_DMA)
    if (getTxBufferValidLen() > 0) // Have Data to Send.
    {
        if (IsUartTxDmaIdle())
        {
            UartTxDmaSend();
        }
    }
    UartRxDmaHandle();
#endif
}

void Uart1_SendByte(char byte)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(USART1, (uint8_t)byte);
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {}
}

void Uart1_SendString(char *pString)
{
    if (NULL == pString) return;

    while(*pString != '\0')
    {
        Uart1_SendByte(*pString);
        pString++;
    }
}

