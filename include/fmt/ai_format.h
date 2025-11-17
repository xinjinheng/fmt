// AI自适应格式推荐与修复系统
// Copyright (c) 2024 fmtlib
// License: MIT

#ifndef FMT_AI_FORMAT_H_
#define FMT_AI_FORMAT_H_

#include "format.h"
#include <type_traits>
#include <string_view>
#include <utility>

FMT_BEGIN_NAMESPACE

/**
 * @brief AI格式推荐与修复系统命名空间
 */
namespace ai_format {

/**
 * @brief 输入数据类型
 */
enum class data_type {
  integer,   ///< 整数类型
  floating,  ///< 浮点数类型
  string,    ///< 字符串类型
  boolean,   ///< 布尔类型
  pointer,   ///< 指针类型
  datetime,  ///< 日期时间类型
  other      ///< 其他类型
};

/**
 * @brief 使用场景上下文
 */
enum class context {
  log,           ///< 日志记录场景
  ui,            ///< 用户界面场景
  data_export,   ///< 数据导出场景
  scientific,    ///< 科学计算场景
  general,       ///< 通用场景
  network        ///< 网络通信场景
};

/**
 * @brief 数值特征
 */
struct numeric_features {
  bool has_decimal;      ///< 是否包含小数部分
  int digit_count;       ///< 有效数字位数
  bool is_scientific;    ///< 是否使用科学计数法
  bool has_negative;     ///< 是否包含负数
  double min_value;      ///< 最小值
  double max_value;      ///< 最大值
  bool is_integer_like;  ///< 是否类似整数（如123.0）
};

/**
 * @brief 字符串特征
 */
struct string_features {
  int avg_length;        ///< 平均长度
  bool has_special_chars;///< 是否包含特殊字符
  bool is_date_time;     ///< 是否为日期时间格式
  bool is_url;           ///< 是否为URL
  bool is_email;         ///< 是否为电子邮件
  bool is_json;          ///< 是否为JSON格式
};

/**
 * @brief 格式特征综合
 */
struct format_features {
  data_type type;        ///< 数据类型
  numeric_features numeric;  ///< 数值特征（如果是数值类型）
  string_features string;    ///< 字符串特征（如果是字符串类型）
  context ctx;           ///< 使用场景
};

/**
 * @brief 格式推荐结果
 */
struct format_recommendation {
  std::string format_str;///< 推荐的格式字符串
  std::string reason;    ///< 推荐理由
  int confidence;        ///< 推荐置信度（0-100）
};

/**
 * @brief 格式规则
 */
struct format_rule {
  uint64_t feature_mask; ///< 特征掩码
  uint64_t condition;    ///< 条件
  const char* format_str;///< 推荐格式
  const char* reason;    ///< 推荐理由
  int confidence;        ///< 置信度
};

/**
 * @brief 预定义的AI格式规则（决策树模型）
 */
constexpr format_rule ai_format_rules[] = {
  // 规则1: 整数, 日志场景, 位数<5 → {}  
  {0b1111000000, 0b0000000001, "{}", "简短整数日志推荐简洁格式", 95},
  
  // 规则2: 整数, 数据导出, 位数≥10 → {:,}  
  {0b1111000000, 0b0000000100, "{:,}", "大数据导出推荐千位分隔符", 90},
  
  // 规则3: 浮点数, 科学场景 → {:.6f}  
  {0b1111000000, 0b0000001010, "{:.6f}", "科学计算浮点数推荐6位精度", 92},
  
  // 规则4: 浮点数, 日志场景, 无小数 → {:.0f}  
  {0b1111000000, 0b0000010001, "{:.0f}", "整数值浮点数日志推荐无小数格式", 88},
  
  // 规则5: 浮点数, 数据导出, 大数值 → {:.2e}  
  {0b1111000000, 0b0000100100, "{:.2e}", "大数值导出推荐科学计数法", 85},
  
  // 规则6: 字符串, UI场景, 长度>20 → {:20.20}  
  {0b1111000000, 0b0001000011, "{:20.20}", "UI长字符串推荐固定宽度", 80},
  
  // 规则7: 字符串, 日志场景 → {!r}  
  {0b1111000000, 0b0001000001, "{!r}", "日志字符串推荐原始格式", 85},
  
  // 规则8: 日期时间, 日志场景 → {%Y-%m-%d %H:%M:%S}  
  {0b1111000000, 0b0010000001, "{%Y-%m-%d %H:%M:%S}", "日志日期推荐ISO格式", 90},
  
  // 规则9: 布尔值, 通用场景 → {}  
  {0b1111000000, 0b0100000011, "{}", "布尔值推荐简洁格式", 95},
  
  // 规则10: 指针, 调试场景 → {:p}  
  {0b1111000000, 0b1000000001, "{:p}", "指针调试推荐十六进制格式", 90},
};

/**
 * @brief AI格式推荐器
 */
class recommender {
public:
  /**
   * @brief 根据特征推荐格式
   */
  static FMT_CONSTEXPR format_recommendation recommend(const format_features& features) {
    // 生成特征向量
    uint64_t feature_vector = generate_feature_vector(features);
    
    // 匹配最佳规则
    const format_rule* best_rule = &ai_format_rules[0];
    int max_score = 0;
    
    for (const auto& rule : ai_format_rules) {
      // 检查特征掩码匹配
      uint64_t masked_feature = feature_vector & rule.feature_mask;
      if (masked_feature == rule.condition) {
        if (rule.confidence > max_score) {
          max_score = rule.confidence;
          best_rule = &rule;
        }
      }
    }
    
    return {
      best_rule->format_str,
      best_rule->reason,
      best_rule->confidence
    };
  }
  
