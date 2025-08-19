project(RNet)

# 创建可执行程序
add_executable(RNet
    src/main.cpp
)

# target_include_directories(RNet PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

add_library(RNet_lib INTERFACE)
# 添加本项目头文件路径（可选）
target_include_directories(RNet_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(RNet PRIVATE
    third_party_lib
    RNet_lib
)
