/*
 * include/asm-arm/arch-at91rm9200/gpio.h
 *
 *  Copyright (C) 2005 HP Labs
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __AT91_GPIO_H__
#define __AT91_GPIO_H__

#define PIN_BASE		32

#define MAX_GPIO_BANKS		3

/* these pin numbers double as IRQ numbers, like AT91xxx_ID_* values */

#define	AT91_PIN_PA0	(PIN_BASE + 0x00 + 0)
#define	AT91_PIN_PA1	(PIN_BASE + 0x00 + 1)
#define	AT91_PIN_PA2	(PIN_BASE + 0x00 + 2)
#define	AT91_PIN_PA3	(PIN_BASE + 0x00 + 3)
#define	AT91_PIN_PA4	(PIN_BASE + 0x00 + 4)
#define	AT91_PIN_PA5	(PIN_BASE + 0x00 + 5)
#define	AT91_PIN_PA6	(PIN_BASE + 0x00 + 6)
#define	AT91_PIN_PA7	(PIN_BASE + 0x00 + 7)
#define	AT91_PIN_PA8	(PIN_BASE + 0x00 + 8)
#define	AT91_PIN_PA9	(PIN_BASE + 0x00 + 9)
#define	AT91_PIN_PA10	(PIN_BASE + 0x00 + 10)
#define	AT91_PIN_PA11	(PIN_BASE + 0x00 + 11)
#define	AT91_PIN_PA12	(PIN_BASE + 0x00 + 12)
#define	AT91_PIN_PA13	(PIN_BASE + 0x00 + 13)
#define	AT91_PIN_PA14	(PIN_BASE + 0x00 + 14)
#define	AT91_PIN_PA15	(PIN_BASE + 0x00 + 15)
#define	AT91_PIN_PA16	(PIN_BASE + 0x00 + 16)
#define	AT91_PIN_PA17	(PIN_BASE + 0x00 + 17)
#define	AT91_PIN_PA18	(PIN_BASE + 0x00 + 18)
#define	AT91_PIN_PA19	(PIN_BASE + 0x00 + 19)
#define	AT91_PIN_PA20	(PIN_BASE + 0x00 + 20)
#define	AT91_PIN_PA21	(PIN_BASE + 0x00 + 21)
#define	AT91_PIN_PA22	(PIN_BASE + 0x00 + 22)
#define	AT91_PIN_PA23	(PIN_BASE + 0x00 + 23)
#define	AT91_PIN_PA24	(PIN_BASE + 0x00 + 24)
#define	AT91_PIN_PA25	(PIN_BASE + 0x00 + 25)
#define	AT91_PIN_PA26	(PIN_BASE + 0x00 + 26)
#define	AT91_PIN_PA27	(PIN_BASE + 0x00 + 27)
#define	AT91_PIN_PA28	(PIN_BASE + 0x00 + 28)
#define	AT91_PIN_PA29	(PIN_BASE + 0x00 + 29)
#define	AT91_PIN_PA30	(PIN_BASE + 0x00 + 30)
#define	AT91_PIN_PA31	(PIN_BASE + 0x00 + 31)

