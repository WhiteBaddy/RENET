cmake_minimum_required(VERSION 3.20.0)

project(RENET)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#静态库的输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
#动态库的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
#可执行文件的输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)


# 添加 third_party 统一入口
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party)
# 链接 third_party 聚合库
# target_link_libraries(RENET PRIVATE third_party_lib)

add_subdirectory(${PROJECT_SOURCE_DIR}/RNet)
# add_subdirectory(${PROJECT_SOURCE_DIR}/RtmpServer)
add_subdirectory(${PROJECT_SOURCE_DIR}/RERtmpServer)
add_subdirectory(${PROJECT_SOURCE_DIR}/SigServer)
#add_subdirectory(${PROJECT_SOURCE_DIR}/LoadBanceServer)
#add_subdirectory(${PROJECT_SOURCE_DIR}/LoginServer)


