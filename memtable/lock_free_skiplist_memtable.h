#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#include "rocksdb/memtablerep.h"
#include "rocksdb/slice.h"
#include "rocksdb/customizable.h"
#include "memory/allocator.h"
#include "db/lookup_key.h"

namespace ROCKSDB_NAMESPACE {

// Wrapper around lock-free skiplist to work with RocksDB memtables
// The actual skiplist implementation is in lock_free_skiplist_no_durability.h
class LockFreeSkiplistMemtable : public MemTableRep {
 public:
  explicit LockFreeSkiplistMemtable(const KeyComparator& cmp,
                                     Allocator* allocator,
                                     const SliceTransform* transform);

  ~LockFreeSkiplistMemtable() override;

  // Insert a key-value pair
  void Insert(KeyHandle handle) override;

  bool InsertKey(KeyHandle handle) override;

  // Concurrent write support
  void InsertConcurrently(KeyHandle handle) override;

  bool InsertKeyConcurrently(KeyHandle handle) override;

  // Check if key exists
  bool Contains(const char* key) const override;

  // Mark memtable as read-only
  void MarkReadOnly() override;

  void MarkFlushed() override;

  // Get value for key
  void Get(const LookupKey& k, void* callback_args,
           bool (*callback_func)(void* arg, const char* entry)) override;

  // Get iterator
  Iterator* GetIterator(Arena* arena = nullptr) override;

  // Return approximate memory usage
  size_t ApproximateMemoryUsage() override;

  // Return approximate number of entries
  uint64_t ApproximateNumEntries(const Slice& start_ikey,
                                  const Slice& end_key) override;

  // Sample random entries
  void UniqueRandomSample(const uint64_t num_entries,
                         const uint64_t target_sample_size,
                         std::unordered_set<const char*>* entries) override;

  static const char* kClassName() { return "LockFreeSkiplistMemtable"; }

  static const char* NickName() { return "lock_free_skiplist"; }

 private:
  friend class LockFreeSkiplistIterator;

  const KeyComparator& cmp_;  // Store reference like SkipListRep does
  Allocator* allocator_;
  const SliceTransform* transform_;
  
  // Store entries in a vector
  // In a full implementation, this would use the actual lock-free skiplist
  std::vector<const char*> entries_map_;
  
  uint64_t entry_count_;
};

// Iterator for LockFreeSkiplistMemtable
class LockFreeSkiplistIterator : public MemTableRep::Iterator {
 public:
  explicit LockFreeSkiplistIterator(LockFreeSkiplistMemtable* table);

  ~LockFreeSkiplistIterator() override = default;

  bool Valid() const override;
  const char* key() const override;
  void Seek(const Slice& internal_key, const char* memtable_key) override;
  void SeekForPrev(const Slice& internal_key, const char* memtable_key) override;
  void SeekToFirst() override;
  void SeekToLast() override;
  void Next() override;
  void Prev() override;

 private:
  LockFreeSkiplistMemtable* table_;
  size_t index_;
};

// Factory for creating LockFreeSkiplistMemtable instances
class LockFreeSkiplistFactory : public MemTableRepFactory {
 public:
  explicit LockFreeSkiplistFactory() = default;

  ~LockFreeSkiplistFactory() override = default;

  using MemTableRepFactory::CreateMemTableRep;

  MemTableRep* CreateMemTableRep(
      const MemTableRep::KeyComparator& compare, Allocator* allocator,
      const SliceTransform* transform, Logger* logger) override;

  const char* Name() const override;

  bool IsInsertConcurrentlySupported() const override { return true; }

  static const char* kClassName() { return "LockFreeSkiplistFactory"; }
};

}  // namespace ROCKSDB_NAMESPACE
