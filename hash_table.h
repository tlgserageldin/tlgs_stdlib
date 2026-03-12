#pragma once

#include "q_strings.h"
#include <stddef.h>

typedef struct ht ht;

// allocate the ht on the heap
ht *ht_create(void);

// frees the table and changes the ptr to null
// non-zero return on error
int ht_destroy(ht **table);

// returns null if not found, or missuse
void *ht_search(ht *table, str key);

// non-zero return on error
// 2^53 max entries
int ht_insert(ht *table, str key, void *value);

// non-zero error
int ht_remove(ht *table, str key);

// typedef struct {
//   void *value;
//   str *key;
//
//   // PRIVATE
//   ht *_table;
//   size_t _index;
// } ht_iter;
//
// ht_iter ht_iterator(ht *table);
//
// ht_iter ht_next(ht_iter iterator);
