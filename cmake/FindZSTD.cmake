
find_library(
  ZSTD_LIBRARIES
  NAMES zstd_static zstd
)

find_path(
  ZSTD_INCLUDE_DIRS
  zstd.h
)
