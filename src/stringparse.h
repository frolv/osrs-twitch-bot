/*
 * stringparse.h: functions to parse various data types from C strings
 */

#ifndef _STRINGPARSE_H
#define _STRINGPARSE_H

#include <stdint.h>

/* parsenum: read number from s into num */
int parsenum(const char *s, int64_t *res);

/*
 * parsenum_mult: read number from s into num, supporting numbers ending
 * in k, m or b representing multiples of 1e3, 1e6 and 1e9, respectively
 */
int parsenum_mult(const char *s, int64_t *amt);

#endif
