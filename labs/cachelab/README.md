# `trans.c` Optimization Summary

## 1. Cache Model and Why Blocking Matters

The transpose function in Cache Lab is evaluated on a `1KB` direct-mapped cache with a block size of `32B`.

- `int` is `4B`
- one cache block can hold `8` integers
- the cache is direct-mapped, so conflict misses are very common

This means the optimization target is not reducing arithmetic work, but reducing cache misses.

For matrix transpose:

- reading `A[i][j]` row by row has good spatial locality
- writing `B[j][i]` column by column has poor spatial locality
- `A` and `B` can easily map to the same cache sets and evict each other

Because of this, the main idea in `trans.c` is:

1. use blocking to keep accesses inside a small working set
2. read contiguous elements from `A` whenever possible
3. reduce conflict misses between `A` and `B`
4. handle diagonal blocks carefully

## 2. Overall Strategy in `transpose_submit`

`transpose_submit` dispatches to different implementations based on matrix size:

- `32x32`: use `8x8` blocking with diagonal deferral
- `64x64`: use `8x8` blocking, but split each block into four `4x4` sub-blocks
- `61x67`: use a general blocked transpose with boundary checks

This is necessary because the miss behavior of the three cases is very different.

## 3. `32x32` Optimization

### Core idea

For `32x32`, `8x8` blocking matches the cache block size very well:

- one row of an `8x8` tile contains exactly `8` integers
- this fits exactly into one cache block
- **reading a row from `A` can fully utilize the fetched cache line**

So the algorithm processes the matrix tile by tile:

- outer loops iterate over `8x8` blocks
- inner loops transpose elements inside the current tile

### Diagonal conflict handling

The diagonal blocks need special care.

If we are transposing a block where `i == j`, then accesses such as:

- read `A[k][k]`
- write `B[k][k]`

can map to the same cache set. In a direct-mapped cache, writing to `B[k][k]` may evict the cache line of `A` that is still being used.

To reduce this conflict:

- non-diagonal elements are written immediately
- diagonal elements are stored temporarily in a register
- after finishing the row, the diagonal value is written back to `B`

This technique is usually called **diagonal deferral** or **delayed diagonal write-back**.

Its purpose is not correctness, but miss reduction.

## 4. `64x64` Optimization

### Why the `32x32` strategy is not enough

For `64x64`, a simple `8x8` blocked transpose still suffers many conflict misses.

The problem is that:

- each row is larger
- the mapping pattern in the direct-mapped cache becomes more hostile
- `A` and `B` more frequently collide in the same cache sets

As a result, directly transposing one full `8x8` tile causes heavy self-conflict and `A`/`B` conflict.

### What exactly conflicts in the cache

The important cache facts are:

- cache size = `1KB`
- block size = `32B`
- number of sets = `1024 / 32 = 32`
- one `int` = `4B`
- one cache block holds `8` integers

For a `64x64` matrix:

- one row has `64` integers
- one row occupies `64 * 4 = 256B`
- `256 / 32 = 8` cache blocks per row

If a cache set index is computed as:

```text
set_index = (address / 32) % 32
```

then moving from `A[r][c]` to `A[r + 1][c]` changes the address by one full row:

```text
64 ints * 4B = 256B
```

That means the set index moves by:

```text
256 / 32 = 8
```

So for one `8x8` tile, each row slice `A[i + r][j..j + 7]` occupies exactly one cache block, and the set pattern becomes:

```text
row offset: 0   1   2   3   4   5   6   7
set offset: 0   8  16  24   0   8  16  24
```

This is the core problem:

- row `0` and row `4` of the tile map to the same set
- row `1` and row `5` map to the same set
- row `2` and row `6` map to the same set
- row `3` and row `7` map to the same set

Because the cache is direct-mapped, each set can hold only one line.
**So the top half and bottom half of the same `8x8` tile naturally evict each other.**

The destination matrix `B` has the same row stride, so its cache mapping has the same repeating pattern.
When we directly write `B[j + c][i + r]`, the writes also compete for the same small group of sets.

So a naive `8x8` transpose causes two layers of conflict:

1. **the top 4 rows and bottom 4 rows of the current tile in `A` collide with each other**
2. writes into `B` collide with useful lines from `A`, and also with other lines from `B`

That is why the `64x64` case is much harder than `32x32`.

### Tile decomposition

Each `8x8` block is viewed as four `4x4` sub-blocks:

```text
[ A11 | A12 ]
[ A21 | A22 ]
```

The goal in `B` is:

```text
[ A11^T | A21^T ]
[ A12^T | A22^T ]
```

The point of this decomposition is not that `4x4` is magically optimal by itself.
The point is that it lets the algorithm avoid keeping two conflicting halves of the `8x8` tile active at the same time.

With `4x4` quadrants:

- the top half (`A11`, `A12`) is processed first
- the bottom half (`A21`, `A22`) is processed after that
- one off-diagonal quadrant is temporarily parked in `B`
- then the parked data is swapped into its final place when the other half is loaded

This scheduling reduces the number of simultaneously live cache lines that compete for the same sets.

### Three-stage procedure

#### Stage 1

Read the top 4 rows of the current `8x8` block.