#define	AT91_PIN_PB0	(PIN_BASE + 0x20 + 0)
#define	AT91_PIN_PB1	(PIN_BASE + 0x20 + 1)
#define	AT91_PIN_PB2	(PIN_BASE + 0x20 + 2)
#define	AT91_PIN_PB3	(PIN_BASE + 0x20 + 3)
#define	AT91_PIN_PB4	(PIN_BASE + 0x20 + 4)
#define	AT91_PIN_PB5	(PIN_BASE + 0x20 + 5)
#define	AT91_PIN_PB6	(PIN_BASE + 0x20 + 6)
#define	AT91_PIN_PB7	(PIN_BASE + 0x20 + 7)
#define	AT91_PIN_PB8	(PIN_BASE + 0x20 + 8)
#define	AT91_PIN_PB9	(PIN_BASE + 0x20 + 9)
#define	AT91_PIN_PB10	(PIN_BASE + 0x20 + 10)
#define	AT91_PIN_PB11	(PIN_BASE + 0x20 + 11)
#define	AT91_PIN_PB12	(PIN_BASE + 0x20 + 12)
#define	AT91_PIN_PB13	(PIN_BASE + 0x20 + 13)
#define	AT91_PIN_PB14	(PIN_BASE + 0x20 + 14)
#define	AT91_PIN_PB15	(PIN_BASE + 0x20 + 15)
#define	AT91_PIN_PB16	(PIN_BASE + 0x20 + 16)
#define	AT91_PIN_PB17	(PIN_BASE + 0x20 + 17)
#define	AT91_PIN_PB18	(PIN_BASE + 0x20 + 18)
#define	AT91_PIN_PB19	(PIN_BASE + 0x20 + 19)
#define	AT91_PIN_PB20	(PIN_BASE + 0x20 + 20)
#define	AT91_PIN_PB21	(PIN_BASE + 0x20 + 21)
#define	AT91_PIN_PB22	(PIN_BASE + 0x20 + 22)
#define	AT91_PIN_PB23	(PIN_BASE + 0x20 + 23)
#define	AT91_PIN_PB24	(PIN_BASE + 0x20 + 24)
#define	AT91_PIN_PB25	(PIN_BASE + 0x20 + 25)
#define	AT91_PIN_PB26	(PIN_BASE + 0x20 + 26)
#define	AT91_PIN_PB27	(PIN_BASE + 0x20 + 27)
#define	AT91_PIN_PB28	(PIN_BASE + 0x20 + 28)
#define	AT91_PIN_PB29	(PIN_BASE + 0x20 + 29)
#define	AT91_PIN_PB30	(PIN_BASE + 0x20 + 30)
#define	AT91_PIN_PB31	(PIN_BASE + 0x20 + 31)

#define	AT91_PIN_PC0	(PIN_BASE + 0x40 + 0)
#define	AT91_PIN_PC1	(PIN_BASE + 0x40 + 1)
#define	AT91_PIN_PC2	(PIN_BASE + 0x40 + 2)
#define	AT91_PIN_PC3	(PIN_BASE + 0x40 + 3)
#define	AT91_PIN_PC4	(PIN_BASE + 0x40 + 4)
#define	AT91_PIN_PC5	(PIN_BASE + 0x40 + 5)
#define	AT91_PIN_PC6	(PIN_BASE + 0x40 + 6)
#define	AT91_PIN_PC7	(PIN_BASE + 0x40 + 7)
#define	AT91_PIN_PC8	(PIN_BASE + 0x40 + 8)
#define	AT91_PIN_PC9	(PIN_BASE + 0x40 + 9)
#define	AT91_PIN_PC10	(PIN_BASE + 0x40 + 10)
#define	AT91_PIN_PC11	(PIN_BASE + 0x40 + 11)
#define	AT91_PIN_PC12	(PIN_BASE + 0x40 + 12)
#define	AT91_PIN_PC13	(PIN_BASE + 0x40 + 13)
#define	AT91_PIN_PC14	(PIN_BASE + 0x40 + 14)
#define	AT91_PIN_PC15	(PIN_BASE + 0x40 + 15)
#define	AT91_PIN_PC16	(PIN_BASE + 0x40 + 16)
#define	AT91_PIN_PC17	(PIN_BASE + 0x40 + 17)
#define	AT91_PIN_PC18	(PIN_BASE + 0x40 + 18)
#define	AT91_PIN_PC19	(PIN_BASE + 0x40 + 19)
#define	AT91_PIN_PC20	(PIN_BASE + 0x40 + 20)
#define	AT91_PIN_PC21	(PIN_BASE + 0x40 + 21)
#define	AT91_PIN_PC22	(PIN_BASE + 0x40 + 22)
#define	AT91_PIN_PC23	(PIN_BASE + 0x40 + 23)
#define	AT91_PIN_PC24	(PIN_BASE + 0x40 + 24)
#define	AT91_PIN_PC25	(PIN_BASE + 0x40 + 25)
#define	AT91_PIN_PC26	(PIN_BASE + 0x40 + 26)
#define	AT91_PIN_PC27	(PIN_BASE + 0x40 + 27)
#define	AT91_PIN_PC28	(PIN_BASE + 0x40 + 28)
#define	AT91_PIN_PC29	(PIN_BASE + 0x40 + 29)
#define	AT91_PIN_PC30	(PIN_BASE + 0x40 + 30)
#define	AT91_PIN_PC31	(PIN_BASE + 0x40 + 31)


