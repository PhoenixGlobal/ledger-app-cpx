/*
 * MIT License, see root folder for full license.
 */
#include "cpx.h"

#include <string.h>

/** if true, show a screen with the transaction version. */
#define SHOW_TX_VERSION false

/** if true, show a screen with the transaction type. */
#define SHOW_TX_TYPE true

/** if true, show a screen with the transaction from address. */
#define SHOW_FROM_ADDRESS false


/**
 * CPX TX fields
 */
/** length of tx.version.value */
#define CPX_TX_VERSION_LEN 4

/** length of tx.txid.value */
#define CPX_TX_ID_LEN 1

/** length of tx.output.value */
#define VALUE_LEN 8

/** length of tx.output.asset_id */
#define ASSET_ID_LEN 32

/** length of tx.output.script_hash */
#define SCRIPT_HASH_LEN 20

/** length of the checksum used to convert a tx.output.script_hash into an Address. */
#define SCRIPT_HASH_CHECKSUM_LEN 4

/** length of a tx.output Address, after Base58 encoding. */
#define ADDRESS_BASE58_LEN 35

/** length of a tx.output Address before encoding, which is the length of <address_prefix>+<script_hash>+<checksum> */
#define ADDRESS_LEN (2 + SCRIPT_HASH_LEN + SCRIPT_HASH_CHECKSUM_LEN)

/** the length of a SHA256 hash */
#define SHA256_HASH_LEN 32

#define APEX_ADDRESS_PREFIX_1	0x05	// bin for 'A'
#define APEX_ADDRESS_PREFIX_2	0x48	// bin for 'P'

/**
 * transaction types.
 *
 * Currently only Claim and Contract are tested, as they are the only ones supported by the current wallets.
 */
enum TX_TYPE {
	TX_MINER = 0x00, TX_TRANSFER = 0x01, TX_DEPLOY = 0x02, TX_CALL = 0x03, TX_REFUND = 0x04, TX_SCHEDULE = 0x05
};


/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** CPX asset's label. */
static const char TXT_ASSET_CPX[] = "CPX";

/** CPX asset's label. */
static const char TXT_ASSET_FEE[] = "Fee (KGP)";

/** Version label */
static const char TXT_VERSION[] = "Version";

/** Type label */
static const char TXT_TX_TYPE[] = "Tx Type";

/** Label when displaying a Miner transaction */
static const char TX_MINER_NM[] = "Miner";

/** Label when displaying a Transfer transaction */
static const char TX_TRANSFER_NM[] = "Transfer";

/** Label when displaying a Deploy transaction */
static const char TX_DEPLOY_NM[] = "Deploy";

/** Label when displaying a Call transaction */
static const char TX_CALL_NM[] = "Call";

/** Label when displaying a Refund transaction */
static const char TX_REFUND_NM[] = "Refund";

/** Label when displaying a Schedule transaction */
static const char TX_SCHEDULE_NM[] = "Schedule";

/** Version label */
static const char TXT_ADDRESS_FROM[] = "From:";

/** Version label */
static const char TXT_ADDRESS_TO[] = "To:";


/** Label when a public key has not been set yet */
static const char NO_PUBLIC_KEY_0[] = "No Public Key";
static const char NO_PUBLIC_KEY_1[] = "Requested Yet";

int screen_index_page_type[MAX_TX_TEXT_SCREENS];

/** array of capital letter hex values */
static const char HEX_CAP[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

/** array of base58 aplhabet letters */
static const char BASE_58_ALPHABET[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q',
		'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z' };

/** array of base10 aplhabet letters */
static const char BASE_10_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

/** skips the given number of bytes in the transaction */
static void skip_raw_tx(const unsigned int tx_skip);

/** returns the number of bytes in the next variable byte record, called prior to reading a variable byte record to know how many bytes to read. */
static unsigned char next_raw_tx_varbytes_num();

/** reads a set of bytes into the array pointed to by the arr parameter, reads as many bytes as the length parameter specifies. */
static void next_raw_tx_arr(unsigned char * arr, const unsigned int length);

/** reads the next byte out of the transaction, or throws an error if there are no more bytes left. */
static unsigned char next_raw_tx();

/** returns the minimum of i0 and i1 */
static unsigned int min(const unsigned int i0, const unsigned int i1);

/** reads bytes out of src, converts each byte into two hex characters, and writes the hex characters into dest. Only converts enough characters to fill dest_len. */
static void to_hex(char * dest, const unsigned char * src, const unsigned int dest_len);

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
		const unsigned int out_length);

