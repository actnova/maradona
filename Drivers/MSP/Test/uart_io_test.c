#include <string.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "errno_ex.h"
#include "uart_io.h"
#include "uart_io_private.h"
#include "devicefs.h"

extern void* 	get_testdata(void);
extern void 	set_testdata(void* data);

struct uart_device_testdata {
	
	struct file 									file;
	struct msp_factory 						msp;
	struct UARTEX_HandleTypeDef		uartex_handle;
	struct uart_device						udev;
	
	int	msp_create_uart_handle_by_port_fail_countdown;
	int msp_create_uart_handle_by_port_called;
	
	int msp_destroy_uartex_handle_fail_countdown;
	int msp_destroy_uartex_handle_called;
	
	int uartex_handle_init_fail_countdown;
	int uartex_handle_init_called;
	
	int uartex_handle_deinit_fail_countdown;
	int uartex_handle_deinit_called;
	
	int uartex_handle_recv_fail_countdown;
	int uartex_handle_recv_called;
};

typedef struct 
{
	/** simulated registers **/
	uint32_t 								rx_m0ar;
	int 										rx_ndtr;
	
	uint32_t								tx_m0ar;
 	int											tx_ndtr;
	
	char*										rxdma_buf;
	char*										txdma_buf;
	
} uio_testdata_t;

const static char literal[] = 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789" 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789" 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"			 
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789"
	"abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789";

const char* literal_end = literal + sizeof(literal);

HAL_StatusTypeDef m_init_hal_ok(UART_HandleTypeDef *huart) {
	
	TEST_ASSERT_EQUAL(HAL_UART_STATE_RESET, huart->State);
	return HAL_OK;
}

HAL_StatusTypeDef m_init_hal_err(UART_HandleTypeDef *huart) {
	
	return HAL_ERROR;
}


static void fill_rx_testdata(	struct uart_device* huio, 	/** handle 								**/
															int index,										/** upper									**/
															int offset,										/** upper offset 					**/
															const char* upper,						/** content for upper 		**/
															int upper_size,								/** upper buffer fillsize **/
															const char* dma,							/** dma buffer 						**/
															int dma_size,									/** size to copy into dma buffer **/
															int rx_size)									/** previously set dma recv size **/
{
	int other;
	uio_testdata_t* td = huio->handle->testdata;
	
	huio->rx_upper = huio->rbuf[index];
	huio->rx_head = &huio->rx_upper[offset];
	huio->rx_tail = huio->rx_head + upper_size;
	
	if (upper_size)
	{
		memmove(huio->rx_head, upper, upper_size);
	}
	
	other = (index == 0) ? 1 : 0;
	
	td->rx_m0ar = (uint32_t)huio->rbuf[other];
	td->rx_ndtr = rx_size - dma_size;
	td->rxdma_buf = huio->rbuf[other];
	
	if (dma_size)
	{
		memmove(td->rxdma_buf, dma, dma_size);
	}
}

static void fill_tx_testdata( struct uart_device* huio,
															int index,
															const char* src,
															int upper_size,
															const char* dma,							/** dma buffer 						**/
															int dma_size,									/** size to copy into dma buffer **/
															int tx_sent)									/** previously set dma recv size **/
{
	/** dma and dma_size should be considered as the arguments previously 
			passed to HAL_UART_Transmit_DMA, and tx_sent should be 'expected' number of bytes 
			already sent. **/
	
	int other;
	uio_testdata_t* td = huio->handle->testdata;
	
	huio->tx_head = huio->tbuf[index];
	huio->tx_tail = huio->tx_head + upper_size;
	
	if (upper_size)
	{
		memmove(huio->tx_head, src, upper_size);
	}
	
	other = (index == 0) ? 1 : 0;
	
	td->tx_m0ar = (uint32_t)huio->tbuf[other];
	td->txdma_buf = huio->tbuf[other];
	
	if (dma_size)
	{
		memmove(td->txdma_buf, dma, dma_size);
	}
	
	td->tx_ndtr = dma_size - tx_sent;
}
															



///////////////////////////////////////////////////////////////////////////////
// Read
//
TEST_GROUP(UsartIO_DMA);
TEST_SETUP(UsartIO_DMA)
{
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	
	/**
		struct uart_device_testdata testdata = {
		.file = {0},
		.msp = {0},
		.uartex_handle = {0},
		.udev = {
			.dev = { .name = "UART2", .number = 2, },
			.msp = &testdata.msp,
			.rbuf_size = 13,
			.tbuf_size = 17,
		},
	};**/
	memset(&td->file, 0, sizeof(td->file));
	memset(&td->msp, 0, sizeof(td->msp));
	memset(&td->uartex_handle, 0, sizeof(td->uartex_handle));
	
	td->msp_create_uart_handle_by_port_fail_countdown = -1;
	td->msp_create_uart_handle_by_port_called = 0;
	
	td->msp_destroy_uartex_handle_fail_countdown = -1;
	td->msp_destroy_uartex_handle_called = 0;
	
	td->uartex_handle_init_fail_countdown = -1;
	td->uartex_handle_init_called = 0;
	
	td->uartex_handle_deinit_fail_countdown = -1;
	td->uartex_handle_deinit_called = 0;
	
	td->uartex_handle_recv_fail_countdown = -1;
	td->uartex_handle_recv_called = 0;
}

