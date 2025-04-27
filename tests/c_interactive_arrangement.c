#define _GNU_SOURCE 	// GNU-specific extensions like
			// sched_setaffinity() sched_getaffinity() 
			// these are not part of standard POSIX API

#include <sched.h>      // Set thread's CPU affinity
#include <unistd.h> 	// System-related functions like getpid()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include <dlfcn.h> 	// dynamic linking library 

#include "perf_utils.h"
#include "memcpy.h"

#define TEXT_MAX_SIZE  (1 << 19)
#define TITLE_MAX_SIZE (1 << 9 )

#define COLOR_MAX_SIZE		512

// positive value ? -1 : 0
// -1 is invalid size
#define BUILD_BUG_ON_ZERO(expr) ((int)(sizeof(struct { int:(-!!(expr)); })))

// sametype ? 1 : 0;
#define __same_type(a,b) __builtin_types_compatible_p(typeof(a), typeof(b))

#define ARRAY_SIZE(arr) \
	((void)BUILD_BUG_ON_ZERO( \
		__same_type((arr), &(arr)[0])), \
		(sizeof(arr) / sizeof((arr)[0])) \
	)

// Converts argument into string
#define STRINGIFY__(x) #x
#define STRINGIFY(x)  STRINGIFY__(x)

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

typedef struct {
	char   *array_pt;
	size_t  array_len;
	size_t  element_len;
} SlicedArr;

typedef struct {
	int r;
	int g;
	int b;
} RGB;

typedef struct {
	int h;
	int s;
	int v;
} HSV;

void *aligned_malloc(size_t size, size_t alignment) {

	void *ptr = NULL;
	int   res = posix_memalign(&ptr, alignment, size);

	assert(res == 0 && "Posix_memalign failed in aligned_malloc()");

	return ptr;
}

size_t align_to(size_t el, size_t alignment) {

	/* This works better for larger numbers
	const size_t remainder = el % alignment;
	const size_t misalignment = alignment - remainder;

	return el + misalignment;
	*/

	/*
	Faster implementation for powers of 2
	return (el + alignment - 1) & (~(alignment - 1));
	*/
	
	size_t res=0;
	
	while(res < el) {
		res += alignment;
	}
	return res;
}

int wrap(int value, int min, int max) {
	
	int range  =   max   - min,
	    result = ((value - min) % range + range) % range + min;

	return result;
}

void fill(
	char *restrict const buffer, 
	char *restrict const pattern,
	size_t 		     bufsize
) {
	
	const size_t divisor        = strlen(pattern);
	const size_t numberofcopies = (bufsize) / divisor;
	      size_t reminder	    = (bufsize) % divisor;
	      char  *dst_chunk      = buffer; 

	for (size_t i=0; i < numberofcopies; i++) { 
		memcpy(dst_chunk, pattern, divisor);
		
		dst_chunk += divisor;
	}

	for (size_t i=0; i < reminder; i++) {
		memcpy(dst_chunk, pattern+i, 1);
	
		dst_chunk ++;
		reminder --;
	} buffer[bufsize] = '\0';
}

char *generate_symbols(size_t character_count, char symbol) {

	char *line_pt = malloc(character_count+1);
	
	for (size_t i=0; i < character_count; i++) {
		line_pt[i] = symbol;
	} line_pt[character_count] = '\0';
	
	return line_pt;
}

