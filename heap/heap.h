
#ifndef HEAP_HEAP_H
#define HEAP_HEAP_H

#include "types.h"
#include "ordered_array.h"

/*
 * My test defines for setting up a non-paged
 * aligned heap. Starts at 10MB exactly,
 * and is of size 1MB. It can thus be expanded
 * up to 4MB. Index size consists of the first
 * 128KB of the entire space. For now the heap can
 * not be contracted, only expanded. Also in my example
 * the heap_t structure will be allocated staticly on
 * compiling. It will be a bit above 1MB.
 */
#define KHEAP_START 			0x00a00000
#define KHEAP_INITIAL_SIZE 		0x00100000
#define KHEAP_MAXSIZE			0x00400000
#define HEAP_INDEX_SIZE 		0x00020000
#define HEAP_MAGIC				0x123890ab
#define HEAP_MIN_SIZE			0x00070000
/*
 * Size information for a hole / block
 */
typedef struct {
	u32 magic;
	u8 is_hole;
	u32 size;
} header_t;

typedef struct {
	u32 magic;
	header_t *header;
} footer_t;

typedef struct {
	ordered_array_t index;
	u32 start_address;
	u32 end_address;
	u32 max_address;
	u8 supervisor;
	u8 readonly;
} heap_t;

/*
 * Internal new kmalloc routine.
 */
u32 kmalloc_int(u32 sz, int align, u32 *phys);
/*
 * Page aligned new kmalloc wrapper
 * routine.
 */
u32 kmalloc_a(u32 sz);
/*
 * New kmalloc wrapper routine which
 * returns a physical address.
 */
u32 kmalloc_p(u32 sz, u32 *phys);
/*
 * Page aligned new kmalloc wrapper
 * which returns a physical address.
 */
u32 kmalloc_ap(u32 sz, u32 *phys);
/*
 * New kmalloc vanilla wrapper routine.
 */
u32 kmalloc(u32 sz);

/*
 * Create a new heap
 */
heap_t *create_heap(u32 start, u32 end_addr, u32 max,
		   		    u8 supervisor, u8 readonly);

/*
 * Allocate a contiguous region of memory 'size' in size.
 * If page_align == 1, it creates a block starting on a
 * page boundary
 */
void *alloc(u32 size, u8 page_align, heap_t *heap);

/*
 * Release a block allocated with 'alloc'
 */
void free(void *p, heap_t *heap);

#endif /* HEAP_HEAP_H */