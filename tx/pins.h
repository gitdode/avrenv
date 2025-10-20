/* 
 * File:   pins.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 18. Mai 2025
 */

#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define MOSI_PA4        PIN4
#define MISO_PA5        PIN5
#define SCK_PA6         PIN6
#define SS_PA7          PIN7
#define USART_TX_PC0    PIN0
#define USART_RX_PC1    PIN1
#define TH_PWR_PC2      PIN2
#define TH_ADC_PA3      PIN3 // AIN23
#define RFM_RST_PD0     PIN0
#define RFM_CS_PD1      PIN1
#define RFM_INT0_PD2    PIN2
#define RFM_INT1_PD3    PIN3
#define BME_CS_PD4      PIN4

#endif /* PINS_H */