RGB hsv_to_rgb(HSV hsv) {
	
	assert((hsv.h >= 0 && hsv.h <= 360 && hsv.h == floor(hsv.h)) && "Invalid h in hsv_to_rgb()");	
	assert((hsv.s >= 0 && hsv.s <= 100 && hsv.s == floor(hsv.s)) && "Invalid s in hsv_to_rgb()");	
	assert((hsv.v >= 0 && hsv.v <= 100 && hsv.v == floor(hsv.v)) && "Invalid v in hsv_to_rgb()");	

	RGB rgb = {0};

	float r = 0, g = 0, b = 0,

	      s = (float)hsv.s/100, 
	      v = (float)hsv.v/100,

	      c = v * s,
	      x = c * (1 - 
	      	  fabs(
		  	fmod((hsv.h / 60.0), 2) - 1)
		  ),
	      m = v - c;


	if (hsv.h == 360)
		hsv.h = 0;

	switch (hsv.h) {
		case 0 ... 59:    r = c; g = x; b = 0; break;
		case 60 ... 119:  r = x; g = c; b = 0; break;
		case 120 ... 179: r = 0; g = c; b = x; break;
		case 180 ... 239: r = 0; g = x; b = c; break;
		case 240 ... 299: r = x; g = 0;	b = c; break;
		default:	  r = c; g = 0;	b = x;
	}
	
	float 	r__ = r + m, 
		g__ = g + m, 
		b__ = b + m;
	
	rgb.r = (int)(CLAMP(r__, 0, 1) * 255);
	rgb.g = (int)(CLAMP(g__, 0, 1) * 255);
	rgb.b = (int)(CLAMP(b__, 0, 1) * 255);

	assert((rgb.r >= 0 && rgb.r < 256) && "Invalid r in hsv_to_rgb()");	
	assert((rgb.g >= 0 && rgb.g < 256) && "Invalid g in hsv_to_rgb()");	
	assert((rgb.b >= 0 && rgb.b < 256) && "Invalid b in hsv_to_rgb()");	

	return rgb;
}

/* 
Fn to print element of a column
ARGS: 
	column length,
	pointer to string you want to print,
	alignment (center, left);
This will NOT automatically include NEWLINE 
So that you can have multi-column out
*/
void print_column_el(size_t column_len, const char *str, const char *align, HSV *hsv) {

	assert(column_len && "Missing column_len in print_column_el()");	
	assert(str	  && "Missing str in print_column_el()");	
	assert(align	  && "Missing align in print_column_el()");	

	assert((strcmp(align, "left")   == 0  ||
		strcmp(align, "center") == 0  ||
		strcmp(align, "right")  == 0) &&
		"Incorrect align value in print_column_el()");		
	
	size_t padding_size  = (column_len - strlen(str));
	if (strcmp(align, "center") == 0) 
		padding_size = padding_size/2;

	char  *padding  = generate_symbols(padding_size, ' ');	

	char color_str[COLOR_MAX_SIZE], 
	     color_reset[] = "\033[38;2;255;255;255m";

	hsv->h = wrap(hsv->h - 1, 0, 360); 
	RGB rgb = hsv_to_rgb(*hsv);
	
	sprintf(color_str, 
		"\033[38;2;%d;%d;%dm", 
		rgb.r, 
		rgb.g, 
		rgb.b);
		
	if (strcmp(align, "left") == 0) 
		printf("%s%s%s%s", 
			color_str, 
			str, 
			padding, 
			color_reset);

	if (strcmp(align, "center") == 0) 
		printf("%s%s%s%s%s", 
			color_str, 
			padding, 
			str, 
			padding, 
			color_reset);

	if (strcmp(align, "right") == 0) 
		printf("%s%s%s%s", 
			color_str,
			padding,
			str, 
			color_reset);
}

size_t count_digits(size_t num) {

	if (num == 0)
		return  1;
	
	return 	(size_t)
		(
		ceil(
		log10(num) + 1)
		);
}

 SlicedArr slice(
	size_t       max_text_size, 
	const char  *str 
) {
	size_t text_size = strlen(str);
	
	assert(text_size     && "Invalid text argument in slice()");
	assert(max_text_size && "Invalid max_text_size argument in slice()");

	/*
	SlicedArr struct elements:
		char   *array_pt;
		size_t  array_len;
		size_t  element_len;
	*/
	SlicedArr sl;
	sl.element_len      = max_text_size + 1; // Include space for null terminator	

	size_t  full_chunks = text_size / max_text_size,
		remainder   = text_size - full_chunks * max_text_size;

	sl.array_len        = align_to(text_size, max_text_size) / max_text_size;
	assert(	sl.array_len
		&& "sl.array_len calculation failed in slice()");

	sl.array_pt         = calloc(sl.array_len, sl.element_len);
	assert( sl.array_pt 
		&& "Calloc failed in slice()");

	for (size_t i=0; i < full_chunks; i++) {

		size_t  curr_el   = sl.element_len * i,
			last_char = curr_el + max_text_size;

		memcpy( sl.array_pt + 
			curr_el,
			
			str + 
			curr_el - i,

			max_text_size
			);

		sl.array_pt[last_char] = '\0'; 
	}
	
	if(remainder) {
	
		size_t  n_chunk   = sl.array_len - 1,
			last_el   = sl.element_len * n_chunk, 
			last_char = last_el + remainder,

			off_len	  = max_text_size * n_chunk; 

		memcpy( sl.array_pt +
			last_el,

			str + off_len,
			
			remainder
			);
	
		sl.array_pt[last_char] = '\0'; 
	}

	return sl;
}

