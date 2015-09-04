
#include "heap.h"
#include "page.h"

heap_t *kheap = 0;

extern u32 end;
u32 placement_address = (u32)&end;
extern page_directory_t *kernel_directory;

/*
 * Placement dynamic memory routine before
 * we get the heap up and running.
 */
u32 kmalloc_int(u32 sz, int align, u32 *phys)
{
	if (kheap) {
		void *addr = alloc(sz, (u8)align, kheap);
		if (phys) {
			page_t *page = get_page((u32)addr, 0, kernel_directory);
			*phys = page->frame * 0x1000 + ((u32)addr & 0xfff);
		}
		return (u32)addr;
	}
	if (align) {
		/*
		 * Align it.
		 */
		placement_address &= 0xfffff000;
		placement_address += 0x1000;
	}
	if (phys) {
		*phys = placement_address;
	}
	u32 tmp = placement_address;
	placement_address += sz;
	return tmp;
}

u32 kmalloc_a(u32 sz)
{
	return kmalloc_int(sz, 1, 0);
}

u32 kmalloc_p(u32 sz, u32 *phys)
{
	return kmalloc_int(sz, 0, phys);
}

u32 kmalloc_ap(u32 sz, u32 *phys)
{
	return kmalloc_int(sz, 1, phys);
}

u32 kmalloc(u32 sz)
{
	return kmalloc_int(sz, 0, 0);
}

/*
 * Simply put this routine returns either a -1 that tells us
 * that there is no hole to be allocated in the index list.
 * Otherwise it returns an index that references a header_t in
 * the index list, which again references a block of free memory.

 * Due to that the size of free blocks is ordered from low-high size,
 * finding a free block of memory is incredibly easy, and gives us a
 * small algorithm for the task.
 */
static s32 find_smallest_hole(u32 size, u8 page_align, heap_t *heap)
{
	/* Find the smallest hole that will fit */
	u32 i = 0;

	while (i < heap->index.size) {
		header_t *header = (header_t *)lookup_ordered_array(i, &heap->index);
		if (header->size >= size)
			break;
		i++;
	}
	/* Why did the loop exit? */
	if (i == heap->index.size)
		return -1;
	return i;
}

static s8 header_t_less_than(type_t a, type_t b)
{
	return (((header_t *)a)->size < ((header_t *)b)->size) ? 1 : 0;
}

heap_t *create_heap(u32 start, u32 end_addr, u32 max,
					u8 supervisor, u8 readonly)
{
	heap_t *heap = (heap_t *)kmalloc(sizeof(heap_t));
	/* Initialize the index */
	heap->index = place_ordered_array((void *)start, HEAP_INDEX_SIZE,
									 header_t_less_than);
	start += HEAP_INDEX_SIZE * sizeof(type_t);

	heap->start_address = start;
	heap->end_address = end_addr;
	heap->max_address = max;
	heap->supervisor = supervisor;
	heap->readonly = readonly;

	/* We start off with one large hole in the index */
	header_t *hole = (header_t *)start;
	hole->size = end_addr - start;
	hole->magic = HEAP_MAGIC;
	hole->is_hole = 1;
	insert_ordered_array((void *)hole, &heap->index);

	return heap;
}

static u32 expand(u32 new_size, heap_t *heap)
{
	/* If new_size is < than current size. Return -1 */
	if (new_size < (heap->end_address - heap->start_address))
		return -1;

	/* Make sure new_size is <= than max size */
	if (heap->start_address + new_size > heap->max_address)
		return -1;
	heap->end_address = heap->start_address + new_size;
}

static u32 contract(u32 new_size, heap_t *heap)
{
	if (new_size > (heap->end_address - heap->start_address))
		return -1;
	if (new_size < HEAP_MIN_SIZE)
		new_size = HEAP_MIN_SIZE;

	heap->end_address = heap->start_address + new_size;

	return new_size;
}

