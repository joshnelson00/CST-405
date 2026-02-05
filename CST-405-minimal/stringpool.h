#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#include <stddef.h>

/* STRING INTERNING
 * Stores a single shared copy of each unique string.
 * Useful for identifiers (variables, temporaries) used across AST/symtab.
 */

#ifndef STRING_POOL_SIZE
#define STRING_POOL_SIZE 65536
#endif

#ifndef STRING_HASH_SIZE
#define STRING_HASH_SIZE 4093
#endif

typedef struct StringNode {
    char* str;
    struct StringNode* next;
} StringNode;

typedef struct PoolBlock {
    char* mem;
    size_t used;
    size_t size;
    struct PoolBlock* next;
} PoolBlock;

typedef struct {
    PoolBlock* head;
    PoolBlock* current;
    size_t unique_strings;
    size_t total_requests;
    StringNode* buckets[STRING_HASH_SIZE];
} StringPool;

void init_string_pool(void);
char* intern_string(const char* str);
void free_string_pool(void);
size_t string_pool_count(void);
size_t string_pool_requests(void);

#endif
