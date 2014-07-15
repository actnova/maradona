#include <stdlib.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "irq.h"
#include "dma.h"


/** DMA Clock (Resource Manager) Singleton **/
DMA_ClockProviderTypeDef 						DMA_ClockProvider = {0};


/******************************************************************************
#define DMA1                ((DMA_TypeDef *) DMA1_BASE)
#define DMA1_Stream0        ((DMA_Stream_TypeDef *) DMA1_Stream0_BASE)
#define DMA1_Stream1        ((DMA_Stream_TypeDef *) DMA1_Stream1_BASE)
#define DMA1_Stream2        ((DMA_Stream_TypeDef *) DMA1_Stream2_BASE)
#define DMA1_Stream3        ((DMA_Stream_TypeDef *) DMA1_Stream3_BASE)
#define DMA1_Stream4        ((DMA_Stream_TypeDef *) DMA1_Stream4_BASE)
#define DMA1_Stream5        ((DMA_Stream_TypeDef *) DMA1_Stream5_BASE)
#define DMA1_Stream6        ((DMA_Stream_TypeDef *) DMA1_Stream6_BASE)
#define DMA1_Stream7        ((DMA_Stream_TypeDef *) DMA1_Stream7_BASE)
#define DMA2                ((DMA_TypeDef *) DMA2_BASE)
#define DMA2_Stream0        ((DMA_Stream_TypeDef *) DMA2_Stream0_BASE)
#define DMA2_Stream1        ((DMA_Stream_TypeDef *) DMA2_Stream1_BASE)
#define DMA2_Stream2        ((DMA_Stream_TypeDef *) DMA2_Stream2_BASE)
#define DMA2_Stream3        ((DMA_Stream_TypeDef *) DMA2_Stream3_BASE)
#define DMA2_Stream4        ((DMA_Stream_TypeDef *) DMA2_Stream4_BASE)
#define DMA2_Stream5        ((DMA_Stream_TypeDef *) DMA2_Stream5_BASE)
#define DMA2_Stream6        ((DMA_Stream_TypeDef *) DMA2_Stream6_BASE)
#define DMA2_Stream7        ((DMA_Stream_TypeDef *) DMA2_Stream7_BASE)
******************************************************************************/

int DMA_Stream_Instance_To_Index(DMA_Stream_TypeDef* stream)
{
	if (stream == DMA1_Stream0 || stream == DMA2_Stream0)
		return 0;
	if (stream == DMA1_Stream1 || stream == DMA2_Stream1)
		return 1;
	if (stream == DMA1_Stream2 || stream == DMA2_Stream2)
		return 2;
	if (stream == DMA1_Stream3 || stream == DMA2_Stream3)
		return 3;
	if (stream == DMA1_Stream4 || stream == DMA2_Stream4)
		return 4;
	if (stream == DMA1_Stream5 || stream == DMA2_Stream5)
		return 5;
	if (stream == DMA1_Stream6 || stream == DMA2_Stream6)
		return 6;
	if (stream == DMA1_Stream7 || stream == DMA2_Stream7)
		return 7;
	
	return -1;
}

void 	DMAEX_Init(DMAEX_HandleTypeDef* dmaex)
{
	DMA_Clock_Get(dmaex->clk, dmaex->hdma.Instance);
	HAL_DMA_Init(&dmaex->hdma);
	IRQ_Init(dmaex->hirq, &dmaex->hdma);				/** make the link **/
	dmaex->state = DMAEX_HANDLE_STATE_SET;
}

void	DMAEX_DeInit(DMAEX_HandleTypeDef* dmaex)
{
	IRQ_DeInit(dmaex->hirq);
	HAL_DMA_DeInit(&dmaex->hdma);
	DMA_Clock_Put(dmaex->clk, dmaex->hdma.Instance);
	dmaex->state = DMAEX_HANDLE_STATE_RESET;
}

void DMA_Clock_Get(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream)
{
	int pos = 0;
	
	if (DMA1_Stream0 == stream ||
			DMA1_Stream1 == stream ||
			DMA1_Stream2 == stream ||
			DMA1_Stream3 == stream ||
			DMA1_Stream4 == stream ||
			DMA1_Stream5 == stream ||
			DMA1_Stream6 == stream ||
			DMA1_Stream7 == stream)
	{
		pos = (stream - DMA1_Stream0);
		
		dma_clk->dma1 |= (1 << pos);
		if (dma_clk->dma1)
		{
			__DMA1_CLK_ENABLE();
		}
		
		return;
	}
	
	if (DMA2_Stream0 == stream ||
			DMA2_Stream1 == stream ||
			DMA2_Stream2 == stream ||
			DMA2_Stream3 == stream ||
			DMA2_Stream4 == stream ||
			DMA2_Stream5 == stream ||
			DMA2_Stream6 == stream ||
			DMA2_Stream7 == stream)
	{
		pos = (stream - DMA2_Stream0);
		
		(dma_clk->dma2) |= (1 << pos);
		if (dma_clk->dma2)
		{
			__DMA2_CLK_ENABLE();
		}
		
		return;		
	}
}