TEST_TEAR_DOWN(UsartIO_DMA)
{
}

TEST(UsartIO_DMA, ReadInvalidArgs)
{
	char c;
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	
	/** These tests are enough, don't validate 'internal state' of opaque struct, pointless. **/
	TEST_ASSERT_EQUAL(-1, uart_device_read(0, &c, 1));					/** null handle **/
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(-1, uart_device_read(&huio, 0, 1));				/** null buf p	**/
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(0, uart_device_read(&huio, &c, 0));				/** no error checking as linux **/
}

TEST(UsartIO_DMA, ReadWhenBytesToReadLessThanOrEqualToBytesInBuffer)
{
	char sample[] = "test";
	char buf[64];
	int read;
	
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };	
	UARTEX_HandleTypeDef hue;
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;
	
	// fill_rx_upper(&huio, 1, 1, sample, strlen(sample));	/** use buffer 1 as upper buffer **/
	fill_rx_testdata(&huio, 1, 1, sample, strlen(sample), 0, 0, 0);
	
	memset(buf, 0, sizeof(buf));	
	read = uart_device_read(&huio, buf, strlen(sample));
	
	TEST_ASSERT_EQUAL(strlen(sample), read);
	TEST_ASSERT_EQUAL_MEMORY(sample, buf, strlen(sample));
}


HAL_StatusTypeDef swap_mock_hal_ok(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	uio_testdata_t* testdata = (uio_testdata_t*)h->testdata;
	
	if (m0ar) *m0ar = testdata->rx_m0ar;
	if (ndtr) *ndtr = testdata->rx_ndtr;
	
	testdata->rx_m0ar = (uint32_t)buf;
	testdata->rx_ndtr = size;
	
	return HAL_OK;
}

TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalReady)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;

	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
//	struct UARTEX_Operations uart_ops;
	
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;

//	memset(&uart_ops, 0, sizeof(uart_ops));
//	uart_ops.swap = swap_mock_hal_ok;
//	huio.handle->ops = &uart_ops;	
	memset(&huio.handle->ops, 0, sizeof(struct UARTEX_Operations));
	huio.handle->ops.swap = swap_mock_hal_ok;
	
	memset(&td, 0, sizeof(td));	
	huio.handle->testdata = &td;
	fill_rx_testdata(&huio, 0, 1, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);
	
	memset(buf, 0, sizeof(buf));
	read = uart_device_read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft) + strlen(hard), read);
	TEST_ASSERT_EQUAL_STRING("testdriven", buf);
}

static HAL_StatusTypeDef swap_mock_hal_error(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_ERROR;
}

static HAL_StatusTypeDef swap_mock_hal_busy(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_BUSY;
}

static HAL_StatusTypeDef swap_mock_hal_timeout(UARTEX_HandleTypeDef* h, uint8_t* buf, size_t size, uint32_t* m0ar, int* ndtr) 
{
	return HAL_TIMEOUT;
}

TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferNotEmpty)
{
	const char soft[] = "test";
	const char hard[] = "driven";
	char buf[64];
	int read;

	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
//	struct UARTEX_Operations uart_ops;
	
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;

//	memset(&uart_ops, 0, sizeof(uart_ops));
//	huio.handle->ops = &uart_ops;	
	memset(&huio.handle->ops, 0, sizeof(huio.handle->ops));
	
	memset(&td, 0, sizeof(td));	
	huio.handle->testdata = &td;
	
	// fill and mock
	fill_rx_testdata(&huio, 0, 1, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);	
	// uart_ops.swap = swap_mock_hal_error; // swap_mock_hal_ok;
	huio.handle->ops.swap = swap_mock_hal_error;
	
	memset(buf, 0, sizeof(buf));	
	read = uart_device_read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);
	
	// refill and remock
	fill_rx_testdata(&huio, 1, 1, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);	
	// uart_ops.swap = swap_mock_hal_busy;
	huio.handle->ops.swap = swap_mock_hal_busy;
	
	memset(buf, 0, sizeof(buf));	
	read = uart_device_read(&huio, buf, sizeof(buf));

	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);
	
	// refill and remock
	fill_rx_testdata(&huio, 1, 2, soft, strlen(soft), hard, strlen(hard), UART_IO_BUFFER_SIZE);	
	// uart_ops.swap = swap_mock_hal_timeout;
	huio.handle->ops.swap = swap_mock_hal_timeout;
	
	memset(buf, 0, sizeof(buf));	
	read = uart_device_read(&huio, buf, sizeof(buf));

	TEST_ASSERT_EQUAL(strlen(soft), read);
	TEST_ASSERT_EQUAL_STRING(soft, buf);	
}

