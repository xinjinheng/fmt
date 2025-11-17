// 分布式场景下的并行格式化与一致性保障机制
// Copyright (c) 2024 fmtlib
// License: MIT

#ifndef FMT_DISTRIBUTED_FORMAT_H_
#define FMT_DISTRIBUTED_FORMAT_H_

#include "format.h"
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <atomic>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <optional>
#include <thread>
#include <cstdio>
#include <openssl/sha.h>

FMT_BEGIN_NAMESPACE

/**
 * @brief 分布式格式化命名空间
 */
namespace distributed_format {

/**
 * @brief 格式规则版本信息
 */
struct format_rule_version {
  uint64_t version;               ///< 版本号
  std::string format_str;         ///< 格式字符串
  uint64_t timestamp;             ///< 时间戳
  std::string checksum;           ///< 校验和（SHA-1）
  
  /**
   * @brief 比较版本
   */
  bool operator>(const format_rule_version& other) const {
    return version > other.version;
  }
  
  bool operator<(const format_rule_version& other) const {
    return version < other.version;
  }
  
  bool operator==(const format_rule_version& other) const {
    return version == other.version && checksum == other.checksum;
  }
};

/**
 * @brief 数据分片
 */
template <typename T>
struct data_shard {
  std::vector<T> data;            ///< 分片数据
  size_t shard_id;                ///< 分片ID
  size_t total_shards;            ///< 总分片数
  bool is_last_shard;             ///< 是否为最后一个分片
};

/**
 * @brief 并行格式化结果
 */
template <typename T>
struct format_result {
  std::vector<std::string> formatted_data;  ///< 格式化后的数据
  format_rule_version used_rule;            ///< 使用的格式规则版本
  size_t shard_id;                          ///< 分片ID
  bool success;                             ///< 格式化是否成功
  std::string error_message;                ///< 错误信息（如果失败）
};

/**
 * @brief Raft节点状态
 */
enum class raft_node_state {
  follower,    ///< 追随者
  candidate,   ///< 候选人
  leader       ///< 领导人
};

/**
 * @brief 简化版Raft协议实现
 * 用于格式规则的版本控制与原子更新
 */
class raft_format_manager {
public:
  explicit raft_format_manager(size_t node_id) : node_id_(node_id), state_(raft_node_state::follower) {
    // 初始化
    current_term_ = 0;
    voted_for_ = std::nullopt;
    commit_index_ = 0;
    last_applied_ = 0;
  }
  
  /**
   * @brief 开始节点
   */
  void start() {
    // 启动选举定时器
    election_timer_ = std::thread(&raft_format_manager::election_timeout_loop, this);
  }
  
  /**
   * @brief 设置格式规则
   */
  bool set_format_rule(const std::string& format_str) {
    // 生成新的版本信息
    format_rule_version new_version;
    new_version.version = current_version_.version + 1;
    new_version.format_str = format_str;
    new_version.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    new_version.checksum = generate_sha1(format_str);
    
    // 如果是领导者，直接应用
    if (state_ == raft_node_state::leader) {
      current_version_ = new_version;
      return true;
    }
    
    // 否则，需要转发给领导者
    // 这里简化实现，直接返回false
    return false;
  }
  
  /**
   * @brief 获取当前格式规则
   */
  const format_rule_version& get_current_format_rule() const {
    return current_version_;
  }
  
private:
  /**
   * @brief 选举超时循环
   */
  void election_timeout_loop() {
    while (true) {
      // 随机选举超时（150-300ms）
      std::this_thread::sleep_for(std::chrono::milliseconds(150 + rand() % 151));
      
      // 如果还是追随者，发起选举
      if (state_ == raft_node_state::follower) {
        start_election();
      }
    }
  }
  
  /**
   * @brief 开始选举
   */
  void start_election() {
    // 切换为候选人
    state_ = raft_node_state::candidate;
    ++current_term_;
    voted_for_ = node_id_;
    
    // 这里简化实现，直接成为领导者
    state_ = raft_node_state::leader;
  }
  
  /**
   * @brief 生成SHA-1校验和
   */
  std::string generate_sha1(const std::string& data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);
    
    // 转换为十六进制字符串
    char hex_hash[SHA_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
      sprintf(hex_hash + i * 2, "%02x", hash[i]);
    }
    hex_hash[SHA_DIGEST_LENGTH * 2] = '\0';
    
    return std::string(hex_hash);
  }
  
  size_t node_id_;
  raft_node_state state_;
  uint64_t current_term_;
  std::optional<size_t> voted_for_;
  size_t commit_index_;
  size_t last_applied_;
  format_rule_version current_version_;
  std::thread election_timer_;
  
