/* 
 * File:   pa1616s.h
 * Author: torsten.roemer@luniks.net
 *
 * Created on 28.10.2025, 20:47
 */

#ifndef PA1616S_H
#define PA1616S_H

#include "usart.h"

bool pasInit(void);

uint8_t getNmeaMsg(char *data, uint8_t len);

#endif /* PA1616S_H */