TEST(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferEmpty)
{
	/** This case says if upper buffer is empty and HAL in error state, the function should return error **/
	char buf[64];
	int read;
	
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
//	struct UARTEX_Operations uart_ops;
	
	char rxbuf0[64];
	char rxbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.rbuf[0] = rxbuf0;
	huio.rbuf[1] = rxbuf1;

//	memset(&uart_ops, 0, sizeof(uart_ops));
//	huio.handle->ops = &uart_ops;	
	memset(&huio.handle->ops, 0, sizeof(huio.handle->ops));
	
	memset(&td, 0, sizeof(td));	
	huio.handle->testdata = &td;
	
	fill_rx_testdata(&huio, 1, 1, 0, 0, 0, 0, 0);
	huio.handle->ops.swap = swap_mock_hal_error;
	
	memset(buf, 0, sizeof(buf));
	read = uart_device_read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EINVAL, errno);	
	
	fill_rx_testdata(&huio, 1, 1, 0, 0, 0, 0, 0);
	huio.handle->ops.swap = swap_mock_hal_busy;
	
	memset(buf, 0, sizeof(buf));
	read = uart_device_read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EBUSY, errno);	

	fill_rx_testdata(&huio, 1, 1, 0, 0, 0, 0, 0);
	huio.handle->ops.swap = swap_mock_hal_timeout;
	
	memset(buf, 0, sizeof(buf));
	read = uart_device_read(&huio, buf, sizeof(buf));
	
	TEST_ASSERT_EQUAL(-1, read);
	TEST_ASSERT_EQUAL(EIO, errno);	
}

///////////////////////////////////////////////////////////////////////////////
#if 0 // open tests commented out
/******************************************************************************
 * 
 * Test Cases for USART_IO_Open
 *
 *****************************************************************************/

//TEST(UsartIO_DMA, OpenInvalidArgs)
//{
//	TEST_ASSERT_EQUAL(0, uart_device_open(0));
//	TEST_ASSERT_EQUAL(EINVAL, errno);
//	TEST_ASSERT_EQUAL(0, uart_device_open(7));
//	TEST_ASSERT_EQUAL(EINVAL, errno);
//}

//TEST(UsartIO_DMA, OpenPortUnavailable)	/** handle does not exist **/
//{
//	UART_IO_SetHandle(6, 0);
//	
//	TEST_ASSERT_EQUAL(0, uart_device_open(6));
//	TEST_ASSERT_EQUAL(ENODEV, errno);
//}

//TEST(UsartIO_DMA, OpenInvalidHandle)
//{
//	m_huio.handle = 0;
//	UART_IO_SetHandle(6, &m_huio);
//	TEST_ASSERT_EQUAL(0, uart_device_open(6));
//}

//TEST(UsartIO_DMA, OpenHalNotResetState)
//{
//	m_huio.handle->State = HAL_UART_STATE_READY;
//	
//	UART_IO_SetHandle(6, &m_huio);
//	
//	TEST_ASSERT_EQUAL(0, uart_device_open(6));
//	TEST_ASSERT_EQUAL(EBUSY, errno);
//}

//TEST(UsartIO_DMA, OpenHalInitOkReceiveOk)
//{
//	UART_IO_SetHandle(6, &m_huio);
//	usart_apis.HAL_UART_Init = m_init_hal_ok;
//	usart_apis.HAL_UART_Receive_DMA = m_receive_hal_ok;
//	
//	TEST_ASSERT_EQUAL(&m_huio, uart_device_open(6));
//	
//	/** should we check HAL state? I don't think so, cause
//	the function trust the return value, rather than 
//	checking state **/
//	
//	/** all following should be checked since they ARE
//	the results of action of the function under test **/
//	TEST_ASSERT_TRUE(m_rx_m0ar == (uint32_t)m_huio.rbuf[0]);
//	TEST_ASSERT_TRUE(m_rx_ndtr == UART_IO_BUFFER_SIZE);
//	
//	TEST_ASSERT_TRUE(m_huio.rx_upper == m_huio.rbuf[1]);
//	TEST_ASSERT_TRUE(m_huio.rx_tail == &m_huio.rx_upper[UART_IO_BUFFER_SIZE]);
//	TEST_ASSERT_TRUE(m_huio.rx_head == m_huio.rx_tail);
//	
//	TEST_ASSERT_TRUE(m_huio.tx_head == &m_huio.tbuf[1][0]);
//	TEST_ASSERT_TRUE(m_huio.tx_tail == m_huio.tx_head);
//}
#endif

///////////////////////////////////////////////////////////////////////////////
// Write
//
TEST(UsartIO_DMA, WriteInvalidArgs)
{
	char c;
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	
	TEST_ASSERT_EQUAL(-1, uart_device_write(0, &c, 1));
	TEST_ASSERT_EQUAL(EINVAL, errno);
	TEST_ASSERT_EQUAL(-1, uart_device_write(&huio, 0, 1));
	TEST_ASSERT_EQUAL(EINVAL, errno);
	
	/** no effect for errno**/
	/** may be changed future, man page write **/
	TEST_ASSERT_EQUAL(0, uart_device_write(&huio, &c, 0)); 
}

