#ifndef __DMA_H_
#define __DMA_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "irq.h"

typedef struct {
	
	uint8_t	dma1;
	uint8_t	dma2;
	
} DMA_ClockProviderTypeDef;

void 	DMA_Clock_Get(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream);
void 	DMA_Clock_Put(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream);
bool 	DMA_Clock_Status(DMA_ClockProviderTypeDef* dma_clk, DMA_Stream_TypeDef* stream);

/** please use these singletons/globals in DI pattern **/
extern DMA_ClockProviderTypeDef 					DMA_ClockProvider;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef enum {
	
	DMAEX_HANDLE_STATE_RESET = 0,
	DMAEX_HANDLE_STATE_SET,
} DMAEX_HandleStateTypeDef;

typedef struct {
	
	DMA_HandleTypeDef					hdma;			// dma handle
	DMA_ClockProviderTypeDef	*clk;			// reference to dma clock resource manager
	IRQ_HandleTypeDef					*hirq;		// IRQ handle
	
	DMAEX_HandleStateTypeDef	state;		// the state of this struct, SET or RESET
	
} DMAEX_HandleTypeDef;

DMAEX_HandleTypeDef*	DMAEX_Handle_Ctor(DMA_Stream_TypeDef *stream, const DMA_InitTypeDef *init,
	DMA_ClockProviderTypeDef *clk, IRQ_HandleTypeDef *hirq);

void 	DMAEX_Handle_Dtor(DMAEX_HandleTypeDef* handle);

void 	DMAEX_Init(DMAEX_HandleTypeDef* dmaex);
void	DMAEX_DeInit(DMAEX_HandleTypeDef* dmaex);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct {
	
	DMA_ClockProviderTypeDef							*clk;
	IRQ_HandlerObjectRegistryTypeDef			*reg;
	
} DMAEX_Handle_FactoryTypeDef;

DMAEX_HandleTypeDef*	DMAEX_Handle_FactoryCreate(DMAEX_Handle_FactoryTypeDef* factory, 
																									const DMA_HandleTypeDef* hdma,
																									const IRQ_HandleTypeDef* hirq);
																						
/** the factory isn't really required in impl. pass it for user not calling the wrong factory **/																									
void DMAEX_Handle_FactoryDestroy(DMAEX_Handle_FactoryTypeDef* factory, DMAEX_HandleTypeDef* handle);																									

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
																									
const extern DMA_HandleTypeDef		DMA_Handle_Uart2Rx_Default;
const extern DMA_HandleTypeDef		DMA_Handle_Uart2Tx_Default;
const extern DMAEX_HandleTypeDef 	DMAEX_Handle_Uart2Rx_Default;
const extern DMAEX_HandleTypeDef 	DMAEX_Handle_Uart2Tx_Default;																									
																									

#endif

