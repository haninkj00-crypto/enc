#include "tables.h"
#include "FWDBLK.H"
#include "RVBLK.H"
#include "CP437.INC"
#include "CP1252.INC"
#define CP949_BLOCK_SHIFT 10
#define CP949_BLOCK_MASK 1023UL
static const CP949Entry ET_FAR * const forward_blocks[] = {
#include "FWDLIST.H"
};
static const CP949ReverseEntry ET_FAR * const reverse_blocks[] = {
#include "RVLIST.H"
};
const unsigned long cp949_forward_count = 17048UL;
const unsigned long cp949_reverse_count = 17048UL;
CP949Entry cp949_forward_at(unsigned long index)
{
    return forward_blocks[index >> CP949_BLOCK_SHIFT][index & CP949_BLOCK_MASK];
}
CP949ReverseEntry cp949_reverse_at(unsigned long index)
{
    return reverse_blocks[index >> CP949_BLOCK_SHIFT][index & CP949_BLOCK_MASK];
}