TEST(UsartIO_DMA, WriteHalStateTimeoutErrorReset)
{
	// HAL_UART_STATE_TIMEOUT means hardware error.
	// HAL_UART_STATE_RESET should not happen.
	// HAL_UART_STATE_ERROR are not used in firmware 1.1
	char c;
	UARTEX_HandleTypeDef huartex;
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	huio.handle = &huartex;
	
	errno = 0;
	huio.handle->huart.State = HAL_UART_STATE_TIMEOUT;
	TEST_ASSERT_EQUAL(-1, uart_device_write(&huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);
	
	errno = 0;
	huio.handle->huart.State = HAL_UART_STATE_RESET;
	TEST_ASSERT_EQUAL(-1, uart_device_write(&huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);

	errno = 0;
	huio.handle->huart.State = HAL_UART_STATE_ERROR;
	TEST_ASSERT_EQUAL(-1, uart_device_write(&huio, &c, 1));
	TEST_ASSERT_EQUAL(EIO, errno);
}


static HAL_StatusTypeDef send_mock_hal_ok(UARTEX_HandleTypeDef *huartex, uint8_t *pData, uint16_t Size)
{
	uio_testdata_t* td = huartex->testdata;
	
//	memset(tx_dma_buffer, 0, UART_IO_BUFFER_SIZE);
//	memmove(tx_dma_buffer, pData, Size);
	
//	memset(td->txdma_buf, 0, UART_IO_BUFFER_SIZE);
//	memmove(td->txdma_buf, pData, Size);
	
	td->txdma_buf = (char*)pData;
		
//	m_tx_m0ar = (uint32_t)pData;
//	m_tx_ndtr = Size;
	
	td->tx_m0ar = (uint32_t)pData;
	td->tx_ndtr = Size;
	
	return HAL_OK;
}

static HAL_StatusTypeDef send_mock_hal_busy(UARTEX_HandleTypeDef *huartex, uint8_t *pData, uint16_t Size)
{
	return HAL_BUSY;
}

TEST(UsartIO_DMA, WriteBufferSpaceAdequateAndHalReady)	// write 
{
	char s1[] = "test";
	char s2[] = "driven";
	char s3[] = "testdriven";
	int written;
	
	///////////////////////////
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
//	struct UARTEX_Operations uart_ops;
	
	char txbuf0[64];
	char txbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.tbuf[0] = txbuf0;
	huio.tbuf[1] = txbuf1;

//	memset(&uart_ops, 0, sizeof(uart_ops));
//	huio.handle->ops = &uart_ops;	
	memset(&huio.handle->ops, 0, sizeof(huio.handle->ops));
	
	memset(&td, 0, sizeof(td));	
	huio.handle->testdata = &td;	
	//////////////////////////////
	
	// HAL_UART_Transmit_DMA

	fill_tx_testdata(&huio, 1, s1, strlen(s1), 0, 0, 0);
	// uart_ops.send = send_mock_hal_ok;
	huio.handle->ops.send = send_mock_hal_ok;
	huio.handle->huart.State = HAL_UART_STATE_READY;
	
	written = uart_device_write(&huio, s2, strlen(s2));
	
	TEST_ASSERT_EQUAL(strlen(s2), written);
	TEST_ASSERT_EQUAL_MEMORY(s3, td.txdma_buf, strlen(s3));
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[1], (char*)td.tx_m0ar);
	TEST_ASSERT_EQUAL(strlen(s3), td.tx_ndtr);
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[0], huio.tx_head);
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[0], huio.tx_tail);
	
}

TEST(UsartIO_DMA, WriteBufferSpaceAdequateAndHalBusy)
{
	char s1[] = "test";
	char s2[] = "driven";
	char s3[] = "testdriven";
	int written;
	
	///////////////////////////
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
//	struct UARTEX_Operations uart_ops;
	
	char txbuf0[64];
	char txbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.tbuf[0] = txbuf0;
	huio.tbuf[1] = txbuf1;

//	memset(&uart_ops, 0, sizeof(uart_ops));
//	huio.handle->ops = &uart_ops;	
	memset(&huio.handle->ops, 0, sizeof(huio.handle->ops));
	
	memset(&td, 0, sizeof(td));	
	huio.handle->testdata = &td;	
	//////////////////////////////

	// uart_ops.send = send_mock_hal_busy;
	huio.handle->ops.send = send_mock_hal_busy;
	hue.huart.State = HAL_UART_STATE_READY;
	fill_tx_testdata(&huio, 1, s1, strlen(s1), 0, 0, 0);
	
	written = uart_device_write(&huio, s2, strlen(s2));
	
	TEST_ASSERT_EQUAL(strlen(s2), written);
	TEST_ASSERT_EQUAL_MEMORY(s3, huio.tbuf[1], strlen(s3));
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[1], huio.tx_head);
	TEST_ASSERT_EQUAL(strlen(s3), huio.tx_tail - huio.tx_head);
	