//EZ9260-V2 PIN Define

#define PIN_UART0_TXD		AT91_PIN_PB4
#define PIN_UART0_RXD		AT91_PIN_PB5

#define PIN_UART1_TXD		AT91_PIN_PB6
#define PIN_UART1_RXD		AT91_PIN_PB7

#define PIN_UART2_TXD		AT91_PIN_PB8
#define PIN_UART2_RXD		AT91_PIN_PB9
#define PIN_UART2_RTS		AT91_PIN_PA4
#define PIN_UART2_CTS		AT91_PIN_PA5

#define PIN_UART3_TXD		AT91_PIN_PB10
#define PIN_UART3_RXD		AT91_PIN_PB11

#define PIN_UART4_RXD		AT91_PIN_PA30
#define PIN_UART4_TXD		AT91_PIN_PA31

#define PIN_UART5_TXD		AT91_PIN_PB12
#define PIN_UART5_RXD		AT91_PIN_PB13

#define PIN_I2S_SCK		AT91_PIN_PB16
#define PIN_I2S_FRM		AT91_PIN_PB17
#define PIN_I2S_SDO		AT91_PIN_PB18
#define PIN_I2S_SDI		AT91_PIN_PB19
#define PIN_I2S_MCK		AT91_PIN_PB31

#define PIN_CSI_D0		AT91_PIN_PB20
#define PIN_CSI_D1		AT91_PIN_PB21
#define PIN_CSI_D2		AT91_PIN_PB22
#define PIN_CSI_D3		AT91_PIN_PB23
#define PIN_CSI_D4		AT91_PIN_PB24
#define PIN_CSI_D5		AT91_PIN_PB25
#define PIN_CSI_D6		AT91_PIN_PB26
#define PIN_CSI_D7		AT91_PIN_PB27
#define PIN_CSI_PCK		AT91_PIN_PB28
#define PIN_CSI_VSYNC		AT91_PIN_PB29
#define PIN_CSI_HSYNC		AT91_PIN_PB30

#define PIN_MCI_CMD		AT91_PIN_PA7
#define PIN_MCI_CLK		AT91_PIN_PA8
#define PIN_MCI_DA0		AT91_PIN_PA6
#define PIN_MCI_DA1		AT91_PIN_PA9
#define PIN_MCI_DA2		AT91_PIN_PA10
#define PIN_MCI_DA3		AT91_PIN_PA11
#define PIN_MCI_CD		AT91_PIN_PA22

#define PIN_TIOA0		AT91_PIN_PA26
#define PIN_TIOA1		AT91_PIN_PA27
#define PIN_TIOA2		AT91_PIN_PA28

#define PIN_BZR_CTL		AT91_PIN_PA29

#define PIN_SPX_MISO		AT91_PIN_PB0
#define PIN_SPX_MOSI		AT91_PIN_PB1
#define PIN_SPX_SPCK		AT91_PIN_PB2
#define PIN_SPX_nCS0		AT91_PIN_PB3
#define PIN_SPX_nCS1		AT91_PIN_PC5
#define PIN_SPX_nCS2		AT91_PIN_PC4

#define PIN_ADC_D0		AT91_PIN_PC0
#define PIN_ADC_D1		AT91_PIN_PC1
#define PIN_ADC_D2		AT91_PIN_PC2
#define PIN_ADC_D3		AT91_PIN_PC3

#define PIN_USB_CNX		AT91_PIN_PC6

#define PIN_PIO_D0		AT91_PIN_PC7
#define PIN_PIO_D1		AT91_PIN_PC10

#endif

