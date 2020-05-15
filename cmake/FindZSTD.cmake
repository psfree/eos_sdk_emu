
if(USE_ZSTD_COMPRESS)
  find_library(
    ZSTD_LIBRARIES
    NAMES zstd_static zstd
  )
  
  find_path(
    ZSTD_INCLUDE
    zstd.h
  )
endif()
