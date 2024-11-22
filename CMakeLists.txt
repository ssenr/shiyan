cmake_minimum_required(VERSION 3.28)

project(cnipe VERSION 1.0 DESCRIPTION "idk rn")

add_subdirectory(
  src/include/DPP 
)

add_executable(${PROJECT_NAME}
  src/main.cc
  src/scr.cc
)

target_link_libraries(${PROJECT_NAME}
  dpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)