void DMA_Clock_Put(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream)
{
	int pos = 0;
	
	if (DMA1_Stream0 == stream ||
			DMA1_Stream1 == stream ||
			DMA1_Stream2 == stream ||
			DMA1_Stream3 == stream ||
			DMA1_Stream4 == stream ||
			DMA1_Stream5 == stream ||
			DMA1_Stream6 == stream ||
			DMA1_Stream7 == stream)
	{
		pos = (stream - DMA1_Stream0);
		
		(dma_clk->dma1) &= ~(1 << pos);
		if (dma_clk->dma1 == 0)
		{
			__DMA1_CLK_DISABLE();
		}
		
		return;
	}
	
	if (DMA2_Stream0 == stream ||
			DMA2_Stream1 == stream ||
			DMA2_Stream2 == stream ||
			DMA2_Stream3 == stream ||
			DMA2_Stream4 == stream ||
			DMA2_Stream5 == stream ||
			DMA2_Stream6 == stream ||
			DMA2_Stream7 == stream)
	{
		pos = (stream - DMA2_Stream0);
		
		(dma_clk->dma2) &= ~(1 << pos);
		if (dma_clk->dma2 == 0)
		{
			__DMA2_CLK_DISABLE();
		}
		
		return;		
	}	
}

bool DMA_Clock_Status(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream)
{
	int pos = 0;
	
	if (DMA1_Stream0 == stream ||
			DMA1_Stream1 == stream ||
			DMA1_Stream2 == stream ||
			DMA1_Stream3 == stream ||
			DMA1_Stream4 == stream ||
			DMA1_Stream5 == stream ||
			DMA1_Stream6 == stream ||
			DMA1_Stream7 == stream)
	{
		pos = (stream - DMA1_Stream0);
		return (dma_clk->dma1 & (1 << pos)) ? true : false;
	}
	
	if (DMA2_Stream0 == stream ||
			DMA2_Stream1 == stream ||
			DMA2_Stream2 == stream ||
			DMA2_Stream3 == stream ||
			DMA2_Stream4 == stream ||
			DMA2_Stream5 == stream ||
			DMA2_Stream6 == stream ||
			DMA2_Stream7 == stream)
	{
		pos = (stream - DMA2_Stream0);
		return (dma_clk->dma2 & (1 << pos)) ? true : false;;		
	}	
	
	assert_param(false);
	return false;
}




DMAEX_HandleTypeDef*	DMAEX_Handle_Ctor(DMA_Stream_TypeDef *instance, const DMA_InitTypeDef *init,
	DMA_ClockProviderTypeDef *clk, IRQ_HandleTypeDef *hirq)
{
	DMAEX_HandleTypeDef* h = (DMAEX_HandleTypeDef*)malloc(sizeof(DMAEX_HandleTypeDef));
	if (!h) return NULL;
	
	memset(h, 0, sizeof(DMAEX_HandleTypeDef));
	
	h->clk = clk;
	h->hdma.Instance = instance;
	memmove(&h->hdma.Init, init, sizeof(DMA_InitTypeDef));
	h->hirq = hirq;
	h->state = DMAEX_HANDLE_STATE_RESET;
	
	return h;
}

DMAEX_HandleTypeDef*	DMAEX_Handle_FactoryCreate(DMAEX_Handle_FactoryTypeDef* factory, 
																									const DMA_HandleTypeDef* hdma,
																									const IRQ_HandleTypeDef* hirq)
{
	DMAEX_HandleTypeDef* dmaExH;
	IRQ_HandleTypeDef* irqH;
	
	irqH = IRQ_Handle_Ctor_By_Template(hirq, factory->reg);
	if (irqH == NULL)
		return NULL;
	
	dmaExH = DMAEX_Handle_Ctor(hdma->Instance, &hdma->Init, factory->clk, irqH);
	if (dmaExH == NULL)
	{
		IRQ_Handle_Dtor(irqH);
		return NULL;
	}

	return dmaExH;
}

