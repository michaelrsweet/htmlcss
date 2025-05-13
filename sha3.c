//
// SHA3 hash implementation for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2015-2025 Michael R Sweet. All rights reserved.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//
// The following code is adapted from the
// "Keccak-readable-and-compact.c" source code from the official
// Keccak reference implementation code repository:
//
//     https://github.com/gvanas/KeccakCodePackage.git
//
// The original code bore the following notice:
//
//     Implementation by the Keccak, Keyak and Ketje Teams, namely,
//     Guido Bertoni, Joan Daemen, Michaël Peeters, Gilles Van Assche
//     and Ronny Van Keer, hereby denoted as "the implementer".
//
//     For more information, feedback or questions, please refer to
//     our websites:
//
//         http://keccak.noekeon.org/
//         http://keyak.noekeon.org/
//         http://ketje.noekeon.org/
//
//     To the extent possible under law, the implementer has waived
//     all copyright and related or neighboring rights to the source
//     code in this file.
//
//         http://creativecommons.org/publicdomain/zero/1.0/
//

#include "sha3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>


//
// Endian tests to get LITTLE_ENDIAN defined to 1 on little-endian
// systems; force it with "-DLITTLE_ENDIAN=1" to get the "optimized"
// implementation.
//

#ifndef LITTLE_ENDIAN
#  ifdef _WIN32
#    define LITTLE_ENDIAN 1	// Windows is always little-endian
#  elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define LITTLE_ENDIAN 1	// GCC/Clang preprocessor check
#  else
#    define LITTLE_ENDIAN 0	// Otherwise don't assume little-endian
#  endif // _WIN32
#endif // !LITTLE_ENDIAN


//
// Local types...
//

typedef unsigned char UINT8;		// @private@
typedef unsigned long long int UINT64;	// @private@
typedef UINT64 tKeccakLane;		// @private@


//
// Local functions...
//

static void	KeccakF1600_StatePermute(void *state);


//
// 'hcSHA3Final()' - Finalize the SHA3-256 hash, putting the result in "hash".
//

void
hcSHA3Final(hc_sha3_t     *ctx,		// I - Hash context
	    unsigned char *hash,	// I - Hash buffer
	    size_t        hashlen)	// I - Bytes to copy from hash
{
  ctx->state[ctx->used]      ^= 0x06;
  ctx->state[ctx->block - 1] ^= 0x80;

  KeccakF1600_StatePermute(ctx->state);

  if (hashlen < sizeof(ctx->state))
  {
    memcpy(hash, ctx->state, hashlen);
  }
  else
  {
    memcpy(hash, ctx->state, sizeof(ctx->state));
    memset(hash + sizeof(ctx->state), 0, hashlen - sizeof(ctx->state));
  }
}


//
// 'hcSHA3Init()' - Initialize a SHA3-256 hashing context.
//

void
hcSHA3Init(hc_sha3_t *ctx)		// I - Hash context
{
  memset(ctx, 0, sizeof(hc_sha3_t));
  ctx->block = 72;
}


//
// 'hcSHA3Update()' - Update the SHA3-256 hashing context with the given data.
//

void
hcSHA3Update(hc_sha3_t  *ctx,		// I - Hash context
	     const void *data,		// I - Data to hash
	     size_t     datalen)	// I - Number of bytes of data
{
  const unsigned char	*dataptr;	// Pointer into data


  dataptr = (const unsigned char *)data;

  while (datalen > 0)
  {
    while (ctx->used < ctx->block && datalen > 0)
    {
      ctx->state[ctx->used++] ^= *dataptr++;
      datalen --;
    }

    if (ctx->used == ctx->block)
    {
      KeccakF1600_StatePermute(ctx->state);
      ctx->used = 0;
    }
  }
}



#if !LITTLE_ENDIAN
//
// 'load64()' - Load a 64-bit value using the little-endian (LE) convention.
//
// On a LE platform, this can be greatly simplified using a cast.
//

static UINT64
load64(const UINT8 *x)
{
  int i;
  UINT64 u=0;


  for (i=7; i>=0; --i)
  {
    u <<= 8;
    u |= x[i];
  }

  return u;
}

//
// 'store64()' - Store a 64-bit value using the little-endian (LE) convention.
//
// On a LE platform, this can be greatly simplified using a cast.
//

static void
store64(UINT8 *x, UINT64 u)
{
  unsigned int i;


  for (i=0; i<8; ++i)
  {
    x[i] = u;
    u >>= 8;
  }
}


