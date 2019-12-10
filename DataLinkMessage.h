/*
 * @file    DataLinkMessage.h
 * @brief   Contains information necessary to form and process the data link
 *          layer message format
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 * @date    9-Dec-2019 (edited)
 */
#pragma once
#include "AppLayerMessage.h"

/* Define mailboxes used within data link layer */
#define APPDATALINKMB   (8)
#define PHYSDATALINKMB  (9)

/* Enumeration of data link layer control types */
enum DataLinkType
{
  DATA  = 0,
  ACK   = 1,
  NACK  = 2
};

/* Structure of data link control field */
#pragma pack(1)
typedef struct DataLinkControl
{
  unsigned receivedNum      : 3;
  unsigned sequenceNum      : 3;
  enum DataLinkType type    : 2;
} DLControl;

/* Structure of data link messages */
typedef struct DataLinkMessage
{
  struct DataLinkControl control;
  unsigned char length;
  AppMessage appMessage;
} DLMessage;

/* Union of DLMessage pointer and character pointer */
union DLFromMB
{
    char * recvAddr;
    DLMessage * msgAddr;
};


void DataLinkfromAppHandler(void);
void DataLinkfromPhysHandler(void);