/*
	Line wraps around the maximal line size, you can also align around it
*/
void line(const char *text, size_t size, const char *align, HSV *hsv) {

	assert(text  && "Incorrect text value in line()");		
	assert(size  && "Incorrect size value in line()");		
	assert(align && "Missing align value in line()");
	assert(hsv   && "Incorrect hsv value in line()");		

	assert((strcmp(align, "center") == 0  || 
		strcmp(align, "left")   == 0  ||
		strcmp(align, "right")  == 0) && 
		"Incorrect align value in line()");

	size_t text_size = strlen(text);
	if (text_size <= size) {

		print_column_el(size, text, align, hsv);
		puts("");
	}

	if (text_size > size) {
	
		double lines_dbl   = (double)text_size / (double)size;
		int    lines       = (int)(ceil(lines_dbl));
		int    size_left   = text_size;

		int    act_size = size + 1;
		char   temp[act_size];

		int    curr_size  = 0;

		for (int i=0; i<lines; i++) {

			if (size_left < size)
				curr_size = size_left;

			if (size_left >= size)
				curr_size = size;

			memcpy(temp, text + (i * size), curr_size);
			temp[curr_size] = '\0';

			size_left-=size;
			print_column_el(size, temp, align, hsv);
			puts("");
		}
	}
}

