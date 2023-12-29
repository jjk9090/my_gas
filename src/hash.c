/*
 * @Author: hxy
 * @Date: 2023-10-04 16:15:33
 * @LastEditTime: 2023-10-04 16:15:34
 * @Description: 请填写简介
 */


#include "as.h"

/* Hash function for a string_tuple.  */

hashval_t
hash_string_tuple (const void *e)
{
	string_tuple_t *tuple = (string_tuple_t *) e;
	return htab_hash_string (tuple->key);
}

/* Equality function for a string_tuple.  */

int
eq_string_tuple (const void *a, const void *b)
{
	const string_tuple_t *ea = (const string_tuple_t *) a;
	const string_tuple_t *eb = (const string_tuple_t *) b;

	return strcmp (ea->key, eb->key) == 0;
}

/* Insert ELEMENT into HTAB.  If REPLACE is non-zero existing elements
   are overwritten.  If ELEMENT already exists, a pointer to the slot
   is returned.  Otherwise NULL is returned.  */

void **
htab_insert (htab_t htab, void *element, int replace)
{
	void **slot = htab_find_slot (htab, element, INSERT);
	if (*slot != NULL){
			if (replace) {
				if (htab->del_f)
					(*htab->del_f) (*slot);
				*slot = element;
			}
			return slot;
		}
	*slot = element;
	return NULL;
}

/* Print statistics about a hash table.  */

void
htab_print_statistics (FILE *f, const char *name, htab_t table)
{
	fprintf (f, "%s hash statistics:\n", name);
	fprintf (f, "\t%u searches\n", table->searches);
	fprintf (f, "\t%u collisions\n", table->collisions);
	fprintf (f, "\t%lu elements\n", (unsigned long) htab_elements (table));
	fprintf (f, "\t%lu table size\n", (unsigned long) htab_size (table));
}
