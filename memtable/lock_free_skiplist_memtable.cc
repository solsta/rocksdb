#include "memtable/lock_free_skiplist_memtable.h"
#include <cassert>
#include <cstdio>
#include "rocksdb/comparator.h"
#include "rocksdb/slice.h"
#include "db/lookup_key.h"

namespace ROCKSDB_NAMESPACE {

// ==================== LockFreeSkiplistMemtable ====================

LockFreeSkiplistMemtable::LockFreeSkiplistMemtable(
    const KeyComparator& cmp, Allocator* allocator,
    const SliceTransform* transform)
    : MemTableRep(allocator),
      cmp_(cmp),
      allocator_(allocator),
      transform_(transform),
      entry_count_(0) {
  printf("[STUB] LockFreeSkiplistMemtable::constructor\n");
}

LockFreeSkiplistMemtable::~LockFreeSkiplistMemtable() {
  printf("[STUB] LockFreeSkiplistMemtable::destructor\n");
}

void LockFreeSkiplistMemtable::Insert(KeyHandle handle) {
  (void)handle;
  printf("[STUB] LockFreeSkiplistMemtable::Insert\n");
  assert(false && "Insert stub - not implemented");
}

bool LockFreeSkiplistMemtable::InsertKey(KeyHandle handle) {
  (void)handle;
  printf("[STUB] LockFreeSkiplistMemtable::InsertKey\n");
  assert(false && "InsertKey stub - not implemented");
  return false;
}

bool LockFreeSkiplistMemtable::Contains(const char* key) const {
  (void)key;
  printf("[STUB] LockFreeSkiplistMemtable::Contains\n");
  assert(false && "Contains stub - not implemented");
  return false;
}

void LockFreeSkiplistMemtable::MarkReadOnly() {
  printf("[STUB] LockFreeSkiplistMemtable::MarkReadOnly\n");
}

void LockFreeSkiplistMemtable::MarkFlushed() {
  printf("[STUB] LockFreeSkiplistMemtable::MarkFlushed\n");
}

void LockFreeSkiplistMemtable::Get(const LookupKey& k, void* callback_args,
                                    bool (*callback_func)(void* arg,
                                                         const char* entry)) {
  (void)k;
  (void)callback_args;
  (void)callback_func;
  printf("[STUB] LockFreeSkiplistMemtable::Get\n");
  assert(false && "Get stub - not implemented");
}

MemTableRep::Iterator* LockFreeSkiplistMemtable::GetIterator(Arena* arena) {
  (void)arena;
  printf("[STUB] LockFreeSkiplistMemtable::GetIterator\n");
  assert(false && "GetIterator stub - not implemented");
  return nullptr;
}

size_t LockFreeSkiplistMemtable::ApproximateMemoryUsage() {
  printf("[STUB] LockFreeSkiplistMemtable::ApproximateMemoryUsage\n");
  assert(false && "ApproximateMemoryUsage stub - not implemented");
  return 0;
}

uint64_t LockFreeSkiplistMemtable::ApproximateNumEntries(
    const Slice& start_ikey, const Slice& end_key) {
  (void)start_ikey;
  (void)end_key;
  printf("[STUB] LockFreeSkiplistMemtable::ApproximateNumEntries\n");
  assert(false && "ApproximateNumEntries stub - not implemented");
  return 0;
}

void LockFreeSkiplistMemtable::UniqueRandomSample(
    const uint64_t num_entries, const uint64_t target_sample_size,
    std::unordered_set<const char*>* entries) {
  (void)num_entries;
  (void)target_sample_size;
  (void)entries;
  printf("[STUB] LockFreeSkiplistMemtable::UniqueRandomSample\n");
  assert(false && "UniqueRandomSample stub - not implemented");
}

// ==================== LockFreeSkiplistIterator ====================

LockFreeSkiplistIterator::LockFreeSkiplistIterator(
    LockFreeSkiplistMemtable* table)
    : table_(table), index_(0) {
  printf("[STUB] LockFreeSkiplistIterator::constructor\n");
}

bool LockFreeSkiplistIterator::Valid() const {
  printf("[STUB] LockFreeSkiplistIterator::Valid\n");
  assert(false && "Valid stub - not implemented");
  return false;
}

const char* LockFreeSkiplistIterator::key() const {
  printf("[STUB] LockFreeSkiplistIterator::key\n");
  assert(false && "key stub - not implemented");
  return nullptr;
}

void LockFreeSkiplistIterator::Seek(const Slice& internal_key,
                                   const char* memtable_key) {
  (void)internal_key;
  (void)memtable_key;
  printf("[STUB] LockFreeSkiplistIterator::Seek\n");
  assert(false && "Seek stub - not implemented");
}

void LockFreeSkiplistIterator::SeekForPrev(const Slice& internal_key,
                                          const char* memtable_key) {
  (void)internal_key;
  (void)memtable_key;
  printf("[STUB] LockFreeSkiplistIterator::SeekForPrev\n");
  assert(false && "SeekForPrev stub - not implemented");
}

void LockFreeSkiplistIterator::SeekToFirst() {
  printf("[STUB] LockFreeSkiplistIterator::SeekToFirst\n");
  assert(false && "SeekToFirst stub - not implemented");
}

void LockFreeSkiplistIterator::SeekToLast() {
  printf("[STUB] LockFreeSkiplistIterator::SeekToLast\n");
  assert(false && "SeekToLast stub - not implemented");
}

void LockFreeSkiplistIterator::Next() {
  printf("[STUB] LockFreeSkiplistIterator::Next\n");
  assert(false && "Next stub - not implemented");
}

void LockFreeSkiplistIterator::Prev() {
  printf("[STUB] LockFreeSkiplistIterator::Prev\n");
  assert(false && "Prev stub - not implemented");
}

// ==================== LockFreeSkiplistFactory ====================

MemTableRep* LockFreeSkiplistFactory::CreateMemTableRep(
    const MemTableRep::KeyComparator& compare, Allocator* allocator,
    const SliceTransform* transform, Logger* logger) {
  (void)logger;
  printf("[STUB] LockFreeSkiplistFactory::CreateMemTableRep\n");
  return new LockFreeSkiplistMemtable(compare, allocator, transform);
}

const char* LockFreeSkiplistFactory::Name() const {
  return "LockFreeSkiplistFactory";
}

}  // namespace ROCKSDB_NAMESPACE
