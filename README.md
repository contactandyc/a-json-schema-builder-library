# a-json-schema-library

High-performance C bitset implementations with two flavors:

1. **Fixed-size `abitset_t`** – size chosen at init, supports rich bitwise ops. Memory allocated from an **a-memory-library** `aml_pool_t`.
2. **Auto-expanding `abitset_expandable_t`** – grows automatically on `set` / `unset`; simpler API (no explicit bitwise combine helpers) and independent of `aml_pool_t`.

Both provide fast bit access, counting, serialization to/from contiguous 64‑bit word arrays, and predictable O(1) operations (aside from occasional resize in the expandable variant).

---

## Features

| Capability                           | `abitset_t`                          | `abitset_expandable_t`            |
| ------------------------------------ | ------------------------------------ | --------------------------------- |
| Fixed size                           | ✅                                    | ❌ (auto-grows)                    |
| Uses `aml_pool_t`                    | ✅                                    | ❌                                 |
| Bit get/set/unset                    | ✅                                    | ✅                                 |
| Boolean assign (`abitset_boolean`)   | ✅                                    | ❌                                 |
| Count bits set                       | ✅ (`abitset_count`)                  | ✅ (`abitset_expandable_count`)    |
| Count & zero                         | ✅ (`abitset_count_and_zero`)         | ❌                                 |
| First set bit                        | ✅ (`abitset_first_enabled`)          | ❌ (can be added externally)       |
| Bulk set all true/false              | ✅ (`abitset_true` / `abitset_false`) | ❌ (loop externally)               |
| Bitwise ops AND / OR / AND-NOT / NOT | ✅                                    | ❌ (operate manually after `repr`) |
| Serialization (`*_repr`)             | ✅                                    | ✅                                 |
| Load from representation             | ✅ (`abitset_load`)                   | ✅ (`abitset_expandable_load`)     |

---

## API Summary

### Fixed-size (`abitset.h`)

Initialization & lifecycle:

```c
abitset_t *bs = abitset_init(pool, size_bits);
abitset_t *copy = abitset_copy(pool, bs);
uint32_t n = abitset_size(bs);
uint64_t *words = abitset_repr(bs);        // length = ceil(n / 64.0)
abitset_t *loaded = abitset_load(pool, words, n, /*make_copy=*/true);
```

Bit ops:

```c
abitset_set(bs, id);
abitset_unset(bs, id);
abitset_boolean(bs, id, true_or_false);
bool on = abitset_enabled(bs, id);
uint32_t c = abitset_count(bs);
uint32_t c_and_reset = abitset_count_and_zero(bs);
int32_t first = abitset_first_enabled(bs);
abitset_true(bs);  // all 1s
abitset_false(bs); // all 0s
abitset_not(bs);
abitset_and(dest, src);
abitset_or(dest, src);
abitset_and_not(dest, mask);
```

### Expandable (`abitset_expandable.h`)

```c
abitset_expandable_t *ebs = abitset_expandable_init();
uint32_t n = abitset_expandable_size(ebs);      // current capacity in bits
abitset_expandable_set(ebs, id);                // grows if id beyond current size
abitset_expandable_unset(ebs, id);              // also grows to cover id, then clears
bool on = abitset_expandable_enabled(ebs, id);
uint32_t c = abitset_expandable_count(ebs);
uint64_t *words = abitset_expandable_repr(ebs);
abitset_expandable_t *loaded = abitset_expandable_load(words, n);
abitset_expandable_destroy(ebs);
```

---

## Serialization Format

Both `*_repr` functions return a newly allocated array of 64‑bit words (least significant bits correspond to lower bit indices). For `abitset_t` the array length is fixed; for `abitset_expandable_t` it reflects the current size. Use the allocator pairings defined by **a-memory-library** (e.g., `aml_free`) for deallocation where noted in headers.

---

## Choosing an Implementation

* Use **`abitset_t`** when you know the maximum size ahead of time, want pooled allocations, and need bulk / bitwise operations.
* Use **`abitset_expandable_t`** when maximum size is unknown or unbounded and you only need fundamental per-bit operations plus counting & serialization.

You can mix both: prototype with expandable, then switch to fixed-size for tighter control.

---

## Complexity & Performance Notes

* Single bit set/test/unset: O(1).
* Counting: O(n/wordSize). `abitset_count_and_zero` fuses count+clear, reducing passes.
* Expand growth: amortized; infrequent reallocation when a higher index is touched.
* Bitwise ops provided only on fixed-size variant for speed and simplicity.

---

## Example (Fixed-size)

```c
#include "a-bitset-library/abitset.h"
#include "a-memory-library/aml_pool.h"

aml_pool_t *pool = aml_pool_create();
abitset_t *flags = abitset_init(pool, 1000);
abitset_true(flags);              // mark all
abitset_unset(flags, 42);         // clear one
if (abitset_enabled(flags, 42)) { /* ... */ }
abitset_not(flags);               // flip all bits
uint32_t active = abitset_count(flags);
uint64_t *data = abitset_repr(flags); // serialize
// ... write data, then free with aml_free(data) per allocator contract
```

## Example (Expandable)

```c
#include "a-bitset-library/abitset_expandable.h"

abitset_expandable_t *dyn = abitset_expandable_init();
abitset_expandable_set(dyn, 10);
abitset_expandable_set(dyn, 500000); // auto-resizes
uint32_t ones = abitset_expandable_count(dyn);
uint64_t *repr = abitset_expandable_repr(dyn);
// persist repr then free with aml_free(repr)
abitset_expandable_destroy(dyn);
```

---

## Error Handling

Functions assume valid pointers; out-of-range indices on the expandable version trigger growth, while on the fixed-size version they are undefined behavior (caller must guard). Always ensure `id < abitset_size()` for `abitset_t`.

---

## License & Attribution

Apache-2.0. SPDX headers embedded:

```
SPDX-FileCopyrightText: 2023-2025 Andy Curtis
SPDX-FileCopyrightText: 2024-2025 Knode.ai
SPDX-License-Identifier: Apache-2.0
```

---

## Roadmap Ideas

* Optional iterator for set bits (pop iteration).
* Bitwise ops for expandable version.
* Inline helpers/macros for hot paths.
* SIMD-accelerated population count when available.

Contributions & issues welcome.
