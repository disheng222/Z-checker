# Find librocksdb.a - key/value storage system

find_path(RocksDB_INCLUDE_DIR NAMES rocksdb/db.h)
find_library(RocksDB_LIBRARY NAMES rocksdb)

if(RocksDB_INCLUDE_DIR AND RocksDB_LIBRARY)
  set(RocksDB_FOUND TRUE)
endif(RocksDB_INCLUDE_DIR AND RocksDB_LIBRARY)

if(RocksDB_FOUND)
  if(NOT RocksDB_FIND_QUIETLY)
    message(STATUS "Found RocksDB: ${RocksDB_LIBRARY}")
  endif(NOT RocksDB_FIND_QUIETLY)
else(RocksDB_FOUND)
  if(RocksDB_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find rocksdb library.")
  endif(RocksDB_FIND_REQUIRED)
endif(RocksDB_FOUND)
