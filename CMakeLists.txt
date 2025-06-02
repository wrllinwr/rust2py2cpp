cmake_minimum_required(VERSION 3.16)  
project(lin)  
  
set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
# 查找SDL2  
find_package(SDL2 REQUIRED)  
  
# 添加可执行文件  
add_executable(lin  
    main.cpp  
    pack.cpp  
    tileset.cpp  
    map_loader.cpp  
    auto_map_loader.cpp  
)  
  
# 包含头文件目录  
target_include_directories(lin PRIVATE ${SDL2_INCLUDE_DIRS})  
  
# 链接库  
target_link_libraries(lin ${SDL2_LIBRARIES})  
  
# 如果是Windows，可能需要额外的库  
if(WIN32)  
    target_link_libraries(lin SDL2main)  
endif()