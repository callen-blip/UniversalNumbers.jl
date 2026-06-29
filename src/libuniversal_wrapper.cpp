#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstring>
#include <limits>
#include <type_traits>   // std::is_constant_evaluated -- libc++ (macOS) needs it explicit
#include <utility>

// configure the universal library
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/number/dfloat/dfloat.hpp>

#include <universal/number/posit/manipulators.hpp>
#include <universal/number/cfloat/manipulators.hpp>
#include <universal/number/lns/manipulators.hpp>
#include <universal/number/takum/manipulators.hpp>
#include <universal/number/bfloat16/manipulators.hpp>
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/hfloat/manipulators.hpp>
#include <universal/number/dfloat/manipulators.hpp>

// Bridge Macro: Stamps out C-compatible functions for scalar operations.
// IT  = interface type (the C struct field type; sized to match sizeof(CppType)).
//       For types where sizeof(CppType) > sizeof(block_type), IT is larger
//       (e.g. hfp64 uses IT=uint64_t even though its block_type is uint32_t).
// ... = the C++ type (CppType), passed as variadic to survive commas in templates.
#define DEFINE_BRIDGE_SCALAR(NAME, IT, ...) \
    typedef struct { IT data; } NAME##_t; \
    \
    extern "C" NAME##_t NAME##_from_double(double d) { \
        __VA_ARGS__ v(d); \
        NAME##_t out; \
        std::memcpy(&out.data, &v, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" double NAME##_to_double(NAME##_t v) { \
        __VA_ARGS__ val; \
        std::memcpy(&val, &v.data, sizeof(IT)); \
        return (double)val; \
    } \
    \
    extern "C" NAME##_t NAME##_add(NAME##_t a, NAME##_t b) { \
        __VA_ARGS__ va, vb, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        std::memcpy(&vb, &b.data, sizeof(IT)); \
        vr = va + vb; \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_sub(NAME##_t a, NAME##_t b) { \
        __VA_ARGS__ va, vb, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        std::memcpy(&vb, &b.data, sizeof(IT)); \
        vr = va - vb; \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_mul(NAME##_t a, NAME##_t b) { \
        __VA_ARGS__ va, vb, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        std::memcpy(&vb, &b.data, sizeof(IT)); \
        vr = va * vb; \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_div(NAME##_t a, NAME##_t b) { \
        __VA_ARGS__ va, vb, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        std::memcpy(&vb, &b.data, sizeof(IT)); \
        vr = va / vb; \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" bool NAME##_eq(NAME##_t a, NAME##_t b) { \
        __VA_ARGS__ va, vb; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        std::memcpy(&vb, &b.data, sizeof(IT)); \
        return (va == vb); \
    } \
    \
    extern "C" bool NAME##_lt(NAME##_t a, NAME##_t b) { \
        __VA_ARGS__ va, vb; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        std::memcpy(&vb, &b.data, sizeof(IT)); \
        return (va < vb); \
    } \
    \
    extern "C" bool NAME##_le(NAME##_t a, NAME##_t b) { \
        __VA_ARGS__ va, vb; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        std::memcpy(&vb, &b.data, sizeof(IT)); \
        return (va <= vb); \
    } \
    \
    extern "C" NAME##_t NAME##_sqrt(NAME##_t a) { \
        __VA_ARGS__ va, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        vr = sw::universal::sqrt(va); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_abs(NAME##_t a) { \
        __VA_ARGS__ va, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        vr = sw::universal::abs(va); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_eps() { \
        __VA_ARGS__ vr = std::numeric_limits<__VA_ARGS__>::epsilon(); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_min() { \
        __VA_ARGS__ vr = std::numeric_limits<__VA_ARGS__>::min(); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_max() { \
        __VA_ARGS__ vr = std::numeric_limits<__VA_ARGS__>::max(); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" bool NAME##_isnan(NAME##_t a) { \
        __VA_ARGS__ va; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        return sw::universal::isnan(va); \
    } \
    \
    extern "C" bool NAME##_isinf(NAME##_t a) { \
        __VA_ARGS__ va; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        return sw::universal::isinf(va); \
    } \
    \
    extern "C" NAME##_t NAME##_next(NAME##_t a) { \
        __VA_ARGS__ va; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        ++va; \
        NAME##_t out; \
        std::memcpy(&out.data, &va, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_prev(NAME##_t a) { \
        __VA_ARGS__ va; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        --va; \
        NAME##_t out; \
        std::memcpy(&out.data, &va, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" void NAME##_printbits(IT raw) { \
        __VA_ARGS__ va; \
        std::memcpy(&va, &raw, sizeof(IT)); \
        std::cout << sw::universal::color_print(va) << std::flush; \
    }

// Bridge Macro: Stamps out transcendental functions
#define DEFINE_BRIDGE_MATH(NAME, IT, ...) \
    extern "C" NAME##_t NAME##_sin(NAME##_t a) { \
        __VA_ARGS__ va, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        vr = sw::universal::sin(va); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_cos(NAME##_t a) { \
        __VA_ARGS__ va, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        vr = sw::universal::cos(va); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_exp(NAME##_t a) { \
        __VA_ARGS__ va, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        vr = sw::universal::exp(va); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" NAME##_t NAME##_log(NAME##_t a) { \
        __VA_ARGS__ va, vr; \
        std::memcpy(&va, &a.data, sizeof(IT)); \
        vr = sw::universal::log(va); \
        NAME##_t out; \
        std::memcpy(&out.data, &vr, sizeof(IT)); \
        return out; \
    }

// --- The Type Registry ---
// X(name, IT, CppType)
//   IT      = interface type -- the C struct field type, sized to match sizeof(CppType).
//             For single-word types IT == block_type.  For multi-word types (e.g. hfp64
//             uses two uint32_t blocks for 64 bits) IT is uint64_t so the memcpy is correct.
//   CppType = the Universal C++ template instantiation.
#define TYPE_REGISTRY_FULL \
    X(posit8_0,   uint8_t,  sw::universal::posit<8, 0, uint8_t>) \
    X(posit8_1,   uint8_t,  sw::universal::posit<8, 1, uint8_t>) \
    X(posit8_2,   uint8_t,  sw::universal::posit<8, 2, uint8_t>) \
    X(posit12_1,  uint16_t, sw::universal::posit<12, 1, uint16_t>) \
    X(posit16_1,  uint16_t, sw::universal::posit<16, 1, uint16_t>) \
    X(posit16_2,  uint16_t, sw::universal::posit<16, 2, uint16_t>) \
    X(posit32_2,  uint32_t, sw::universal::posit<32, 2, uint32_t>) \
    X(posit19_3,  uint32_t, sw::universal::posit<19, 3, uint32_t>) \
    X(posit19_2,  uint32_t, sw::universal::posit<19, 2, uint32_t>) \
    X(posit64_2,  uint64_t, sw::universal::posit<64, 2, uint64_t>) \
    X(posit64_3,  uint64_t, sw::universal::posit<64, 3, uint64_t>) \
    X(cfloat8_2,  uint8_t,  sw::universal::cfloat<8, 2, uint8_t, true, false, false>) \
    X(cfloat8_3,  uint8_t,  sw::universal::cfloat<8, 3, uint8_t, true, false, false>) \
    X(cfloat8_4,  uint8_t,  sw::universal::cfloat<8, 4, uint8_t, true, false, false>) \
    X(cfloat8_5,  uint8_t,  sw::universal::cfloat<8, 5, uint8_t, true, false, false>) \
    X(cfloat24_5, uint32_t, sw::universal::cfloat<24, 5, uint32_t, true, false, false>) \
    X(lns16_5,    uint16_t, sw::universal::lns<16, 5, uint16_t>) \
    X(lns32_16,   uint32_t, sw::universal::lns<32, 16, uint32_t>) \
    X(bfloat16,   uint16_t, sw::universal::bfloat16) \
    X(takum8,     uint8_t,  sw::universal::takum<8,  3, uint8_t>) \
    X(takum16,    uint16_t, sw::universal::takum<16, 3, uint16_t>) \
    X(takum32,    uint32_t, sw::universal::takum<32, 3, uint32_t>) \
    X(takum64,    uint64_t, sw::universal::takum<64, 3, uint64_t>) \
    X(dd, __uint128_t, sw::universal::dd) \
    X(fixed8_4,   uint8_t,  sw::universal::fixpnt<8,  4,  sw::universal::Modulo, uint8_t>) \
    X(fixed16_8,  uint16_t, sw::universal::fixpnt<16, 8,  sw::universal::Modulo, uint16_t>) \
    X(fixed32_16, uint32_t, sw::universal::fixpnt<32, 16, sw::universal::Modulo, uint32_t>) \
    X(hfp32,      uint32_t, sw::universal::hfloat<6, 7, uint32_t>) \
    X(hfp64,      uint64_t, sw::universal::hfloat<14, 7, uint32_t>) \
    X(decimal32,  uint32_t, sw::universal::dfloat<7, 6, sw::universal::DecimalEncoding::BID, uint32_t>) \
    X(decimal64,  uint64_t, sw::universal::dfloat<16, 8, sw::universal::DecimalEncoding::BID, uint32_t>)

// Expand Full Registry
#define X(NAME, IT, ...) \
    DEFINE_BRIDGE_SCALAR(NAME, IT, __VA_ARGS__) \
    DEFINE_BRIDGE_MATH(NAME, IT, __VA_ARGS__)
TYPE_REGISTRY_FULL
#undef X

// ============================================================================
// Quire bridge (posit-only): exact fused-dot-product accumulator.
//
// The quire is Universal's exact accumulator -- products of posits are summed
// with NO intermediate rounding, and a single rounding happens on conversion
// back to a posit (quire_resolve).  It is a posit construct in Universal and is
// purely additive here: nothing about the scalar bridge above changes.
//
// Unlike the scalar types (which fit in a {IT data} struct and pass by value),
// the quire is a large stateful object, so it is exposed as an opaque heap
// handle (void*).  The posit/blocktriple v2 API is used: quire<posit<N,ES,IT>>,
// q += quire_mul(a,b), and quire_resolve(q)  (see posit/fdp.hpp).
//
// Args/returns use the raw IT (matching the scalar bridge's ABI convention:
// the single-field NAME_t struct is ABI-identical to IT).
#define DEFINE_BRIDGE_QUIRE(NAME, IT, N, ES) \
    using NAME##_posit_t = sw::universal::posit<N, ES, IT>; \
    using NAME##_quire_t = sw::universal::quire<NAME##_posit_t>; \
    \
    extern "C" void* NAME##_quire_new()        { return new NAME##_quire_t(); } \
    extern "C" void  NAME##_quire_free(void* q) { delete static_cast<NAME##_quire_t*>(q); } \
    extern "C" void  NAME##_quire_clear(void* q){ static_cast<NAME##_quire_t*>(q)->clear(); } \
    extern "C" int   NAME##_quire_bits()        { return sw::universal::quire_size<N, ES>(); } \
    \
    extern "C" void NAME##_quire_fma(void* q, IT a, IT b) { \
        NAME##_posit_t pa, pb; \
        std::memcpy(&pa, &a, sizeof(IT)); \
        std::memcpy(&pb, &b, sizeof(IT)); \
        *static_cast<NAME##_quire_t*>(q) += sw::universal::quire_mul(pa, pb); \
    } \
    \
    extern "C" IT NAME##_quire_round(void* q) { \
        NAME##_posit_t p = sw::universal::quire_resolve(*static_cast<NAME##_quire_t*>(q)); \
        IT out; \
        std::memcpy(&out, &p, sizeof(IT)); \
        return out; \
    } \
    \
    extern "C" IT NAME##_fdp(const IT* a, const IT* b, std::size_t n) { \
        NAME##_quire_t q; \
        NAME##_posit_t pa, pb; \
        for (std::size_t i = 0; i < n; ++i) { \
            std::memcpy(&pa, &a[i], sizeof(IT)); \
            std::memcpy(&pb, &b[i], sizeof(IT)); \
            q += sw::universal::quire_mul(pa, pb); \
        } \
        NAME##_posit_t p = sw::universal::quire_resolve(q); \
        IT out; \
        std::memcpy(&out, &p, sizeof(IT)); \
        return out; \
    }

// --- Posit Quire Registry ---  Q(name, IT, N, ES)
// Mirrors the posit entries of TYPE_REGISTRY_FULL; keep the two in sync.
#define POSIT_QUIRE_REGISTRY \
    Q(posit8_0,  uint8_t,   8, 0) \
    Q(posit8_1,  uint8_t,   8, 1) \
    Q(posit8_2,  uint8_t,   8, 2) \
    Q(posit12_1, uint16_t, 12, 1) \
    Q(posit16_1, uint16_t, 16, 1) \
    Q(posit16_2, uint16_t, 16, 2) \
    Q(posit32_2, uint32_t, 32, 2) \
    Q(posit19_3, uint32_t, 19, 3) \
    Q(posit19_2, uint32_t, 19, 2) \
    Q(posit64_2, uint64_t, 64, 2) \
    Q(posit64_3, uint64_t, 64, 3)

#define Q(NAME, IT, N, ES) DEFINE_BRIDGE_QUIRE(NAME, IT, N, ES)
POSIT_QUIRE_REGISTRY
#undef Q
