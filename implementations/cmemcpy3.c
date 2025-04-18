#include <stdlib.h>
#include <stdio.h>

#include <stddef.h>

/* 
	inline at the end is a hint for the compiler
	to generate the fastest possible call to a function,
	usually it means inlining
*/
#define INLINE   __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline, noclone))

INLINE void *upto128(
	      void *restrict const dest_, 
	const void *restrict const src_,
	size_t                     size) 
{

	const size_t divisor      = sizeof(long long int);
	const size_t numberofints = size/divisor;
	      size_t remainder     = size % divisor;

	/* Copy 64 bit chunks */
	long long int * dst_u64 = (long long int *)dest_;
	long long int * src_u64 = (long long int *)src_;
	for (int i = 0; i < numberofints; i++) {
		*dst_u64 = *src_u64;
		++dst_u64;
		++src_u64;
	}

	/* Copy remainder */
	char * dst = (char *)dst_u64;
	char * src = (char *)src_u64;
	while (remainder) {
		*dst = *src;
		++dst;
		++src;

		--remainder;
	}

	return dest_;
}
