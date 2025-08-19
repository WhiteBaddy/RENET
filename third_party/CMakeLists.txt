
# 库名
set(LIBNAME third_party_lib)

# 设置统一的库类型变量, 可选值: STATIC | SHARED | MODULE | INTERFACE
set(THIRD_PARTY_LIB_TYPE STATIC)

# 作为聚合库：只传递依赖，不生成目标文件
add_library(${LIBNAME} INTERFACE)

# 子模块
add_subdirectory(Buffer)
add_subdirectory(DataPayload)
add_subdirectory(String)
add_subdirectory(Tools)
add_subdirectory(fmt)

# 聚合依赖（全部用 INTERFACE）
target_link_libraries(${LIBNAME}
    INTERFACE Buffer
    INTERFACE DataPayload
    INTERFACE String
    INTERFACE Tools
    INTERFACE fmt::fmt
)

# Linux 专属
if(UNIX AND NOT APPLE)
    add_subdirectory(Epoll)
    target_link_libraries(${LIBNAME} INTERFACE Epoll)
endif()

# # （可选）提供命名空间别名，方便引用
# add_library(third_party::all ALIAS ${LIBNAME})

# # 判断 这个是否真的存在
# get_target_property(_tmp fmt::fmt TYPE)
# message(STATUS "fmt::fmt type = ${_tmp}")

