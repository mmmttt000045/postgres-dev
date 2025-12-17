//
// Created by 明泰 on 2025/12/17.
//

/*-------------------------------------------------------------------------
 *
 * affinty.h
 *    Partition affinity mapping interface
 *
 * This module provides functionality for managing partition mappings
 * using a hash table. It loads partition assignments from disk and
 * provides efficient lookup capabilities.
 *
 *-------------------------------------------------------------------------
 */
#ifndef AFFINTY_H
#define AFFINTY_H

#include <stdbool.h>
#include <stdint.h>

/* Type definitions from hash64.c */
typedef int64_t key_t;
typedef uint32_t value_t;

/*
 * Initialize the partition map from a file.
 *
 * filepath: Path to the partition map file containing tab-separated
 *           key-value pairs (one per line: "key\tvalue\n")
 *
 * Returns: true on success, false on failure
 */
extern bool init_partition_map();

/*
 * Load partition map data from disk.
 *
 * This function is called internally by init_partition_map().
 * The partition_map must be initialized before calling this.
 *
 * filepath: Path to the partition map file
 *
 * Returns: true on success, false on failure
 */
extern bool load_partition_map_disk(const char *filepath);

/*
 * Query the partition for a given key.
 *
 * key: The key to look up in the partition map
 *
 * Returns: The partition value (value_t) on success, -1 if not found
 */
extern value_t query_partition(key_t key);

/*
 * Cleanup and free the partition map.
 *
 * This should be called when the partition map is no longer needed
 * to release allocated memory.
 */
extern void cleanup_partition_map(void);

#endif /* AFFINTY_H */

