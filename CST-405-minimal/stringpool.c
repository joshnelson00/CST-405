#include <stdlib.h>
#include <string.h>
#include "stringpool.h"

// stringpool.c implementation
StringPool string_pool = {0};

static unsigned int hash_string(const char* str) {
    unsigned int hash = 0;
    const unsigned char* p = (const unsigned char*)str;
    while (*p) {
        hash = hash * 31u + *p++;
    }
    return hash % STRING_HASH_SIZE;
}

static PoolBlock* alloc_pool_block(size_t min_size) {
    size_t size = (min_size > STRING_POOL_SIZE) ? min_size : STRING_POOL_SIZE;
    PoolBlock* block = malloc(sizeof(PoolBlock));
    if (!block) return NULL;
    block->mem = malloc(size);
    if (!block->mem) {
        free(block);
        return NULL;
    }
    block->used = 0;
    block->size = size;
    block->next = NULL;
    return block;
}

void init_string_pool(void) {
    if (string_pool.head || string_pool.current) {
        free_string_pool();
    }

    memset(string_pool.buckets, 0, sizeof(string_pool.buckets));
    string_pool.unique_strings = 0;
    string_pool.total_requests = 0;

    string_pool.head = alloc_pool_block(STRING_POOL_SIZE);
    string_pool.current = string_pool.head;
}

char* intern_string(const char* str) {
    if (!str) return NULL;
    if (!string_pool.current) {
        init_string_pool();
        if (!string_pool.current) return NULL;
    }

    string_pool.total_requests++;

    unsigned int hash = hash_string(str);

    // Check if already interned
    StringNode* node = string_pool.buckets[hash];
    while (node) {
        if (strcmp(node->str, str) == 0) {
            return node->str;  // Return existing
        }
        node = node->next;
    }

    // Add new string
    size_t len = strlen(str) + 1;
    if (string_pool.current->used + len > string_pool.current->size) {
        PoolBlock* new_block = alloc_pool_block(len);
        if (!new_block) {
            return NULL;
        }
        string_pool.current->next = new_block;
        string_pool.current = new_block;
    }

    char* interned = string_pool.current->mem + string_pool.current->used;
    memcpy(interned, str, len);
    string_pool.current->used += len;

    // Add to hash table
    StringNode* new_node = malloc(sizeof(StringNode));
    if (!new_node) {
        return interned; // Best effort; string is in pool
    }
    new_node->str = interned;
    new_node->next = string_pool.buckets[hash];
    string_pool.buckets[hash] = new_node;

    string_pool.unique_strings++;
    return interned;
}

void free_string_pool(void) {
    // Free hash table nodes
    for (int i = 0; i < STRING_HASH_SIZE; i++) {
        StringNode* node = string_pool.buckets[i];
        while (node) {
            StringNode* next = node->next;
            free(node);
            node = next;
        }
        string_pool.buckets[i] = NULL;
    }

    // Free all pool blocks
    PoolBlock* block = string_pool.head;
    while (block) {
        PoolBlock* next = block->next;
        free(block->mem);
        free(block);
        block = next;
    }

    string_pool.head = NULL;
    string_pool.current = NULL;
    string_pool.unique_strings = 0;
    string_pool.total_requests = 0;
}

size_t string_pool_count(void) {
    return string_pool.unique_strings;
}

size_t string_pool_requests(void) {
    return string_pool.total_requests;
}
