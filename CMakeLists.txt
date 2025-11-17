cmake_minimum_required(VERSION 3.10)
project(fmt-extensions)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenSSL库
find_package(OpenSSL REQUIRED)

# 包含头文件目录
include_directories(include)

# 源文件
set(FMT_SOURCES
    src/format.cc
    src/cross_format.cc
    src/distributed_format.cc
)

# 库文件
add_library(fmt SHARED ${FMT_SOURCES})
target_link_libraries(fmt PRIVATE OpenSSL::Crypto)

# 测试程序
add_executable(test_complete_features test_complete_features.cpp)
target_link_libraries(test_complete_features fmt OpenSSL::Crypto)

# 安装目标
install(TARGETS fmt test_complete_features
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/fmt DESTINATION include)