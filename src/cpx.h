/*
 * MIT License, see root folder for full license.
 */

#ifndef CPX_H
#define CPX_H

#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "ui.h"
#include "sha256_hash_len.h"


/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
unsigned char display_tx_desc(void);

/** displays the "no public key" message, prior to a public key being requested. */
void display_no_public_key(void);

/** displays the public key, assumes length is 65. */
void display_public_key(const unsigned char * public_key);

#endif // CPX_H
