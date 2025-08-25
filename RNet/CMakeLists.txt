set(LIBNAME RNet_lib)
set(EXECNAME RNet)

project(${EXECNAME})

# 创建可执行程序
add_executable(${EXECNAME}
    src/main.cpp
)

add_library(${LIBNAME} INTERFACE)
# 添加本项目头文件路径（可选）
target_include_directories(${LIBNAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${EXECNAME} PRIVATE
    third_party_lib
    RNet_lib
)
