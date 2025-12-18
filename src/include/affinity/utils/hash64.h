/*-------------------------------------------------------------------------
 *
 * hash64.h
 *    Hash table implementation for int64 keys
 *
 * This module provides a hash table optimized for int64 keys with
 * automatic resizing and collision resolution using chaining.
 *
 * IDENTIFICATION
 *    src/include/affinity/utils/hash64.h
 *
 * Created by 明泰 on 2025/12/18.
 *
 *-------------------------------------------------------------------------
 */

#ifndef POSTGRES_DEV_HASH64_H
#define POSTGRES_DEV_HASH64_H

#include <stdbool.h>
#include <stdint.h>

/* ==========================================
 * Type definitions
 * ========================================== */

/* Key type: signed 64-bit integer */
typedef int64_t affinity_key_t;

/* Unsigned key type for hash computation */
typedef uint64_t affinity_ukey_t;

/* Value type: unsigned 32-bit integer */
typedef uint32_t affinity_value_t;

/* Forward declaration of HashTable structure */
typedef struct HashTable HashTable;

/* ==========================================
 * Hash table function declarations
 * ========================================== */

/*
 * ht_create
 *
 * Create a new hash table with the specified number of buckets.
 *
 * Parameters:
 *   size - Number of buckets (recommend prime numbers like 1024, 65537)
 *
 * Returns:
 *   Pointer to newly created HashTable, or NULL on failure
 */
extern HashTable *ht_create(size_t size);

/*
 * ht_put
 *
 * Insert or update a key-value pair in the hash table.
 * Automatically resizes when load factor exceeds 0.75.
 *
 * Parameters:
 *   ht    - Hash table
 *   key   - Key to insert/update
 *   value - Value to associate with the key
 */
extern void ht_put(HashTable *ht, affinity_key_t key, affinity_value_t value);

/*
 * ht_get
 *
 * Retrieve a value from the hash table.
 *
 * Parameters:
 *   ht        - Hash table
 *   key       - Key to look up
 *   out_value - Pointer to store the retrieved value (can be NULL)
 *
 * Returns:
 *   true if key was found, false otherwise
 */
extern bool ht_get(HashTable *ht, affinity_key_t key, affinity_value_t *out_value);

/*
 * ht_get_value
 *
 * Retrieve a value from the hash table (simplified version).
 *
 * Parameters:
 *   ht  - Hash table
 *   key - Key to look up
 *
 * Returns:
 *   Value associated with the key, or -1 if not found
 */
extern affinity_value_t ht_get_value(HashTable *ht, affinity_key_t key);

/*
 * ht_contains
 *
 * Check if a key exists in the hash table.
 *
 * Parameters:
 *   ht  - Hash table
 *   key - Key to check
 *
 * Returns:
 *   true if key exists, false otherwise
 */
extern bool ht_contains(HashTable *ht, affinity_key_t key);

/*
 * ht_size
 *
 * Get the number of elements in the hash table.
 *
 * Parameters:
 *   ht - Hash table
 *
 * Returns:
 *   Number of key-value pairs currently stored
 */
extern size_t ht_size(HashTable *ht);

/*
 * ht_load_factor
 *
 * Calculate the current load factor (count / bucket_size).
 *
 * Parameters:
 *   ht - Hash table
 *
 * Returns:
 *   Load factor as a double
 */
extern double ht_load_factor(HashTable *ht);

/*
 * ht_remove
 *
 * Remove a key-value pair from the hash table.
 *
 * Parameters:
 *   ht  - Hash table
 *   key - Key to remove
 *
 * Returns:
 *   true if key was found and removed, false otherwise
 */
extern bool ht_remove(HashTable *ht, affinity_key_t key);

/*
 * ht_destroy
 *
 * Destroy the hash table and free all associated memory.
 *
 * Parameters:
 *   ht - Hash table to destroy
 */
extern void ht_destroy(HashTable *ht);

/*
 * ht_clear
 *
 * Remove all elements from the hash table but keep the structure.
 *
 * Parameters:
 *   ht - Hash table to clear
 */
extern void ht_clear(HashTable *ht);

#endif /* POSTGRES_DEV_HASH64_H */
