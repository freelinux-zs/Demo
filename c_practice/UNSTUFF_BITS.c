#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const unsigned int __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		unsigned int __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res & __mask;						\
	})



void main(void)
{
	
	unsigned int cid[4] = {0x15010046,0x4e36324d,0x42033248,0xd06a5509};
	unsigned int  *resp = cid;
	int year = UNSTUFF_BITS(resp ,8 ,4);
	int mouth =  UNSTUFF_BITS(resp ,12 ,4);
	unsigned int manid = UNSTUFF_BITS(resp, 104, 24);
	printf("%x %x %x %lx\n",resp[0],year, mouth, manid);


	return ;
}