//	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_busy;
//	fill_tx_upper(1, s1, strlen(s1));
//	m_huio.handle->huart.State = HAL_UART_STATE_READY;	/** otherwise trapped **/
//	
//	written = uart_device_write(&m_huio, s2, strlen(s2));

//	TEST_ASSERT_EQUAL(strlen(s2), written);
//	
//	TEST_ASSERT_EQUAL_MEMORY(s3, m_huio.tbuf[1], strlen(s3));
//	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], m_huio.tx_head);
//	TEST_ASSERT_EQUAL(strlen(s3), m_huio.tx_tail - m_huio.tx_head);
}

TEST(UsartIO_DMA, WriteBufferSpaceInadequateAndHalReady)
{
	char s1[UART_IO_BUFFER_SIZE/2];
	char s2[UART_IO_BUFFER_SIZE]; 
	char s3[UART_IO_BUFFER_SIZE*2];
	int written;

	///////////////////////////
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
//	struct UARTEX_Operations uart_ops;
	
	char txbuf0[64];
	char txbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.tbuf[0] = txbuf0;
	huio.tbuf[1] = txbuf1;

//	memset(&uart_ops, 0, sizeof(uart_ops));
//	huio.handle->ops = &uart_ops;	
	memset(&huio.handle->ops, 0, sizeof(huio.handle->ops));
	
	memset(&td, 0, sizeof(td));	
	huio.handle->testdata = &td;	
	//////////////////////////////	
	
	memset(s1, 'a', sizeof(s1));
	memset(s2, 'b', sizeof(s2));
	memset(s3, 0, sizeof(s3));
	memset(s3, 'a', sizeof(s1));
	memset(&s3[sizeof(s1)], 'b', sizeof(s2));
	
	// uart_ops.send = send_mock_hal_ok;
	huio.handle->ops.send = send_mock_hal_ok;
	hue.huart.State = HAL_UART_STATE_READY;
	fill_tx_testdata(&huio, 1, s1, sizeof(s1), 0, 0, 0);
	
	written = uart_device_write(&huio, s2, sizeof(s2));
	
	TEST_ASSERT_EQUAL(sizeof(s2), written);
	
	/** the dma buffer **/
	TEST_ASSERT_EQUAL_MEMORY(s3, td.txdma_buf, UART_IO_BUFFER_SIZE);
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[1], (char*)td.tx_m0ar);
	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE, td.tx_ndtr);
	
	/** the upper buffer **/
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[0], huio.tx_head);
	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE - UART_IO_BUFFER_SIZE/2, huio.tx_tail - huio.tx_head);
	TEST_ASSERT_EQUAL_MEMORY(&s3[UART_IO_BUFFER_SIZE], huio.tx_head, UART_IO_BUFFER_SIZE/2);
	
//	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_ok;
//	fill_tx_upper(1, s1, sizeof(s1));
//	m_huio.handle->huart.State = HAL_UART_STATE_READY;	/** otherwise trapped **/
//	
//	written = uart_device_write(&m_huio, s2, sizeof(s2));

//	TEST_ASSERT_EQUAL(sizeof(s2), written);
//	
//	TEST_ASSERT_EQUAL_MEMORY(s3, tx_dma_buffer, UART_IO_BUFFER_SIZE);
//	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], (char*)m_tx_m0ar);
//	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE, m_tx_ndtr);
//	
//	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[0], m_huio.tx_head);
//	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE - UART_IO_BUFFER_SIZE/2, m_huio.tx_tail-m_huio.tx_head);
//	TEST_ASSERT_EQUAL_MEMORY(&s3[UART_IO_BUFFER_SIZE], m_huio.tx_head, UART_IO_BUFFER_SIZE/2);
}

