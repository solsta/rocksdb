# Building and Running Lock-Free Skiplist Example

## Quick Build & Run (After Code Changes)

```bash
cd /home/se00598/home/se00598/rocksdb

# 1. Delete all old object files and library
find . -name "*.o" -delete
rm -f librocksdb.a

# 2. Rebuild library with RTTI enabled (uses all cores)
make static_lib USE_RTTI=1 -j16

# 3. Build example with RTTI
cd examples
rm -f lock_free_skiplist_example
make lock_free_skiplist_example USE_RTTI=1

# 4. Run the example
./lock_free_skiplist_example
```

## Why These Steps?

- **Delete .o files**: Old object files compiled without RTTI cause linking errors. Must start clean.
- **USE_RTTI=1**: Required because our code uses virtual inheritance. Without it, `typeinfo` symbols are missing.
- **-j16**: Uses 16 cores for fast parallel compilation (adjust based on your CPU cores).

## Files to Edit

When modifying the stub methods, edit:
- `/home/se00598/home/se00598/rocksdb/memtable/lock_free_skiplist_memtable.cc`

Build config (already updated):
- `/home/se00598/home/se00598/rocksdb/src.mk` - contains `memtable/lock_free_skiplist_memtable.cc` in LIB_SOURCES

## Current Implementation Status

- `ApproximateMemoryUsage()` - ✓ Returns 0 (implemented)
- All other methods - Stubbed with printf + assert(false)

## Rebuild Time Estimate

- First build (all .o files): ~2-3 minutes with 16 cores
- Incremental (just changed file): ~10-15 seconds

## Common Issues

**Linking error: "undefined reference to typeinfo"**
→ Library was compiled without RTTI. Delete .o files and rebuild with `USE_RTTI=1`

**"Nothing to be done for static_lib"**
→ Library already built. This is OK. Continue to build example.

**Example won't run**
→ Check that example compiled without errors. If linker errors, rebuild library.