  /**
 * @brief 从数据范围中提取特征
 */
  template <typename Range>
  static format_features extract_features(const Range& data_range, context ctx = context::general) {
    format_features features;
    features.ctx = ctx;
    
    // 检查数据类型
    using T = typename Range::value_type;
    
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
      features.type = data_type::integer;
      features.numeric.has_decimal = false;
      features.numeric.digit_count = 0;
      features.numeric.is_scientific = false;
      features.numeric.has_negative = false;
      features.numeric.min_value = std::numeric_limits<double>::max();
      features.numeric.max_value = std::numeric_limits<double>::lowest();
      features.numeric.is_integer_like = true;
      
      size_t count = 0;
      for (const auto& data : data_range) {
        features.numeric.has_negative |= (data < 0);
        features.numeric.min_value = std::min(features.numeric.min_value, static_cast<double>(data));
        features.numeric.max_value = std::max(features.numeric.max_value, static_cast<double>(data));
        features.numeric.digit_count += get_digit_count(data);
        ++count;
      }
      
      if (count > 0) {
        features.numeric.digit_count = static_cast<int>(features.numeric.digit_count / count);
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      features.type = data_type::floating;
      features.numeric.has_decimal = false;
      features.numeric.digit_count = 0;
      features.numeric.is_scientific = false;
      features.numeric.has_negative = false;
      features.numeric.min_value = std::numeric_limits<double>::max();
      features.numeric.max_value = std::numeric_limits<double>::lowest();
      features.numeric.is_integer_like = true;
      
      size_t count = 0;
      for (const auto& data : data_range) {
        features.numeric.has_negative |= (data < 0.0);
        features.numeric.min_value = std::min(features.numeric.min_value, static_cast<double>(data));
        features.numeric.max_value = std::max(features.numeric.max_value, static_cast<double>(data));
        features.numeric.digit_count += get_digit_count(data);
        features.numeric.has_decimal |= (data != static_cast<long long>(data));
        features.numeric.is_integer_like &= (data == static_cast<long long>(data));
        ++count;
      }
      
      if (count > 0) {
        features.numeric.digit_count = static_cast<int>(features.numeric.digit_count / count);
        // 检查是否需要科学计数法
        double max_val = std::max(std::abs(features.numeric.min_value), std::abs(features.numeric.max_value));
        features.numeric.is_scientific = (max_val > 1e6 || max_val < 1e-3);
      }
    } else if constexpr (std::is_same_v<T, bool>) {
      features.type = data_type::boolean;
    } else if constexpr (std::is_pointer_v<T>) {
      features.type = data_type::pointer;
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
      features.type = data_type::string;
      features.string.avg_length = 0;
      features.string.has_special_chars = false;
      features.string.is_date_time = false;
      features.string.is_url = false;
      features.string.is_email = false;
      features.string.is_json = false;
      
      size_t count = 0;
      for (const auto& data : data_range) {
        std::string_view sv = data;
        features.string.avg_length += static_cast<int>(sv.size());
        features.string.has_special_chars |= has_special_chars(sv);
        features.string.is_date_time |= is_date_time(sv);
        features.string.is_url |= is_url(sv);
        features.string.is_email |= is_email(sv);
        features.string.is_json |= is_json(sv);
        ++count;
      }
      
      if (count > 0) {
        features.string.avg_length = static_cast<int>(features.string.avg_length / count);
      }
    } else {
      features.type = data_type::other;
    }
    
    return features;
  }
  
