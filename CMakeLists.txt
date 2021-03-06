cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_FLAGS "-pthread -Wall -std=c++17 -pedantic ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,undefined -D_GLIBCXX_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_COVERAGE "-g --coverage")
set(LINK_FLAGS "-pthread ${LINK_FLAGS}")

include_directories(.)
add_subdirectory(gtest)
#find_package(GTest REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/gtest)

add_library(counted counted.h counted.cpp fault_injection.h fault_injection.cpp gtest/gtest-all.cc gtest/gtest_main.cc)

add_executable(std std.cpp)
target_link_libraries(std counted)

add_executable(fedorova_irina fedorova_irina.cpp fedorova_irina.h)
target_link_libraries(fedorova_irina counted gtest)

add_executable(smirnov_roman smirnov_roman.cpp smirnov_roman.h)
target_link_libraries(smirnov_roman counted gtest)

#add_executable(anikienko_anton anikienko_anton.cpp anikienko_anton.h)
#target_link_libraries(anikienko_anton counted gtest)

add_executable(krivopaltsev_dmitriy krivopaltsev_dmitriy.cpp krivopaltsev_dmitriy.h)
target_link_libraries(krivopaltsev_dmitriy counted gtest)

add_executable(shelepov_anton shelepov_anton.cpp shelepov_anton.h)
target_link_libraries(shelepov_anton counted gtest)

add_executable(hil_valeria hil_valeria.cpp hil_valeria.h)
target_link_libraries(hil_valeria counted gtest)

add_executable(sharipov_samariddin sharipov_samariddin.cpp sharipov_samariddin.h)
target_link_libraries(sharipov_samariddin counted gtest)

add_executable(savinov_nikita savinov_nikita.cpp savinov_nikita.h)
target_link_libraries(savinov_nikita counted gtest)

add_executable(pushkin_nikita pushkin_nikita.cpp pushkin_nikita.h)
target_link_libraries(pushkin_nikita counted gtest)

add_executable(valeev_nursan valeev_nursan.cpp valeev_nursan.h)
target_link_libraries(valeev_nursan counted gtest)

add_executable(ustinov_artem ustinov_artem.cpp ustinov_artem.h)
target_link_libraries(ustinov_artem counted gtest)

#add_executable(nefedov_dmitriy nefedov_dmitriy.cpp nefedov_dmitriy.h)
#target_link_libraries(nefedov_dmitriy counted gtest)
