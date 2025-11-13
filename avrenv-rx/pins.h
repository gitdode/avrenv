/* 
 * File:   pins.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 18. Mai 2025
 */

#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define I2C_SDA_PA2     PIN2
#define I2C_SCL_PA3     PIN3
#define MOSI_PA4        PIN4
#define MISO_PA5        PIN5
#define SCK_PA6         PIN6
#define SS_PA7          PIN7
#define USART_TX_PC0    PIN0
#define USART_RX_PC1    PIN1
#define RFM_RST_PD0     PIN0
#define RFM_CS_PD1      PIN1
#define RFM_INT0_PD2    PIN2
#define RFM_INT1_PD3    PIN3
#define TFT_CS_PD4      PIN4
#define TFT_DC_PD5      PIN5
#define TFT_RST_PD6     PIN6

#endif /* PINS_H */

