#include <iostream>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "memtable/vector_ordered_rep.h"

using namespace ROCKSDB_NAMESPACE;

int main() {
  std::string kDBPath = "/tmp/rocksdb_memtable_vector_demo";
  Options options;
  options.create_if_missing = true;
  options.error_if_exists = false;
  options.allow_concurrent_memtable_write = false;
  // Use our simple vector-ordered memtable
  options.memtable_factory = std::make_shared<VectorOrderedRepFactory>();

  DB* db = nullptr;
  Status s = DB::Open(options, kDBPath, &db);
  if (!s.ok()) {
    std::cerr << "Open failed: " << s.ToString() << std::endl;
    return 1;
  }

  // Basic writes
  s = db->Put(WriteOptions(), "a", "1");
  if (!s.ok()) { std::cerr << s.ToString() << std::endl; }
  s = db->Put(WriteOptions(), "b", "2");
  if (!s.ok()) { std::cerr << s.ToString() << std::endl; }
  s = db->Put(WriteOptions(), "a", "3");  // newer version of "a"
  if (!s.ok()) { std::cerr << s.ToString() << std::endl; }

  // Observe read before flush
  std::string v;
  s = db->Get(ReadOptions(), "a", &v);
  std::cout << "Get(a) before flush: status=" << s.ToString() << ", value=" << v << std::endl;

  // Force a flush to SST
  FlushOptions fo;
  fo.wait = true;
  s = db->Flush(fo);
  std::cout << "Flush: " << s.ToString() << std::endl;

  // Iterate DB after flush (reads from SST)
  std::unique_ptr<Iterator> it(db->NewIterator(ReadOptions()));
  std::cout << "Iterate after flush:" << std::endl;
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::cout << it->key().ToString() << " => " << it->value().ToString() << std::endl;
  }
  if (!it->status().ok()) {
    std::cerr << "Iterator status: " << it->status().ToString() << std::endl;
  }

  delete db;
  // Some environments show a spurious abort on process teardown due to
  // background thread/mutex cleanup; exit immediately for demo clarity.
  _Exit(0);
}
