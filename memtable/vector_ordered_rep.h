// A minimal MemTableRep implementation backed by a simple vector of entry
// pointers. It captures inserted entry pointers (allocated from the arena)
// and provides a sorted snapshot iterator for flush.
//
// Goals:
// - Keep the encoded entry layout identical to MemTable::Add expectations.
// - Provide ordered iteration in InternalKey order (user key asc, seq desc).
// - No concurrent insert support (simple, single-writer path).
// - Lightweight duplicate handling: do not detect duplicates (factory returns
//   CanHandleDuplicatedKey() = false).

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "rocksdb/memtablerep.h"

namespace ROCKSDB_NAMESPACE {

// VectorOrderedRep: simple MemTableRep storing raw entry pointers and
// returning a sorted snapshot on iterator creation.
class VectorOrderedRep : public MemTableRep {
 public:
  VectorOrderedRep(const MemTableRep::KeyComparator& cmp, Allocator* allocator)
      : MemTableRep(allocator), cmp_(cmp) {}

  // MemTableRep overrides
  void Insert(KeyHandle handle) override {
    entries_.push_back(reinterpret_cast<const char*>(handle));
    sorted_ = false;
  }

  bool InsertKey(KeyHandle handle) override {
    Insert(handle);
    return true;  // no duplicate detection
  }

  void InsertWithHint(KeyHandle handle, void** /*hint*/) override {
    Insert(handle);
  }

  bool InsertKeyWithHint(KeyHandle handle, void** /*hint*/) override {
    return InsertKey(handle);
  }

  bool Contains(const char* key) const override {
    // Simple linear search when unsorted; otherwise binary search on snapshot
    if (!sorted_) {
      for (const char* p : entries_) {
        if (cmp_(p, key) == 0) return true;
      }
      return false;
    }
    return ContainsSorted(key);
  }

  void MarkReadOnly() override { SortInPlace(); }

  void MarkFlushed() override {}

  size_t ApproximateMemoryUsage() override {
    return entries_.capacity() * sizeof(const char*);
  }

  class VecIterator : public MemTableRep::Iterator {
   public:
    VecIterator(const MemTableRep::KeyComparator& cmp,
                const std::vector<const char*>& snapshot)
        : cmp_(cmp), index_(snapshot), pos_(static_cast<size_t>(-1)) {}

    bool Valid() const override { return pos_ < index_.size(); }
    const char* key() const override {
      return index_[pos_];
    }
    void Next() override {
      if (pos_ < index_.size()) ++pos_;
    }
    Status NextAndValidate(bool) override {
      Next();
      return Status::OK();
    }
    void Prev() override {
      if (pos_ == 0) {
        pos_ = static_cast<size_t>(-1);
      } else if (pos_ != static_cast<size_t>(-1)) {
        --pos_;
      }
    }
    Status PrevAndValidate(bool) override {
      Prev();
      return Status::OK();
    }
    void Seek(const Slice& internal_key, const char* /*memtable_key*/) override {
      // lower_bound to first >= internal_key
      auto it = std::lower_bound(
          index_.begin(), index_.end(), internal_key,
          [this](const char* a, const Slice& b) { return cmp_(a, b) < 0; });
      pos_ = static_cast<size_t>(it - index_.begin());
    }
    Status SeekAndValidate(const Slice& internal_key, const char*, bool, bool,
                           const std::function<Status(const char*, bool)>&) override {
      Seek(internal_key, nullptr);
      return Status::OK();
    }
    void SeekForPrev(const Slice& internal_key, const char* /*memtable_key*/) override {
      // Find first >= key, then step back if > key
      auto it = std::lower_bound(
          index_.begin(), index_.end(), internal_key,
          [this](const char* a, const Slice& b) { return cmp_(a, b) < 0; });
      if (it == index_.begin()) {
        if (it != index_.end() && cmp_(*it, internal_key) <= 0) {
          pos_ = 0;
        } else {
          pos_ = static_cast<size_t>(-1);  // invalid
        }
      } else {
        if (it == index_.end() || cmp_(*it, internal_key) > 0) {
          --it;
        }
        pos_ = static_cast<size_t>(it - index_.begin());
      }
    }
    void SeekToFirst() override { pos_ = index_.empty() ? static_cast<size_t>(-1) : 0; }
    void SeekToLast() override {
      pos_ = index_.empty() ? static_cast<size_t>(-1) : (index_.size() - 1);
    }

   private:
    const MemTableRep::KeyComparator& cmp_;
    std::vector<const char*> index_;
    size_t pos_;
  };

  Iterator* GetIterator(Arena* arena = nullptr) override;

  Iterator* GetDynamicPrefixIterator(Arena* arena = nullptr) override;

  void UniqueRandomSample(const uint64_t /*num_entries*/,
                          const uint64_t target_sample_size,
                          std::unordered_set<const char*>* out) override {
    if (entries_.empty() || target_sample_size == 0) return;
    uint64_t step = std::max<uint64_t>(1, entries_.size() / target_sample_size);
    for (uint64_t i = 0; i < entries_.size(); i += step) {
      out->insert(entries_[static_cast<size_t>(i)]);
      if (out->size() >= target_sample_size) break;
    }
  }

 private:
  void SortInPlace() {
    if (!sorted_) {
      std::sort(entries_.begin(), entries_.end(),
                [this](const char* a, const char* b) { return cmp_(a, b) < 0; });
      sorted_ = true;
    }
  }

  bool ContainsSorted(const char* key) const {
    auto it = std::lower_bound(entries_.begin(), entries_.end(), key,
                               [this](const char* a, const char* b_key) {
                                 return cmp_(a, b_key) < 0;
                               });
    return it != entries_.end() && cmp_(*it, key) == 0;
  }

  const MemTableRep::KeyComparator& cmp_;
  std::vector<const char*> entries_;
  bool sorted_ = false;
};

class VectorOrderedRepFactory : public MemTableRepFactory {
 public:
  static const char* kClassName() { return "VectorOrderedRepFactory"; }
  static const char* kNickName() { return "vector_ordered"; }

  const char* Name() const override { return kClassName(); }
  const char* NickName() const override { return kNickName(); }

  using MemTableRepFactory::CreateMemTableRep;
  MemTableRep* CreateMemTableRep(const MemTableRep::KeyComparator& cmp,
                                 Allocator* allocator,
                                 const SliceTransform* /*slice_transform*/,
                                 Logger* /*logger*/) override {
    return new VectorOrderedRep(cmp, allocator);
  }

  bool IsInsertConcurrentlySupported() const override { return false; }
  bool CanHandleDuplicatedKey() const override { return false; }
};

}  // namespace ROCKSDB_NAMESPACE
