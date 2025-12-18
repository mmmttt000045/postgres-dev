#include <stdbool.h> // 用于 bool 类型
#include <stdint.h>  // 用于 int64_t, int8_t
#include <stdio.h>
#include <stdlib.h>

#include "affinity/utils/hash64.h"

// 哈希表节点
typedef struct Entry {
    affinity_key_t key;
    affinity_value_t value;
    struct Entry *next; // 链表指针，用于解决哈希冲突
} Entry;

// 哈希表主体
typedef struct HashTable {
    Entry **buckets; // 桶数组
    size_t size; // 桶的数量
    size_t count; // 当前元素数量（用于负载因子计算）
} HashTable;

/* ==========================================
 * 函数实现
 * ========================================== */

// 哈希函数：使用混合位运算优化哈希分布
// 基于 splitmix64 算法，确保更均匀的分布
static inline unsigned int hash(affinity_key_t key, size_t size) {
    affinity_ukey_t h = key;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return (unsigned int) (h % size);
}

// 1. 创建哈希表
// size 是桶的数量（建议根据数据量设置为质数，如 1024, 65537 等）
HashTable *ht_create(size_t size) {
    HashTable *ht = (HashTable *) malloc(sizeof(HashTable));
    if (!ht) return NULL;

    ht->buckets = (Entry **) calloc(size, sizeof(Entry *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }

    ht->size = size;
    ht->count = 0;
    return ht;
}

// 辅助函数：重新哈希以调整大小
static void ht_resize(HashTable *ht, size_t new_size) {
    Entry **old_buckets = ht->buckets;
    size_t old_size = ht->size;

    // 创建新的桶数组
    ht->buckets = (Entry **) calloc(new_size, sizeof(Entry *));
    if (!ht->buckets) {
        ht->buckets = old_buckets; // 恢复旧的桶
        fprintf(stderr, "Warning: Resize failed, keeping old size\n");
        return;
    }

    ht->size = new_size;
    ht->count = 0;

    // 重新插入所有元素
    for (size_t i = 0; i < old_size; i++) {
        Entry *current = old_buckets[i];
        while (current != NULL) {
            Entry *next = current->next;

            // 重新计算索引并插入
            unsigned int new_index = hash(current->key, new_size);
            current->next = ht->buckets[new_index];
            ht->buckets[new_index] = current;
            ht->count++;

            current = next;
        }
    }

    free(old_buckets);
}

// 2. 插入或更新 (Insert/Update)
// 自动扩容：当负载因子超过 0.75 时，扩大为原来的 2 倍
void ht_put(HashTable *ht, affinity_key_t key, affinity_value_t value) {
    unsigned int index;
    Entry *current;
    Entry *new_entry;

    // 检查是否需要扩容（负载因子 > 0.75）
    if (ht_load_factor(ht) > 0.75) {
        ht_resize(ht, ht->size * 2);
    }

    index = hash(key, ht->size);
    current = ht->buckets[index];

    // 遍历链表，查看 Key 是否已存在
    while (current != NULL) {
        if (current->key == key) {
            current->value = value; // 存在则更新
            return;
        }
        current = current->next;
    }

    // 不存在，使用头插法插入新节点
    new_entry = (Entry *) malloc(sizeof(Entry));
    if (!new_entry) {
        fprintf(stderr, "Error: Malloc failed in ht_put\n");
        return;
    }
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = ht->buckets[index]; // 新节点指向旧头节点
    ht->buckets[index] = new_entry; // 更新桶的头节点
    ht->count++;
}

// 3. 查找 (Get)
// 返回 true 表示找到，通过 out_value 传出值
// 返回 false 表示未找到
bool ht_get(HashTable *ht, affinity_key_t key, affinity_value_t *out_value) {
    unsigned int index = hash(key, ht->size);
    Entry *current = ht->buckets[index];

    while (current != NULL) {
        if (current->key == key) {
            if (out_value) {
                *out_value = current->value;
            }
            return true;
        }
        current = current->next;
    }
    return false;
}

affinity_value_t ht_get_value(HashTable *ht, affinity_key_t key) {
    unsigned int index = hash(key, ht->size);
    Entry *current = ht->buckets[index];

    while (current != NULL) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return -1; //理论上不会走到这里来
}

// 3a. 检查键是否存在 (Contains)
bool ht_contains(HashTable *ht, affinity_key_t key) {
    return ht_get(ht, key, NULL);
}

// 3b. 获取哈希表大小
size_t ht_size(HashTable *ht) {
    return ht->count;
}

// 3c. 获取负载因子
double ht_load_factor(HashTable *ht) {
    return (double) ht->count / (double) ht->size;
}

// 4. 删除 (Remove)
// 返回 true 表示删除成功，false 表示未找到
bool ht_remove(HashTable *ht, affinity_key_t key) {
    unsigned int index = hash(key, ht->size);
    Entry *current = ht->buckets[index];
    Entry *prev = NULL;

    while (current != NULL) {
        if (current->key == key) {
            // 找到了，执行删除
            if (prev == NULL) {
                // 删除的是头节点
                ht->buckets[index] = current->next;
            } else {
                // 删除的是中间或尾部节点
                prev->next = current->next;
            }
            free(current);
            ht->count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

// 5. 销毁哈希表 (Destroy)
void ht_destroy(HashTable *ht) {
    if (!ht) return;

    for (size_t i = 0; i < ht->size; i++) {
        Entry *current = ht->buckets[i];
        while (current != NULL) {
            Entry *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}

// 6. 清空哈希表 (Clear) - 保留结构，删除所有元素
void ht_clear(HashTable *ht) {
    if (!ht) return;

    for (size_t i = 0; i < ht->size; i++) {
        Entry *current = ht->buckets[i];
        while (current != NULL) {
            Entry *temp = current;
            current = current->next;
            free(temp);
        }
        ht->buckets[i] = NULL;
    }
    ht->count = 0;
}

/* ==========================================
 * Main 测试入口
 * ==========================================
int main() {
    // 创建一个包含 8 个桶的哈希表（小容量以测试自动扩容）
    HashTable *map = ht_create(8);
    printf("--- 哈希表已创建 (初始桶数: 8) ---\n");
    printf("当前元素数: %zu, 负载因子: %.2f\n", ht_size(map), ht_load_factor(map));

    // 1. 插入数据
    // Key: int64, Value: int8 (-128 ~ 127)
    ht_put(map, 1001, 1);
    ht_put(map, 2002, -50);
    ht_put(map, 99999999999, 127); // 大数值 Key
    printf("\n插入 3 个元素后:\n");
    printf("当前元素数: %zu, 负载因子: %.2f\n", ht_size(map), ht_load_factor(map));

    // 触发扩容：插入更多元素
    for (affinity_key_t i = 0; i < 10; i++) {
        ht_put(map, i * 1000, (affinity_value_t)(i % 100));
    }
    printf("\n插入更多元素后（应触发自动扩容）:\n");
    printf("当前元素数: %zu, 负载因子: %.2f\n", ht_size(map), ht_load_factor(map));

    // 2. 查询数据
    affinity_value_t val;
    affinity_key_t keys_to_find[] = {1001, 2002, 99999999999, 8888}; // 8888 是不存在的

    printf("\n--- 查询测试 ---\n");
    for (int i = 0; i < 4; i++) {
        affinity_key_t k = keys_to_find[i];
        if (ht_get(map, k, &val)) {
            printf("Key [%ld] => Value [%d]\n", (long)k, val);
        } else {
            printf("Key [%ld] => 未找到\n", (long)k);
        }
    }

    // 测试 ht_contains
    printf("\n--- 测试 ht_contains ---\n");
    printf("包含 Key 1001? %s\n", ht_contains(map, 1001) ? "是" : "否");
    printf("包含 Key 8888? %s\n", ht_contains(map, 8888) ? "是" : "否");

    // 3. 修改数据
    printf("\n--- 修改 Key 1001 的值为 99 ---\n");
    ht_put(map, 1001, 99);
    if (ht_get(map, 1001, &val)) {
        printf("Key [1001] 新的值 => [%d]\n", val);
    }

    // 4. 删除数据
    printf("\n--- 删除 Key 2002 ---\n");
    bool removed = ht_remove(map, 2002);
    printf("删除结果: %s\n", removed ? "成功" : "失败");
    if (!ht_get(map, 2002, &val)) {
        printf("Key [2002] 已确认删除\n");
    }
    printf("当前元素数: %zu\n", ht_size(map));

    // 5. 清空测试
    printf("\n--- 清空哈希表 ---\n");
    ht_clear(map);
    printf("清空后元素数: %zu, 负载因子: %.2f\n", ht_size(map), ht_load_factor(map));
    printf("包含 Key 1001? %s\n", ht_contains(map, 1001) ? "是" : "否");

    // 6. 销毁
    ht_destroy(map);
    printf("\n--- 哈希表已销毁，程序结束 ---\n");

    return 0;
}*/