void *alloc(u32 size, u8 page_align, heap_t *heap)
{
	/*
	 * Make sure we take the size of the header/footer
	 * into account.
	 */
	u32 new_size = size + sizeof(header_t) + sizeof(footer_t);
	/*
	 * Find the smallest hole that will fit
	 */
	s32 i = find_smallest_hole(new_size, page_align, heap);

	if (i == -1) {
		/*
		 * If we didnt find a suitable hole
		 */
		 u32 old_length = heap->end_address - heap->start_address;
		 u32 old_end_address = heap->end_address;

		 expand(old_length + new_size, heap);
		 u32 new_length = heap->end_address - heap->start_address;

		 /*
		  * Find the endmost header. Not endmost in size, but in
		  * location.
		  */
		 u32 i = 0, idx = -1, value = 0x0;
		 while (i < heap->index.size) {
		 	u32 tmp = (u32)lookup_ordered_array(i, &heap->index);
		 	if (tmp > value) {
		 		value = tmp;
		 		idx = i;
		 	}
		 	i++;
		 }
		 /*
		  * If we didnt find any headers, we need to add one
		  */
		 if (idx == -1) {
		 	header_t *header = (header_t *)old_end_address;
		 	header->magic = HEAP_MAGIC;
		 	header->size = new_length - old_length;
		 	header->is_hole = 1;
		 	footer_t *footer = (footer_t *)(old_end_address + header->size - sizeof(footer_t));
		 	footer->magic = HEAP_MAGIC;
		 	footer->header = header;
		 	insert_ordered_array((void *)header, &heap->index);
		 }
		 else {
		 	/*
		 	 * The last header needs adjusting
		 	 */
		 	header_t *header = lookup_ordered_array(idx, &heap->index);
		 	header->size += new_length - old_length;
		 	/* Rewrite the footer */
		 	footer_t *footer = (footer_t *)((u32)header + header->size - sizeof(footer_t));
		 	footer->header = header;
		 	footer->magic = HEAP_MAGIC;
		 }
		 /*
		  * We now have enough space. Recurse call the function
		  * again.
		  */
		 return alloc(size, page_align, heap);
	}

	header_t *orig_hole_header = (header_t *)lookup_ordered_array(i, &heap->index);
	u32 orig_hole_pos = (u32)orig_hole_header;
	u32 orig_hole_size = orig_hole_header->size;

	if (orig_hole_size - new_size < sizeof(header_t) + sizeof(footer_t)) {
		size += orig_hole_size - new_size;
		new_size = orig_hole_size;
	}
	
	// If we need to page-align the data, do it now and make a new hole in front of our block.
    if (page_align && orig_hole_pos&0xFFFFF000)
    {
        u32 new_location      = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(header_t);
        header_t *hole_header = (header_t *)orig_hole_pos;
        hole_header->size     = 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(header_t);
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        footer_t *hole_footer = (footer_t *) ( (u32)new_location - sizeof(footer_t) );
        hole_footer->magic    = HEAP_MAGIC;
        hole_footer->header   = hole_header;
        orig_hole_pos         = new_location;
        orig_hole_size        = orig_hole_size - hole_header->size;
    }
    else
    {
        // Else we don't need this hole any more, delete it from the index.
        remove_ordered_array(i, &heap->index);
    }
	header_t *block_header = (header_t *)orig_hole_pos;
	block_header->magic = HEAP_MAGIC;
	block_header->is_hole = 0;
	block_header->size = new_size;

	footer_t *block_footer = (footer_t *)(orig_hole_pos + sizeof(header_t) + size);
	block_footer->magic = HEAP_MAGIC;
	block_footer->header = block_header;

	/*
	 * We may need to write a new hole after the allocated block.
	 * We do this only if the new hole would have a positive size
	 */
	if (orig_hole_size - new_size > 0) {
		header_t *hole_header = (header_t *)(orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
		hole_header->magic = HEAP_MAGIC;
		hole_header->is_hole = 1;
		hole_header->size = orig_hole_size - new_size;
		footer_t *hole_footer = (footer_t *)((u32)hole_header + orig_hole_size - new_size - sizeof(footer_t));
		hole_footer->magic = HEAP_MAGIC;
		hole_footer->header = hole_header;
		
		if ((u32)hole_footer < heap->end_address) {
			hole_footer->magic = HEAP_MAGIC;
			hole_footer->header = hole_header;
		}
		/*
		 * Put the new hole into the index.
		 */
		insert_ordered_array((void *)hole_header, &heap->index);
	}

	return (void *)((u32)block_header + sizeof(header_t));
}

void free(void *p, heap_t *heap)
{
	/*
	 * Exit gracefully for null pointers.
	 */
	if (!p)
		return;
	/*
	 * Get the header and footer associated with this
	 * pointer.
	 */
	header_t *header = (header_t *)((u32)p - sizeof(header_t));
	footer_t *footer = (footer_t *)((u32)header + header->size - sizeof(footer_t));

	/*
	 * Check the magic numbers of both the header
	 * and footer. If they do not match this is a bogus
	 * pointer passed.
	 */
	if (header->magic != HEAP_MAGIC)
		return;
	if (footer->magic != HEAP_MAGIC)
		return;
	/*
	 * Make us a hole.
	 */
	header->is_hole = 1;
	/*
	 * Do we want to add this header into the "free holes"
	 * index?
	 */
	u8 do_add = 1;
	
	/*
	 * Unify left. If the thing immediately to the left of us
	 * is a footer.
	 */
	footer_t *test_footer = (footer_t *)((u32)header - sizeof(footer_t));
	if (test_footer->magic == HEAP_MAGIC &&
		test_footer->header->is_hole) {
		/*
		 * Cache our current size.
		 */
		u32 cache_size = header->size;
		/*
		 * Rewrite our header with the new one.
		 */
		header = test_footer->header;
		/*
		 * Rewrite our footer to point to the new
		 * header.
		 */
		footer->header = header;
		/*
		 * Change the size of the new header.
		 */
		header->size += cache_size;
		/*
		 * Since this header is already in the index, we dont
		 * want to add it again.
		 */
		do_add = 0;
	}

	/*
	 * Unify right. If the thing immediately to the right of us
	 * is a header.
	 */
	header_t *test_header = (header_t *)((u32)footer + sizeof(footer_t));
	if (test_header->magic == HEAP_MAGIC &&
		test_header->is_hole) {
		/*
		 * Increase our current size.
		 */
		header->size += test_header->size;
		/*
		 * Rewrite its footer to point to our header.
		 */
		test_footer = (footer_t *)((u32)test_header + test_header->size -
								   sizeof(footer_t));
		footer = test_footer;

		footer->header = header;
		/*
		 * Find and remove test_header from the index.
		 */
		u32 i = 0;
		while (i < heap->index.size &&
			   lookup_ordered_array(i, &heap->index) != (void *)test_header)
			i++;
		/*
		 * Make sure we actually found the item. Error
		 * debugging will be added later. For now just
		 * return.
		 */
		if (i == heap->index.size)
			return;
		/*
		 * Remove the pointer from the index list.
		 */
		 remove_ordered_array(i, &heap->index);
	}
	/*
	 * If there was no unification, or there was a
	 * unify right, but not left, simply add the block of
	 * memory into the index list.
	 */
	if (do_add)
		insert_ordered_array((void *)header, &heap->index);
}