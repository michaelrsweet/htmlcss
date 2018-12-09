/*
 * SHA3 hash definitions for HTMLCSS library.
 *
 * Copyright © 2015-2018 by Michael R Sweet. All rights reserved.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_SHA3_H
#  define HTMLCSS_SHA3_H

/*
 * Include necessary headers...
 */

#  include <sys/types.h>

/*
 * C++ magic...
 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Constants...
 */

#  define HC_SHA3_256_SIZE 32		/* SHA3-256 */
#  define HC_SHA3_512_SIZE 64		/* SHA3-512 */


/*
 * Types...
 */

typedef struct hc_sha3_s		/* SHA3 hashing context */
{
  unsigned char	used,			/* Bytes "used" in state */
		block,			/* Bytes per block */
		state[200];		/* SHA3 state */
} hc_sha3_t;

typedef unsigned char hc_sha3_256_t[HC_SHA3_256_SIZE];
typedef unsigned char hc_sha3_512_t[HC_SHA3_512_SIZE];


/*
 * Prototypes...
 */

extern void	hcSHA3Final(hc_sha3_t *ctx, unsigned char *hash, size_t hashlen);
extern void	hcSHA3Init(hc_sha3_t *ctx);
extern void	hcSHA3Update(hc_sha3_t *ctx, const void *data, size_t datalen);


/*
 * C++ magic...
 */

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !HTMLCSS_SHA3_H */
