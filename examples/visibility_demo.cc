#include <iostream>
#include <string>
#include <thread>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "memtable/vector_ordered_rep.h"

using namespace ROCKSDB_NAMESPACE;

int main() {
  std::string kDBPath = "/tmp/rocksdb_visibility_demo";
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

  std::cout << "=== Visibility vs Durability Demo ===" << std::endl;
  std::cout << "\nThis shows the DIFFERENCE between durability and visibility:" << std::endl;

  // Test 1: sync=true (durable in WAL, but NOT yet flushed to SST)
  std::cout << "\n1. Write with sync=true (durable in WAL):" << std::endl;
  WriteOptions write_sync;
  write_sync.sync = true;
  db->Put(write_sync, "key1", "value_in_wal");
  
  std::cout << "   Data is now:" << std::endl;
  std::cout << "   - DURABLE: Yes (in WAL on disk)" << std::endl;
  std::cout << "   - VISIBLE: Yes (in memtable, readers can see it)" << std::endl;
  
  std::string val;
  s = db->Get(ReadOptions(), "key1", &val);
  std::cout << "   Reader immediately sees: key1 = " << val << std::endl;

  // Test 2: What if we crash before flush?
  std::cout << "\n2. What happens on crash BEFORE flush:" << std::endl;
  std::cout << "   Scenario: Process crashes NOW (before Flush)" << std::endl;
  std::cout << "   - Memtable data: LOST (not in SST)" << std::endl;
  std::cout << "   - WAL data: RECOVERED from WAL file on restart" << std::endl;
  std::cout << "   - Reader behavior on restart: Would see the data" << std::endl;

  std::cout << "\n3. Now flush to SST:" << std::endl;
  db->Flush(FlushOptions());
  std::cout << "   After flush:" << std::endl;
  std::cout << "   - Memtable: Empty or new" << std::endl;
  std::cout << "   - SST file: Contains the data" << std::endl;
  std::cout << "   - Reader: Still sees the same data (now from SST)" << std::endl;

  s = db->Get(ReadOptions(), "key1", &val);
  std::cout << "   Reader sees: key1 = " << val << std::endl;

  std::cout << "\n=== Key Insight ===" << std::endl;
  std::cout << "\nYou CANNOT prevent readers from seeing memtable data!" << std::endl;
  std::cout << "\nOnce data is in the memtable, readers see it:" << std::endl;
  std::cout << "  - sync=false: In memtable (not durable)" << std::endl;
  std::cout << "  - sync=true:  In memtable + WAL (durable)" << std::endl;
  std::cout << "\nBoth cases: Readers see the data IMMEDIATELY" << std::endl;
  std::cout << "\nThe sync flag ONLY controls durability, NOT visibility." << std::endl;

  delete db;
  _Exit(0);
}
