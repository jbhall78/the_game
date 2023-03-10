#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "utf8.h"

//
// I am purposely not using loops here to make it more legible
// for those not really familiar with binary math
//
// it could be condensed with either loops and/or
// by using larger bit masks to copy more bits in one operation
// and could be clarified by using macros to name the operation
// being performed
//
// BUG: need to fix endian issues
//
void
unicode_to_utf8(uint32_t u, utf8_char_t *utf8_char)
{
    utf8_char->c = u;
    memset(utf8_char->str, 0, sizeof(utf8_char->str));

    if (u <= 0x007F) {
	utf8_char->size = 1;
	utf8_char->str[0] = (char)u;
    } else if (u <= 0x07FF) {
	// 2 bytes, 11 bits
	char s[2];
	memset(s, 0, sizeof(s));

	/* byte 1 */
	s[0] |= (1 << 7);
	s[0] |= (1 << 6);
	s[0] |= (0 << 5);
	s[0] |= (((u >> 10) & 1) << 4);
	s[0] |= (((u >> 9)  & 1) << 3);
	s[0] |= (((u >> 8)  & 1) << 2);
	s[0] |= (((u >> 7)  & 1) << 1);
	s[0] |= ((u  >> 6)  & 1);

	/* byte 2 */
	s[1] |= (1 << 7);
	s[1] |= (0 << 6);
	s[1] |= (((u >> 5) & 1) << 5);
	s[1] |= (((u >> 4) & 1) << 4);
	s[1] |= (((u >> 3) & 1) << 3);
	s[1] |= (((u >> 2) & 1) << 2);
	s[1] |= (((u >> 1) & 1) << 1);
	s[1] |= (u & 1);

	utf8_char->size   = 2;
	utf8_char->str[0] = s[0];
	utf8_char->str[1] = s[1];
    } else if (u <= 0xFFFF) {
	// 3 bytes, 16 bits
	char s[3];
	memset(s, 0, sizeof(s));

	/* byte 1 */
	s[0] |= (1 << 7);
	s[0] |= (1 << 6);
	s[0] |= (1 << 5);
	s[0] |= (0 << 4);
	s[0] |= (((u >> 15) & 1) << 3);
	s[0] |= (((u >> 14) & 1) << 2);
	s[0] |= (((u >> 13) & 1) << 1);
	s[0] |= ((u  >> 12) & 1);

	/* byte 2 */
	s[1] |= (1 << 7);
	s[1] |= (0 << 6);
	s[1] |= (((u >> 11) & 1) << 5);
	s[1] |= (((u >> 10) & 1) << 4);
	s[1] |= (((u >> 9)  & 1) << 3);
	s[1] |= (((u >> 8)  & 1) << 2);
	s[1] |= (((u >> 7)  & 1) << 1);
	s[1] |= ((u  >> 6)  & 1);

	/* byte 3 */
	s[2] |= (1 << 7);
	s[2] |= (0 << 6);
	s[2] |= (((u >> 5) & 1) << 5);
	s[2] |= (((u >> 4) & 1) << 4);
	s[2] |= (((u >> 3) & 1) << 3);
	s[2] |= (((u >> 2) & 1) << 2);
	s[2] |= (((u >> 1) & 1) << 1);
	s[2] |= (u & 1);

	utf8_char->size   = 3;
	utf8_char->str[0] = s[0];
	utf8_char->str[1] = s[1];
	utf8_char->str[2] = s[2];
    } else if (u <= 0x1FFFFF) {
	// 4 bytes, 21 bits
	char s[4];
	memset(s, 0, sizeof(s));

	/* byte 1 */
	s[0] |= (1 << 7);
	s[0] |= (1 << 6);
	s[0] |= (1 << 5);
	s[0] |= (1 << 4);
	s[0] |= (0 << 3);
	s[0] |= (((u >> 20) & 1) << 2);
	s[0] |= (((u >> 19) & 1) << 1);
	s[0] |= ((u  >> 18) & 1);

	/* byte 2 */
	s[1] |= (1 << 7);
	s[1] |= (0 << 6);
	s[1] |= (((u >> 17) & 1) << 5);
	s[1] |= (((u >> 16) & 1) << 4);
	s[1] |= (((u >> 15) & 1) << 3);
	s[1] |= (((u >> 14) & 1) << 2);
	s[1] |= (((u >> 13) & 1) << 1);
	s[1] |= ((u  >> 12) & 1);

	/* byte 3 */
	s[2] |= (1 << 7);
	s[2] |= (0 << 6);
	s[2] |= (((u >> 11) & 1) << 5);
	s[2] |= (((u >> 10) & 1) << 4);
	s[2] |= (((u >> 9) & 1) << 3);
	s[2] |= (((u >> 8) & 1) << 2);
	s[2] |= (((u >> 7) & 1) << 1);
	s[2] |= ((u  >> 6) & 1);

	/* byte 4 */
	s[3] |= (1 << 7);
	s[3] |= (0 << 6);
	s[3] |= (((u >> 5) & 1) << 5);
	s[3] |= (((u >> 4) & 1) << 4);
	s[3] |= (((u >> 3) & 1) << 3);
	s[3] |= (((u >> 2) & 1) << 2);
	s[3] |= (((u >> 1) & 1) << 1);
	s[3] |= (u & 1);

	utf8_char->size   = 4;
	utf8_char->str[0] = s[0];
	utf8_char->str[1] = s[1];
	utf8_char->str[2] = s[2];
	utf8_char->str[3] = s[3];
    } else if (u <= 0x3FFFFFF) {
	// 5 bytes, 26 bits
	char s[5];
	memset(s, 0, sizeof(s));

	/* byte 1 */
	s[0] |= (1 << 7);
	s[0] |= (1 << 6);
	s[0] |= (1 << 5);
	s[0] |= (1 << 4);
	s[0] |= (1 << 3);
	s[0] |= (0 << 2);
	s[0] |= (((u >> 25) & 1) << 1);
	s[0] |= ((u  >> 24) & 1);

	/* byte 2 */
	s[1] |= (1 << 7);
	s[1] |= (0 << 6);
	s[1] |= (((u >> 23) & 1) << 5);
	s[1] |= (((u >> 22) & 1) << 4);
	s[1] |= (((u >> 21) & 1) << 3);
	s[1] |= (((u >> 20) & 1) << 2);
	s[1] |= (((u >> 19) & 1) << 1);
	s[1] |= ((u  >> 18) & 1);

	/* byte 3 */
	s[2] |= (1 << 7);
	s[2] |= (0 << 6);
	s[2] |= (((u >> 17) & 1) << 5);
	s[2] |= (((u >> 16) & 1) << 4);
	s[2] |= (((u >> 15) & 1) << 3);
	s[2] |= (((u >> 14) & 1) << 2);
	s[2] |= (((u >> 13) & 1) << 1);
	s[2] |= ((u  >> 12) & 1);

	/* byte 3 */
	s[3] |= (1 << 7);
	s[3] |= (0 << 6);
	s[3] |= (((u >> 11) & 1) << 5);
	s[3] |= (((u >> 10) & 1) << 4);
	s[3] |= (((u >> 9)  & 1) << 3);
	s[3] |= (((u >> 8)  & 1) << 2);
	s[3] |= (((u >> 7)  & 1) << 1);
	s[3] |= ((u  >> 6)  & 1);

	/* byte 4 */
	s[4] |= (1 << 7);
	s[4] |= (0 << 6);
	s[4] |= (((u >> 5) & 1) << 5);
	s[4] |= (((u >> 4) & 1) << 4);
	s[4] |= (((u >> 3) & 1) << 3);
	s[4] |= (((u >> 2) & 1) << 2);
	s[4] |= (((u >> 1) & 1) << 1);
	s[4] |= (u & 1);

	utf8_char->size   = 5;
	utf8_char->str[0] = s[0];
	utf8_char->str[1] = s[1];
	utf8_char->str[2] = s[2];
	utf8_char->str[3] = s[3];
	utf8_char->str[4] = s[4];
    } else if (u <= 0x7FFFFFFF) {
	// 6 bytes, 31 bits
	char s[6];
	memset(s, 0, sizeof(s));

	/* byte 1 */
	s[0] |= (1 << 7);
	s[0] |= (1 << 6);
	s[0] |= (1 << 5);
	s[0] |= (1 << 4);
	s[0] |= (1 << 3);
	s[0] |= (1 << 2);
	s[0] |= (0 << 1);
	s[0] |= ((u  >> 30) & 1);

	/* byte 2 */
	s[1] |= (1 << 7);
	s[1] |= (0 << 6);
	s[1] |= (((u >> 29) & 1) << 5);
	s[1] |= (((u >> 28) & 1) << 4);
	s[1] |= (((u >> 27) & 1) << 3);
	s[1] |= (((u >> 26) & 1) << 2);
	s[1] |= (((u >> 25) & 1) << 1);
	s[1] |= ((u  >> 24) & 1);

	/* byte 3 */
	s[2] |= (1 << 7);
	s[2] |= (0 << 6);
	s[2] |= (((u >> 23) & 1) << 5);
	s[2] |= (((u >> 22) & 1) << 4);
	s[2] |= (((u >> 21) & 1) << 3);
	s[2] |= (((u >> 20) & 1) << 2);
	s[2] |= (((u >> 19) & 1) << 1);
	s[2] |= ((u  >> 18) & 1);

	/* byte 3 */
	s[3] |= (1 << 7);
	s[3] |= (0 << 6);
	s[3] |= (((u >> 17) & 1) << 5);
	s[3] |= (((u >> 16) & 1) << 4);
	s[3] |= (((u >> 15) & 1) << 3);
	s[3] |= (((u >> 14) & 1) << 2);
	s[3] |= (((u >> 13) & 1) << 1);
	s[3] |= ((u  >> 12) & 1);

	/* byte 4 */
	s[4] |= (1 << 7);
	s[4] |= (0 << 6);
	s[4] |= (((u >> 11) & 1) << 5);
	s[4] |= (((u >> 10) & 1) << 4);
	s[4] |= (((u >> 9)  & 1) << 3);
	s[4] |= (((u >> 8)  & 1) << 2);
	s[4] |= (((u >> 7)  & 1) << 1);
	s[4] |= ((u  >> 6)  & 1);

	/* byte 4 */
	s[5] |= (1 << 7);
	s[5] |= (0 << 6);
	s[5] |= (((u >> 5) & 1) << 5);
	s[5] |= (((u >> 4) & 1) << 4);
	s[5] |= (((u >> 3) & 1) << 3);
	s[5] |= (((u >> 2) & 1) << 2);
	s[5] |= (((u >> 1) & 1) << 1);
	s[5] |= (u & 1);

	utf8_char->size   = 6;
	utf8_char->str[0] = s[0];
	utf8_char->str[1] = s[1];
	utf8_char->str[2] = s[2];
	utf8_char->str[3] = s[3];
	utf8_char->str[4] = s[4];
	utf8_char->str[5] = s[5];
    }
}

#if 0 /* usage */
    uint32_t i, j;
    utf8_char_t uc;
    unicode_to_utf8(i, &uc);
    for (j = 0; j < uc.size; j++) {
	putchar(uc.str[j]);
    }
#endif
