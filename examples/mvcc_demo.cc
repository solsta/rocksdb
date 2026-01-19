#include <iostream>
#include <string>
#include <thread>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/snapshot.h"
#include "memtable/vector_ordered_rep.h"

using namespace ROCKSDB_NAMESPACE;

int main() {
  std::string kDBPath = "/tmp/rocksdb_mvcc_demo";
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

  std::cout << "=== MVCC (Multi-Version Concurrency Control) Demo ===" << std::endl;

  // Write initial data
  std::cout << "\n1. Writing initial data..." << std::endl;
  db->Put(WriteOptions(), "key1", "value_v1");
  db->Put(WriteOptions(), "key2", "value_v1");

  // Take a snapshot (reader freezes at this point)
  std::cout << "\n2. Taking snapshot (reader view freezes here)..." << std::endl;
  const Snapshot* snapshot = db->GetSnapshot();
  std::cout << "   Snapshot sequence number: " << snapshot->GetSequenceNumber() << std::endl;

  // Modify data in memtable
  std::cout << "\n3. Writer modifies data in memtable..." << std::endl;
  db->Put(WriteOptions(), "key1", "value_v2");
  db->Put(WriteOptions(), "key2", "value_v2");
  
  // Check current sequence number
  std::cout << "   Current DB sequence number: " << db->GetLatestSequenceNumber() << std::endl;

  // Read with current version (sees new data)
  std::cout << "\n4. Current reader sees new data:" << std::endl;
  std::string val;
  db->Get(ReadOptions(), "key1", &val);
  std::cout << "   key1 = " << val << " (new version)" << std::endl;

  // Read with snapshot (sees old data - MVCC in action!)
  std::cout << "\n5. Snapshot reader sees OLD data (MVCC protection):" << std::endl;
  ReadOptions snapshot_read;
  snapshot_read.snapshot = snapshot;
  db->Get(snapshot_read, "key1", &val);
  std::cout << "   key1 = " << val << " (old version from snapshot)" << std::endl;

  // Flush to SST
  std::cout << "\n6. Flushing memtable to SST file..." << std::endl;
  std::cout << "   What happens internally:" << std::endl;
  std::cout << "   - Memtable is written to SST file" << std::endl;
  std::cout << "   - Old memtable is kept in memory (NOT deleted)" << std::endl;
  std::cout << "   - Because snapshot at seq=" << snapshot->GetSequenceNumber() 
            << " still needs it" << std::endl;
  db->Flush(FlushOptions());

  // Reader can still use old snapshot
  std::cout << "\n7. After flush, snapshot reader STILL sees old data:" << std::endl;
  db->Get(snapshot_read, "key1", &val);
  std::cout << "   key1 = " << val << " (still protected by MVCC)" << std::endl;
  std::cout << "   Why? Because old memtable is still in memory!" << std::endl;

  // Current reader sees flushed data
  std::cout << "\n8. Current reader sees data from SST:" << std::endl;
  db->Get(ReadOptions(), "key1", &val);
  std::cout << "   key1 = " << val << " (from SST file)" << std::endl;

  // Release snapshot
  std::cout << "\n9. Releasing snapshot..." << std::endl;
  db->ReleaseSnapshot(snapshot);

  std::cout << "\n=== End of MVCC Demo ===" << std::endl;

  delete db;
  _Exit(0);
}