TEST(UsartIO_DMA, WriteBufferSpaceInadequateAndHalBusy)
{
	char s1[UART_IO_BUFFER_SIZE/2];
	char s2[UART_IO_BUFFER_SIZE]; 
	char s3[UART_IO_BUFFER_SIZE*2];
	int written;
	
	memset(s1, 'a', sizeof(s1));
	memset(s2, 'b', sizeof(s2));
	memset(s3, 0, sizeof(s3));
	memset(s3, 'a', sizeof(s1));
	memset(&s3[sizeof(s1)], 'b', sizeof(s2));
	
	///////////////////////////
	struct uart_device huio = { .rbuf_size = 64, .tbuf_size = 64, };
	UARTEX_HandleTypeDef hue;
	uio_testdata_t td;
	
	char txbuf0[64];
	char txbuf1[64];
	
	memset(&huio, 0, sizeof(huio));
	memset(&hue, 0, sizeof(hue));
	hue.huart.State = HAL_UART_STATE_RESET;
	huio.handle = &hue;
	huio.tbuf[0] = txbuf0;
	huio.tbuf[1] = txbuf1;

//	memset(&uart_ops, 0, sizeof(uart_ops));
//	huio.handle->ops = &uart_ops;	
	memset(&huio.handle->ops, 0, sizeof(huio.handle->ops));
	
	memset(&td, 0, sizeof(td));	
	huio.handle->testdata = &td;	
	//////////////////////////////	
	// uart_ops.send = send_mock_hal_busy;
	huio.handle->ops.send = send_mock_hal_busy;
	hue.huart.State = HAL_UART_STATE_READY;
	fill_tx_testdata(&huio, 1, s1, sizeof(s1), 0, 0, 0);
	
	written = uart_device_write(&huio, s2, sizeof(s2));	
	
//	usart_apis.HAL_UART_Transmit_DMA = m_transmit_hal_busy;
//	fill_tx_upper(1, s1, sizeof(s1));
//	m_huio.handle->huart.State = HAL_UART_STATE_READY;	/** otherwise trapped **/
//	
//	written = uart_device_write(&m_huio, s2, sizeof(s2));

	TEST_ASSERT_EQUAL(UART_IO_BUFFER_SIZE - sizeof(s1), written);
	TEST_ASSERT_EQUAL_HEX32(huio.tbuf[1], huio.tx_head);
	TEST_ASSERT_EQUAL_HEX32(&huio.tbuf[1][UART_IO_BUFFER_SIZE], huio.tx_tail);	// full
	TEST_ASSERT_EQUAL_MEMORY(s3, huio.tx_head, UART_IO_BUFFER_SIZE);						// content
	
//	TEST_ASSERT_EQUAL_HEX32(m_huio.tbuf[1], m_huio.tx_head);
//	TEST_ASSERT_EQUAL_HEX32(&m_huio.tbuf[1][UART_IO_BUFFER_SIZE], m_huio.tx_tail);
//	TEST_ASSERT_EQUAL_MEMORY(s3, m_huio.tx_head, UART_IO_BUFFER_SIZE);	
}

///////////////////////////////////////////////////////////////////////////////
// Open Test
// 1. all success
// 2. create uartex handle failed
// 3. uartex init failed (NO this case !)
// 4. malloc failed x 4
// 5. recv failed (NO this case !)
UARTEX_HandleTypeDef* mock_msp_create_uartex_handle_by_port(struct msp_factory * msp, int num)
{
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	
	td->msp_create_uart_handle_by_port_called++;
	
	// assert arguments
	TEST_ASSERT_EQUAL_PTR(&td->msp, msp);
	TEST_ASSERT_EQUAL(td->udev.dev.number, num);
	
	if (td->msp_create_uart_handle_by_port_fail_countdown == 0)
	{
		return NULL;
	}
	
	td->msp_create_uart_handle_by_port_fail_countdown--;
	return &td->uartex_handle;
}

void mock_msp_destroy_uartex_handle(struct msp_factory* msp, UARTEX_HandleTypeDef* handle)
{
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	
	td->msp_destroy_uartex_handle_called++;
	
	TEST_ASSERT_EQUAL_PTR(&td->msp, msp);
	TEST_ASSERT_EQUAL_PTR(&td->uartex_handle, handle);
	
	if (td->msp_destroy_uartex_handle_fail_countdown == 0)
		return;
	
	return;
}

HAL_StatusTypeDef mock_uartex_handle_init(UARTEX_HandleTypeDef * hue)
{
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	
	td->uartex_handle_init_called++;
	
	TEST_ASSERT_EQUAL(&td->uartex_handle, hue);
	
	if (td->uartex_handle_init_fail_countdown == 0)
		return HAL_ERROR;	// the right one?
	
	td->uartex_handle_init_fail_countdown--;
	return HAL_OK;
}




HAL_StatusTypeDef mock_uartex_handle_deinit(UARTEX_HandleTypeDef * hue)
{
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	
	td->uartex_handle_deinit_called++;
	
	TEST_ASSERT_EQUAL_HEX32(&td->uartex_handle, hue);
	
	if (td->uartex_handle_deinit_fail_countdown == 0)
	{
		return HAL_ERROR;
	}
	
	td->uartex_handle_deinit_fail_countdown--;
	return HAL_OK;
}

HAL_StatusTypeDef mock_uartex_handle_recv(UARTEX_HandleTypeDef * hue, uint8_t *pData, uint16_t Size)	
{
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	
	td->uartex_handle_recv_called++;
	
	TEST_ASSERT_EQUAL_HEX32(&td->uartex_handle, hue);
	TEST_ASSERT_EQUAL_HEX32(td->udev.rbuf[0], pData);
	TEST_ASSERT_EQUAL_HEX32(td->udev.rbuf_size, Size);
	
	if (td->uartex_handle_recv_fail_countdown == 0)
	{
		return HAL_ERROR;
	}
	
	td->uartex_handle_recv_fail_countdown--;
	return HAL_OK;
}