  // 构造函数和析构函数
  ~raft_format_manager() {
    // 处理线程
    if (election_timer_.joinable()) {
      election_timer_.detach();
    }
  }
};
  /**
   * @brief 停止节点
   */
  void stop();
  
  /**
   * @brief 更新格式规则
   * @return 是否成功
   */
  bool update_format_rule(const std::string& new_format);
  
  /**
   * @brief 获取当前格式规则
   * @return 格式规则版本
   */
  format_rule_version get_current_rule() const;
  
  /**
   * @brief 检查格式规则是否为最新
   * @return 是否为最新版本
   */
  bool is_latest_version() const;
  
  /**
   * @brief 获取节点ID
   */
  size_t node_id() const { return node_id_; }
  
  /**
   * @brief 获取节点状态
   */
  raft_node_state state() const { return state_; }
  
private:
  size_t node_id_;
  raft_node_state state_;
  std::atomic<uint64_t> current_term_;
  std::optional<size_t> voted_for_;
  std::atomic<uint64_t> commit_index_;
  std::atomic<uint64_t> last_applied_;
  format_rule_version current_rule_;
  
  // Raft日志条目
  struct log_entry {
    uint64_t term;
    format_rule_version rule;
  };
  std::vector<log_entry> log_;
  
  // 选举定时器
  std::atomic<bool> election_timer_running_;
  std::future<void> election_future_;
  
  // 心跳定时器
  std::atomic<bool> heartbeat_timer_running_;
  std::future<void> heartbeat_future_;
  
  /**
   * @brief 发起选举
   */
  void start_election();
  
  /**
   * @brief 处理选举超时
   */
  void handle_election_timeout();
  
  /**
   * @brief 发送请求投票
   */
  int send_request_votes();
  
  /**
   * @brief 处理请求投票
   */
  bool handle_request_vote(size_t candidate_id, uint64_t term);
  
  /**
   * @brief 发送心跳
   */
  void send_heartbeats();
  
  /**
   * @brief 处理心跳
   */
  bool handle_heartbeat(size_t leader_id, uint64_t term);
  
  /**
   * @brief 复制日志条目
   */
  bool replicate_log(const log_entry& entry);
  
  /**
   * @brief 生成校验和
   */
  std::string generate_checksum(const std::string& data) const;
  
  /**
   * @brief 重置选举定时器
   */
  void reset_election_timer();
};

/**
 * @brief 并行格式化引擎
 */
template <typename T>
class parallel_formatter {
public:
  using formatter_func = std::function<std::string(const T&, const format_rule_version&)>;
  
  /**
   * @brief 构造函数
   * @param formatter 格式化函数
   */
  explicit parallel_formatter(formatter_func formatter = default_formatter)
      : formatter_(std::move(formatter)) {
  }
  
  /**
   * @brief 并行格式化多个分片
   * @param shards 数据分片
   * @param rule 格式规则版本
   * @param max_threads 最大线程数（0表示自动选择）
   * @return 格式化结果
   */
  std::vector<format_result<T>> format(
      const std::vector<data_shard<T>>& shards,
      const format_rule_version& rule,
      size_t max_threads = 0) const;
  
  /**
   * @brief 并行格式化单个数据集合
   * @param data 数据集合
   * @param rule 格式规则版本
   * @param num_shards 分片数
   * @param max_threads 最大线程数
   * @return 格式化结果
   */
  std::vector<std::string> format_single(
      const std::vector<T>& data,
      const format_rule_version& rule,
      size_t num_shards = 0,
      size_t max_threads = 0) const;
  
  /**
   * @brief 设置自定义格式化函数
   */
  void set_formatter(formatter_func formatter) {
    formatter_ = std::move(formatter);
  }
  
  /**
   * @brief 设置格式规则管理器
   */
  void set_format_manager(std::shared_ptr<raft_format_manager> manager) {
    format_manager_ = manager;
  }
  
private:
  formatter_func formatter_;
  std::shared_ptr<raft_format_manager> format_manager_;
  
  /**
   * @brief 默认格式化函数
   */
  static std::string default_formatter(const T& data, const format_rule_version& rule) {
    return fmt::format(rule.format_str, data);
  }
  
  /**
   * @brief 计算最佳分片数
   */
  static size_t calculate_optimal_shards(size_t data_size) {
    // 简单策略：每1000个元素一个分片
    return (data_size + 999) / 1000;
  }
};

/**
 * @brief 格式规则同步器
 */
class format_rule_synchronizer {
public:
  /**
   * @brief 构造函数
   */
  format_rule_synchronizer() : raft_manager_(generate_node_id()) {
  }
  
  /**
   * @brief 连接到分布式集群
   * @param nodes 节点列表
   * @return 是否成功
   */
  bool connect(const std::vector<size_t>& nodes);
  
  /**
   * @brief 更新格式规则
   * @param new_format 新的格式字符串
   * @return 是否成功
   */
  bool update_rule(const std::string& new_format);
  
  /**
   * @brief 获取当前格式规则
   * @return 格式规则版本
   */
  format_rule_version get_current_rule() const;
  
  /**
   * @brief 等待格式规则更新
   * @param timeout 超时时间
   * @return 是否成功
   */
  bool wait_for_update(std::chrono::milliseconds timeout = std::chrono::seconds(10));
  
