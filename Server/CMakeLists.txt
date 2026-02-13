# 设置最低版本的CMake要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(Server)

file(GLOB_RECURSE APP_SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# 2. 创建可执行文件
add_executable(Server ${APP_SRC_FILES})