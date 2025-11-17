// 跨语言格式字符串兼容层
// Copyright (c) 2024 fmtlib
// License: MIT

#ifndef FMT_CROSS_FORMAT_H_
#define FMT_CROSS_FORMAT_H_

#include "format.h"
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>

FMT_BEGIN_NAMESPACE

/**
 * @brief 跨语言格式转换命名空间
 */
namespace cross_format {

/**
 * @brief 支持的格式语言类型
 */
enum class language {
  fmt,                ///< fmt库格式
  python_fstring,     ///< Python f-string
  python_format,      ///< Python str.format()
  java_formatter,     ///< Java Formatter
  csharp_stringformat,///< C# string.Format
  c_printf,           ///< C printf
  go_format           ///< Go fmt.Sprintf
};

/**
 * @brief AST节点类型
 */
enum class ast_node_type {
  text,               ///< 普通文本节点
  format_field,       ///< 格式字段节点
  conversion_spec     ///< 转换说明符节点
};

/**
 * @brief 对齐方式
 */
enum class ast_align {
  none,               ///< 无对齐
  left,               ///< 左对齐
  right,              ///< 右对齐
  center              ///< 居中对齐
};

/**
 * @brief 符号显示方式
 */
enum class ast_sign {
  none,               ///< 无符号
  plus,               ///< 显示+/-符号
  space               ///< 正数前加空格
};

/**
 * @brief 转换说明符
 */
struct conversion_info {
  char type = ' ';               ///< 转换类型 (d, f, s, etc.)
  int width = -1;               ///< 宽度
  int precision = -1;           ///< 精度
  ast_align align = ast_align::none;  ///< 对齐方式
  char fill = ' ';              ///< 填充字符
  bool alternate = false;       ///< #标记
  ast_sign sign = ast_sign::none;     ///< 符号标记
  bool zero_pad = false;        ///< 0填充
  bool upper_case = false;      ///< 大写转换
};

/**
 * @brief 格式字段信息
 */
struct field_info {
  std::optional<int> index;     ///< 参数索引
  std::optional<std::string> name;  ///< 参数名称
  conversion_info conversion;    ///< 转换说明
  std::vector<std::string> format_modifiers;  ///< 格式修饰符
};

/**
 * @brief AST节点基类
 */
struct ast_node {
  ast_node_type type;
  explicit ast_node(ast_node_type t) : type(t) {}
  virtual ~ast_node() = default;
  virtual std::string to_string(language target_lang) const = 0;
};

/**
 * @brief 文本节点
 */
struct text_node : public ast_node {
  std::string text;
  explicit text_node(std::string txt) : ast_node(ast_node_type::text), text(std::move(txt)) {}
  std::string to_string(language target_lang) const override {
    return text;
  }
};

/**
 * @brief 格式转换函数
 */
std::string format_conversion(const conversion_info& conversion, language target_lang) {
  std::string result;
  
  switch (target_lang) {
    case language::fmt:
    case language::csharp_stringformat:
      if (conversion.type != ' ') {
        result += ":"; // fmt使用冒号分隔格式说明
        
        // 填充和对齐
        if (conversion.fill != ' ' && conversion.align != ast_align::none) {
          result += conversion.fill;
          result += (conversion.align == ast_align::left ? '<' : 
                    conversion.align == ast_align::right ? '>' : 
                    conversion.align == ast_align::center ? '^' : ' ');
        }
        
        // 宽度
        if (conversion.width != -1) {
          result += std::to_string(conversion.width);
        }
        
        // 精度
        if (conversion.precision != -1) {
          result += "." + std::to_string(conversion.precision);
        }
        
        // 转换类型
        result += conversion.type;
      }
      break;
      
    case language::python_format:
    case language::python_fstring:
      if (conversion.type != ' ') {
        result += ":"; // Python使用冒号分隔格式说明
        
        // 填充和对齐
        if (conversion.fill != ' ' && conversion.align != ast_align::none) {
          result += conversion.fill;
          result += (conversion.align == ast_align::left ? '<' : 
                    conversion.align == ast_align::right ? '>' : 
                    conversion.align == ast_align::center ? '^' : ' ');
        }
        
        // 宽度
        if (conversion.width != -1) {
          result += std::to_string(conversion.width);
        }
        
        // 精度
        if (conversion.precision != -1) {
          result += "." + std::to_string(conversion.precision);
        }
        
        // 转换类型
        result += conversion.type;
      }
      break;
      
    case language::java_formatter:
      if (conversion.type != ' ') {
        result += "";
        
        // 填充和对齐
        if (conversion.align != ast_align::none) {
          result += (conversion.align == ast_align::left ? '-' : 
                    (conversion.align == ast_align::center ? '^' : ''));
        }
        
        // 宽度
        if (conversion.width != -1) {
          result += std::to_string(conversion.width);
        }
        
        // 精度
        if (conversion.precision != -1) {
          result += "." + std::to_string(conversion.precision);
        }
        
        // 转换类型
        result += conversion.type;
      }
      break;
      
    case language::c_printf:
      if (conversion.type != ' ') {
        result += "";
        
        // 符号
        if (conversion.sign == ast_sign::plus) result += '+';
        else if (conversion.sign == ast_sign::space) result += ' ';
        
        // 零填充
        if (conversion.zero_pad) result += '0';
        
        // 宽度
        if (conversion.width != -1) {
          result += std::to_string(conversion.width);
        }
        
        // 精度
        if (conversion.precision != -1) {
          result += "." + std::to_string(conversion.precision);
        }
        
        // 转换类型
        result += conversion.type;
      }
      break;
      
    case language::go_format:
      if (conversion.type != ' ') {
        // Go格式比较简单，只支持基本类型
        result += ":"; // Go使用冒号分隔格式说明
        result += conversion.type;
      }
      break;
  }
  
  return result;
}

/**
 * @brief 格式字段节点
 */
struct format_field_node : public ast_node {
  field_info field;
  explicit format_field_node(field_info fld) : ast_node(ast_node_type::format_field), field(std::move(fld)) {}
  std::string to_string(language target_lang) const override {
    // 简单的转换实现
    std::string result = "{";
    if (field.index.has_value()) {
      result += std::to_string(field.index.value());
    } else if (field.name.has_value()) {
      result += field.name.value();
    }
    
    // 添加格式说明
    result += format_conversion(field.conversion, target_lang);
    
    result += "}";
    return result;
  }
};

/**
 * @brief 格式字符串抽象语法树
 */
class format_ast {
public:
  void add_node(std::unique_ptr<ast_node> node) {
    nodes_.push_back(std::move(node));
  }
  
