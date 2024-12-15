#pragma once

#define SIGNATURE_SIZE 64
#define NUMBER_OF_TRANSACTIONS_PER_TICK 1024 // Must be 2^N
#define MAX_NUMBER_OF_PENDING_TRANSACTIONS_PER_COMPUTOR 128
#define MAX_NUMBER_OF_CONTRACTS 1024 // Must be 1024
#define NUMBER_OF_COMPUTORS 676
#define QUORUM (NUMBER_OF_COMPUTORS * 2 / 3 + 1)
#define NUMBER_OF_EXCHANGED_PEERS 4

#define SPECTRUM_DEPTH 24 // Defines SPECTRUM_CAPACITY (1 << SPECTRUM_DEPTH)
#define SPECTRUM_CAPACITY (1ULL << SPECTRUM_DEPTH) // Must be 2^N

#define ASSETS_CAPACITY 0x1000000ULL // Must be 2^N
#define ASSETS_DEPTH 24 // Is derived from ASSETS_CAPACITY (=N)

#define MAX_INPUT_SIZE 1024ULL
#define ISSUANCE_RATE 1000000000000LL
#define MAX_AMOUNT (ISSUANCE_RATE * 1000ULL)
#define MAX_SUPPLY (ISSUANCE_RATE * 200ULL)


// If you want to use the network_meassges directory in your project without dependencies to other code,
// you may define NETWORK_MESSAGES_WITHOUT_CORE_DEPENDENCIES before including any header or change the
// following line to "#if 1".
#if defined(NETWORK_MESSAGES_WITHOUT_CORE_DEPENDENCIES)

#ifdef __aarch64__
#include <arm_neon.h>
#define __int8 __int8_t
#define __int16 __int16_t
#define __int32 __int32_t
#define __int64 __int64_t
#define __uint8 __uint8_t
#define __uint16 __uint16_t
#define __uint32 __uint32_t
#define __uint64 __uint64_t
#else
#include <intrin.h>
#endif

typedef union m256i
{
    __int8              m256i_i8[32];
    __int16             m256i_i16[16];
    __int32             m256i_i32[8];
    __int64             m256i_i64[4];
    __uint8     m256i_u8[32];
    __uint16    m256i_u16[16];
    __uint32    m256i_u32[8];
    __uint64    m256i_u64[4];
} m256i;

#else

#include "../platform/m256.h"

#endif

typedef union IPv4Address
{
  __uint8     u8[4];
  __uint32    u32;
} IPv4Address;

static_assert(sizeof(IPv4Address) == 4, "Unexpected size!");

static inline bool operator==(const IPv4Address& a, const IPv4Address& b)
{
    return a.u32 == b.u32;
}

static inline bool operator!=(const IPv4Address& a, const IPv4Address& b)
{
    return a.u32 != b.u32;
}

// Compute the siblings array of each level of tree. This function is not thread safe
// make sure resource protection is handled outside
template <unsigned int depth>
static void getSiblings(int digestIndex, const m256i* digests, m256i siblings[depth])
{
    const unsigned int capacity = (1ULL << depth);
    int siblingIndex = digestIndex;
    unsigned int digestOffset = 0;
    for (unsigned int j = 0; j < depth; j++)
    {
        siblings[j] = digests[digestOffset + (siblingIndex ^ 1)];
        digestOffset += (capacity >> j);
        siblingIndex >>= 1;
    }
}