static void assert_device_and_file_intact(struct uart_device * udev, struct file * filp)
{
	TEST_ASSERT_NULL(udev->handle);
	TEST_ASSERT_NULL(udev->rbuf[0]);
	TEST_ASSERT_NULL(udev->rbuf[1]);
	TEST_ASSERT_NULL(udev->rx_upper);
	TEST_ASSERT_NULL(udev->rx_head);
	TEST_ASSERT_NULL(udev->rx_tail);
	TEST_ASSERT_NULL(udev->tbuf[0]);
	TEST_ASSERT_NULL(udev->tbuf[1]);
	TEST_ASSERT_NULL(udev->tx_head);
	TEST_ASSERT_NULL(udev->tx_tail);
	TEST_ASSERT_EQUAL(0, udev->open_count);
	TEST_ASSERT_NULL(filp->private_data);
	TEST_ASSERT_NULL(filp->f_ops);
}
///////////////////////////////////////////////////////////////////////////////
// The func should 
// 1) create uartex handle
// 2) init uart with the handle
// 3) if OK, allocate and init buffer
// 4) start to receive
// 5) init file 
TEST(UsartIO_DMA, OpenWhenDeviceNotOpenedAllSuccess)
{	
	int ret;
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	TEST_ASSERT_NOT_NULL(td);
	
	td->msp.create_uartex_handle_by_port = mock_msp_create_uartex_handle_by_port;
	td->uartex_handle.ops.init = mock_uartex_handle_init;
	td->uartex_handle.ops.recv = mock_uartex_handle_recv;
		
	ret = uart_device_open(&td->udev.dev, &td->file);
	
	TEST_ASSERT_EQUAL(1, td->msp_create_uart_handle_by_port_called);
	TEST_ASSERT_EQUAL(1, td->uartex_handle_init_called);
	TEST_ASSERT_EQUAL(1, td->uartex_handle_recv_called);

	/////////////////////////////////////////////////////////////////////////////
	// handle created.
	TEST_ASSERT_EQUAL(0, ret);																	// return success
	TEST_ASSERT_NOT_NULL(td->udev.handle);											// udev handle set
	TEST_ASSERT_EQUAL_HEX32(&td->uartex_handle, td->udev.handle);		// udev handle mocked
	
	/////////////////////////////////////////////////////////////////////////////
	// hal inited. this test is pointless since the state is set by mock if being tested.
	// TEST_ASSERT_EQUAL(HAL_UART_STATE_READY, td->udev.handle->huart.State);
	
	/////////////////////////////////////////////////////////////////////////////
	// device internals
	TEST_ASSERT_MEMSIZE(td->udev.rbuf_size, td->udev.rbuf[0]);
	TEST_ASSERT_MEMSIZE(td->udev.rbuf_size, td->udev.rbuf[1]);
		
	TEST_ASSERT_EQUAL_HEX32(td->udev.rbuf[1], td->udev.rx_upper);		// rbuf 1 as upper, 0 as dma
	TEST_ASSERT_EQUAL_HEX32(td->udev.rbuf[1], td->udev.rx_head);
	TEST_ASSERT_EQUAL_HEX32(td->udev.rbuf[1], td->udev.rx_tail);	
	
	TEST_ASSERT_MEMSIZE(td->udev.tbuf_size, td->udev.tbuf[0]);
	TEST_ASSERT_MEMSIZE(td->udev.tbuf_size, td->udev.tbuf[1]);	
	
	TEST_ASSERT_EQUAL_HEX32(td->udev.tbuf[1], td->udev.tx_head);
	TEST_ASSERT_EQUAL_HEX32(td->udev.tbuf[1], td->udev.tx_tail);
	
	TEST_ASSERT_EQUAL(1, td->udev.open_count);
	
	/////////////////////////////////////////////////////////////////////////////
	// file initialized
	TEST_ASSERT_EQUAL(&td->udev, td->file.private_data);
	TEST_ASSERT_EQUAL(td->udev.dev.f_ops, td->file.f_ops);
	
	/////////////////////////////////////////////////////////////////////////////
	// clean up
	free(td->udev.rbuf[0]);
	free(td->udev.rbuf[1]);
	free(td->udev.tbuf[0]);
	free(td->udev.tbuf[1]);
}

TEST(UsartIO_DMA, OpenWhenDeviceNotOpenedCreateHandleFail)
{
	int ret;
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	TEST_ASSERT_NOT_NULL(td);
	
	td->msp_create_uart_handle_by_port_fail_countdown = 0;
	td->msp.create_uartex_handle_by_port = mock_msp_create_uartex_handle_by_port;
	
	ret = uart_device_open(&td->udev.dev, &td->file);
	
	TEST_ASSERT_EQUAL(-EFATAL, ret);
	TEST_ASSERT_EQUAL(1, td->msp_create_uart_handle_by_port_called);
	assert_device_and_file_intact(&td->udev, &td->file);
}