  std::string to_string(language target_lang) const {
    std::string result;
    for (const auto& node : nodes_) {
      result += node->to_string(target_lang);
    }
    return result;
  }
  
  const std::vector<std::unique_ptr<ast_node>>& nodes() const {
    return nodes_;
  }
  
private:
  std::vector<std::unique_ptr<ast_node>> nodes_;
};

/**
 * @brief 格式解析器基类
 */
class format_parser {
public:
  virtual ~format_parser() = default;
  virtual format_ast parse(std::string_view fmt_str) const = 0;
};

/**
 * @brief fmt格式解析器
 */
class fmt_parser : public format_parser {
public:
  format_ast parse(std::string_view fmt_str) const override;
};

/**
 * @brief Python f-string解析器
 */
class python_fstring_parser : public format_parser {
public:
  format_ast parse(std::string_view fmt_str) const override;
};

/**
 * @brief Java Formatter解析器
 */
class java_formatter_parser : public format_parser {
public:
  format_ast parse(std::string_view fmt_str) const override;
};

/**
 * @brief C# string.Format解析器
 */
class csharp_parser : public format_parser {
public:
  format_ast parse(std::string_view fmt_str) const override;
};

/**
 * @brief C printf解析器
 */
class printf_parser : public format_parser {
public:
  format_ast parse(std::string_view fmt_str) const override;
};

/**
 * @brief 格式生成器基类
 */
class format_generator {
public:
  virtual ~format_generator() = default;
  virtual std::string generate(const format_ast& ast) const = 0;
};

/**
 * @brief fmt格式生成器
 */
class fmt_generator : public format_generator {
public:
  std::string generate(const format_ast& ast) const override;
};

/**
 * @brief Python f-string生成器
 */
class python_fstring_generator : public format_generator {
public:
  std::string generate(const format_ast& ast) const override;
};

/**
 * @brief Java Formatter生成器
 */
class java_formatter_generator : public format_generator {
public:
  std::string generate(const format_ast& ast) const override;
};

/**
 * @brief 跨语言格式转换引擎
 */
class format_converter {
public:
  /**
   * @brief 转换格式字符串
   * @param fmt_str 输入格式字符串
   * @param from_lang 输入格式类型
   * @param to_lang 输出格式类型
   * @return 转换后的格式字符串
   */
  std::string convert(std::string_view fmt_str, language from_lang, language to_lang) const;
  