void DMAEX_Handle_FactoryDestroy(DMAEX_Handle_FactoryTypeDef* factory, DMAEX_HandleTypeDef* handle)
{
	if (handle)
	{
		if (handle->hirq)
		{
			free(handle->hirq);
		}
		
		free(handle);
	}
}

void DMAEX_Handle_Dtor(DMAEX_HandleTypeDef* handle)
{
	if (handle) free(handle);
}

void DMA1_Stream5_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(DMA1_Stream5_IRQn);

	DMA_HandleTypeDef* handle = IRQ_HandlerObject_Get(&IRQ_HandlerObjectRegistry, DMA1_Stream5_IRQn);
	if (handle)
	{
		HAL_DMA_IRQHandler(handle);
	}
}

void DMA1_Stream6_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(DMA1_Stream6_IRQn);
	
	DMA_HandleTypeDef* handle = IRQ_HandlerObject_Get(&IRQ_HandlerObjectRegistry, DMA1_Stream6_IRQn);
	if (handle)
	{
		HAL_DMA_IRQHandler(handle);
	}	
}



///////////////////////////////////////////////////////////////////////////////
//
// Defaults
//
///////////////////////////////////////////////////////////////////////////////

//const DMA_InitTypeDef DMA_Init_Uart2Rx_Default =
//{
//	.Channel = DMA_CHANNEL_4,
//	.Direction = DMA_PERIPH_TO_MEMORY,
//	.PeriphInc = DMA_PINC_DISABLE,
//	.MemInc = DMA_MINC_ENABLE,
//	.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
//	.MemDataAlignment = DMA_MDATAALIGN_BYTE,
//	.Mode = DMA_NORMAL,
//	.Priority = DMA_PRIORITY_LOW,
//	.FIFOMode = DMA_FIFOMODE_DISABLE,
//};

const DMA_HandleTypeDef	DMA_Handle_Uart2Rx_Default = 
{
	.Instance = DMA1_Stream5,
	.Init = 
	{
		.Channel = DMA_CHANNEL_4,
		.Direction = DMA_PERIPH_TO_MEMORY,
		.PeriphInc = DMA_PINC_DISABLE,
		.MemInc = DMA_MINC_ENABLE,
		.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
		.MemDataAlignment = DMA_MDATAALIGN_BYTE,
		.Mode = DMA_NORMAL,
		.Priority = DMA_PRIORITY_LOW,
		.FIFOMode = DMA_FIFOMODE_DISABLE,
	},	
};

const DMA_HandleTypeDef	DMA_Handle_Uart2Tx_Default = 
{
	.Instance = DMA1_Stream6,
	.Init = 
	{
		.Channel = DMA_CHANNEL_4,
		.Direction = DMA_MEMORY_TO_PERIPH,
		.PeriphInc = DMA_PINC_DISABLE,
		.MemInc = DMA_MINC_ENABLE,
		.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
		.MemDataAlignment = DMA_MDATAALIGN_BYTE,
		.Mode = DMA_NORMAL,
		.Priority = DMA_PRIORITY_LOW,
		.FIFOMode = DMA_FIFOMODE_DISABLE,
	},
};

const DMAEX_HandleTypeDef DMAEX_Handle_Uart2Rx_Default =
{
	.clk = &DMA_ClockProvider,
	.hdma =
	{	
		.Instance = DMA1_Stream5,
		.Init = 
		{
			.Channel = DMA_CHANNEL_4,
			.Direction = DMA_PERIPH_TO_MEMORY,
			.PeriphInc = DMA_PINC_DISABLE,
			.MemInc = DMA_MINC_ENABLE,
			.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
			.MemDataAlignment = DMA_MDATAALIGN_BYTE,
			.Mode = DMA_NORMAL,
			.Priority = DMA_PRIORITY_LOW,
			.FIFOMode = DMA_FIFOMODE_DISABLE,
		},
	},
};

const DMAEX_HandleTypeDef DMAEX_Handle_Uart2Tx_Default = 
{
	.hdma =
	{
		.Instance = DMA1_Stream6,
		.Init = 
		{
			.Channel = DMA_CHANNEL_4,
			.Direction = DMA_MEMORY_TO_PERIPH,
			.PeriphInc = DMA_PINC_DISABLE,
			.MemInc = DMA_MINC_ENABLE,
			.PeriphDataAlignment = DMA_PDATAALIGN_BYTE,
			.MemDataAlignment = DMA_MDATAALIGN_BYTE,
			.Mode = DMA_NORMAL,
			.Priority = DMA_PRIORITY_LOW,
			.FIFOMode = DMA_FIFOMODE_DISABLE,
		},
	},	
	
	.clk = &DMA_ClockProvider,
};

