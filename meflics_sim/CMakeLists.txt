cmake_minimum_required(VERSION 3.2)
project(MEFLICS_SIM)
set(EXECUTABLE_NAME "MEFLICS_SIM")
message("****************** this example only works after libCacheSim has been installed ******************")

find_library(libCacheSim libCacheSim)
find_path(libCacheSimInclude libCacheSim)
message(STATUS "libCacheSim found? ${libCacheSim} ${libCacheSimInclude}")
include_directories(${libCacheSimInclude})

# pkg-config로 glib 찾기
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})
message(STATUS "glib found? ${GLIB2_FOUND}, lib = ${GLIB2_LIBRARIES}, header = ${GLIB2_INCLUDE_DIRS}")

# pkg-config로 zstd 찾기
pkg_check_modules(ZSTD REQUIRED libzstd)
include_directories(${ZSTD_INCLUDE_DIRS})
link_directories(${ZSTD_LIBRARY_DIRS})
add_definitions(${ZSTD_CFLAGS_OTHER})

# 실행파일 정의
add_executable(MEFLICS_SIM main.c)

# 링크 설정
target_link_libraries(MEFLICS_SIM libCacheSim m ${GLIB2_LIBRARIES} ${ZSTD_LIBRARIES})
