#ifndef TABLES_H
#define TABLES_H

#include "..\CODEC.H"

extern const U32 cp437_to_unicode[256];
extern const U32 cp1252_to_unicode[256];

extern const unsigned long cp949_forward_count;
extern const unsigned long cp949_reverse_count;

CP949Entry cp949_forward_at(unsigned long index);
CP949ReverseEntry cp949_reverse_at(unsigned long index);

#endif
