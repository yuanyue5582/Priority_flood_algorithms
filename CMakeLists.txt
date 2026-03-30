cmake_minimum_required(VERSION 3.10)
project(DEM_Filling)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")
endif()

# 查找GDAL库
find_package(GDAL REQUIRED)

# 添加可执行文件
add_executable(DEM_Filling
    main.cpp
    dem.cpp
    utils.cpp
    FillDEM_Barnes.cpp
    FillDEM_PD.cpp
    FillDEM_Wang.cpp
    fillDEM_Wei.cpp
    FillDEM_Zhou_OnePass.cpp
    FillDEM_Zhou_TwoPass.cpp
)

# 设置头文件目录
target_include_directories(DEM_Filling PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GDAL_INCLUDE_DIR}
)

# 链接GDAL库
target_link_libraries(DEM_Filling
    ${GDAL_LIBRARY}
)

# 可选：设置输出目录
set_target_properties(DEM_Filling PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)