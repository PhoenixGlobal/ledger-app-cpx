/*******************************************************************************
 *   Ledger Blue
 *   (c) 2016 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "ui.h"
#include "neo.h"
#include "bagl.h"

/** IO buffer to communicate with the outside world. */
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

/** start of the buffer, reject any transmission that doesn't start with this, as it's invalid. */
#define CLA 0x80

/** #### instructions start #### **/
/** instruction to sign transaction and send back the signature. */
#define INS_SIGN 0x02

/** instruction to send back the public key. */
#define INS_GET_PUBLIC_KEY 0x04
/** #### instructions end #### */

/** BIP44 path, used to derive the private key from the mnemonic by calling os_perso_derive_node_bip32. */
// TODO: init path here, instead of at the function call.
static unsigned int path[5];

// TODO: figure out what this function does, as it was copy/pasted from the sample app.
unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
	switch (channel & ~(IO_FLAGS)) {
	case CHANNEL_KEYBOARD:
		break;

		// multiplexed io exchange over a SPI channel and TLV encapsulated protocol
	case CHANNEL_SPI:
		if (tx_len) {
			io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

			if (channel & IO_RESET_AFTER_REPLIED) {
				reset();
			}
			// nothing received from the master so far
			//(it's a tx transaction)
			return 0;
		} else {
			return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
		}

	default:
		THROW(INVALID_PARAMETER);
	}
	return 0;
}

/** main loop. */
static void neo_main(void) {
	volatile unsigned int rx = 0;
	volatile unsigned int tx = 0;
	volatile unsigned int flags = 0;

	// DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
	// goal is to retrieve APDU.
	// When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
	// sure the io_event is called with a
	// switch event, before the apdu is replied to the bootloader. This avoid
	// APDU injection faults.
	for (;;) {
		volatile unsigned short sw = 0;

		BEGIN_TRY
			{
				TRY
					{
						rx = tx;
						// ensure no race in catch_other if io_exchange throws an error
						tx = 0;
						rx = io_exchange(CHANNEL_APDU | flags, rx);
						flags = 0;

						// no apdu received, well, reset the session, and reset the
						// bootloader configuration
						if (rx == 0) {
							THROW(0x6982);
						}

						// if the buffer doesn't start with the magic byte, return an error.
						if (G_io_apdu_buffer[0] != CLA) {
							THROW(0x6E00);
						}

						// check the second byte (0x01) for the instruction.
						switch (G_io_apdu_buffer[1]) {

						// we're getting a transaction to sign, in parts.
						case INS_SIGN: {
							// check the third byte (0x02) for the instruction subtype.
							if ((G_io_apdu_buffer[2] != P1_MORE) && (G_io_apdu_buffer[2] != P1_LAST)) {
								THROW(0x6A86);
							}

							// if this is the first transaction part, reset the hash and all the other temporary variables.
							if (hashTainted) {
								cx_sha256_init(&hash);
								hashTainted = 0;
								raw_tx_ix = 0;
								raw_tx_len = 0;
							}

							// move the contents of the buffer into raw_tx, and update raw_tx_ix to the end of the buffer, to be ready for the next part of the tx.
							unsigned int len = get_apdu_buffer_length();
							unsigned char * in = G_io_apdu_buffer + APDU_HEADER_LENGTH;
							unsigned char * out = raw_tx + raw_tx_ix;
							os_memmove(out, in, len);
							raw_tx_ix += len;

							// set the screen to be the first screen.
							curr_scr_ix = 0;

							// set the buffer to end with a zero.
							G_io_apdu_buffer[APDU_HEADER_LENGTH + len] = '\0';

							// if this is the last part of the transaction, parse the transaction into human readable text, and display it.
							if (G_io_apdu_buffer[2] == P1_LAST) {
								raw_tx_len = raw_tx_ix;
								raw_tx_ix = 0;

								// parse the transaction into human readable text.
								display_tx_desc();

								// display the UI, starting at the top screen which is "Sign Tx Now".
								ui_top_sign();
							}

							flags |= IO_ASYNCH_REPLY;

							// if this is not the last part of the transaction, do not display the UI, and approve the partial transaction.
							// this adds the TX to the hash.
							if (G_io_apdu_buffer[2] == P1_MORE) {
								io_seproxyhal_touch_approve(NULL);
							}
						}
							break;

						// we're asked for the public key.
						case INS_GET_PUBLIC_KEY: {
							cx_ecfp_public_key_t publicKey;
							cx_ecfp_private_key_t privateKey;

							// put the private key into working memory.
							os_memmove(&privateKey, &N_privateKey, sizeof(cx_ecfp_private_key_t));

							// generate the public key.
							cx_ecfp_generate_pair(CX_CURVE_256R1, &publicKey, &privateKey, 1);

							// push the public key onto the response buffer.
							os_memmove(G_io_apdu_buffer, publicKey.W, 65);
							tx = 65;

							// return 0x9000 OK.
							THROW(0x9000);
						}
							break;

						case 0xFF: // return to dashboard
							goto return_to_dashboard;

						// we're asked to do an unknown command
						default:
							// return an error.
							THROW(0x6D00);
							break;
						}
					}
					CATCH_OTHER(e)
					{
						switch (e & 0xF000) {
						case 0x6000:
						case 0x9000:
							sw = e;
							break;
						default:
							sw = 0x6800 | (e & 0x7FF);
							break;
						}
						// Unexpected exception => report
						G_io_apdu_buffer[tx] = sw >> 8;
						G_io_apdu_buffer[tx + 1] = sw;
						tx += 2;
					}
					FINALLY
				{
				}
			}
			END_TRY;
	}

	return_to_dashboard: return;
}