TEST(UsartIO_DMA, OpenWhenDeviceNotOpenedMallocFail)
{
	
	int ret;
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	TEST_ASSERT_NOT_NULL(td);
	
	// keep in mind that all ops for uartex handle and msp are removed in SETUP,
	// hence no need to worry anything are called before all malloc done.
	
	UnityMalloc_MakeMallocFailAfterCount(0);
	ret = uart_device_open(&td->udev.dev, &td->file);
	TEST_ASSERT_EQUAL(-ENOMEM, ret);											// return ENOMEM
	assert_device_and_file_intact(&td->udev, &td->file);

	UnityMalloc_MakeMallocFailAfterCount(1);
	ret = uart_device_open(&td->udev.dev, &td->file);
	TEST_ASSERT_EQUAL(-ENOMEM, ret);											// return ENOMEM
	assert_device_and_file_intact(&td->udev, &td->file);	
	
	UnityMalloc_MakeMallocFailAfterCount(2);
	ret = uart_device_open(&td->udev.dev, &td->file);
	TEST_ASSERT_EQUAL(-ENOMEM, ret);											// return ENOMEM
	assert_device_and_file_intact(&td->udev, &td->file);
	
	UnityMalloc_MakeMallocFailAfterCount(3);
	ret = uart_device_open(&td->udev.dev, &td->file);
	TEST_ASSERT_EQUAL(-ENOMEM, ret);											// return ENOMEM
	assert_device_and_file_intact(&td->udev, &td->file);	
}

///////////////////////////////////////////////////////////////////////////////
//	Release
TEST(UsartIO_DMA, Release)
{
	int ret;
	struct uart_device_testdata * td = (struct uart_device_testdata *)get_testdata();
	TEST_ASSERT_NOT_NULL(td);	
	
	td->msp.create_uartex_handle_by_port = mock_msp_create_uartex_handle_by_port;
	td->msp.destroy_uartex_handle = mock_msp_destroy_uartex_handle;
	td->uartex_handle.ops.init = mock_uartex_handle_init;
	td->uartex_handle.ops.deinit = mock_uartex_handle_deinit;
	td->uartex_handle.ops.recv = mock_uartex_handle_recv;
	
	uart_device_open(&td->udev.dev, &td->file);
	
	ret = uart_device_release(&td->udev.dev, &td->file);
	
	TEST_ASSERT_EQUAL(0, ret);
	TEST_ASSERT_EQUAL(1, td->uartex_handle_deinit_called);
	TEST_ASSERT_EQUAL(1, td->msp_destroy_uartex_handle_called);
	
	assert_device_and_file_intact(&td->udev, &td->file);
}

/******************************************************************************
 *
 * Group Runner
 *
 *****************************************************************************/
TEST_GROUP_RUNNER(UsartIO_DMA)
{
	struct uart_device_testdata testdata = {
		.file = {0},
		.msp = {0},
		.uartex_handle = {0},
		.udev = {
			.dev = { .name = "UART2", .number = 2, },
			.msp = &testdata.msp,
			.rbuf_size = 13,
			.tbuf_size = 17,
		},
	};

	set_testdata(&testdata);	
	
	// args invalid
	// args valid, read n bytes, n <= bytes in upper buffer
	// args valid, n > bytes in upper buffer, and HAL READY
	// args valid, n > bytes in upper buffer, and HAL not READY and upper buffer not empty
	// args valid, n > bytes in upper buffer, and HAL not READY adn upper buffer empty
	RUN_TEST_CASE(UsartIO_DMA, ReadInvalidArgs);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadLessThanOrEqualToBytesInBuffer);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferNotEmpty);	
	RUN_TEST_CASE(UsartIO_DMA, ReadWhenBytesToReadMoreThanBytesInBufferAndHalErrorBusyTimeoutAndBufferEmpty);	
	
	// args invalid
	// args valid, hal state not continuable
	// otherwise, whether (upper) buffer space has enough space or not
	// and whether hal tx state are busy (tx_busy & tx_rx_busy are busy, 
	// ready and rx_busy are not (tx) busy) are orthogonal (parallel) states.
	// no need to further consider HAL_UART_Transmit_DMA return values.
	// This function returns only HAL_OK when tx not busy and HAL_BUSY when tx busy.
	// (except invalid args, HAL_ERROR, we make sure this won't happen.)
	RUN_TEST_CASE(UsartIO_DMA, WriteInvalidArgs);
	RUN_TEST_CASE(UsartIO_DMA, WriteHalStateTimeoutErrorReset);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceAdequateAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceAdequateAndHalBusy);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceInadequateAndHalReady);
	RUN_TEST_CASE(UsartIO_DMA, WriteBufferSpaceInadequateAndHalBusy);
	

	// open test	
	RUN_TEST_CASE(UsartIO_DMA, OpenWhenDeviceNotOpenedCreateHandleFail);
	RUN_TEST_CASE(UsartIO_DMA, OpenWhenDeviceNotOpenedMallocFail);	
	RUN_TEST_CASE(UsartIO_DMA, OpenWhenDeviceNotOpenedAllSuccess);
	// release
	RUN_TEST_CASE(UsartIO_DMA, Release);
	
	set_testdata(NULL);
}

