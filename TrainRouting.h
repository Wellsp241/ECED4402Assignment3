/*
 * @file    TrainRouting.h
 * @brief   Contains information necessary to process Routing Table entries
 *          This header also contains the state of all trains
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    9-Dec-2019 (created)
 * @date
 */
#pragma once

/* Definition of number of switches on train set alongside
 * indicator of no switches
 */
#define NO_SWITCH       (0)
#define NUM_SWITCHES    (6)

/* Definition of number of sensors on train set */
#define NUM_SENSORS     (24)

/* Definition of possible directions */
#define DIR_CW          (0)
#define DIR_CCW         (1)

/* Definition of possible switch states */
#define SW_DIVERGED     (0)
#define SW_STRAIGHT     (1)

/* Definition of number of trains */
#define NUM_TRAINS      (1)

/* Definition of default train state values */
#define DEFAULT_SPEED       (5)
#define DEFAULT_DESTINATION (1)


/* Structure of routing table entries.
 * dir: 0 -> CW, 1 -> CCW
 * switchnum: index of switch to be thrown (0 to NUM_SWITCHES)
 * switchstate: 0 -> diverged, 1 -> straight
 * stop: 0 -> train does not stop, 1 -> train stops
 */
struct RoutingTableEntry
{
    unsigned dir : 1;
    unsigned switchnum : 3;
    unsigned switchstate : 1;
    unsigned stop : 1;
};


/* Structure of train state */
struct TrainState
{
    unsigned char speed;
    unsigned char destination;
};


extern struct TrainState States[NUM_TRAINS];

/* Function used to get path between two hall sensors */
struct RoutingTableEntry getPath(unsigned char start, unsigned char finish);
