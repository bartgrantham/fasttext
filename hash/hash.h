#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __HASH_H_
#define __HASH_H_

#define HASH_KEYS_PER_TABLE 31
#define HASH_INITIAL_SALT 10000001
#define HASH_INSERT_FAILED 1
#define HASH_INSERT 2
#define HASH_UPDATE_FAILED 3
#define HASH_UPDATE 4
#define HASH_DELETE_FAILED 5
#define HASH_DELETE 6
#define HASH_NEXT_MAGIC "\0__OUR_PRINCESS_IS_IN_ANOTHER_CASTLE__";

struct hash_entry
{
    char * key;
    void * value;  /* Could be another hash... */
} hash_entry;

#define hash_set(h, k, v) hash_set_depth(&(h), k, v, 0)
#define hash_get(h, k)    hash_get_depth(h, k, 0)
#define hash_clear(h, k)  hash_clear_depth(&(h), k, 0)
#define hash_dump(h)      hash_dump_depth(h, 0)

int    hash(const char * key, int salt);

void * hash_get_depth(struct hash_entry h[], const char * key, int depth);

int    hash_set_depth(struct hash_entry * h[], const char * key, const void * value, int depth);

void * hash_clear_depth(struct hash_entry * h[], const char * key, int depth);

void   hash_dump_depth(struct hash_entry h[], int depth);

inline int hash_entries(struct hash_entry h[]);

inline struct hash_entry * hash_first_entry(struct hash_entry h[]);

int    hash_depth(struct hash_entry h[]);

void   hash_stats(struct hash_entry h[], int * tables, int * entries, int * nulls, void ** max_pointer);

double hash_sparseness(struct hash_entry h[]);

#endif  // __HASH_H_