//
// 'xor64()' - XOR into a 64-bit value using the little-endian (LE) convention.
//
// On a LE platform, this can be greatly simplified using a cast.
//

static void
xor64(UINT8 *x, UINT64 u)
{
  unsigned int i;

  for (i=0; i<8; ++i)
  {
    x[i] ^= u;
    u >>= 8;
  }
}
#endif // !LITTLE_ENDIAN


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ROL64(a, offset) ((((UINT64)a) << offset) ^ (((UINT64)a) >> (64-offset)))
#define i(x, y) ((x)+5*(y))

#if LITTLE_ENDIAN
#  define readLane(x, y)          (((tKeccakLane*)state)[i(x, y)])
#  define writeLane(x, y, lane)   (((tKeccakLane*)state)[i(x, y)]) = (lane)
#  define XORLane(x, y, lane)     (((tKeccakLane*)state)[i(x, y)]) ^= (lane)
#else
#  define readLane(x, y)          load64((UINT8*)state+sizeof(tKeccakLane)*i(x, y))
#  define writeLane(x, y, lane)   store64((UINT8*)state+sizeof(tKeccakLane)*i(x, y), lane)
#  define XORLane(x, y, lane)     xor64((UINT8*)state+sizeof(tKeccakLane)*i(x, y), lane)
#endif

//
// 'LFSR86540()' - Compute the linear feedback shift register (LFSR) used to
//                 define the round constants (see [Keccak Reference, Section
//                 1.2]).
//
static int
LFSR86540(UINT8 *LFSR)
{
  int result = ((*LFSR) & 0x01) != 0;


  if (((*LFSR) & 0x80) != 0)
  {
    // Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1
    (*LFSR) = (UINT8)(((*LFSR) << 1) ^ 0x71);
  }
  else
  {
    (*LFSR) <<= 1;
  }

  return result;
}

//
// 'KeccakF1600_StatePermute()' - Compute the Keccak-f[1600] permutation on the
//                                given state.
//

static void
KeccakF1600_StatePermute(void *state)
{
  unsigned int round, x, y, j, t;
  UINT8 LFSRstate = 0x01;


  for (round=0; round<24; round++)
  {
    {   // === θ step (see [Keccak Reference, Section 2.3.2]) ===
      tKeccakLane C[5], D;

      // Compute the parity of the columns
      for (x=0; x<5; x++)
	C[x] = readLane(x, 0) ^ readLane(x, 1) ^ readLane(x, 2) ^ readLane(x, 3) ^ readLane(x, 4);
      for (x=0; x<5; x++)
      {
	// Compute the θ effect for a given column
	D = C[(x+4)%5] ^ ROL64(C[(x+1)%5], 1);
	// Add the θ effect to the whole column
	for (y=0; y<5; y++)
	  XORLane(x, y, D);
      }
    }

    {   // === ρ and π steps (see [Keccak Reference, Sections 2.3.3 and 2.3.4]) ===
      tKeccakLane current, temp;
      // Start at coordinates (1 0)
      x = 1; y = 0;
      current = readLane(x, y);
      // Iterate over ((0 1)(2 3))^t * (1 0) for 0 ≤ t ≤ 23
      for (t=0; t<24; t++)
      {
	// Compute the rotation constant r = (t+1)(t+2)/2
	unsigned int r = ((t+1)*(t+2)/2)%64;
	// Compute ((0 1)(2 3)) * (x y)
	unsigned int Y = (2*x+3*y)%5; x = y; y = Y;
	// Swap current and state(x,y), and rotate
	temp = readLane(x, y);
	writeLane(x, y, ROL64(current, r));
	current = temp;
      }
    }

    {   // === χ step (see [Keccak Reference, Section 2.3.1]) ===
      tKeccakLane temp[5];
      for (y=0; y<5; y++)
      {
	// Take a copy of the plane
	for (x=0; x<5; x++)
	  temp[x] = readLane(x, y);
	// Compute χ on the plane
	for (x=0; x<5; x++)
	  writeLane(x, y, temp[x] ^((~temp[(x+1)%5]) & temp[(x+2)%5]));
      }
    }

    {   // === ι step (see [Keccak Reference, Section 2.3.5]) ===
      for (j=0; j<7; j++)
      {
	unsigned int bitPosition = (1<<j)-1; //2^j-1
	if (LFSR86540(&LFSRstate))
	  XORLane(0, 0, (tKeccakLane)1<<bitPosition);
      }
    }
  }
}
