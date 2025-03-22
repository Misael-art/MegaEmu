#include <gtest/gtest.h>#include "../core/optimizer.hpp"#include "../core/sprite_cache.hpp"class OptimizerTests : public ::testing::Test {protected:    SpriteOptimizer optimizer;    SpriteCache cache;};TEST_F(OptimizerTests, OptimizeSpriteLoad) {    std::vector<uint8_t> testData = {1, 2, 3, 4};    auto optimized = optimizer.optimizeSpriteLoad(testData);    ASSERT_EQ(optimized.size(), testData.size());}TEST_F(OptimizerTests, CachePerformance) {    std::string key = "test_sprite";    std::vector<uint8_t> data = {1, 2, 3, 4};        cache.put(key, data);    auto cachedData = cache.get(key);        ASSERT_TRUE(cachedData != nullptr);    ASSERT_EQ(*cachedData, data);}