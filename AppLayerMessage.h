/*
 * @file    AppLayerMessage.h
 * @brief   Contains information necessary to form and process the application
 *          layer message format
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    28-Nov-2019 (created)
 * @date    7-Dec-2019 (edited)
 */
#pragma once

/* Define application layer mailboxe */
#define APPLAYERMB  (2)

/* Define all indicator for messages sent from application layer */
#define ALL           (0xFF)

/* Define stop indicator for magnitude/direction set message */
#define STOP          (0)

/* Enumeration of application layer message codes */
enum AppLayerCode
{
  HALL_TRIGGERED      = 0xA0,
  HALL_TRIGGERED_ACK  = 0xA2,
  HALL_RESET_REQUEST  = 0xA8,
  HALL_RESET_ACK      = 0xAA,
  MAG_DIR_SET         = 0xC0,
  MAG_DIR_ACK         = 0xC2,
  SWITCH_THROW        = 0xE0,
  SWITCH_THROW_ACK    = 0xE2
};

/* Structure of magnitude/direction message speed */
struct AppLayerSpeed
{
  unsigned magnitude  : 4;
  unsigned ignored    : 3;
  unsigned direction  : 1;
};

/* Union of unsigned character and AppLayerSpeed */
union Mag_Dir
{
  unsigned char rawByte;
  struct AppLayerSpeed Speed;
};

/* Structure of application layer messages */
typedef struct AppLayerMessage
{
  enum AppLayerCode code;
  unsigned char arg1;
  unsigned char arg2;
} AppMessage;

/* Union of unsigned character pointer and AppMessage pointer */
union AppFromMB
{
    char * recvAddr;
    AppMessage * msgAddr;
};
