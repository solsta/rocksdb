#include <iostream>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "memtable/vector_ordered_rep.h"

using namespace ROCKSDB_NAMESPACE;

static void PrintLevels(DB* db) {
  uint64_t l0 = 0, l1 = 0, pending = 0;
  db->GetIntProperty("rocksdb.num-files-at-level0", &l0);
  db->GetIntProperty("rocksdb.num-files-at-level1", &l1);
  db->GetIntProperty("rocksdb.compaction-pending", &pending);
  std::string sstables;
  db->GetProperty("rocksdb.sstables", &sstables);
  std::cout << "Files: L0=" << l0 << ", L1=" << l1
            << ", compaction-pending=" << pending << std::endl;
  std::cout << sstables << std::endl;
}

int main() {
  std::string kDBPath = "/tmp/rocksdb_two_flushes_compaction_demo";

  Options options;
  options.create_if_missing = true;
  options.error_if_exists = false;
  options.allow_concurrent_memtable_write = false; // our simple memtable isn't concurrent
  options.use_fsync = true;
  options.memtable_factory = std::make_shared<VectorOrderedRepFactory>();

  // Start clean for reproducibility
  DestroyDB(kDBPath, options);

  DB* db = nullptr;
  Status s = DB::Open(options, kDBPath, &db);
  if (!s.ok()) {
    std::cerr << "Open failed: " << s.ToString() << std::endl;
    return 1;
  }

  // First batch: write and flush -> creates one L0 SST
  std::cout << "Write batch 1 and flush" << std::endl;
  s = db->Put(WriteOptions(), "a", "1");
  s = db->Put(WriteOptions(), "b", "1");
  FlushOptions fo; fo.wait = true;
  s = db->Flush(fo);
  std::cout << "Flush1: " << s.ToString() << std::endl;
  PrintLevels(db);

  // Second batch: overwrite and add new key, then flush -> another L0 SST
  std::cout << "Write batch 2 (overwrite a, add c) and flush" << std::endl;
  s = db->Put(WriteOptions(), "a", "2");
  s = db->Put(WriteOptions(), "c", "3");
  s = db->Flush(fo);
  std::cout << "Flush2: " << s.ToString() << std::endl;
  PrintLevels(db);

  // Iterate current view (merging iterator across L0 files)
  std::cout << "Iterate after two flushes:" << std::endl;
  std::unique_ptr<Iterator> it(db->NewIterator(ReadOptions()));
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::cout << it->key().ToString() << " => " << it->value().ToString() << std::endl;
  }
  if (!it->status().ok()) {
    std::cerr << "Iterator status: " << it->status().ToString() << std::endl;
  }

  // Force a manual compaction across the whole key range; this merges L0 SSTs into lower levels
  std::cout << "Force CompactRange over entire DB" << std::endl;
  CompactRangeOptions cro;
  s = db->CompactRange(cro, nullptr, nullptr);
  std::cout << "CompactRange: " << s.ToString() << std::endl;
  PrintLevels(db);

  // Iterate again after compaction
  std::cout << "Iterate after compaction:" << std::endl;
  it.reset(db->NewIterator(ReadOptions()));
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::cout << it->key().ToString() << " => " << it->value().ToString() << std::endl;
  }
  if (!it->status().ok()) {
    std::cerr << "Iterator status: " << it->status().ToString() << std::endl;
  }

  delete db;
  _Exit(0);
}