- `A11` is written directly to its final transposed position
- `A12` is temporarily stored in the top-right quadrant of `B`, which is not its final destination

This avoids immediately triggering the worst conflict pattern.
More importantly, it avoids loading the bottom half of the tile before we have finished exploiting the top half.

#### Stage 2

Process the left half of the bottom 4 rows, which corresponds to `A21`.

At the same time:

- move the temporarily stored `A12^T` values into their final destination
- place `A21^T` into its correct top-right position

This stage is the key optimization.

Instead of naively reading and writing everything in final order, the code uses `B` as temporary storage to rearrange the two off-diagonal `4x4` blocks with fewer conflicts.
At this moment:

- the previously parked `A12^T` values are still available in `B`
- the newly loaded `A21` values arrive from the bottom half of `A`
- the algorithm swaps the two off-diagonal quadrants into their final positions

This is exactly where splitting into four `4x4` blocks pays off.
We only bring in the bottom half after the top half has already been consumed, so the two conflicting halves do not keep evicting each other back and forth during the same phase.

#### Stage 3

Finally transpose `A22`, the bottom-right `4x4` block, directly into place.

At this point the other three quadrants are already arranged correctly, so the remaining work is simple.

### Why this works

This design reduces misses because:

- it keeps accesses localized inside one `8x8` region
- it uses registers to hold row values temporarily
- it avoids repeatedly reloading the same lines from `A`
- it avoids the worst direct-mapped conflicts caused by writing `B` too early into the final locations

Another way to summarize it is:

- naive `8x8` transpose keeps conflicting lines alive at the same time
- the `4x4`-quadrant method turns one large conflict-heavy operation into several smaller stages
- each stage uses fewer mutually conflicting lines
- temporary placement in `B` is used to control write timing, not just to hold values

## 5. `61x67` Optimization

This case does not need the same amount of special handling.

The implementation uses a general blocked transpose with a moderate tile size such as `16x16`.

The goals are:

- keep good locality
- avoid complicated special cases
- safely handle the boundary tiles

Because `61x67` is not a square matrix and is not aligned to `8`, boundary checks are necessary when processing the last tiles.

### Why choose `16`

The choice of `16` is mainly a practical compromise rather than a mathematically unique optimum.

For this matrix size, the main goal is still to get locality benefits from blocking, but without introducing the heavy special-case logic used for `64x64`.

`16` works well for several reasons:

1. it still keeps reads from `A` reasonably contiguous
2. it reduces the total number of tiles compared with smaller block sizes
3. it is not so large that the working set becomes difficult to manage in a direct-mapped cache
4. boundary handling stays simple

When scanning one row inside a `16x16` tile:

- `16` integers from `A` occupy `64B`
- that corresponds to only `2` cache blocks

So each row slice still has decent spatial locality.

At the same time, using `16` means the whole matrix is split into relatively few tiles:

- `67` rows become about `5` tile rows
- `61` columns become about `4` tile columns

So the algorithm only needs to process about `20` tiles in total.
This is much less fragmented than using `8x8` blocks.

### Why not use `8`

`8` is also a reasonable block size and would still work correctly.
However, it creates many more tiles:

- roughly `9 x 8 = 72` tiles instead of about `20`

That means:

- more loop overhead
- more tile switching
- less work done per tile

So while `8` often has good locality, it is not automatically better for an irregular size like `61x67`.

### Why not use a much larger value such as `32`

A much larger block reduces the number of tiles, but it also increases the amount of active data involved in one blocked step.

In a direct-mapped cache, this can make conflict behavior worse because:

- more lines from `A` are active in the same phase
- more lines from `B` are written in the same phase
- the chance that useful lines evict each other becomes larger

So `32` is usually too large to be a comfortable default here.

### Why not use an odd value such as `17`

Sometimes people choose an odd block size to further disrupt regular conflict patterns.
That can work, and `17` is not an unreasonable choice.

But in this implementation, `16` is preferred because:

- it is simple
- the loop bounds are clean
- boundary handling is easy to read
- performance is usually already good enough for the `61x67` case

So the choice of `16` should be understood as a stable engineering tradeoff:

- better locality than no blocking
- less fragmentation than `8`
- less pressure than `32`
- much simpler than inventing a more specialized irregular-tile strategy

## 6. Role of Registers

Temporary variables such as `a0` to `a7` in the `64x64` case are important.

They help in two ways:

1. read one row segment from `A` once, then reuse the values without fetching again
2. rearrange data before writing to `B`, which reduces extra memory traffic

Using registers is especially helpful when one tile is being reorganized in multiple stages.

## 7. Summary

The optimization ideas in `trans.c` can be summarized as:

- use blocking to improve locality
- choose block sizes according to cache block size
- handle diagonal blocks specially to reduce `A`/`B` conflict misses
- use a more careful multi-stage layout transformation for `64x64`
- use a simpler general blocked method for irregular sizes like `61x67`

Among these ideas, diagonal conflict handling is especially important in the `32x32` case:

- directly writing diagonal elements can evict useful data from `A`
- delaying the diagonal write helps preserve locality
- this is a classic optimization for transpose on direct-mapped caches
