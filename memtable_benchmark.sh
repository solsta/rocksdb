#!/bin/bash

# Parse command line arguments
THREADS=${1:-1}
ROCKSDB_BIN=$(dirname "$0")/db_bench
NUM=10000
BENCHMARKS="fillseq,readrandom"

if [ ! -f "$ROCKSDB_BIN" ]; then
    echo "Error: db_bench not found at $ROCKSDB_BIN"
    echo "Usage: $0 [threads]"
    exit 1
fi

echo "Comparing different memtable implementations with $THREADS threads..."
echo "====================================================================="

# Test SkipList (default)
echo ""
echo "SkipList (default):"
$ROCKSDB_BIN --benchmarks=$BENCHMARKS --num=$NUM --threads=$THREADS --memtablerep=skip_list --compression_type=none --allow_concurrent_memtable_write=true 2>&1 | grep -E "(fillseq|readrandom)"

# Test Vector
#echo ""
#echo "Vector:"
#$ROCKSDB_BIN --benchmarks=$BENCHMARKS --num=$NUM --threads=$THREADS --memtablerep=vector --compression_type=none --allow_concurrent_memtable_write=true 2>&1 | grep -E "(fillseq|readrandom)"

# Test VectorOrderedRepFactory
echo ""
echo "VectorOrderedRepFactory (custom):"
$ROCKSDB_BIN --benchmarks=$BENCHMARKS --num=$NUM --threads=1 --memtablerep=VectorOrderedRepFactory --compression_type=none --allow_concurrent_memtable_write=false 2>&1 | grep -E "(fillseq|readrandom)"