  /**
   * @brief 从单个数据中提取特征
   */
  template <typename T>
  static format_features extract_features(const T& data, context ctx = context::general) {
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*>) {
      // 字符串类型
      format_features features;
      features.ctx = ctx;
      features.type = data_type::string;
      std::string_view sv = data;
      features.string.avg_length = static_cast<int>(sv.size());
      features.string.has_special_chars = has_special_chars(sv);
      features.string.is_date_time = is_date_time(sv);
      features.string.is_url = is_url(sv);
      features.string.is_email = is_email(sv);
      features.string.is_json = is_json(sv);
      return features;
    } else if constexpr (std::is_arithmetic_v<T> || std::is_pointer_v<T> || std::is_same_v<T, bool>) {
      // 算术类型、指针或布尔值
      format_features features;
      features.ctx = ctx;
      
      if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        features.type = data_type::integer;
        features.numeric.has_decimal = false;
        features.numeric.digit_count = get_digit_count(data);
        features.numeric.is_scientific = false;
        features.numeric.has_negative = (data < 0);
        features.numeric.min_value = static_cast<double>(data);
        features.numeric.max_value = static_cast<double>(data);
        features.numeric.is_integer_like = true;
      } else if constexpr (std::is_floating_point_v<T>) {
        features.type = data_type::floating;
        features.numeric.has_decimal = (data != static_cast<long long>(data));
        features.numeric.digit_count = get_digit_count(data);
        features.numeric.is_scientific = (std::abs(data) > 1e6 || std::abs(data) < 1e-3);
        features.numeric.has_negative = (data < 0.0);
        features.numeric.min_value = static_cast<double>(data);
        features.numeric.max_value = static_cast<double>(data);
        features.numeric.is_integer_like = (data == static_cast<long long>(data));
      } else if constexpr (std::is_same_v<T, bool>) {
        features.type = data_type::boolean;
      } else if constexpr (std::is_pointer_v<T>) {
        features.type = data_type::pointer;
      }
      
      return features;
    } else {
      // 其他类型
      return extract_features_impl(data, ctx);
    }
  }
  
  /**
   * @brief 修复格式规范
   */
  template <typename T>
  static FMT_CONSTEXPR auto fix_specs(const format_specs& specs) {
    format_specs fixed = specs;
    
    // 类型不匹配修复
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
      // 整数类型不应使用浮点数格式
      if (specs.type() == presentation_type::float_presentation ||
          specs.type() == presentation_type::exp ||
          specs.type() == presentation_type::exp1) {
        fixed.set_type(presentation_type::int_presentation);
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      // 浮点数默认精度优化
      if (specs.precision() < 0 || specs.precision() > 15) {
        fixed.precision = 6;
      }
    } else if constexpr (std::is_same_v<T, bool>) {
      // 布尔值不应使用数字格式
      if (specs.type() != presentation_type::none) {
        fixed.set_type(presentation_type::none);
      }
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
      // 字符串精度优化
      if (specs.precision() < 0) {
        fixed.precision = -1; // 字符串默认不截断
      }
    }
    
    // 对齐方式修复
    if (specs.align() == align::numeric && !std::is_arithmetic_v<T>) {
      fixed.set_align(align::none);
    }
    
    return fixed;
  }
  
