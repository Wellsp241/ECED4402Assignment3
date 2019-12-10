/*
 * @file    TrainRouting.c
 * @brief   Contains functions used to guide a train from its current position
 *          to its destination
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    9-Dec-2019 (created)
 * @date
 */
#include "TrainRouting.h"
#include "DataLinkMessage.h"

/* Define default switch states */
#define DEFAULT_SWITCH  (0xFF)

/* Container of train state(s) */
//TODO: Make non-global
struct TrainState TState = {{DEFAULT_SPEED, 0, DEFAULT_DIRECTION}, DEFAULT_DESTINATION, DEFAULT_STOP};

/* Switch state register.
 * Each bit corresponds to a different switch (e.g. Bit 2 represents switch 2).
 */
//TODO: Make non-global
unsigned char Switch_States = DEFAULT_SWITCH;


/*
 * @brief   Function used to send instructions on how to go from one
 *          hall sensor the the next on the train track
 * @param   [in] unsigned char start: Index of starting hall sensor
 * @param   [in] unsigned char finish: Index of ending hall sensor
 */
void Go(unsigned char start, unsigned char finish)
{
    /* Routing table used to instruct a train how to go from one location to another */
    static struct RoutingTableEntry RoutingTable[NUM_SENSORS][NUM_SENSORS] =
    {
    /*From      TO      1                                          2                                         3                                          4                                         5                                         6                                         7                                         8                                        9                                        10                                        11                                        12                                        13                                        14                                        15                                        16                                         17                                        18                                       19                                        20                                         21                                        22                                       23                                        24               */
    /* 1 */{{DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP}, {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO}},
    /* 2 */{{DIR_CW, 6, SW_STRAIGHT, PATH_GO},         {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 5, SW_DIVERGED, PATH_GO},       {DIR_CCW, 5, SW_DIVERGED, PATH_GO},       {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO}},
    /* 3 */{{DIR_CW, 6, SW_STRAIGHT, PATH_GO},         {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 5, SW_STRAIGHT, PATH_GO},       {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 5, SW_DIVERGED, PATH_GO},       {DIR_CCW, 5, SW_DIVERGED, PATH_GO},       {DIR_CW, 6, SW_STRAIGHT, PATH_GO},        {DIR_CW, 6, SW_STRAIGHT, PATH_GO}},
    /* 4 */{{DIR_CW, 5, SW_STRAIGHT, PATH_GO},         {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO}},
    /* 5 */{{DIR_CW, 5, SW_STRAIGHT, PATH_GO},         {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CW, 5, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 4, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 4, SW_STRAIGHT, PATH_GO}},
    /* 6 */{{DIR_CW, 4, SW_STRAIGHT, PATH_GO},         {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO}},
    /* 7 */{{DIR_CW, 4, SW_STRAIGHT, PATH_GO},         {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO}},
    /* 8 */{{DIR_CW, 4, SW_STRAIGHT, PATH_GO},         {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO}},
    /* 9 */{{DIR_CW, 4, SW_STRAIGHT, PATH_GO},         {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CW, 4, SW_DIVERGED, PATH_GO},        {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CCW, 3, SW_DIVERGED, PATH_GO},       {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CW, 4, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 3, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 3, SW_STRAIGHT, PATH_GO}},
    /* 10*/{{DIR_CCW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_DIVERGED, PATH_GO},       {DIR_CCW, 2, SW_DIVERGED, PATH_GO}},
    /* 11*/{{DIR_CCW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 2, SW_STRAIGHT, PATH_GO},       {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CW, 3, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 2, SW_DIVERGED, PATH_GO},       {DIR_CCW, 2, SW_DIVERGED, PATH_GO}},
    /* 12*/{{DIR_CCW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO}},
    /* 13*/{{DIR_CCW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 1, SW_STRAIGHT, PATH_GO},       {DIR_CW, 2, SW_STRAIGHT, PATH_GO},        {DIR_CW, 2, SW_STRAIGHT, PATH_GO}},
    /* 14*/{{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO}},
    /* 15*/{{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO}},
    /* 16*/{{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CCW, 6, SW_DIVERGED, PATH_GO},       {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CW, 1, SW_DIVERGED, PATH_GO},        {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CCW, 6, SW_STRAIGHT, PATH_GO},       {DIR_CW, 1, SW_STRAIGHT, PATH_GO},        {DIR_CW, 1, SW_STRAIGHT, PATH_GO}},
    /* 17*/{{DIR_CW, 6, SW_DIVERGED, PATH_GO},         {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO}},
    /* 18*/{{DIR_CW, 6, SW_DIVERGED, PATH_GO},         {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CW, 6, SW_DIVERGED, PATH_GO},        {DIR_CCW, 4, SW_DIVERGED, PATH_GO},       {DIR_CCW, 4, SW_DIVERGED, PATH_GO}},
    /* 19*/{{DIR_CCW, 1, SW_DIVERGED, PATH_GO},        {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO}},
    /* 20*/{{DIR_CCW, 1, SW_DIVERGED, PATH_GO},        {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CCW, 1, SW_DIVERGED, PATH_GO},       {DIR_CW, 3, SW_DIVERGED, PATH_GO},        {DIR_CW, 3, SW_DIVERGED, PATH_GO}},
    /* 21*/{{DIR_CW, 5, SW_DIVERGED, PATH_GO},         {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}, {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO}},
    /* 22*/{{DIR_CW, 5, SW_DIVERGED, PATH_GO},         {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CW, 5, SW_DIVERGED, PATH_GO},        {DIR_CW, 5, SW_DIVERGED, PATH_GO}},
    /* 23*/{{DIR_CW, 2, SW_DIVERGED, PATH_GO},         {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP},{DIR_CCW, NO_SWITCH, NO_SWITCH, PATH_GO}},
    /* 24*/{{DIR_CW, 2, SW_DIVERGED, PATH_GO},         {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, 2, SW_DIVERGED, PATH_GO},        {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_GO},  {DIR_CW, NO_SWITCH, NO_SWITCH, PATH_STOP}}
    };

    /* Reserve space for application layer message */
    char Msg[sizeof(AppMessage)];
    union AppFromMB reply;
    reply.recvAddr = Msg;
    union Mag_Dir replySpeed;
    replySpeed.rawByte = &(reply.msgAddr->arg2);

    /* Get path from start to finish */
    struct RoutingTableEntry * path = &RoutingTable[start][finish];

    /* Check whether train must be stopped */
    if(path->stop == PATH_STOP)
    {
        /* Locomotive is at its destination so it must be stopped */
        reply.msgAddr->code = MAG_DIR_SET;
        reply.msgAddr->arg1 = 0;
        *(replySpeed.rawByte) = STOP;

        DataLinkfromAppHandler(reply.recvAddr);

        /* Update train's state */
        TState.stop = PATH_STOP;
    }
    else
    {
        /* Determine whether any messages need to be sent to adjust the train's course.
         * Start by checking for a needed magnitude/direction set message.
         */
        if((path->dir != TState.speed.direction) || (TState.stop == PATH_STOP))
        {
            /* Direction needs to be changed so send speed change request */
            reply.msgAddr->code = MAG_DIR_SET;
            reply.msgAddr->arg1 = 0;
            replySpeed.Speed->direction = path->dir;
            replySpeed.Speed->magnitude = TState.speed.magnitude;

            DataLinkfromAppHandler(reply.recvAddr);

            /* Update train's state */
            TState.speed.direction = path->dir;
            TState.stop = PATH_GO;
        }

        /* Check whether a switch-throw request must be sent */
        if(((Switch_States & (1 << path->switchnum)) == 0) != path->switchstate)
        {
            /* Build and send switch-throw request message */
            reply.msgAddr->code = SWITCH_THROW;
            reply.msgAddr->arg1 = path->switchnum;
            reply.msgAddr->arg2 = path->switchstate;
            DataLinkfromAppHandler(reply.recvAddr);

            /* Update switch's state */
            if(path->switchstate == SW_DIVERGED)
            {
                Switch_States &= ~(1 << path->switchnum);
            }
            else
            {
                Switch_States |= 1 << path->switchnum;
            }
        }
    }

    return;
}

