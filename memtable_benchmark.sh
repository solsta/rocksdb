#!/bin/bash

ROCKSDB_BIN=/home/se00598/home/se00598/rocksdb/db_bench
NUM=10000
BENCHMARKS="fillseq,readrandom"

echo "Comparing different memtable implementations..."
echo "=============================================="

# Test SkipList (default)
echo ""
echo "SkipList (default):"
$ROCKSDB_BIN --benchmarks=$BENCHMARKS --num=$NUM --memtablerep=skip_list --compression_type=none --allow_concurrent_memtable_write=true 2>&1 | grep -E "(fillseq|readrandom)"

# Test Vector
echo ""
echo "Vector:"
$ROCKSDB_BIN --benchmarks=$BENCHMARKS --num=$NUM --memtablerep=vector --compression_type=none --allow_concurrent_memtable_write=true 2>&1 | grep -E "(fillseq|readrandom)"

# Test VectorOrderedRepFactory
echo ""
echo "VectorOrderedRepFactory (custom):"
$ROCKSDB_BIN --benchmarks=$BENCHMARKS --num=$NUM --memtablerep=VectorOrderedRepFactory --compression_type=none --allow_concurrent_memtable_write=false 2>&1 | grep -E "(fillseq|readrandom)"