/** encodes in_length bytes from in into base-10, writes the converted bytes to out, stopping when it converts out_length bytes.  */
static unsigned int encode_base_10(const void *in, const unsigned int in_length, char *out, const unsigned int out_length) {
	return encode_base_x(BASE_10_ALPHABET, sizeof(BASE_10_ALPHABET), in, in_length, out, out_length);
}

/** encodes in_length bytes from in into base-58, writes the converted bytes to out, stopping when it converts out_length bytes.  */
static unsigned int encode_base_58(const void *in, const unsigned int in_len, char *out, const unsigned int out_len) {
	return encode_base_x(BASE_58_ALPHABET, sizeof(BASE_58_ALPHABET), in, in_len, out, out_len);
}

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
		const unsigned int out_length) {
	char tmp[164];
	char buffer[164];
	unsigned char buffer_ix;
	unsigned char startAt;
	unsigned char zeroCount = 0;
	if (in_length > sizeof(tmp)) {
		hashTainted = 1;
		THROW(0x6D11);
	}
	os_memmove(tmp, in, in_length);
	while ((zeroCount < in_length) && (tmp[zeroCount] == 0)) {
		++zeroCount;
	}
	buffer_ix = 2 * in_length;
	if (buffer_ix > sizeof(buffer)) {
		hashTainted = 1;
		THROW(0x6D12);
	}

	startAt = zeroCount;
	while (startAt < in_length) {
		unsigned short remainder = 0;
		unsigned char divLoop;
		for (divLoop = startAt; divLoop < in_length; divLoop++) {
			unsigned short digit256 = (unsigned short) (tmp[divLoop] & 0xff);
			unsigned short tmpDiv = remainder * 256 + digit256;
			tmp[divLoop] = (unsigned char) (tmpDiv / alphabet_len);
			remainder = (tmpDiv % alphabet_len);
		}
		if (tmp[startAt] == 0) {
			++startAt;
		}
		buffer[--buffer_ix] = (unsigned char)alphabet[remainder];
	}

    while ((buffer_ix < (2 * in_length)) && (buffer[buffer_ix] == alphabet[0])) {
        ++buffer_ix;
    }
    while (zeroCount-- > 0) {
        buffer[--buffer_ix] = alphabet[0];
    }

	const unsigned int total_length = (2 * in_length) - buffer_ix;
	if (total_length > out_length) {
		THROW(0x6D14);
	}
	os_memmove(out, (buffer + buffer_ix), total_length);
	return total_length;
}

bool adjustDecimals(char *src, uint32_t srcLength, char *target,
                    uint32_t targetLength, uint8_t decimals) {
    uint32_t startOffset;
    uint32_t lastZeroOffset = 0;
    uint32_t offset = 0;

    if ((srcLength == 1) && (*src == '0')) {
        if (targetLength < 2) {
            return false;
        }
        target[offset++] = '0';
        target[offset++] = '\0';
        return true;
    }
    if (srcLength <= decimals) {
        uint32_t delta = decimals - srcLength;
        if (targetLength < srcLength + 1 + 2 + delta) {
            return false;
        }
        target[offset++] = '0';
        target[offset++] = '.';
        for (uint32_t i = 0; i < delta; i++) {
            target[offset++] = '0';
        }
        startOffset = offset;
        for (uint32_t i = 0; i < srcLength; i++) {
            target[offset++] = src[i];
        }
        target[offset] = '\0';
    } else {
        uint32_t sourceOffset = 0;
        uint32_t delta = srcLength - decimals;
        if (targetLength < srcLength + 1 + 1) {
            return false;
        }
        while (offset < delta) {
            target[offset++] = src[sourceOffset++];
        }
        if (decimals != 0) {
            target[offset++] = '.';
        }
        startOffset = offset;
        while (sourceOffset < srcLength) {
            target[offset++] = src[sourceOffset++];
        }
        target[offset] = '\0';
    }
    for (uint32_t i = startOffset; i < offset; i++) {
        if (target[i] == '0') {
            if (lastZeroOffset == 0) {
                lastZeroOffset = i;
            }
        } else {
            lastZeroOffset = 0;
        }
    }
    if (lastZeroOffset != 0) {
        target[lastZeroOffset] = '\0';
        if (target[lastZeroOffset - 1] == '.') {
            target[lastZeroOffset - 1] = '\0';
        }
    }
    return true;
}
unsigned short print_amount(uint64_t amount, uint8_t *out,
                                uint32_t outlen, uint8_t sun) {
    char tmp[20];
    char tmp2[25];
    uint32_t numDigits = 0, i;
    uint64_t base = 1;
    while (base <= amount) {
        base *= 10;
        numDigits++;
    }
    if (numDigits > sizeof(tmp) - 1) {
        THROW(0x6a80);
    }
    base /= 10;
    for (i = 0; i < numDigits; i++) {
        tmp[i] = '0' + ((amount / base) % 10);
        base /= 10;
    }
    tmp[i] = '\0';
    adjustDecimals(tmp, i, tmp2, 25, sun);
    if (strlen(tmp2) < outlen - 1) {
        strcpy((char *)out, tmp2);
    } else {
        out[0] = '\0';
    }
    return strlen((char *)out);
}

