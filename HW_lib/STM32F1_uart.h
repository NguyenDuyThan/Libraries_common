/*------------------------------------------------------------------------------------------
 * INTRODUCE
 * Brief: This is header file contain function to setup USART peripherals.
 * Author: Duy Vinh To - Team DinhViSo
 * Detail:
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/
#define	UART1_TX_PIN	GPIO_Pin_9
#define UART1_TX_PORT	GPIOA
#define	UART1_RX_PIN	GPIO_Pin_10
#define UART1_RX_PORT	GPIOA

#define	UART2_TX_PIN	GPIO_Pin_2
#define UART2_TX_PORT	GPIOA
#define	UART2_RX_PIN	GPIO_Pin_3
#define UART2_RX_PORT	GPIOA

#define	UART3_TX_PIN	GPIO_Pin_10
#define UART3_TX_PORT	GPIOB
#define	UART3_RX_PIN	GPIO_Pin_11
#define UART3_RX_PORT	GPIOB

#define	UART4_TX_PIN	GPIO_Pin_10
#define UART4_TX_PORT	GPIOC
#define	UART4_RX_PIN	GPIO_Pin_11
#define UART4_RX_PORT	GPIOC

#define	UART5_TX_PIN	GPIO_Pin_12
#define UART5_TX_PORT	GPIOC
#define	UART5_RX_PIN	GPIO_Pin_2
#define UART5_RX_PORT	GPIOD

/*------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------------------------------------------------------*/
typedef void (*UART_readc_CB)(uint8_t c);
/*------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -------------------------------------------------------------------------------------------*/
extern void UART_setup(uint8_t port, uint32_t br, UART_readc_CB readcCB);
extern void UART_sendc(uint8_t port, uint8_t c);
extern void UART_sends(uint8_t port, const uint8_t *s, uint16_t l);
