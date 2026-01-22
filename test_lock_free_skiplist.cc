#include <iostream>
#include <cassert>
#include "memtable/lock_free_skiplist_memtable.h"

using namespace ROCKSDB_NAMESPACE;

// Mock KeyComparator for testing
struct TestKeyComparator {
  int operator()(const char* a, const char* b) const {
    std::string sa(a), sb(b);
    if (sa < sb) return -1;
    if (sa > sb) return 1;
    return 0;
  }
};

int main() {
  std::cout << "Testing LockFreeSkiplistMemtable..." << std::endl;
  
  TestKeyComparator cmp;
  Arena arena;
  
  LockFreeSkiplistMemtable memtable(cmp, &arena, nullptr);
  
  // Test 1: InsertKey and Contains
  const char* key1 = "key1";
  bool inserted = memtable.InsertKey(const_cast<char*>(key1));
  std::cout << "Insert key1: " << (inserted ? "success" : "failed") << std::endl;
  assert(inserted);
  
  bool contains = memtable.Contains(key1);
  std::cout << "Contains key1: " << (contains ? "found" : "not found") << std::endl;
  assert(contains);
  
  // Test 2: Iterator
  Arena iter_arena;
  MemTableRep::Iterator* iter = memtable.GetIterator(&iter_arena);
  iter->SeekToFirst();
  std::cout << "Iterator valid: " << (iter->Valid() ? "yes" : "no") << std::endl;
  assert(iter->Valid());
  
  std::cout << "All tests passed!" << std::endl;
  delete iter;
  return 0;
}
