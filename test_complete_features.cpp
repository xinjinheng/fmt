#include <fmt/format.h>
#include <fmt/ai_format.h>
#include <fmt/cross_format.h>
#include <fmt/distributed_format.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>

using namespace fmt;
using namespace fmt::ai_format;
using namespace fmt::cross_format;
using namespace fmt::distributed_format;

int main() {
    std::cout << "Testing AI Format Recommendation System...\n";
    
    // Test 1: AI Format Recommendation
    double pi = 3.1415926535;
    int large_num = 123456789;
    std::string url = "https://fmt.dev";
    std::string date = "2024-01-01";
    
    // 创建推荐器
    recommender recommender;
    
    // 测试数字格式推荐
    format_recommendation rec_pi = recommender.recommend(pi, context::printing);
    format_recommendation rec_large = recommender.recommend(large_num, context::logging);
    
    std::cout << "AI Recommendation for pi (printing): " << rec_pi.format_spec << " -> " << fmt::format(rec_pi.format_spec, pi) << std::endl;
    std::cout << "AI Recommendation for large number (logging): " << rec_large.format_spec << " -> " << fmt::format(rec_large.format_spec, large_num) << std::endl;
    
    // 测试字符串格式推荐
    format_recommendation rec_url = recommender.recommend(url, context::network);
    format_recommendation rec_date = recommender.recommend(date, context::database);
    
    std::cout << "AI Recommendation for URL (network): " << rec_url.format_spec << " -> " << fmt::format(rec_url.format_spec, url) << std::endl;
    std::cout << "AI Recommendation for date (database): " << rec_date.format_spec << " -> " << fmt::format(rec_date.format_spec, date) << std::endl;
    
    std::cout << "\nTesting Cross-Language Format Conversion...\n";
    
    // Test 2: Cross-Language Format Conversion
    std::string fmt_format = "Hello, {}! Today is {} and the pi is {:.2f}";
    
    // 转换为Python格式
    std::string python_format = convert(fmt_format, language::fmt, language::python_format);
    std::cout << "fmt -> Python: " << python_format << std::endl;
    
    // 转换为Java格式
    std::string java_format = convert(fmt_format, language::fmt, language::java_formatter);
    std::cout << "fmt -> Java: " << java_format << std::endl;
    
    // 转换为C printf格式
    std::string c_format = convert(fmt_format, language::fmt, language::c_printf);
    std::cout << "fmt -> C printf: " << c_format << std::endl;
    
    std::cout << "\nTesting Distributed Formatting...\n";
    
    // Test 3: Distributed Formatting
    
    // 创建Raft格式管理器
    auto format_manager = std::make_shared<raft_format_manager>(1);
    format_manager->start();
    
    // 设置格式规则
    format_manager->set_format_rule("{:.4f}");
    
    // 创建并行格式化器
    parallel_formatter<double> formatter(4);
    formatter.set_format_manager(format_manager);
    
    // 生成大量数据
    std::vector<double> data;
    for (int i = 0; i < 1000; ++i) {
        data.push_back(3.1415926535 + i * 0.001);
    }
    
    // 并行格式化
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::string> results = formatter.format(data);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "Parallel formatting completed in " << 
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms" << std::endl;
    std::cout << "First 5 results: " << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "  " << results[i] << std::endl;
    }
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    
    return 0;
}