// TODO: figure out what this function does, as it was copy/pasted from the sample app.
void io_seproxyhal_display(const bagl_element_t *element) {
	io_seproxyhal_display_default((bagl_element_t *) element);
}

// TODO: figure out what this function does, as it was copy/pasted from the sample app.
unsigned char io_event(unsigned char channel) {
// nothing done with the event, throw an error on the transport layer if
// needed

// can't have more than one tag in the reply, not supported yet.
	switch (G_io_seproxyhal_spi_buffer[0]) {
	case SEPROXYHAL_TAG_FINGER_EVENT:
		UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
		UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
		if (UX_DISPLAYED()) {
			// TODO perform actions after all screen elements have been
			// displayed
		} else {
			UX_DISPLAYED_EVENT();
		}
		break;

	case SEPROXYHAL_TAG_TICKER_EVENT:
		UX_REDISPLAY();
		break;

		// unknown events are acknowledged
	default:
		break;
	}

// close the event if not done previously (by a display or whatever)
	if (!io_seproxyhal_spi_is_status_sent()) {
		io_seproxyhal_general_status();
	}

// command has been processed, DO NOT reset the current APDU transport
	return 1;
}

/** boot up the app and intialize it */
__attribute__((section(".boot"))) int main(void) {
// exit critical section
	__asm volatile("cpsie i");

	curr_scr_ix = 0;
	max_scr_ix = 0;
	raw_tx_ix = 0;
	hashTainted = 1;
	uiState = UI_IDLE;

// ensure exception will work as planned
	os_boot();

	UX_INIT();

	BEGIN_TRY
		{
			TRY
				{
					io_seproxyhal_init();

					// Create the private key if not initialized
					if (N_initialized != 0x01) {
						unsigned char canary;
						cx_ecfp_private_key_t privateKey;
						cx_ecfp_public_key_t publicKey;

						unsigned char privateKeyData[32];

						if (!os_global_pin_is_validated()) {
							return 0;
						}
						// TODO: update path to be a different coin path than bitcoin.
						path[0] = 44 | 0x80000000;
						path[1] = 0 | 0x80000000;
						path[2] = 0 | 0x80000000;
						path[3] = 0;
						path[4] = 0;
						os_perso_derive_node_bip32(CX_CURVE_256R1, path, 5, privateKeyData, NULL);
						cx_ecdsa_init_private_key(CX_CURVE_256R1, privateKeyData, 32, &privateKey);
						cx_ecfp_generate_pair(CX_CURVE_256R1, &publicKey, &privateKey, 1);
						nvm_write((void*) &N_privateKey, &privateKey, sizeof(privateKey));
						canary = 0x01;
						nvm_write((void*) &N_initialized, &canary, sizeof(canary));
					}

					USB_power(0);
					USB_power(1);

					// show idle screen.
					ui_idle();

					// run main event loop.
					neo_main();
				}
				CATCH_OTHER(e)
				{
				}
				FINALLY
			{
			}
		}
		END_TRY;
}
