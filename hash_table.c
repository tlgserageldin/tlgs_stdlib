#include "hash_table.h"
#include "q_strings.h"
#include <float.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int HT_MAGIC = 0xDEADDEAD;

typedef struct {
  str key;
  void *value;
} ht_entry;

struct ht {
  int magic;
  ht_entry *array;
  size_t cap;
  size_t elements;
};

/*
all values are uint64 except the byte_to_be_hashed
which is uint8
fnv1 prime number: 0x100000001b3
fnv offset bias: 0xcbf29ce484222325
hash = hash_offset_bias
for each byte_of_data to be hashed:
  hash := hash xor byte of data
  hash := hash * fnv prime
return hash
*/

const uint64_t FNV_OFFSET = 14695981039346656037ULL;
const uint64_t FNV_PRIME = 1099511628211ULL;

// use clang builtins to check if overflow
// growth rate of 1.5x to enable reuse of old data blocks
static int _ht_increase_cap(size_t old, size_t *dst) {
  size_t half = old >> 1;
  size_t new;

  // will return 1 if overflow
  // stores result = result % 2^(n bits)
  // https://clang.llvm.org/docs/LanguageExtensions.html
  if (__builtin_add_overflow(old, half, &new) != 0) {
    return 1;
  }

  // make sure we grew
  if (new == old) {
    __builtin_add_overflow(old, (size_t)1, &new);
  }

  // ensure floor in wraparound
  if (new < 8) {
    new = 8;
  }

  *dst = new;
  return 0;
}

static uint64_t _ht_hash(str key) {
  uint64_t hash = FNV_OFFSET;
  for (ptrdiff_t i = 0; i < key.len; i++) {
    // need to cast the const char* to a unsigned char
    // (byte) then cast to uint64 to xor
    hash ^= (uint64_t)(unsigned char)key.data[i];
    hash *= FNV_PRIME;
  }
  return hash;
}

static inline bool _ht_needs_to_grow(size_t elements, size_t capacity) {
  return (elements + 1) * 2 > capacity;
}

static inline int _ht_is_valid(const ht *t) {
  return t && t->magic == HT_MAGIC;
}

static uint64_t _ht_index(uint64_t hash, size_t cap) {
  // helps mix upper bits
  // https://en.wikipedia.org/wiki/Hash_function#Fibonacci_hashing
  uint64_t x = hash * 11400714819323198485ull;
  return (x % cap);
}

// zero an already created entry
// we own the key so we must free
static int _ht_zero_entry(ht_entry *e) {
  if (!is_valid_str(e->key) || e == NULL) {
    return 1;
  }
  e->value = NULL; // point at nothing
  free(e->key.data);
  e->key.data = NULL;
  e->key.len = 0;
  return 0;
}

// non-zero if error
static int _ht_resize(ht *table, size_t new_cap) {
  // if too big dont resize
  if (new_cap > SIZE_MAX / 1.5) {
    return 1;
  }

  // alloc space for new table
  ht_entry *new = calloc(new_cap, sizeof(ht_entry));
  if (new == NULL) {
    return 1;
  }
  ht_entry *old = table->array;

  for (size_t i = 0; i < table->cap; i++) {
    ht_entry *src = &old[i];
    // empty noop
    if (src->key.data == NULL) {
      continue;
    }
    // not null need to reinsert
    size_t idx = (size_t)_ht_index(_ht_hash(src->key), new_cap);
    for (;;) { // can inf loop through bc guaranteed to be large enough
      ht_entry *dst = &new[idx];
      if (dst->key.data == NULL) {
        // empty, update the destination
        *dst = *src;
        break;
      }
      // not empty, probe for next empty and try again
      idx = (idx + 1) % new_cap;
    }
  }

  // update values then free the old allocation  
  table->cap = new_cap;
  table->array = new;
  free(old);
  return 0;
}

ht *ht_create(void) {
  ht *n = malloc(sizeof(ht));
  if (n == NULL) {
    return NULL;
  }

  n->cap = 256;
  n->elements = 0;
  n->magic = HT_MAGIC;

  n->array = calloc(n->cap, sizeof(ht_entry));
  if (n->array == NULL) {
    free(n);
    return NULL;
  }

  return n;
}

int ht_destroy(ht **table) {
  // null handle
  if (table == NULL) {
    return 1;
  }

  ht *t = *table;
  if (!_ht_is_valid(t)) {
    return 2;
  }
  for (size_t i = 0; i < t->cap; i++) {
    _ht_zero_entry(&t->array[i]);
  }
  t->magic = 0; // poison
  free(t);
  *table = NULL;
  return 0;
}

// To search for a given key x the cells of T are examined
// beginning with the cell at index h(x) (where h is the hash function)
// and continuing to the adjacent cells h(x) + 1, h(x) + 2, ..., until
// finding either an empty cell or a cell whose stored key is x.
// If a cell containing the key is found, the search returns the value from that
// cell. Otherwise, if an empty cell is found, the key cannot be in the table,
// because it would have been placed in that cell in preference to any later
// cell that has not yet been searched. In this case, the search returns as its
// result that the key is not present in the dictionary

// RETURNS NULL IF NOT FOUND
void *ht_search(ht *table, str key) {
  if (!_ht_is_valid(table) || !is_valid_str(key)) {
    return NULL;
  }

  uint64_t idx = _ht_index(_ht_hash(key), table->cap);
  for (;;) {
    ht_entry *e = &table->array[idx];
    if (e->key.data == NULL) {
      return NULL;
    } else if (are_equal(e->key, key)) {
      return e->value;
    }
    idx = ((idx + 1) % table->cap);
  }
}

// non-zero if failure
int ht_insert(ht *table, str key, void *value) {
#define MAX_ENTRIES 9007199254740992ULL
  if (!is_valid_str(key) || !_ht_is_valid(table)) {
    return 2;
  }
  // doubles cant exactly express more than 2^53
  if (table->elements >= MAX_ENTRIES || table->cap > MAX_ENTRIES) {
    return 2;
  }

  if (_ht_needs_to_grow(table->elements, table->cap)) {
    size_t new_cap;
    if (_ht_increase_cap(table->cap, &new_cap) != 0) {
      // handles overflow in this case
      return 1;
    }
    if (_ht_resize(table, new_cap) != 0) {
      return 1;
    }
  }

  uint64_t idx = _ht_index(_ht_hash(key), table->cap);
  // check for key equality, update ptr
  for (;;) {
    ht_entry *e = &table->array[idx];
    if (are_equal(e->key, key)) {
      e->value = value;
      return 0;
    } else if (e->key.data == NULL) {
      e->key.data = malloc(sizeof(char) * key.len);
      if (e->key.data == NULL) {
        return 1;
      }
      memcpy(e->key.data, key.data, key.len);
      e->key.len = key.len;
      e->value = value;
      table->elements += 1;
      return 0;
    }
    idx = ((idx + 1) % table->cap);
  }
  return 1;
}

// TODO: implement remove as a insert call with NULL as value
int ht_remove(ht *table, str key) {
  if (!is_valid_str(key) || !_ht_is_valid(table)) {
    return 2;
  }
  return 0;
}
