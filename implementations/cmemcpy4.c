#include <stdlib.h>
#include <stdio.h>

#include <stddef.h>
#include <stdint.h>

/* 
	inline at the end is a hint for the compiler
	to generate the fastest possible call to a function,
	usually it means inlining
*/
#define INLINE   __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline, noclone))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// Basic memcpy unaligned
INLINE void *smol_unal4(
	      void *restrict const dest_, 
	const void *restrict const src_,
	size_t                     size) 
{
	const size_t divisor      = sizeof(long long int);
	const size_t numberofints = size/divisor;
	      size_t remainder     = size % divisor;

	/* Copy 64 bit chunks */
	      long long int * dst_u64 = (      long long int *)dest_;
	const long long int * src_u64 = (const long long int *)src_;
	for (size_t i = 0; i < numberofints; i++) {
		*dst_u64 = *src_u64;
		++dst_u64;
		++src_u64;
	}

	/* Copy remainder */
	      char * dst = (      char *)dst_u64;
	const char * src = (const char *)src_u64;

	while (remainder) {
		*dst = *src;
		++dst;
		++src;

		--remainder;
	}

	return dest_;
}

INLINE void *smol_al4(
	      void *restrict const dest_, 
	const void *restrict const src_,
	size_t                     size) 
{
	const size_t divisor      = sizeof(long long int);
	const size_t numberofints = size/divisor;
	      size_t remainder    = size % divisor;

	/* Copy 64 bit chunks */
	      long long int * dst_u64 = __builtin_assume_aligned((	long long int *)dest_, 8);
	const long long int * src_u64 = __builtin_assume_aligned((const long long int *)src_,  8);

	for (size_t i = 0; i < numberofints; i++) {
		*dst_u64 = *src_u64;
		++dst_u64;
		++src_u64;
	}

	/* Copy remainder */
	      char * dst = (	  char *)dst_u64;
	const char * src = (const char *)src_u64;
	while (remainder) {
		*dst = *src;
		++dst;
		++src;

		--remainder;
	}

	return dest_;
}

/*
	Pick best memcpy strategy
*/
void *cmemcpy4(
	      void *restrict const dest_, 
	const void *restrict const src_,
	size_t                     size) 
{
	if (likely(
	   ((uintptr_t)src_) % 8 == 0 
	   && 
	   ((uintptr_t)dest_) % 8 == 0)
	) {
		smol_al4(dest_, src_, size);

	} else {
		smol_unal4(dest_, src_, size);
	}

	return dest_;
}


