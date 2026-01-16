#include "memtable/vector_ordered_rep.h"

#include <algorithm>

#include "memory/arena.h"

namespace ROCKSDB_NAMESPACE {

MemTableRep::Iterator* VectorOrderedRep::GetIterator(Arena* arena) {
  // Build a sorted snapshot for the iterator
  std::vector<const char*> snap = entries_;
  std::sort(snap.begin(), snap.end(),
            [this](const char* a, const char* b) { return cmp_(a, b) < 0; });
  if (arena != nullptr) {
    void* mem = arena->AllocateAligned(sizeof(VecIterator));
    return new (mem) VecIterator(cmp_, snap);
  }
  return new VecIterator(cmp_, snap);
}

MemTableRep::Iterator* VectorOrderedRep::GetDynamicPrefixIterator(Arena* arena) {
  return GetIterator(arena);
}

}  // namespace ROCKSDB_NAMESPACE
