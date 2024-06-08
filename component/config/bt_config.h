#ifndef BT_CONFIG_H_H_H
#define BT_CONFIG_H_H_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

/********************** DEBUG CONFIG **********************/
#define BT_DEBUG printf
#define BT_TRACE_COLOR 1
#define BT_HEX_TRACE_LEVEL 1
#define ZMP_TRACE_LEVEL 4
#define LL_PREPARSE_TRACE_LEVEL 0
#define LL_TRACE_LEVEL 4

/******************** GFSK buffer size *******************/
// be careful !!!!!!!!!!!!!!!!!!!!!, need power of 2
#define BT_GFSK_RING_BUF_SIZE (64*1024) 


#endif