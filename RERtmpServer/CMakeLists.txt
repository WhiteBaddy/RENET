project(RERtmpServer)

# 编译可执行程序
file(GLOB SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
add_executable(RERtmpServer
    ${SRC_FILES}
)

target_link_libraries(RERtmpServer PRIVATE
    third_party_lib
    RNet_lib
)

target_include_directories(RERtmpServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
