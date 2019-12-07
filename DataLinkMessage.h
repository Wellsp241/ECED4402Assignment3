/*
 * @file    DataLinkMessage.h
 * @brief   Contains information necessary to form and process the data link
 *          layer message format
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 * @date    7-Dec-2019 (edited)
 */
#pragma once

/* Define data link layer mailboxes */
#define APPDATALINKMB  (3)
#define PHYSDATALINKMB (4)

/* Enumeration of data link layer control types */
enum DataLinkType
{
  DATA  = 0,
  ACK   = 1,
  NACK  = 2
};

/* Structure of data link control field */
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
  char * message;
} DLMessage;
