#include "utils/hash64.c"
#include <time.h>

#define INIT_BULK_SIZE 20480

static HashTable *partition_map;

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
        ht_put(partition_map, (key_t) key, (value_t) value);
        line_count++;
        int8_t tmp;
        if (value != ht_get_value(partition_map, (key_t) key)) {
            fprintf(stderr, "Error: Mismatch after inserting key %d\n", key);
            fclose(file);
            exit(-1);
        }
    }

    fclose(file);

    printf("Successfully loaded %zu key-value pairs from %s\n", line_count, filepath);
    return true;
}

bool init_partition_map() {
    if (partition_map != NULL) {
        fprintf(stderr, "Warning: partition_map already initialized\n");
        return true;
    }

    partition_map = ht_create(INIT_BULK_SIZE);
    if (!partition_map) {
        fprintf(stderr, "Error: Failed to create partition_map\n");
        return false;
    }
    const char *filepath = "/Users/mingtai/CLionProjects/postgres-dev/src/backend/affinty/utils/test.txt";
    return load_partition_map_disk(filepath);
}

/* Query partition for a given key */
value_t query_partition(key_t key) {
    value_t value = -1;
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
