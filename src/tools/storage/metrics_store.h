#include <stdint.h>typedef struct {    CompressionMethod method;    uint32_t block_size;    float compression_ratio;        void (*compress_block)(const DataBlock* block);    DataBlock* (*decompress_block)(const CompressedBlock* block);    void (*optimize_storage)(void);} StorageOptimizer;typedef struct {    StorageOptimizer* optimizer;    DatabaseConnection* db;    CacheManager* cache;        void (*store_metrics)(const MetricsData* data);    MetricsData* (*retrieve_metrics)(const TimeRange* range);    void (*cleanup_old_data)(uint32_t days_to_keep);    void (*optimize_database)(void);} MetricsStore;MetricsStore* metrics_store_create(const StorageConfig* config);