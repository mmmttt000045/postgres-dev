/*-------------------------------------------------------------------------
 *
 * affinity.h
 *    Partition affinity mapping interface
 *    目前为了测试方便，只是简单地使用了文件路径硬编码，而且只允许一个表的一个列
 * This module provides functionality for managing partition mappings
 * using a hash table. It loads partition assignments from disk and
 * provides efficient lookup capabilities.
 *
 *-------------------------------------------------------------------------
 */
#ifndef AFFINTY_H
#define AFFINTY_H

#include "postgres.h"
#include "affinity/utils/hash64.h"

/* Forward declarations */
typedef struct RelationData *Relation;
typedef struct HeapTupleData *HeapTuple;

/*
 * Initialize the partition map from a file.
 *
 * filepath: Path to the partition map file containing tab-separated
 *           key-value pairs (one per line: "key\tvalue\n")
 *
 * Returns: true on success, false on failure
 */
extern bool init_partition_map(void);

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
 * Returns: The partition value (affinity_value_t) on success, -1 if not found
 */
extern affinity_value_t query_partition(affinity_key_t key);

/*
 * Cleanup and free the partition map.
 *
 * This should be called when the partition map is no longer needed
 * to release allocated memory.
 */
extern void cleanup_partition_map(void);

/*
 * Extract the affinity key from a HeapTuple.
 *
 * relation: The relation (table) that the tuple belongs to
 * tup: The HeapTuple to extract the key from
 *
 * Returns: The affinity key (affinity_key_t) extracted from AFFINITY_COL_ID column,
 *          or -1 if the column is NULL or an error occurs
 */
extern affinity_value_t get_affinity_value(Relation relation, HeapTuple tup);

#endif /* AFFINTY_H */

