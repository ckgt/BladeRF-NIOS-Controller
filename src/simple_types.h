/*
 * simple_types.h
 *
 *  Created on: Mar 23, 2015
 *      Author: Cameron K.
 */

#ifndef SIMPLE_TYPES_H_
#define SIMPLE_TYPES_H_


#include <stdint.h>


#ifndef NULL
 #define NULL ((void*)0)
#endif

typedef enum { false, true } bool;

#ifdef BLADE_NULL_HARDWARE
  #include <stdio.h>
  #define DSHOW(v) 				printf("%s: %d\n",#v,v);
  #define DSHOW_CALL(fname)		printf("%s()\n", fname);
  #define DLOG(s,...)			printf(s,__VA_ARGS__);
#else
  #ifdef BLADE_NIOS_DEBUG
    #define DSHOW(v)				alt_printf("%s: %x\n",#v,v);
    #define DSHOW_CALL(fname)		alt_printf("%s()\n", fname);
    #define DLOG(s,...)				alt_printf(s,__VA_ARGS__);
  #else
    #define DSHOW(v)
    #define DSHOW_CALL(fname)
    #define DLOG(s,...)
  #endif
#endif


#endif /* SIMPLE_TYPES_H_ */
