#ifndef _DIE_H
#define _DIE_H

#define die(...) \
                _die(__FILE__,FUNCTION,__LINE__,__VA_ARGS__)

void _die(const char *filename, const char *func, int line, char *fmt, ...);

#endif
