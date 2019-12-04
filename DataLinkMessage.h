/*
 * @file    DataLinkMessage.h
 * @brief   Contains information necessary to form and process the data link
 *          layer message format
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 */
#pragma once

/* Define data link layer mailboxes */
#define APPDATALINKMB  (3)
#define PHYSDATALINKMB (4)

/* Enumeration of data link layer control types */
typedef enum DataLinkType
{
  DATA  = 0,
  ACK   = 1,
  NACK  = 2
}

/* Structure of data link control field */
struct DataLinkControl
{
  unsigned receivedNum      : 3;
  unsigned sequenceNum      : 3;
  enum DataLinkType type    : 2;
} DLControl;

/* Structure of data link messages */
struct DataLinkMessage
{
  struct DataLinkControl control;
  unsigned char length;
  unsigned char * message;
} DLMessage;
