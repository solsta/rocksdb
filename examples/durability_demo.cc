#include <iostream>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "memtable/vector_ordered_rep.h"

using namespace ROCKSDB_NAMESPACE;

int main() {
  std::string kDBPath = "/tmp/rocksdb_durability_demo";
  Options options;
  options.create_if_missing = true;
  options.error_if_exists = false;
  options.allow_concurrent_memtable_write = false;
  options.memtable_factory = std::make_shared<VectorOrderedRepFactory>();

  DB* db = nullptr;
  Status s = DB::Open(options, kDBPath, &db);
  if (!s.ok()) {
    std::cerr << "Open failed: " << s.ToString() << std::endl;
    return 1;
  }

  std::cout << "=== Durability Guarantees Demo ===" << std::endl;
  std::cout << "\nThis shows when readers can safely see data:" << std::endl;

  // Case 1: Write without sync (NOT durable)
  std::cout << "\n1. Write WITHOUT sync (data in memtable only, NOT durable):" << std::endl;
  WriteOptions write_no_sync;
  write_no_sync.sync = false;  // Default - write to memtable, return immediately
  db->Put(write_no_sync, "key1", "value1_not_durable");
  
  std::string val;
  db->Get(ReadOptions(), "key1", &val);
  std::cout << "   Reader sees: key1 = " << val << std::endl;
  std::cout << "   BUT: If process crashes NOW, this data is LOST!" << std::endl;
  std::cout << "   Risk: Data visible in memtable but not persisted" << std::endl;

  // Case 2: Write WITH sync (durable)
  std::cout << "\n2. Write WITH sync (sync=true, data durable in WAL):" << std::endl;
  WriteOptions write_with_sync;
  write_with_sync.sync = true;  // Write to memtable AND fsync to WAL
  db->Put(write_with_sync, "key2", "value2_durable");
  
  db->Get(ReadOptions(), "key2", &val);
  std::cout << "   Reader sees: key2 = " << val << std::endl;
  std::cout << "   Data is DURABLE in WAL (Write-Ahead Log)" << std::endl;
  std::cout << "   Safe: Even if process crashes, WAL can recover it" << std::endl;

  // Case 3: Flush to make data persistent on disk
  std::cout << "\n3. Flush memtable to SST file (most durable):" << std::endl;
  db->Flush(FlushOptions());
  
  db->Get(ReadOptions(), "key1", &val);
  std::cout << "   Reader sees: key1 = " << val << " (now in SST file)" << std::endl;
  std::cout << "   Data is now PERSISTENT in SST file" << std::endl;
  std::cout << "   Most durable: Survives even WAL loss" << std::endl;

  std::cout << "\n=== Summary ===" << std::endl;
  std::cout << "\nDurability levels (from least to most durable):" << std::endl;
  std::cout << "  1. Memtable only (sync=false): LOST on crash" << std::endl;
  std::cout << "  2. Memtable + WAL (sync=true): Recoverable from WAL" << std::endl;
  std::cout << "  3. SST file (after flush): Persistent on disk" << std::endl;

  std::cout << "\nFor your use case:" << std::endl;
  std::cout << "  - To prevent readers seeing uncommitted data: Use sync=true" << std::endl;
  std::cout << "  - This ensures data is in WAL before readers see it" << std::endl;
  std::cout << "  - If sync=false, readers might see data lost on crash" << std::endl;

  delete db;
  _Exit(0);
}
