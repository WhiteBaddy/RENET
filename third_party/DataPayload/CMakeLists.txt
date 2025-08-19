
#设置库名
set(LIBNAME DataPayload)

# 默认 INTERFACE（如果父级没有指定）
if(NOT DEFINED THIRD_PARTY_LIB_TYPE)
    set(THIRD_PARTY_LIB_TYPE SHARED)
endif()

if (THIRD_PARTY_LIB_TYPE STREQUAL "INTERFACE")
    add_library(${LIBNAME} INTERFACE)
    target_include_directories(
        ${LIBNAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
else()
    file(
        GLOB
        TOOLS_SOURCES
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )
    add_library(
        ${LIBNAME}
        ${THIRD_PARTY_LIB_TYPE}
        ${TOOLS_SOURCES}
    )
    target_include_directories(
        ${LIBNAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endif()