  /**
   * @brief 断开连接
   */
  void disconnect();
  
private:
  raft_format_manager raft_manager_;
  std::unordered_map<size_t, std::string> node_addresses_;
  
  /**
   * @brief 生成节点ID
   */
  static size_t generate_node_id();
};

/**
 * @brief 分布式格式上下文
 */
class distributed_format_context {
public:
  /**
   * @brief 构造函数
   */
  distributed_format_context() : synchronizer_() {
  }
  
  /**
   * @brief 初始化上下文
   * @param cluster_nodes 集群节点
   * @return 是否成功
   */
  bool initialize(const std::vector<size_t>& cluster_nodes) {
    return synchronizer_.connect(cluster_nodes);
  }
  
  /**
   * @brief 设置格式规则
   * @param format_str 格式字符串
   * @return 是否成功
   */
  bool set_format(const std::string& format_str) {
    return synchronizer_.update_rule(format_str);
  }
  
  /**
   * @brief 并行格式化数据
   * @param data 数据集合
   * @param num_shards 分片数
   * @param max_threads 最大线程数
   * @return 格式化后的数据
   */
  template <typename T>
  std::vector<std::string> format(
      const std::vector<T>& data,
      size_t num_shards = 0,
      size_t max_threads = 0) const {
    
    auto current_rule = synchronizer_.get_current_rule();
    if (current_rule.format_str.empty()) {
      throw std::runtime_error("no format rule set");
    }
    
    parallel_formatter<T> formatter;
    return formatter.format_single(data, current_rule, num_shards, max_threads);
  }
  
  /**
   * @brief 格式化单个数据
   * @param data 数据
   * @return 格式化后的字符串
   */
  template <typename T>
  std::string format_single(const T& data) const {
    auto current_rule = synchronizer_.get_current_rule();
    if (current_rule.format_str.empty()) {
      throw std::runtime_error("no format rule set");
    }
    return fmt::format(current_rule.format_str, data);
  }
  
  /**
   * @brief 获取当前格式规则
   */
  format_rule_version get_current_rule() const {
    return synchronizer_.get_current_rule();
  }
  
private:
  format_rule_synchronizer synchronizer_;
};

// 模板实现

template <typename T>
std::vector<format_result<T>> parallel_formatter<T>::format(
    const std::vector<data_shard<T>>& shards,
    const format_rule_version& rule,
    size_t max_threads) const {
  
  size_t num_threads = max_threads;
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
      num_threads = 4; // 默认4线程
    }
  }
  
  // 任务队列
  std::vector<std::future<format_result<T>>> futures;
  futures.reserve(shards.size());
  
  // 提交任务
  for (const auto& shard : shards) {
    futures.emplace_back(std::async(std::launch::async, [&, shard] {
      format_result<T> result;
      result.shard_id = shard.shard_id;
      result.used_rule = rule;
      result.success = true;
      
      try {
        result.formatted_data.reserve(shard.data.size());
        for (const auto& item : shard.data) {
          result.formatted_data.push_back(formatter_(item, rule));
        }
      } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
      }
      
      return result;
    }));
  }
  
  // 收集结果
  std::vector<format_result<T>> results;
  results.reserve(futures.size());
  
  for (auto& future : futures) {
    try {
      results.push_back(future.get());
    } catch (const std::exception& e) {
      format_result<T> error_result;
      error_result.success = false;
      error_result.error_message = e.what();
      results.push_back(std::move(error_result));
    }
  }
  
  return results;
}

template <typename T>
std::vector<std::string> parallel_formatter<T>::format_single(
    const std::vector<T>& data,
    const format_rule_version& rule,
    size_t num_shards,
    size_t max_threads) const {
  
  if (num_shards == 0) {
    num_shards = calculate_optimal_shards(data.size());
  }
  
  // 数据分片
  std::vector<data_shard<T>> shards;
  shards.reserve(num_shards);
  
  size_t shard_size = (data.size() + num_shards - 1) / num_shards;
  for (size_t i = 0; i < num_shards; ++i) {
    size_t start = i * shard_size;
    size_t end = std::min(start + shard_size, data.size());
    
    data_shard<T> shard;
    shard.data = std::vector<T>(data.begin() + start, data.begin() + end);
    shard.shard_id = i;
    shard.total_shards = num_shards;
    shard.is_last_shard = (i == num_shards - 1);
    
    shards.push_back(std::move(shard));
  }
  
  // 并行格式化
  auto results = format(shards, rule, max_threads);
  
  // 合并结果
  std::vector<std::string> merged;
  merged.reserve(data.size());
  
  // 按分片ID排序
  std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
    return a.shard_id < b.shard_id;
  });
  
  for (const auto& result : results) {
    if (result.success) {
      merged.insert(merged.end(), result.formatted_data.begin(), result.formatted_data.end());
    } else {
      throw std::runtime_error(fmt::format("shard {} failed: {}", result.shard_id, result.error_message));
    }
  }
  
  return merged;
}

} // namespace distributed_format

FMT_END_NAMESPACE

#endif // FMT_DISTRIBUTED_FORMAT_H_