  /**
   * @brief 解析格式字符串为AST
   * @param fmt_str 输入格式字符串
   * @param lang 格式类型
   * @return 格式AST
   */
  format_ast parse(std::string_view fmt_str, language lang) const;
  
  /**
   * @brief 从AST生成格式字符串
   * @param ast 格式AST
   * @param lang 目标格式类型
   * @return 生成的格式字符串
   */
  std::string generate(const format_ast& ast, language lang) const;
  
private:
  std::unique_ptr<format_parser> get_parser(language lang) const;
  std::unique_ptr<format_generator> get_generator(language lang) const;
};

/**
 * @brief 转换格式字符串的便捷函数
 * @param fmt_str 输入格式字符串
 * @param from_lang 输入格式类型
 * @param to_lang 输出格式类型
 * @return 转换后的格式字符串
 */
inline std::string convert(std::string_view fmt_str, language from_lang, language to_lang) {
  return format_converter{}.convert(fmt_str, from_lang, to_lang);
}

/**
 * @brief 类型转换映射
 */
constexpr std::pair<const char*, const char*> type_mappings[][7] = {
  // fmt to others
  {
    {"d", "d"},    // integer
    {"f", "f"},    // float
    {"s", "s"},    // string
    {"c", "c"},    // char
    {"p", "p"},    // pointer
    {"x", "x"},    // hex
    {"o", "o"}     // octal
  },
  // Python to fmt
  {
    {"d", "d"},    // integer
    {"f", "f"},    // float
    {"s", "s"},    // string
    {"c", "c"},    // char
    {"x", "x"},    // hex
    {"o", "o"},    // octal
    {"e", "e"}     // scientific
  },
  // Java to fmt
  {
    {"d", "d"},    // integer
    {"f", "f"},    // float
    {"s", "s"},    // string
    {"c", "c"},    // char
    {"x", "x"},    // hex
    {"o", "o"},    // octal
    {"e", "e"}     // scientific
  },
  // C# to fmt
  {
    {"D", "d"},    // integer
    {"F", "f"},    // float
    {"S", "s"},    // string
    {"C", "c"},    // char
    {"X", "X"},    // hex
    {"O", "o"},    // octal
    {"E", "E"}     // scientific
  },
  // printf to fmt
  {
    {"%d", "d"},   // integer
    {"%f", "f"},   // float
    {"%s", "s"},   // string
    {"%c", "c"},   // char
    {"%p", "p"},   // pointer
    {"%x", "x"},   // hex
    {"%o", "o"}    // octal
  }
};

/**
 * @brief 获取类型映射
 */
inline const char* get_type_mapping(const char* from_type, int mapping_type) {
  for (const auto& pair : type_mappings[mapping_type]) {
    if (std::strcmp(pair.first, from_type) == 0) {
      return pair.second;
    }
  }
  return from_type; // 默认返回原类型
}

} // namespace cross_format

FMT_END_NAMESPACE

#endif // FMT_CROSS_FORMAT_H_