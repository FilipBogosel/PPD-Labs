# Lab 3 MPI – Solution Overview

This folder contains the reference solution for the **Big‑Number Addition** laboratory (Lab 3) using MPI. Five variants are provided, each illustrating a different communication strategy:

- **variant0_seq** – Serial baseline (no MPI).
- **variant1_blocking** – Simple blocking send/receive.
- **variant1_1_opt** – Speculative bonus: optimized blocking version.
- **variant2_scatter** – Scatter‑gather using `MPI_Scatter`/`MPI_Gather`.
- **variant3_async** – Non‑blocking asynchronous pipeline.

All binaries are pre‑compiled and placed in `solution/bin/`.

## Test Cases
| Test folder | Description |
|-------------|-------------|
| `tests/test1_16` | Small sanity check (16‑digit numbers). |
| `tests/test2_10k_10k` | Symmetric addition of two 10 000‑digit numbers. |
| `tests/test3_100_100k` | Asymmetric addition (100‑digit + 100 000‑digit). |

## Building & Running
The repository includes a convenience script `run_tests.sh` that:
1. (Re)builds all variants via CMake.
2. Executes the chosen test folder with `mpirun --oversubscribe`.
3. Verifies output using `diff -w`.

```bash
cd Lab3-MPI/solution
./run_tests.sh tests/test1_16      # quick sanity check
./run_tests.sh tests/test2_10k_10k # full benchmark
```

The script automatically selects the appropriate binary for each variant and reports timing information.

## Notes
- **Streaming I/O**: Variants stream digits instead of loading full numbers into memory.
- **Whitespace handling**: Output verification ignores whitespace differences (`diff -w`).
- **macOS**: `--oversubscribe` is required for process counts beyond physical cores.

Feel free to explore the source under `cpp_v2/` for implementation details.

---

## Benchmark Results

Average execution times (ms) across all variants and test cases. Each entry is the average of **10 runs**.

### Test 1 – `test1_16` (16-digit numbers)
| Variant | Processes | Avg Time (ms) |
|---------|-----------|--------------|
| `variant0_seq` (sequential) | 1 | 0.2415 |
| `variant1_blocking` (blocking) | 5 | 0.2571 |
| `variant1_1_opt` (speculative) | 5 | 0.3378 |
| `variant2_scatter` (scatter/gather) | 4 | 0.5993 |
| `variant3_async` (asynchronous) | 5 | 0.2196 |

### Test 2 – `test2_10k_10k` (10 000-digit numbers)
| Variant | Processes | Avg Time (ms) |
|---------|-----------|--------------|
| `variant0_seq` | 1 | 1.1401 |
| `variant1_blocking` | 5 | 1.2875 |
| `variant1_blocking` | 9 | 1.9168 |
| `variant1_blocking` | 17 | 2.7504 |
| `variant1_1_opt` | 5 | 1.7340 |
| `variant1_1_opt` | 9 | 1.7153 |
| `variant1_1_opt` | 17 | 3.3596 |
| `variant2_scatter` | 4 | 1.3380 |
| `variant2_scatter` | 8 | 1.7138 |
| `variant2_scatter` | 16 | 3.0911 |
| `variant3_async` | 5 | 1.3487 |
| `variant3_async` | 9 | 7.1854 |
| `variant3_async` | 17 | 2.4324 |

### Test 3 – `test3_100_100k` (asymmetric: 100 + 100 000 digits)
| Variant | Processes | Avg Time (ms) |
|---------|-----------|--------------|
| `variant0_seq` | 1 | 6.6747 |
| `variant1_blocking` | 5 | 27.0750 |
| `variant1_blocking` | 9 | 16.2639 |
| `variant1_blocking` | 17 | 12.1475 |
| `variant1_1_opt` | 5 | 10.6565 |
| `variant1_1_opt` | 9 | 12.5953 |
| `variant1_1_opt` | 17 | 11.8677 |
| `variant2_scatter` | 4 | 12.2064 |
| `variant2_scatter` | 8 | 20.5083 |
| `variant2_scatter` | 16 | 10.8640 |
| `variant3_async` | 5 | 12.9596 |
| `variant3_async` | 9 | 15.8046 |
| `variant3_async` | 17 | 14.1499 |

*All timings include MPI process startup overhead. Results obtained on macOS with `--oversubscribe` for high process counts.*
