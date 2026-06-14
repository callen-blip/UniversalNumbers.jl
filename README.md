# UniversalNumbers.jl

Julia bindings for the [Stillwater Universal](https://github.com/stillwater-sc/universal)
C++ number-systems library: **posits**, **classic floats (cfloat)**, **logarithmic number
systems (LNS)**, **takum**, **IBM hex float (hfloat)**, and **decimal float (dfloat)**
as first-class Julia numbers.

```julia
julia> using UniversalNumbers

julia> a = Posit{16,1}(1.5)
Posit{16,1}(1.5)

julia> a + 2.5            # standard numbers promote automatically
Posit{16,1}(4.0)

julia> sqrt(Posit{32,2}(2.0))
Posit{32,2}(1.4142135623842478)
```

## Supported types

| Julia type | Universal C++ type | Storage |
|---|---|---|
| `Posit{8,0}` | `posit<8, 0>` | 8 bits |
| `Posit{16,1}` | `posit<16, 1>` | 16 bits |
| `Posit{32,2}` | `posit<32, 2>` | 32 bits |
| `Posit{19,3}` | `posit<19, 3>` | 32 bits (19 used) |
| `CFloat{8,2}` | `cfloat<8, 2>` (quarter) | 8 bits |
| `CFloat{16,5}` | `cfloat<16, 5>` (half) | 16 bits |
| `LNS{16,5}` | `lns<16, 5>` | 16 bits |
| `LNS{32,16}` | `lns<32, 16>` | 32 bits |
| `Takum{16}` | `takum<16, 3>` | 16 bits |
| `Takum{32}` | `takum<32, 3>` | 32 bits |
| `HFloat{6,7}` | `hfloat<6, 7>` (hfp32) | 32 bits |
| `HFloat{14,7}` | `hfloat<14, 7>` (hfp64) | 64 bits |
| `DFloat{7,6}` | `dfloat<7, 6, BID>` (decimal32) | 32 bits |
| `DFloat{16,8}` | `dfloat<16, 8, BID>` (decimal64) | 64 bits |

Universal's types are C++ compile-time templates, so each instantiation must be
compiled into the bridge library. Both sides are generated from a **type
registry** — to add a type (say `Posit{24,1}`), add one line to the
`TYPE_REGISTRY` in `src/libuniversal_wrapper.cpp` and one matching line in
`src/UniversalNumbers.jl`, then rebuild. Using an unregistered type gives an
informative error rather than wrong answers:

```julia
julia> Posit{24,1}(1.0)
ERROR: Posit{24, 1} is not instantiated in the registry. Please add it to
UniversalNumbers.TYPE_REGISTRY and rebuild.
```

## Adding a new type

Universal's C++ templates support any valid `(nbits, es, bt)` combination out of the box —
no changes to the Universal library are needed. Adding a type is a two-line change plus a
rebuild.

**Step 1 — choose the storage word type**

| `nbits` | `bt` (C++) | `StorageT` (Julia) |
|---|---|---|
| 1 – 8 | `uint8_t` | `UInt8` |
| 9 – 16 | `uint16_t` | `UInt16` |
| 17 – 32 | `uint32_t` | `UInt32` |
| 33 – 64 | `uint64_t` | `UInt64` |

**Step 2 — register on the C++ side** (`src/libuniversal_wrapper.cpp`)

Add one line to the appropriate registry macro. For a type with full math support:

```cpp
#define TYPE_REGISTRY_FULL \
    ...
    X(posit24_1, uint32_t, sw::universal::posit<24, 1, uint32_t>) \   // ← new
    X(takum8,    uint8_t,  sw::universal::takum<8, 3, uint8_t>)       // ← new
```

**Step 3 — register on the Julia side** (`src/UniversalNumbers.jl`)

For Posit / CFloat / LNS, add one tuple to `TYPE_REGISTRY`:

```julia
const TYPE_REGISTRY = [
    ...
    (:Posit, 24, 1, "posit24_1", UInt32),   # ← new
]
```

For Takum, add one tuple to `TAKUM_REGISTRY`:

```julia
const TAKUM_REGISTRY = [
    (8,  "takum8",  UInt8),    # ← new
    (16, "takum16", UInt16),
    (32, "takum32", UInt32),
]
```

**Step 4 — rebuild and use**

```bash
cmake --build build
julia --project=. -e 'include("src/UniversalNumbers.jl"); using .UniversalNumbers; println(Posit{24,1}(1.5))'
```

That's it. The X-macro on the C++ side stamps out all ~20 bridge functions automatically,
and the `@eval` loop on the Julia side generates the full method set. Requesting an
unregistered type gives a clear error pointing back to this workflow.

## Building

Requires CMake ≥ 3.22, a C++20 compiler, and Julia ≥ 1.9. The Universal
headers are vendored in `deps/`, so the repo is self-contained.

```bash
cmake -S . -B build && cmake --build build   # builds build/libuniversal.so
julia --project=. test/runtests.jl           # run the test suite
```

## AbstractFloat behavior

All types subtype `AbstractFloat` (via `UniversalNumber`) and support:

- **Construction from any `Real`**: `Posit{16,1}(1.5)`, `CFloat{8,2}(2)`, `LNS{16,5}(pi)`
- **Conversion back**: `Float64(x)`, `Float32(x)`
- **Arithmetic**: `+`, `-`, `*`, `/`, unary `-`, `abs`
- **Math functions**: `sqrt`, `sin`, `cos`, `exp`, `log` — computed by
  Universal's C++ math library in the native number system, with rounding
  determined by the type's template parameters
- **Comparisons**: `==`, `<`, `<=` use Universal's native operators on the
  encoding — exact, with no `Float32` round-trip (adjacent `Posit{32,2}`
  values near 1.0 compare correctly)
- **Promotion**: mixed expressions like `p + 2.5` or `2 * p` convert the
  standard number into the universal type, so the computation happens in
  posit/cfloat/LNS arithmetic
- **Constants and queries**: `zero`, `one`, `eps`, `floatmin`, `floatmax`,
  `iszero`, `isnan`, `isinf`
- **Adjacent values**: `nextfloat(x)` / `prevfloat(x)` return the next or
  previous representable value in the encoding order, delegating to Universal's
  native `++`/`--` operators
- **Random generation**: `rand(Posit{16,1})`

The raw encoding is always available as `x.data` (e.g. for bit-level work).

## NaR: how posits handle "not a real"

Posits have no `Inf` and no signed zeros; instead they reserve a single
encoding `100...0` called **NaR** (Not-a-Real). Its semantics differ from
IEEE NaN — this package follows the posit standard:

```julia
julia> n = Posit{16,1}(NaN)     # float NaN converts to NaR
Posit{16,1}(NaN)

julia> isnan(n)
true

julia> n == n                   # NaR == NaR is TRUE (IEEE NaN: false)
true

julia> n < Posit{16,1}(-1e6)    # NaR sorts below every real value
true

julia> isnan(n + Posit{16,1}(1.0))   # NaR is absorbing in arithmetic
true

julia> isnan(sqrt(Posit{16,1}(-1.0)))  # invalid operations produce NaR
true
```

`-NaR` is NaR (it is its own 2's complement), and `Posit{16,1}(NaN).data ==
0x8000` if you need to test for the bit pattern directly.

## Posit number-system properties

Facts below are verified against this package (see the symmetry checks in the
test suite); all follow from the posit standard, with useed = 2^(2^es).

- **Reciprocal symmetry about 1**: `floatmin(T) * floatmax(T) == 1.0` exactly.
  maxpos = useed^(nbits−2) and minpos = useed^−(nbits−2), so the dynamic range
  is perfectly balanced on a log scale — every magnitude extreme has an exact
  reciprocal counterpart. IEEE floats are lopsided here: for `Float16`,
  `floatmin * floatmax ≈ 4`, not 1.
- **Sign symmetry about 0**: negation is the 2's complement of the encoding,
  so every posit value (including maxpos) has an exact negation.
- **One zero, one NaR**: no `-0`, no `Inf`. The encoding `000...0` is the
  unique zero and `100...0` is the unique NaR (see the NaR section above).
- **Tapered precision**: accuracy is highest near ±1 and tapers toward the
  extremes — the regime bits spend more of the word on precision in the
  center of the range.

Verified ranges for the registered types:

| Type | useed | maxpos | minpos | minpos·maxpos |
|---|---|---|---|---|
| `Posit{8,0}` | 2 | 2^6 = 64 | 2^−6 ≈ 1.56e-2 | 1.0 |
| `Posit{16,1}` | 4 | 4^14 = 2^28 ≈ 2.68e8 | 2^−28 ≈ 3.73e-9 | 1.0 |
| `Posit{32,2}` | 16 | 16^30 = 2^120 ≈ 1.33e36 | 2^−120 ≈ 7.52e-37 | 1.0 |
| `Posit{19,3}` | 256 | 256^17 = 2^136 ≈ 8.71e40 | 2^−136 ≈ 1.15e-41 | 1.0 |

## Linear algebra

The types compose with Julia's generic `LinearAlgebra` routines — matrix
products, dot products, etc. run entirely in the chosen number system:

```julia
using UniversalNumbers, LinearAlgebra

A = [Posit{16,1}(1.0) Posit{16,1}(2.0);
     Posit{16,1}(3.0) Posit{16,1}(4.0)]
v = [Posit{16,1}(1.0), Posit{16,1}(1.0)]

A * v          # 2-element Vector: Posit{16,1}(3.0), Posit{16,1}(7.0)
dot(v, v)      # Posit{16,1}(2.0)
```

This makes it easy to study how low-precision number systems behave in real
numerical kernels — e.g. compare a `Posit{16,1}` matrix factorization against
`Float16`/`Float32` baselines.

Note on LNS: logarithmic numbers quantize in the log domain, so products and
quotients are cheap and accurate, while values like `1.5` are stored as the
nearest power-of-two fraction (`LNS{16,5}(1.5) ≈ 1.509`). This is inherent to
the number system, not a bug.

## Bit-level inspection

`printbits(x)` prints the raw encoding of any registered value to the terminal
with ANSI colors identifying each field, followed by a one-line legend.

```julia
julia> printbits(Posit{16,1}(1.5))
Posit{16,1}(1.5)  0|10|0|1000000000000
S sign  R regime  E exponent  f fraction

julia> printbits(Takum{16}(1.5))
Takum{16}(1.5)  0|1|000|  |10000000000
S sign  D direction  R regime  C characteristic  M mantissa
```

The color coding is taken directly from Universal's own `color_print` functions.
Each number-system family uses a consistent scheme:

| Color | Posit | CFloat | LNS | Takum |
|---|---|---|---|---|
| Red | sign | sign | sign | sign |
| Yellow | regime | — | — | regime |
| Cyan | exponent | exponent | integer part | characteristic |
| Magenta | fraction | fraction | fraction | mantissa |
| Green | — | — | — | direction bit (D) |

The `printbits` function is most useful when working interactively to understand
how a value is encoded, how rounding affects adjacent representations, or how
the regime field expands and contracts across the dynamic range.

## Project layout

```
src/libuniversal_wrapper.cpp   C ABI bridge generated from the C++ type registry
src/UniversalNumbers.jl        Julia module generated from the matching registry
deps/universal/include/        Vendored Universal headers (self-contained)
build/libuniversal.so          Built bridge library
test/runtests.jl               Test suite (arithmetic, NaR, comparisons, linalg)
docs/parametric-types.md       Design note for the parametric type system
ROADMAP.md                     Phase 1 (local) / Phase 2 (JLL + registry) plan
```

## License

MIT — see `LICENSE`.
