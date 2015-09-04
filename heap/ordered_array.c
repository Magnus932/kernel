
#include "ordered_array.h"

s8 standard_lessthan_predicate(type_t a, type_t b)
{
	return (a < b) ? 1 : 0;
}

/*
ordered_array_t create_ordered_array(u32 max_size,
									 lessthan_predicate_t less_than)
{
	ordered_array_t to_ret;

	to_ret.array = (void *)kmalloc(max_size * sizeof(type_t));
	memset(to_ret.array, '\0', max_size * sizeof(type_t));
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;

	return to_ret;
}
*/

ordered_array_t place_ordered_array(void *addr, u32 max_size,
								    lessthan_predicate_t less_than)
{
	ordered_array_t to_ret;

	to_ret.array = (type_t *)addr;
	memset(to_ret.array, '\0', max_size * sizeof(type_t));
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;

	return to_ret;
}

void destroy_ordered_array(ordered_array_t *array)
{
	// kfree(array->array);
}

void insert_ordered_array(type_t item, ordered_array_t *array)
{
	u32 i = 0;

	while (i < array->size && array->less_than(array->array[i], item))
		i++;
	if (i == array->size)
		array->array[array->size++] = item;
	else {
		type_t tmp = array->array[i];
		array->array[i] = item;
		while (i < array->size) {
			i++;
			type_t tmp2 = array->array[i];
			array->array[i] = tmp;
			tmp = tmp2;
		}
		array->size++;
	}
}

type_t lookup_ordered_array(u32 i, ordered_array_t *array)
{
	if (i >= array->size)
		return 0;
	return array->array[i];
}

void remove_ordered_array(u32 i, ordered_array_t *array)
{
	while (i < array->size) {
		array->array[i] = array->array[i + 1];
		i++;
	}
	array->size--;
}