/** converts a CPX scripthash to a CPX address by adding a checksum and encoding in base58 */
static void to_address(char * dest, unsigned int dest_len, const unsigned char * script_hash) {
	static cx_sha256_t address_hash;
	unsigned char address_hash_result_0[SHA256_HASH_LEN];
	unsigned char address_hash_result_1[SHA256_HASH_LEN];

	// concatenate the address prefix ("AP")and the address.
	unsigned char address[ADDRESS_LEN];
	address[0] = APEX_ADDRESS_PREFIX_1;
	address[1] = APEX_ADDRESS_PREFIX_2;
	os_memmove(address + 2, script_hash, SCRIPT_HASH_LEN);

	// do a sha256 hash of the address twice.
	cx_sha256_init(&address_hash);
	cx_hash(&address_hash.header, CX_LAST, address, SCRIPT_HASH_LEN + 2, address_hash_result_0, 32);
	cx_sha256_init(&address_hash);
	cx_hash(&address_hash.header, CX_LAST, address_hash_result_0, SHA256_HASH_LEN, address_hash_result_1, 32);

	// add the first bytes of the hash as a checksum at the end of the address.
	os_memmove(address + 2 + SCRIPT_HASH_LEN, address_hash_result_1, SCRIPT_HASH_CHECKSUM_LEN);

	// encode the version + address + checksum in base58
	encode_base_58(address, ADDRESS_LEN, dest, dest_len);
}

/** converts a byte array in src to a hex array in dest, using only dest_len bytes of dest before stopping. */
static void to_hex(char * dest, const unsigned char * src, const unsigned int dest_len) {
	for (unsigned int src_ix = 0, dest_ix = 0; dest_ix < dest_len; src_ix++, dest_ix += 2) {
		unsigned char src_c = *(src + src_ix);
		unsigned char nibble0 = (src_c >> 4) & 0xF;
		unsigned char nibble1 = src_c & 0xF;

		*(dest + dest_ix + 0) = HEX_CAP[nibble0];
		*(dest + dest_ix + 1) = HEX_CAP[nibble1];
	}
}

/** returns the minimum of two ints. */
static unsigned int min(unsigned int i0, unsigned int i1) {
	if (i0 < i1) {
		return i0;
	} else {
		return i1;
	}
}

/** skips the given number of bytes in the raw_tx buffer. If this goes off the end of the buffer, throw an error. */
static void skip_raw_tx(unsigned int tx_skip) {
	raw_tx_ix += tx_skip;
	if (raw_tx_ix >= raw_tx_len) {
		hashTainted = 1;
		THROW(0x6D03);
	}
}

/** returns the number of bytes to read for the next varbytes array.
 *  Currently throws an error if the encoded value should be over 253,
 *   which should never happen in this use case of a varbyte array
 */
static unsigned char next_raw_tx_varbytes_num() {
	unsigned char num = next_raw_tx();
	switch (num) {
	case 0xFD:
	case 0xFE:
	case 0xFF:
		hashTainted = 1;
		THROW(0x6D04);
		break;
	default:
		break;
	}
	return num;
}

/** fills the array in arr with the given number of bytes from raw_tx. */
static void next_raw_tx_arr(unsigned char * arr, unsigned int length) {
	for (unsigned int ix = 0; ix < length; ix++) {
		*(arr + ix) = next_raw_tx();
	}
}

/** returns the next byte in raw_tx and increments raw_tx_ix. If this would increment raw_tx_ix over the end of the buffer, throw an error. */
static unsigned char next_raw_tx() {
	if (raw_tx_ix < raw_tx_len) {
		unsigned char retval = raw_tx[raw_tx_ix];
		raw_tx_ix += 1;
		return retval;
	} else {
		hashTainted = 1;
		THROW(0x6D05);
		return 0;
	}
}

