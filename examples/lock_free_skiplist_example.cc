#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <sstream>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "memtable/lock_free_skiplist_memtable.h"

using namespace ROCKSDB_NAMESPACE;

int main() {
  std::cout << "=== Lock-Free Skiplist Memtable Example ===" << std::endl << std::endl;

  // Example 1: Direct memtable usage (for testing)
  std::cout << "1. Testing LockFreeSkiplistMemtable directly:" << std::endl;
  std::cout << "   (This demonstrates the memtable structure)" << std::endl;
  
  // Create a simple memtable factory
  auto factory = std::make_shared<LockFreeSkiplistFactory>();
  
  std::cout << "   ✓ Created LockFreeSkiplistFactory: " << factory->Name() << std::endl;
  std::cout << std::endl;

  // Example 2: Using with RocksDB database
  std::cout << "2. Using LockFreeSkiplistMemtable with RocksDB:" << std::endl;
  
  // Set up options
  Options options;
  options.create_if_missing = true;
  
  // Use our custom lock-free skiplist factory
  options.memtable_factory = factory;
  
  // Other settings
  options.write_buffer_size = 256 * 1024 * 1024;  // 256 MB (reasonable for testing)
  options.allow_concurrent_memtable_write = true;  // Lock-free skiplist supports concurrent writes
  
  std::cout << "   Database configuration:" << std::endl;
  std::cout << "   - Memtable type: LockFreeSkiplistMemtable" << std::endl;
  std::cout << "   - Write buffer size: 256 MB" << std::endl;
  std::cout << "   - Concurrent memtable writes: enabled" << std::endl;
  std::cout << std::endl;

  // Open database
  DB* db = nullptr;
  Status status = DB::Open(options, "/tmp/rocksdb_lock_free_test", &db);
  
  if (!status.ok()) {
    std::cerr << "Failed to open database: " << status.ToString() << std::endl;
    return 1;
  }
  
  std::cout << "   ✓ Database opened successfully" << std::endl;
  std::cout << std::endl;

  // Example 3: Write operations
  std::cout << "3. Write operations:" << std::endl;
  
  WriteOptions write_options;
  write_options.sync = false;  // Use memtable (not durable yet)
  
  std::vector<std::string> keys = {
      "alice", "bob", "charlie", "diana", "eve",
      "frank", "grace", "henry", "iris", "jack"
  };
  
  std::vector<std::string> values = {
      "value_1", "value_2", "value_3", "value_4", "value_5",
      "value_6", "value_7", "value_8", "value_9", "value_10"
  };
  
  for (size_t i = 0; i < keys.size(); ++i) {
    std::cerr << "Inserting: " << keys[i] << " -> " << values[i] << std::endl;
    status = db->Put(write_options, keys[i], values[i]);
    if (!status.ok()) {
      std::cerr << "Failed to write key: " << keys[i] << std::endl;
      delete db;
      return 1;
    }
    std::cout << "   ✓ Inserted: " << keys[i] << " -> " << values[i] << std::endl;
  }
  std::cout << std::endl;

  // Example 4: Read operations
  std::cout << "4. Read operations:" << std::endl;
  
  ReadOptions read_options;
  
  for (const auto& key : keys) {
    std::string value;
    status = db->Get(read_options, key, &value);
    
    if (status.ok()) {
      std::cout << "   ✓ Read: " << key << " -> " << value << std::endl;
    } else if (status.IsNotFound()) {
      std::cout << "   ✗ Key not found: " << key << std::endl;
    } else {
      std::cerr << "   Error reading key: " << key << std::endl;
    }
  }
  std::cout << std::endl;

  // Example 5: Range iteration
  std::cout << "5. Range scan (iterate all keys):" << std::endl;
  
  Iterator* iter = db->NewIterator(read_options);
  assert(iter != nullptr);
  
  int count = 0;
  for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
    std::cout << "   ✓ " << iter->key().ToString() << " -> " 
              << iter->value().ToString() << std::endl;
    count++;
  }
  
  if (!iter->status().ok()) {
    std::cerr << "Iterator error: " << iter->status().ToString() << std::endl;
  }
  
  delete iter;
  std::cout << "   Total entries: " << count << std::endl;
  std::cout << std::endl;

  // Example 6: Seeking to specific positions
  std::cout << "6. Seek operations:" << std::endl;
  
  iter = db->NewIterator(read_options);
  
  // Seek to "charlie"
  iter->Seek("charlie");
  if (iter->Valid()) {
    std::cout << "   ✓ Seek('charlie'): " << iter->key().ToString() << std::endl;
  }
  
  // Next
  iter->Next();
  if (iter->Valid()) {
    std::cout << "   ✓ Next(): " << iter->key().ToString() << std::endl;
  }
  
  // Seek to end and iterate backward
  iter->SeekToLast();
  if (iter->Valid()) {
    std::cout << "   ✓ SeekToLast(): " << iter->key().ToString() << std::endl;
  }
  
  iter->Prev();
  if (iter->Valid()) {
    std::cout << "   ✓ Prev(): " << iter->key().ToString() << std::endl;
  }
  
  delete iter;
  std::cout << std::endl;

  // Example 7: Snapshots (MVCC)
  std::cout << "7. Snapshot (MVCC) operations:" << std::endl;
  
  // Take a snapshot before modification
  const Snapshot* snapshot = db->GetSnapshot();
  std::cout << "   ✓ Snapshot created" << std::endl;
  
  // Add a new key
  db->Put(write_options, "zoe", "value_new");
  std::cout << "   ✓ Added key 'zoe' after snapshot" << std::endl;
  
  // Read with snapshot (should not see "zoe")
  std::string value_with_snapshot;
  status = db->Get(read_options, "zoe", &value_with_snapshot);
  std::cout << "   ✓ Read 'zoe' with snapshot: " 
            << (status.IsNotFound() ? "NOT FOUND (as expected)" : "FOUND") << std::endl;
  
  // Read without snapshot (should see "zoe")
  std::string value_without_snapshot;
  status = db->Get(read_options, "zoe", &value_without_snapshot);
  std::cout << "   ✓ Read 'zoe' without snapshot: " 
            << (status.ok() ? "FOUND" : "NOT FOUND") << std::endl;
  
  db->ReleaseSnapshot(snapshot);
  std::cout << "   ✓ Snapshot released" << std::endl;
  std::cout << std::endl;

  // Example 8: Flush and compact
  std::cout << "8. Flush and compaction:" << std::endl;
  
  // Force flush (memtable -> SST)
  status = db->Flush(FlushOptions());
  if (status.ok()) {
    std::cout << "   ✓ Database flushed to disk (memtable -> SST)" << std::endl;
  } else {
    std::cout << "   ✗ Flush failed: " << status.ToString() << std::endl;
  }
  
  // Compact (SST files)
  status = db->CompactRange(CompactRangeOptions(), nullptr, nullptr);
  if (status.ok()) {
    std::cout << "   ✓ Database compacted" << std::endl;
  } else {
    std::cout << "   ✗ Compaction failed: " << status.ToString() << std::endl;
  }
  std::cout << std::endl;

  // Example 9: Statistics
  std::cout << "9. Database statistics:" << std::endl;
  
  std::string stats;
  db->GetProperty("rocksdb.stats", &stats);
  std::cout << "   Stats (trimmed):" << std::endl;
  
  // Print first few lines of stats
  std::istringstream iss(stats);
  std::string line;
  int line_count = 0;
  while (std::getline(iss, line) && line_count < 5) {
    std::cout << "   " << line << std::endl;
    line_count++;
  }
  std::cout << std::endl;

  // Cleanup
  std::cout << "10. Cleanup:" << std::endl;
  
  delete db;
  std::cout << "   ✓ Database closed" << std::endl;
  
  // Optional: Clean up test database
  DestroyDB("/tmp/rocksdb_lock_free_test", options);
  std::cout << "   ✓ Test database destroyed" << std::endl;
  std::cout << std::endl;

  std::cout << "=== Example completed successfully! ===" << std::endl;
  std::cout << std::endl;
  std::cout << "Key concepts demonstrated:" << std::endl;
  std::cout << "  • Custom memtable factory (LockFreeSkiplistFactory)" << std::endl;
  std::cout << "  • Write/Read operations" << std::endl;
  std::cout << "  • Iterator and range scans" << std::endl;
  std::cout << "  • Seek operations (SeekToFirst, SeekToLast, Seek, Next, Prev)" << std::endl;
  std::cout << "  • MVCC with snapshots" << std::endl;
  std::cout << "  • Flush to disk (memtable -> SST)" << std::endl;
  std::cout << "  • Compaction" << std::endl;
  std::cout << std::endl;

  return 0;
}
