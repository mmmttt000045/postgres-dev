#include "postgres.h"
#include "affinity/affinity.h"
#include "utils/rel.h"

#include <time.h>

#include "access/htup.h"
#include "access/htup_details.h"

#define INIT_BULK_SIZE 20480

static HashTable *partition_map;
const int AFFINITY_COL_ID = 1; // 假设分区键在第 1 列

bool load_partition_map_disk(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("Failed to open partition map file");
        return false;
    }

    // 检查 partition_map 是否已初始化
    if (!partition_map) {
        fprintf(stderr, "Error: partition_map not initialized\n");
        fclose(file);
        return false;
    }

    // 逐行读取文件
    int key, value;
    size_t line_count = 0;

    while (fscanf(file, "%d\t%d\n", &key, &value) == 2) {
        ht_put(partition_map, (affinity_key_t) key, (affinity_value_t) value);
        line_count++;
        int8_t tmp;
        if (value != ht_get_value(partition_map, (affinity_key_t) key)) {
            fprintf(stderr, "Error: Mismatch after inserting key %d\n", key);
            fclose(file);
            exit(-1);
        }
    }

    fclose(file);

    printf("Affinity : Successfully loaded %zu key-value pairs from %s\n", line_count, filepath);
    return true;
}

bool init_partition_map() {
    const char *filepath = "/Users/mingtai/CLionProjects/postgres-dev/src/backend/affinity/utils/test.txt";

    if (partition_map != NULL) {
        fprintf(stderr, "Warning: partition_map already initialized\n");
        return true;
    }

    partition_map = ht_create(INIT_BULK_SIZE);
    if (!partition_map) {
        fprintf(stderr, "Error: Failed to create partition_map\n");
        return false;
    }
    return load_partition_map_disk(filepath);
}

/* Query partition for a given key */
affinity_value_t query_partition(affinity_key_t key) {
    affinity_value_t value = -1;
    if (partition_map && ht_get(partition_map, key, &value)) {
        return value;
    }
    return -1;
}

/* Cleanup partition map */
void cleanup_partition_map(void) {
    if (partition_map) {
        ht_destroy(partition_map);
        partition_map = NULL;
    }
}

/*
 * Extract the affinity key from a HeapTuple
 * This function reads the value from AFFINITY_COL_ID column and returns it as affinity_key_t
 **/
affinity_value_t get_affinity_value(Relation relation, HeapTuple tup) {
    TupleDesc tupleDesc;
    bool isnull;
    Datum colValue;
    affinity_value_t value;
    affinity_key_t key;

    /* Get the tuple descriptor for the relation */
    tupleDesc = RelationGetDescr(relation);

    /* Extract the value from AFFINITY_COL_ID column (column 1) */
    colValue = heap_getattr(tup, AFFINITY_COL_ID, tupleDesc, &isnull);

    if (isnull) {
        /* If the column is NULL, return -1 or handle error */
        elog(WARNING, "Affinity key column (column %d) is NULL", AFFINITY_COL_ID);
        return (affinity_key_t) -1;
    }

    /* Convert Datum to affinity_key_t (int64_t) */
    key = (affinity_key_t) DatumGetInt64(colValue);

    if (ht_get(partition_map, key, &value))
        return value;
    else {
        elog(WARNING, "Affinity key %ld not found in partition map", key);
        return (affinity_value_t) 0;
    }
}

