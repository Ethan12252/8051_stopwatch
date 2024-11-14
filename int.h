#ifndef __INT_H__
#define __INT_H__

// Unsigned integer types
typedef unsigned char uint8;    // 8-Bit unsigned integer
typedef unsigned int uint16;    // 16-Bit unsigned integer
typedef unsigned long uint32;   // 32-Bit unsigned integer

// Signed integer types
typedef signed char int8;       // 8-Bit signed integer
typedef signed int int16;       // 16-Bit signed integer
typedef signed long int32;      // 32-Bit signed integer

// Minimum and maximum values for unsigned types
#define UINT8_MIN  0
#define UINT8_MAX  255

#define UINT16_MIN 0
#define UINT16_MAX 65535

#define UINT32_MIN 0
#define UINT32_MAX 4294967295UL

// Minimum and maximum values for signed types
#define INT8_MIN   (-128)
#define INT8_MAX   127

#define INT16_MIN  (-32768)
#define INT16_MAX  32767

#define INT32_MIN  (-2147483648L)
#define INT32_MAX  2147483647

#endif // __INT_H__
