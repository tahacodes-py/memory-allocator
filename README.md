# Memory Allocator in C++

A custom heap memory allocator built from scratch in C++ — no libraries, no `malloc`, just raw pointer arithmetic over a 1MB static memory pool.

---

## Architecture

The allocator manages a flat 1MB byte array as a simulated heap. Every allocation is preceded by a `BlockHeader` struct that lives *inside* the heap itself (an intrusive linked list), storing the block's size, status, and a pointer to the next block.

```
heap[]
┌──────────────┬─────────────────┬──────────────┬─────────────────┬──────────────┬─────────┐
│  BlockHeader │    payload      │  BlockHeader │    payload      │  BlockHeader │  (free) │
│  size=100    │   100 bytes     │  size=64     │   64 bytes      │  size=...    │         │
│  is_free=F   │   (user data)   │  is_free=F   │   (user data)   │  is_free=T   │         │
│  next=──────►│                 │  next=──────►│                 │  next=null   │         │
└──────────────┴─────────────────┴──────────────┴─────────────────┴──────────────┴─────────┘
```

When a block is freed, the allocator checks if the next block is also free and merges them (forward coalescing), reducing fragmentation.

---

## Features

| Function | Description |
|---|---|
| `my_malloc(size)` | First-fit search with block splitting |
| `my_free(ptr)` | Frees block and coalesces with adjacent free block |
| `my_realloc(ptr, size)` | Resizes allocation, safely copying existing data |
| `print_heap()` | Prints every block: address, size, and free/used status |
| `print_stats()` | Summary of total used/free bytes and block counts |
| `benchmark()` | Compares performance against standard `malloc` over 10,000 allocations |

---

## Build & Run

```bash
g++ allocator.cpp -o allocator
./allocator
```

To visualize the benchmark results (requires Python + matplotlib):

```bash
python3 benchmark.py
```

---

## Benchmark

10,000 allocations and frees of 64 bytes each, single-threaded.

| Allocator | Behaviour |
|---|---|
| `my_malloc` | Faster for small same-size allocations — avoids syscall overhead entirely |
| `std::malloc` | Slower here, but handles thread safety, heap growth, and varied sizes |

My allocator wins in this narrow benchmark because it operates on a pre-allocated static array with zero OS calls. Standard `malloc` wins for any real production workload. The point of the comparison is to understand *where* the overhead actually comes from.

---

## Limitations

These are known tradeoffs, not oversights:

- **Fixed 1MB heap** — no `sbrk`/`mmap` growth; returns `nullptr` when full
- **O(n) first-fit search** — no size-class bucketing or segregated free lists
- **Forward-only coalescing** — free blocks only merge with the *next* block, not the previous one
- **Not thread-safe** — global state with no synchronization

A production allocator (jemalloc, tcmalloc) solves all four. Building this made it clear exactly *why* those solutions exist.

---

## What I Learned

- How heap memory is laid out at the byte level
- How `malloc` and `free` actually work under the hood
- Pointer arithmetic and memory casting in C++
- What internal vs external fragmentation means in practice
- How coalescing works and why forward-only is insufficient
- How to benchmark allocators and interpret the results honestly
