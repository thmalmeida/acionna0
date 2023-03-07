#ifndef CONVERT_CHAR_TO_HEX_H__
#define CONVERT_CHAR_TO_HEX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <math.h>

int hex2int(char ch);
int hexchar_to_int(char c);
int hexstr_to_int(char* str, int length);



#ifdef __cplusplus
}
#endif

#endif /* CONVERT_CHAR_TYPE_H__ */
