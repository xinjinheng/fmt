// 跨语言格式字符串兼容层实现
// Copyright (c) 2024 fmtlib
// License: MIT

#include "fmt/cross_format.h"
#include <cctype>
#include <sstream>
#include <stdexcept>

FMT_BEGIN_NAMESPACE

namespace cross_format {

// 文本节点转换
std::string text_node::to_string(language target_lang) const {
  // 简单处理：对于需要转义的语言进行转义
  switch (target_lang) {
    case language::python_fstring:
    case language::python_format:
      // Python需要转义{和}
      std::string escaped = text;
      size_t pos = 0;
      while ((pos = escaped.find('{', pos)) != std::string::npos) {
        escaped.insert(pos, 1, '{');
        pos += 2;
      }
      pos = 0;
      while ((pos = escaped.find('}', pos)) != std::string::npos) {
        escaped.insert(pos, 1, '}');
        pos += 2;
      }
      return escaped;
    default:
      return text;
  }
}

// 格式字段节点转换
std::string format_field_node::to_string(language target_lang) const {
  switch (target_lang) {
    case language::fmt:
      return generate_fmt_format();
    case language::python_fstring:
      return generate_python_fmt();
    case language::java_formatter:
      return generate_java_format();
    case language::csharp_stringformat:
      return generate_csharp_format();
    case language::c_printf:
      return generate_printf_format();
    default:
      return generate_fmt_format();
  }
}

// 生成fmt格式
std::string format_field_node::generate_fmt_format() const {
  std::ostringstream oss;
  oss << '{';
  
  // 参数索引或名称
  if (info.index.has_value()) {
    oss << info.index.value();
  } else if (info.name.has_value()) {
    oss << info.name.value();
  }
  
  // 转换说明
  if (info.conversion.type != ' ') {
    oss << ':';
    
    // 填充和对齐
    if (info.conversion.align != ast_align::none) {
      if (info.conversion.fill != ' ') {
        oss << info.conversion.fill;
      }
      switch (info.conversion.align) {
        case ast_align::left: oss << '<'; break;
        case ast_align::right: oss << '>'; break;
        case ast_align::center: oss << '^'; break;
        default: break;
      }
    }
    
    // 符号
    switch (info.conversion.sign) {
      case ast_sign::plus: oss << '+'; break;
      case ast_sign::space: oss << ' '; break;
      default: break;
    }
    
    // 0填充
    if (info.conversion.zero_pad) {
      oss << '0';
    }
    
    // 替代形式
    if (info.conversion.alternate) {
      oss << '#';
    }
    
    // 宽度
    if (info.conversion.width > 0) {
      oss << info.conversion.width;
    }
    
    // 精度
    if (info.conversion.precision >= 0) {
      oss << '.' << info.conversion.precision;
    }
    
    // 类型
    oss << info.conversion.type;
  }
  
  oss << '}';
  return oss.str();
}

// 生成Python格式
std::string format_field_node::generate_python_fmt() const {
  std::ostringstream oss;
  oss << '{';
  
  // 参数索引或名称
  if (info.index.has_value()) {
    oss << info.index.value();
  } else if (info.name.has_value()) {
    oss << info.name.value();
  }
  
  // 转换说明
  if (info.conversion.type != ' ') {
    oss << ':';
    
    // 填充和对齐
    if (info.conversion.align != ast_align::none) {
      if (info.conversion.fill != ' ') {
        oss << info.conversion.fill;
      }
      switch (info.conversion.align) {
        case ast_align::left: oss << '<'; break;
        case ast_align::right: oss << '>'; break;
        case ast_align::center: oss << '^'; break;
        default: break;
      }
    }
    
    // 符号
    switch (info.conversion.sign) {
      case ast_sign::plus: oss << '+'; break;
      case ast_sign::space: oss << ' '; break;
      default: break;
    }
    
    // 0填充
    if (info.conversion.zero_pad) {
      oss << '0';
    }
    
    // 替代形式
    if (info.conversion.alternate) {
      oss << '#';
    }
    
    // 宽度
    if (info.conversion.width > 0) {
      oss << info.conversion.width;
    }
    
    // 精度
    if (info.conversion.precision >= 0) {
      oss << '.' << info.conversion.precision;
    }
    
    // 类型
    oss << info.conversion.type;
  }
  
  oss << '}';
  return oss.str();
}

// 生成Java格式
std::string format_field_node::generate_java_format() const {
  std::ostringstream oss;
  
  // Java使用%符号
  oss << '%';
  
  // 替代形式
  if (info.conversion.alternate) {
    oss << '#';
  }
  
  // 符号
  switch (info.conversion.sign) {
    case ast_sign::plus: oss << '+'; break;
    case ast_sign::space: oss << ' '; break;
    default: break;
  }
  
  // 0填充
  if (info.conversion.zero_pad) {
    oss << '0';
  }
  
  // 宽度
  if (info.conversion.width > 0) {
    oss << info.conversion.width;
  }
  
  // 精度
  if (info.conversion.precision >= 0) {
    oss << '.' << info.conversion.precision;
  }
  
  // 类型
  char java_type = info.conversion.type;
  // Java类型映射
  switch (java_type) {
    case 'd': break; // 保持不变
    case 'f': break;
    case 's': break;
    case 'c': break;
    case 'x': break;
    case 'o': break;
    case 'e': break;
    default: java_type = 's'; // 默认字符串
  }
  oss << java_type;
  
  return oss.str();
}

// 生成C#格式
std::string format_field_node::generate_csharp_format() const {
  std::ostringstream oss;
  oss << '{';
  
  // 参数索引
  if (info.index.has_value()) {
    oss << info.index.value();
  } else if (info.name.has_value()) {
    oss << info.name.value();
  }
  
  // 转换说明
  if (info.conversion.type != ' ') {
    oss << ':';
    
    // 对齐
    if (info.conversion.align != ast_align::none) {
      oss << (info.conversion.align == ast_align::left ? ',' : ';');
      if (info.conversion.width > 0) {
        oss << (info.conversion.align == ast_align::left ? -info.conversion.width : info.conversion.width);
      }
    }
    
    // 类型
    char csharp_type = info.conversion.type;
    // C#类型映射
    switch (csharp_type) {
      case 'd': csharp_type = 'D'; break;
      case 'f': csharp_type = 'F'; break;
      case 's': csharp_type = 'S'; break;
      case 'c': csharp_type = 'C'; break;
      case 'x': csharp_type = 'X'; break;
      case 'o': csharp_type = 'O'; break;
      case 'e': csharp_type = 'E'; break;
      default: break;
    }
    if (csharp_type != ' ') {
      oss << csharp_type;
    }
    
    // 精度
    if (info.conversion.precision >= 0) {
      oss << info.conversion.precision;
    }
  }
  
  oss << '}';
  return oss.str();
}

// 生成printf格式
std::string format_field_node::generate_printf_format() const {
  std::ostringstream oss;
  oss << '%';
  
  // 符号
  switch (info.conversion.sign) {
    case ast_sign::plus: oss << '+'; break;
    case ast_sign::space: oss << ' '; break;
    default: break;
  }
  
  // 0填充
  if (info.conversion.zero_pad) {
    oss << '0';
  }
  
  // 宽度
  if (info.conversion.width > 0) {
    oss << info.conversion.width;
  }
  
  // 精度
  if (info.conversion.precision >= 0) {
    oss << '.' << info.conversion.precision;
  }
  
  // 类型
  char printf_type = info.conversion.type;
  // printf类型映射
  switch (printf_type) {
    case 'd': break;
    case 'f': break;
    case 's': break;
    case 'c': break;
    case 'x': break;
    case 'o': break;
    case 'e': break;
    case 'p': break;
    default: printf_type = 's';
  }
  oss << printf_type;
  
  return oss.str();
}

// fmt格式解析器
format_ast fmt_parser::parse(std::string_view fmt_str) const {
  format_ast ast;
  
  const char* begin = fmt_str.data();
  const char* end = begin + fmt_str.size();
  const char* p = begin;
  
  while (p != end) {
    if (*p == '{') {
      // 处理普通文本
      if (p > begin) {
        ast.add_node(std::make_unique<text_node>(std::string(begin, p - begin)));
      }
      
      // 跳过{
      ++p;
      
      // 解析格式字段
      field_info info;
      
      // 解析参数索引或名称
      if (std::isdigit(*p)) {
        int index = 0;
        while (std::isdigit(*p) && p != end) {
          index = index * 10 + (*p - '0');
          ++p;
        }
        info.index = index;
      } else if (std::isalpha(*p) || *p == '_') {
        std::string name;
        while ((std::isalnum(*p) || *p == '_' || *p == '.') && p != end) {
          name += *p;
          ++p;
        }
        info.name = name;
      }
      
      // 解析转换说明
      if (p != end && *p == ':') {
        ++p;
        
        // 解析填充和对齐
        if (p + 1 != end && (*(p + 1) == '<' || *(p + 1) == '>' || *(p + 1) == '^')) {
          info.conversion.fill = *p;
          switch (*(p + 1)) {
            case '<': info.conversion.align = ast_align::left; break;
            case '>': info.conversion.align = ast_align::right; break;
            case '^': info.conversion.align = ast_align::center; break;
          }
          p += 2;
        } else if (*p == '<' || *p == '>' || *p == '^') {
          switch (*p) {
            case '<': info.conversion.align = ast_align::left; break;
            case '>': info.conversion.align = ast_align::right; break;
            case '^': info.conversion.align = ast_align::center; break;
          }
          ++p;
        }
        
        // 解析符号
        if (*p == '+' || *p == ' ') {
          info.conversion.sign = (*p == '+') ? ast_sign::plus : ast_sign::space;
          ++p;
        }
        
        // 解析0填充
        if (*p == '0') {
          info.conversion.zero_pad = true;
          ++p;
        }
        
        // 解析替代形式
        if (*p == '#') {
          info.conversion.alternate = true;
          ++p;
        }
        
        // 解析宽度
        if (std::isdigit(*p)) {
          int width = 0;
          while (std::isdigit(*p) && p != end) {
            width = width * 10 + (*p - '0');
            ++p;
          }
          info.conversion.width = width;
        }
        
        // 解析精度
        if (*p == '.') {
          ++p;
          int precision = 0;
          while (std::isdigit(*p) && p != end) {
            precision = precision * 10 + (*p - '0');
            ++p;
          }
          info.conversion.precision = precision;
        }
        
        // 解析类型
        if (p != end && !std::isspace(*p) && *p != '}') {
          info.conversion.type = *p;
          ++p;
        }
      }
      
      // 跳过}
      if (p != end && *p == '}') {
        ++p;
      }
      
      ast.add_node(std::make_unique<format_field_node>(std::move(info)));
      begin = p;
    } else if (*p == '}') {
      // 处理}}转义
      if (p > begin) {
        ast.add_node(std::make_unique<text_node>(std::string(begin, p - begin)));
      }
      ++p;
      if (p != end && *p == '}') {
        ast.add_node(std::make_unique<text_node>("}"));
        ++p;
      }
      begin = p;
    } else {
      ++p;
    }
  }
  
  // 处理剩余文本
  if (p > begin) {
    ast.add_node(std::make_unique<text_node>(std::string(begin, p - begin)));
  }
  
  return ast;
}

// Python f-string解析器
format_ast python_fstring_parser::parse(std::string_view fmt_str) const {
  // 简化实现：Python f-string与fmt格式相似
  // 主要区别在于转义的{{和}}
  std::string fmt_copy(fmt_str);
  
  // 替换{{为{
  size_t pos = 0;
  while ((pos = fmt_copy.find("{{", pos)) != std::string::npos) {
    fmt_copy.replace(pos, 2, "{");
    pos += 1;
  }
  
  // 替换}}为}
  pos = 0;
  while ((pos = fmt_copy.find("}}", pos)) != std::string::npos) {
    fmt_copy.replace(pos, 2, "}");
    pos += 1;
  }
  
  // 使用fmt解析器解析
  fmt_parser parser;
  return parser.parse(fmt_copy);
}

// Java Formatter解析器
format_ast java_formatter_parser::parse(std::string_view fmt_str) const {
  format_ast ast;
  
  const char* begin = fmt_str.data();
  const char* end = begin + fmt_str.size();
  const char* p = begin;
  
  while (p != end) {
    if (*p == '%') {
      // 处理普通文本
      if (p > begin) {
        ast.add_node(std::make_unique<text_node>(std::string(begin, p - begin)));
      }
      
      ++p;
      
      field_info info;
      static int param_index = 0;
      info.index = param_index++;
      
      // 解析标志
      while (p != end && (*p == '#' || *p == '+' || *p == ' ' || *p == '0' || *p == '-')) {
        switch (*p) {
          case '#': info.conversion.alternate = true; break;
          case '+': info.conversion.sign = ast_sign::plus; break;
          case ' ': info.conversion.sign = ast_sign::space; break;
          case '0': info.conversion.zero_pad = true; break;
          case '-': info.conversion.align = ast_align::left; break;
        }
        ++p;
      }
      
      // 解析宽度
      if (std::isdigit(*p)) {
        int width = 0;
        while (std::isdigit(*p) && p != end) {
          width = width * 10 + (*p - '0');
          ++p;
        }
        info.conversion.width = width;
      }
      
      // 解析精度
      if (*p == '.') {
        ++p;
        int precision = 0;
        while (std::isdigit(*p) && p != end) {
          precision = precision * 10 + (*p - '0');
          ++p;
        }
        info.conversion.precision = precision;
      }
      
      // 解析类型
      if (p != end) {
        info.conversion.type = *p;
        ++p;
      }
      
      ast.add_node(std::make_unique<format_field_node>(std::move(info)));
      begin = p;
    } else {
      ++p;
    }
  }
  
  // 处理剩余文本
  if (p > begin) {
    ast.add_node(std::make_unique<text_node>(std::string(begin, p - begin)));
  }
  
  return ast;
}

// C# string.Format解析器
format_ast csharp_parser::parse(std::string_view fmt_str) const {
  // 简化实现：与fmt格式相似
  fmt_parser parser;
  return parser.parse(fmt_str);
}

// C printf解析器
format_ast printf_parser::parse(std::string_view fmt_str) const {
  // 类似于Java Formatter解析
  return java_formatter_parser{}.parse(fmt_str);
}

// fmt格式生成器
std::string fmt_generator::generate(const format_ast& ast) const {
  return ast.to_string(language::fmt);
}

// Python f-string生成器
std::string python_fstring_generator::generate(const format_ast& ast) const {
  return ast.to_string(language::python_fstring);
}

// Java Formatter生成器
std::string java_formatter_generator::generate(const format_ast& ast) const {
  return ast.to_string(language::java_formatter);
}

// 获取解析器
std::unique_ptr<format_parser> format_converter::get_parser(language lang) const {
  switch (lang) {
    case language::fmt: return std::make_unique<fmt_parser>();
    case language::python_fstring: return std::make_unique<python_fstring_parser>();
    case language::python_format: return std::make_unique<python_fstring_parser>(); // 类似
    case language::java_formatter: return std::make_unique<java_formatter_parser>();
    case language::csharp_stringformat: return std::make_unique<csharp_parser>();
    case language::c_printf: return std::make_unique<printf_parser>();
    default: return std::make_unique<fmt_parser>();
  }
}

// 获取生成器
std::unique_ptr<format_generator> format_converter::get_generator(language lang) const {
  switch (lang) {
    case language::fmt: return std::make_unique<fmt_generator>();
    case language::python_fstring: return std::make_unique<python_fstring_generator>();
    case language::python_format: return std::make_unique<python_fstring_generator>();
    case language::java_formatter: return std::make_unique<java_formatter_generator>();
    case language::csharp_stringformat: {
      // C#生成器需要特殊处理
      std::ostringstream oss;
      // 简化实现
      return std::make_unique<fmt_generator>();
    }
    default: return std::make_unique<fmt_generator>();
  }
}

// 解析格式字符串
format_ast format_converter::parse(std::string_view fmt_str, language lang) const {
  auto parser = get_parser(lang);
  return parser->parse(fmt_str);
}

// 生成格式字符串
std::string format_converter::generate(const format_ast& ast, language lang) const {
  return ast.to_string(lang);
}

// 转换格式字符串
std::string format_converter::convert(std::string_view fmt_str, language from_lang, language to_lang) const {
  auto ast = parse(fmt_str, from_lang);
  return generate(ast, to_lang);
}

} // namespace cross_format

FMT_END_NAMESPACE