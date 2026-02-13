cmake_minimum_required(VERSION 3.10)
project(Conversation)

# 全局设置 C++ 标准（如果所有子项目都用 C++17，可以在这里统一设置）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子目录
add_subdirectory(Server)