unsigned char display_tx_desc() {
	unsigned int scr_ix = 0;
	char hex_buffer[MAX_TX_TEXT_WIDTH];
	unsigned int hex_buffer_len = 0;

	unsigned char tx_version[CPX_TX_VERSION_LEN];
	next_raw_tx_arr(tx_version, CPX_TX_VERSION_LEN);
	if (SHOW_TX_VERSION) {
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memmove(tx_desc[scr_ix][0], TXT_VERSION, sizeof(TXT_VERSION));

			hex_buffer_len = min(MAX_HEX_BUFFER_LEN, CPX_TX_VERSION_LEN) * 2;
			to_hex(hex_buffer, tx_version, hex_buffer_len);
			os_memmove(tx_desc[scr_ix][1], hex_buffer, hex_buffer_len);

			screen_index_page_type[scr_ix] = SINGLE_PAGE;
			scr_ix++;
		}
	}

	enum TX_TYPE trans_type = next_raw_tx();
	if (SHOW_TX_TYPE) {
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			// add transaction type screen.
			os_memmove(tx_desc[scr_ix][0], TXT_TX_TYPE, sizeof(TXT_TX_TYPE));
			switch (trans_type) {
			case TX_MINER:
				os_memmove(tx_desc[scr_ix][1], TX_MINER_NM, sizeof(TX_MINER_NM));
				break;

			case TX_TRANSFER:
				os_memmove(tx_desc[scr_ix][1], TX_TRANSFER_NM, sizeof(TX_TRANSFER_NM));
				break;

			case TX_DEPLOY:
				os_memmove(tx_desc[scr_ix][1], TX_DEPLOY_NM, sizeof(TX_DEPLOY_NM));
				break;

			case TX_REFUND:
				os_memmove(tx_desc[scr_ix][1], TX_REFUND_NM, sizeof(TX_REFUND_NM));
				break;

			case TX_SCHEDULE:
				os_memmove(tx_desc[scr_ix][1], TX_SCHEDULE_NM, sizeof(TX_SCHEDULE_NM));
				break;

			default:
				hashTainted = 1;
				THROW(0x6D06);
			}

			screen_index_page_type[scr_ix] = SINGLE_PAGE;
			scr_ix++;
		}
	}

	// from-to addresses
	unsigned char fromAddressHash[SCRIPT_HASH_LEN];
	next_raw_tx_arr(fromAddressHash, SCRIPT_HASH_LEN);

	char address_base58[ADDRESS_BASE58_LEN];
	unsigned int address_base58_len_0 = 12;
	unsigned int address_base58_len_1 = 11;
	unsigned int address_base58_len_2 = 12;
	char * address_base58_0 = address_base58;
	char * address_base58_1 = address_base58 + address_base58_len_0;
	char * address_base58_2 = address_base58 + address_base58_len_0 + address_base58_len_1;

	// from address
	if (SHOW_FROM_ADDRESS) {
		to_address(address_base58, ADDRESS_BASE58_LEN, fromAddressHash);
		// address screen
		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			os_memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);
			os_memmove(tx_desc[scr_ix][0], address_base58_0, address_base58_len_0);
			os_memmove(tx_desc[scr_ix][1], address_base58_1, address_base58_len_1);
			os_memmove(tx_desc[scr_ix][2], address_base58_2, address_base58_len_2);

			screen_index_page_type[scr_ix] = TWO_PAGE;
			scr_ix++;
		}
	}
	// to address
	unsigned char toAddressHash[SCRIPT_HASH_LEN];
	next_raw_tx_arr(toAddressHash, SCRIPT_HASH_LEN);

	to_address(address_base58, ADDRESS_BASE58_LEN, toAddressHash);
	// address screen
	if (scr_ix < MAX_TX_TEXT_SCREENS) {
		os_memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);
		os_memmove(tx_desc[scr_ix][0], address_base58_0, address_base58_len_0);
		os_memmove(tx_desc[scr_ix][1], address_base58_1, address_base58_len_1);
		os_memmove(tx_desc[scr_ix][2], address_base58_2, address_base58_len_2);

		screen_index_page_type[scr_ix] = TWO_PAGE;
		scr_ix++;
	}

	// CPX value
	unsigned char value_len = next_raw_tx();;
	unsigned char value[value_len];
	next_raw_tx_arr(value, value_len);
	uint64_t value_divisor = 1000000000000000;

	if (scr_ix < MAX_TX_TEXT_SCREENS) {
		os_memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);

		os_memmove(tx_desc[scr_ix][0], TXT_ASSET_CPX, sizeof(TXT_ASSET_CPX));
		uint64_t value_int = (uint64_t)value[0] << 56 | (uint64_t)value[1] << 48 | (uint64_t)value[2] << 40 | (uint64_t)value[3] << 32 | (uint64_t)value[4] << 24 | (uint64_t)value[5] << 16 | (uint64_t)value[6] << 8 | (uint64_t)value[7];
		value_int = value_int / value_divisor;
		print_amount(value_int, value, sizeof(value), 3) ;

		os_memmove(tx_desc[scr_ix][1], value, sizeof(value));

		screen_index_page_type[scr_ix] = SINGLE_PAGE;
		scr_ix++;
	}

	// nonce (8bytes)
	unsigned char nonce[8];
	next_raw_tx_arr(nonce, 8);

	// data
	unsigned char data_len = next_raw_tx();;
	unsigned char data[data_len];
	next_raw_tx_arr(data, data_len);

	// fee
	unsigned char fee_len = next_raw_tx();;
	unsigned char fee_price[fee_len];
	next_raw_tx_arr(fee_price, fee_len);
	uint64_t fee_divisor = 1000000000000;

	if (scr_ix < MAX_TX_TEXT_SCREENS) {
		os_memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);

		os_memmove(tx_desc[scr_ix][0], TXT_ASSET_FEE, sizeof(TXT_ASSET_FEE));
		uint64_t fee_int = (uint64_t)fee_price[0] << 40 | (uint64_t)fee_price[1] << 32 | (uint64_t)fee_price[2] << 24 | (uint64_t)fee_price[3] << 16 | (uint64_t)fee_price[4] << 8 | (uint64_t)fee_price[5];
		fee_int = fee_int / fee_divisor;
		print_amount(fee_int, fee_price, sizeof(fee_price), 0) ;

		os_memmove(tx_desc[scr_ix][1], fee_price, sizeof(fee_price));

		screen_index_page_type[scr_ix] = SINGLE_PAGE;
		scr_ix++;
	}

	max_scr_ix = scr_ix;

	os_memmove(curr_tx_desc, tx_desc[curr_scr_ix], CURR_TX_DESC_LEN);

	return 1;
}

