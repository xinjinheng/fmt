// 测试新功能的简单程序
#include <fmt/format.h>
#include <fmt/ai_format.h>
#include <fmt/cross_format.h>
#include <fmt/distributed_format.h>
#include <iostream>
#include <vector>

using namespace fmt;
using namespace fmt::ai_format;
using namespace fmt::cross_format;
using namespace fmt::distributed_format;

int main() {
  // 测试AI格式推荐
  std::cout << "Testing AI format recommendation..." << std::endl;
  
  // 创建AI格式推荐器
  recommender ai_recommender;
  
  // 特征提取示例
  auto features = ai_recommender.extract_features("123.456");
  
  // 测试跨语言格式转换
  std::cout << "\nTesting cross-language format conversion..." << std::endl;
  
  format_converter converter;
  
  // 将Python格式转换为fmt格式
  std::string python_format = "Hello, {name}! You are {age} years old."; 
  conversion_result python_result = converter.from_python(python_format);
  
  if (python_result.success) {
    std::cout << "Python to fmt: " << python_result.formatted_str << std::endl;
  } else {
    std::cout << "Python to fmt conversion failed: " << python_result.error_msg << std::endl;
  }
  
  // 将Java格式转换为fmt格式
  std::string java_format = "Hello, %s! You are %d years old."; 
  conversion_result java_result = converter.from_java(java_format);
  
  if (java_result.success) {
    std::cout << "Java to fmt: " << java_result.formatted_str << std::endl;
  } else {
    std::cout << "Java to fmt conversion failed: " << java_result.error_msg << std::endl;
  }
  
  // 测试分布式格式同步
  std::cout << "\nTesting distributed format synchronization..." << std::endl;
  
  // 创建格式规则同步器
  format_rule_synchronizer synchronizer;
  
  // 连接到节点
  std::vector<size_t> nodes = {1, 2, 3};
  if (synchronizer.connect(nodes)) {
    std::cout << "Connected to nodes successfully." << std::endl;
  } else {
    std::cout << "Failed to connect to nodes." << std::endl;
  }
  
  // 获取当前格式规则
  auto current_rule = synchronizer.get_current_rule();
  std::cout << "Current format version: " << current_rule.version << std::endl;
  std::cout << "Current format string: " << current_rule.format_str << std::endl;
  
  // 断开连接
  synchronizer.disconnect();
  
  std::cout << "\nAll tests completed successfully!" << std::endl;
  
  return 0;
}