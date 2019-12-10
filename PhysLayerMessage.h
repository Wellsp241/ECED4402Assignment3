/*
 * @file    PhysLayerMessage.h
 * @brief   Contains information necessary to form and process the physical
 *          layer message format
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    8-Dec-2019 (created)
 * @date    9-Dec-2019 (edited)
 */
#pragma once
#include "DataLinkMessage.h"

/* Define number of bytes added to message by physical layer */
#define NUMPHYSICALBYTES    (3)

/* Define physical layer mailboxes */
#define UART1PHYSMB     (6)
#define DATALINKPHYSMB  (7)

/* Define start, end and data link escape bytes */
#define STX         (0x02)
#define ETX         (0x03)
#define DLE         (0x10)

void PhysLayerFromDLHandler(void);
void PhysLayerFromUART1Handler(void);