void display_no_public_key() {
	os_memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[0], NO_PUBLIC_KEY_0, sizeof(NO_PUBLIC_KEY_0));
	os_memmove(current_public_key[1], NO_PUBLIC_KEY_1, sizeof(NO_PUBLIC_KEY_1));
	publicKeyNeedsRefresh = 0;
}

void public_key_hash160(unsigned char * in, unsigned short inlen, unsigned char *out) {
	union {
		cx_sha256_t shasha;
		cx_ripemd160_t riprip;
	} u;
	unsigned char buffer[32];

	cx_sha256_init(&u.shasha);
	cx_hash(&u.shasha.header, CX_LAST, in, inlen, buffer, 32);
	cx_ripemd160_init(&u.riprip);
	cx_hash(&u.riprip.header, CX_LAST, buffer, 32, out, 20);
}

void display_public_key(const unsigned char * public_key) {
	os_memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	os_memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));

	//compress public key
	// from https://github.com/CityOfZion/neon-js core.js
	unsigned char public_key_encoded[33];
	public_key_encoded[0] = ((public_key[64] & 1) ? 0x03 : 0x02);
	os_memmove(public_key_encoded + 1, public_key + 1, 32);

	unsigned char verification_script[35];
	verification_script[0] = 0x21;
	os_memmove(verification_script + 1, public_key_encoded, sizeof(public_key_encoded));
	verification_script[sizeof(verification_script) - 1] = 0xAC;

	unsigned char script_hash[SCRIPT_HASH_LEN];
	for (int i = 0; i < SCRIPT_HASH_LEN; i++) {
		script_hash[i] = 0x00;
	}

	public_key_hash160(verification_script, sizeof(verification_script), script_hash);
	unsigned char script_hash_rev[SCRIPT_HASH_LEN];
	for (int i = 0; i < SCRIPT_HASH_LEN; i++) {
		script_hash_rev[i] = script_hash[SCRIPT_HASH_LEN - (i + 1)];
	}

	char address_base58[ADDRESS_BASE58_LEN];
	unsigned int address_base58_len_0 = 12;
	unsigned int address_base58_len_1 = 11;
	unsigned int address_base58_len_2 = 12;
	char * address_base58_0 = address_base58;
	char * address_base58_1 = address_base58 + address_base58_len_0;
	char * address_base58_2 = address_base58 + address_base58_len_0 + address_base58_len_1;

	to_address(address_base58, ADDRESS_BASE58_LEN, script_hash);

	os_memmove(current_public_key[0], address_base58_0, address_base58_len_0);
	os_memmove(current_public_key[1], address_base58_1, address_base58_len_1);
	os_memmove(current_public_key[2], address_base58_2, address_base58_len_2);
}