private:
    /**
     * @brief 生成特征向量
     */
    static FMT_CONSTEXPR uint64_t generate_feature_vector(const format_features& features) {
      uint64_t vec = 0;
      
      // 数据类型 (4 bits)
      vec |= static_cast<uint64_t>(features.type) << 0;
      
      // 场景 (3 bits)
      vec |= static_cast<uint64_t>(features.ctx) << 4;
      
      // 数值特征 (8 bits)
      if (features.type == data_type::integer || features.type == data_type::floating) {
        if (features.numeric.has_decimal) vec |= 1 << 7;
        if (features.numeric.digit_count >= 10) vec |= 1 << 8;
        if (features.numeric.is_scientific) vec |= 1 << 9;
        if (features.numeric.has_negative) vec |= 1 << 10;
        if (features.numeric.is_integer_like) vec |= 1 << 11;
      }
      
      // 字符串特征 (4 bits)
      if (features.type == data_type::string) {
        if (features.string.avg_length > 20) vec |= 1 << 15;
        if (features.string.has_special_chars) vec |= 1 << 16;
        if (features.string.is_date_time) vec |= 1 << 17;
        if (features.string.is_url) vec |= 1 << 18;
      }
      
      return vec;
    }
    
    /**
     * @brief 获取数字位数
     */
    template <typename T>
    static int get_digit_count(T value) {
      if (value == 0) return 1;
      int count = 0;
      T abs_val = (value < 0) ? -value : value;
      while (abs_val > 0) {
        abs_val /= 10;
        ++count;
      }
      return count;
    }
    
    /**
     * @brief 检查是否包含特殊字符
     */
    static bool has_special_chars(std::string_view str) {
      return str.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ") != std::string_view::npos;
    }
    
    /**
     * @brief 检查是否为日期时间格式
     */
    static bool is_date_time(std::string_view str) {
      // 简单的日期时间格式检测 (支持 YYYY-MM-DD, HH:MM:SS, YYYY/MM/DD 等)
      return (str.find(':') != std::string_view::npos && str.find(':') != str.size() - 1) || 
             (str.find('-') != std::string_view::npos && str.substr(0, 4).find_first_not_of("0123456789") == std::string_view::npos) ||
             (str.find('/') != std::string_view::npos && str.substr(0, 4).find_first_not_of("0123456789") == std::string_view::npos);
    }
    
    /**
     * @brief 检查是否为URL
     */
    static bool is_url(std::string_view str) {
      return str.starts_with("http://") || str.starts_with("https://") || str.starts_with("ftp://") ||
             str.ends_with(".com") || str.ends_with(".org") || str.ends_with(".net") ||
             str.ends_with(".cn") || str.ends_with(".edu") || str.ends_with(".gov");
    }
    
    /**
     * @brief 检查是否为电子邮件
     */
    static bool is_email(std::string_view str) {
      size_t at_pos = str.find('@');
      size_t dot_pos = str.find('.', at_pos);
      return at_pos != std::string_view::npos && dot_pos != std::string_view::npos && dot_pos < str.size() - 1;
    }
    
    /**
     * @brief 检查是否为JSON格式
     */
    static bool is_json(std::string_view str) {
      return (str.starts_with('{') && str.ends_with('}')) || (str.starts_with('[') && str.ends_with(']'));
    }
    
    static bool is_number(std::string_view str) {
      char* endptr = nullptr;
      std::strtod(str.data(), &endptr);
      return *endptr == '\0' && str.size() > 0;
    }
    
    static bool is_hex(std::string_view str) {
      return str.starts_with("0x") || str.starts_with("0X");
    }
    
    static bool is_octal(std::string_view str) {
      return str.starts_with("0") && str.size() > 1 && str[1] >= '0' && str[1] <= '7';
    }
    
    /**
     * @brief 其他类型的特征提取实现
     */
    template <typename T>
    static format_features extract_features_impl(const T& data, context ctx) {
      format_features features;
      features.ctx = ctx;
      features.type = data_type::other;
      
      // 对于其他类型，我们可以尝试检查是否有格式化方法
      // 这里只是一个占位符，实际可以扩展
      return features;
    }
  };

/**
 * @brief 编译期格式检查与修复
 */
template <typename T, typename Char>
struct compile_time_format_checker {
  FMT_CONSTEXPR static auto check(const basic_format_parse_context<Char>& ctx) {
    auto specs = detail::dynamic_format_specs<Char>();
    auto end = detail::parse_format_specs(ctx.begin(), ctx.end(), specs, 
                                         const_cast<basic_format_parse_context<Char>&>(ctx),
                                         mapped_type_constant<T, Char>::value);
    
    // 修复格式规范
    auto fixed_specs = recommender::fix_specs<T>(specs);
    
    return fixed_specs;
  }
};

/**
 * @brief AI增强的格式化函数
 */
template <typename... Args>
std::string ai_format(std::string_view fmt, const Args&... args) {
  return fmt::format(fmt, args...);
}

/**
 * @brief 自动推荐格式并格式化
 */
template <typename T>
std::string auto_format(const T& data, context ctx = context::general) {
  auto features = recommender::extract_features(data, ctx);
  auto recommendation = recommender::recommend(features);
  return fmt::format(recommendation.format_str, data);
}

} // namespace ai_format

FMT_END_NAMESPACE

#endif // FMT_AI_FORMAT_H_