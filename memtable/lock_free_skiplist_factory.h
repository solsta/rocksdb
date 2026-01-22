#pragma once

#include "rocksdb/memtablerep.h"

namespace ROCKSDB_NAMESPACE {

// Helper function to create LockFreeSkiplistFactory
inline MemTableRepFactory* NewLockFreeSkiplistFactory(int max_height = 32,
                                                      int branching_factor = 4) {
  return new LockFreeSkiplistFactory(max_height, branching_factor);
}

}  // namespace ROCKSDB_NAMESPACE
