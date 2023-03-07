#include "convert_char_to_hex.h"

int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}
int hexchar_to_int(char c) {
    switch (c) {
        case '0':
            return 0;
        break;

        case '1':
            return 1;
        break;

        case '2':
            return 2;
        break;

        case '3':
            return 3;
        break;

        case '4':
            return 4;
        break;

        case '5':
            return 5;
        break;

        case '6':
            return 6;
        break;

        case '7':
            return 7;
        break;

        case '8':
            return 8;
        break;

        case '9':
            return 9;
        break;

        case 'a':
        case 'A':
            return 10;
            break;

        case 'b':
        case 'B':
            return 11;
        break;
    
        case 'c':
        case 'C':
            return 12;
        break;

        case 'd':
        case 'D':
            return 13;
        break;

        case 'e':
        case 'E':
            return 14;
        break;

        case 'f':
        case 'F':
            return 15;
        break;

        default:
            return -1;
        break;
    }
	return -1;
}
int hexstr_to_int(char* str, int length) {
    int value = 0;
    for(int i=length-1; i>=0; i--) {
        value += hexchar_to_int(str[i])*pow(16, length-1-i);
    }

    return value;
}