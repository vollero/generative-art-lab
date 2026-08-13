cmake_minimum_required(VERSION 3.20)

project(generative_art_lab VERSION 0.6.0 LANGUAGES CXX)

include(CTest)
add_subdirectory(artifacts/001-modular-times-table)
add_subdirectory(artifacts/002-ulam-prime-spiral)
add_subdirectory(artifacts/003-divisor-field)
add_subdirectory(artifacts/004-coprime-lattice)
add_subdirectory(artifacts/005-fourier-epicycles)
add_subdirectory(artifacts/006-modular-flow)