/*
void generate_result_table(const char *title__) {

	assert( title__ && "Incorrect title__ value in generate_result_table()");		
	
	size_t table_len  = 0, column_len = 0;
	struct {
		size_t memcpy;	
		size_t test;	
		size_t size;	
		size_t diff;
		size_t diff_time;
	} max = {0};	

	assert( sizeof(max)  == sizeof(size_t) * 5 
		&& "Incorrect size of struct max in generate_result_table()");
	
	size_t  column_count  = sizeof(max)/ sizeof(size_t),
	        res_size      = ARRAY_SIZE(results.arr);
	assert( res_size     == FULL_TEST_COUNT 
		&& "Incorrect size of results.arr in generate_result_table()");	
	
	if (clock_rate == 0) // Remove diff_time column hack
		column_count--;

	for (uint32_t i=0; i < res_size; i++) {
	
		const Result *res = &results.arr[i];
		
		assert(res->memcpy_name && "Res->memcpy_name missing in generate_result_table()");
		assert(res->test_name	&& "Res->test_name missing in generate_result_table()");
		assert(res->difftime	&& "Res->difftime missing in generate_result_table()");
		assert(res->size	&& "Res->size missing in generate_result_table()");

		size_t  test__ = strlen(res->test_name);
		size_t  mmcp__ = strlen(res->memcpy_name);

		if (res->difftime > max.diff)
			max.diff  = res->difftime;
		if (res->size	  > max.size) 
			max.size  = res->size;
		if (test__ 	  > max.test)
			max.test  = test__;
		if (mmcp__ 	  > max.memcpy)
			max.memcpy = mmcp__;
	}

	max.size = count_digits(max.size);
	max.diff = count_digits(max.diff);

	if (clock_rate != 0) // Remove diff_time column hack
		max.diff_time = count_digits(clock_rate) + max.diff;

	if (column_len < max.memcpy) 
		column_len = max.memcpy;
	if (column_len < max.test) 
		column_len = max.test;
	if (column_len < max.size) 
		column_len = max.size;
	if (column_len < max.diff)  
		column_len = max.diff;
	if (clock_rate != 0 && column_len < max.diff_time) // Remove diff_time column hack
		column_len = max.diff_time;

	assert((column_len && column_len != 0) && "Invalid column_len in generate_result_table()");	
	
	uint64_t column_len_pad = 2,
	         column_len_raw = column_len;
	         column_len    += column_len_pad;
	         table_len      = column_len * column_count;
	char    *line_arr       = generate_symbols(table_len, '-');

	qsort(
		results.arr,
		ARRAY_SIZE(results.arr),
		sizeof(results.arr[0]),	
		&type_comp);	

	char title[TITLE_MAX_SIZE - 16], header[TITLE_MAX_SIZE];
	
	size_t i=0;
	for (; i < strlen(title__); i++) {
		title[i] = toupper(title__[i]);		
	} title[i] = '\0';

	snprintf(header, sizeof(header) ,"%s RESULTS", title);

	const char header_align[] = "center";
	
	// h.max = 360, s.max = 100, v.max = 100
	HSV hsv = {.h = 360, .s = 100, .v = 100};

	print_column_el(table_len, header, header_align, &hsv);
	puts("");
	
	char subh_align[] = "left";
	char *subh[] = {
		"TIME        (CYCLES):",
		"TIME        (NS):",
		"SIZE:",
		"MEMCPY:",
		"TEST:"
	};

	size_t  max_rows    = 0,
		subh_count  = ARRAY_SIZE(subh);
	
	SlicedArr subh_rows[subh_count];

	for (size_t i=0; i < subh_count; i++) {	
		
		if (clock_rate == 0 && i == 1)  // Remove diff_time column hack
			continue;

		subh_rows[i]= slice(column_len_raw, subh[i]);

		assert(subh_rows[i].array_len
			&& "Invalid subh_rows[i].array_len in generate_result_table()");
		assert(subh_rows[i].element_len
			&& "Invalid subh_rows[i].element_len in generate_result_table()");
		assert(subh_rows[i].array_pt 
			&& "Invalid subh_rows[i].array_pt in generate_result_table()");

		if (subh_rows[i].array_len > max_rows) 
			max_rows = subh_rows[i].array_len;
	}
	assert(max_rows > 0 && "Max_rows not updated correctly in generate_result_table()");
	
	char   *empty  = " ";
	size_t  el_len = subh_rows[0].element_len;

	for (size_t i=0; i < max_rows; i++) {	
		for (size_t j=0; j < subh_count; j++) {

			if(clock_rate == 0 && j == 1) // Remove diff_time column hack
				continue;

			if (i < subh_rows[j].array_len) {
			
				print_column_el(
					column_len,
					
					subh_rows[j].array_pt +
					i * el_len, 
					
					subh_align,
					&hsv);
			} else {
				print_column_el(column_len, empty, subh_align, &hsv);
			}
		} puts("");
	}
	print_column_el(table_len, line_arr, header_align, &hsv);
	puts("");

	hsv.v-=20;

	for (uint32_t i=0; i < res_size; i++) {
	
		const Result *res = &results.arr[i];
	
		char diff   [TITLE_MAX_SIZE], 
		     size   [TITLE_MAX_SIZE], 
		     memcpy [TITLE_MAX_SIZE], 
		     test   [TITLE_MAX_SIZE],
		     diff_sc[TITLE_MAX_SIZE];

		strcpy(memcpy, res->memcpy_name);
		strcpy(test,   res->test_name);

		sprintf(diff,    "%zu", res->difftime);
		sprintf(size,    "%zu", res->size);
		if (clock_rate != 0) // Remove diff_time column hack
			sprintf(diff_sc, "%zu", res->difftime * 1000000000 / clock_rate);

		char disp_align[] = "left";
		print_column_el(column_len, diff,    disp_align, &hsv);
		if (clock_rate != 0) // Remove diff_time column hack
			print_column_el(column_len, diff_sc, disp_align, &hsv);
		print_column_el(column_len, size,    disp_align, &hsv);
		print_column_el(column_len, memcpy,  disp_align, &hsv);
		print_column_el(column_len, test,    disp_align, &hsv);
		puts("");
	
	}
	
	for (size_t i=0; i < subh_count; i++) {

		if (clock_rate == 0 && i == 1) // Remove diff_time column hack)
			continue;

		free(subh_rows[i].array_pt);
	}
}
*/

int main(void) {

	const char *example1 = "One liner";
	const char *example2 = "Veeeeeeeeeeeery long one liner";

	const char *example3[] = {
		"cmemcpy",
		"cmemcpy2",
		"cmemcpy3",
		"cmemcpy_al",
		"cmemcpy_unal"
	};


	HSV hsv = {
		.h = 213,
		.s = 100,
		.v = 100,
	};

	line(example1, sizeof(example1)*3, "right", &hsv);
	line(example2, 12, "left", &hsv);
	
	//generate_result_table("Unaligned");
	return 0;
}
