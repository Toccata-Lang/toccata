cmake_minimum_required(VERSION 3.9.4)

project(toccata)

add_definitions(-std=c99)

set (CORE_LIBS pthread)

add_executable(toccata toccata.c core.c)
target_link_libraries(toccata ${CORE_LIBS})