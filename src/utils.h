
/* APSILG header file "utils.h"
 * Declare nice wrappers around std features that can save some lines of code,
 * and other functions and macro that feels like they should end up here.
 * Implementations are in "utils.c". */

#ifndef __APSILG_header_utils__
#define __APSILG_header_utils__

/* The true fundamental circle constant. */
#define TAU 6.28318530717f

/* Same as in "stddef.h". */
typedef unsigned long int size_t;

/* The common malloc wrapper that produces an error if the allocation fails. */
void* xmalloc(size_t size);

/* The common calloc wrapper that produces an error if the allocation fails. */
void* xcalloc(size_t number, size_t size);

/* The common realloc wrapper that produces an error and returns NULL if the
 * reallocation fails. */
void* xrealloc(void* ptr, size_t size);

/* Read the file at the given path and return its content in a buffer that
 * needs to be freed. */
char* read_file(const char* filepath);

/* Swap the two given values. */
#define SWAP(type_, x_, y_) \
	do{ type_ tmp_ = x_; x_ = y_; y_ = tmp_; }while(0)

/* Make sure that inf_ <= sup_ by swapping the two values if needed. */
#define ORDER(type_, inf_, sup_) \
	do{ if (inf_ > sup_) { SWAP(type_, inf_, sup_); } }while(0)

#endif /* __APSILG_header_utils__ */
