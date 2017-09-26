/*
 * MIT License, see root folder for full license.
 */

#include "ui.h"

// default font
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER

// text description font.
#define TX_DESC_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER

enum UI_STATE uiState;

ux_state_t ux;

// private key in flash. const and N_ variable name are mandatory here
const cx_ecfp_private_key_t N_privateKey;

// initialization marker in flash. const and N_ variable name are mandatory here
const unsigned char N_initialized;

// notification to restart the hash
unsigned char hashTainted;

// the hash.
cx_sha256_t hash;

// index of the current screen.
unsigned int curr_scr_ix;

// max index for all screens.
unsigned int max_scr_ix;

// raw transaction data.
unsigned char raw_tx[MAX_TX_RAW_LENGTH];

// current index into raw transaction.
unsigned int raw_tx_ix;

// current length of raw transaction.
unsigned int raw_tx_len;

// all text descriptions.
char tx_desc[MAX_TX_TEXT_SCREENS][MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

// currently displayed text description.
char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

static const bagl_element_t * io_seproxyhal_touch_exit(const bagl_element_t *e);
static const bagl_element_t * io_seproxyhal_touch_deny(const bagl_element_t *e);

static void ui_display_tx_desc(void);
static void ui_sign(void);
static void ui_deny(void);

static void tx_desc_up(void);
static void tx_desc_dn(void);

static const bagl_element_t bagl_ui_idle_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
		{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* center text */
		{ { BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Wake Up, NEO...", 0, 0, 0, NULL, NULL, NULL, },
		/* left icon is a X */
		{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_idle_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_exit(NULL);
		break;
	}

	return 0;
}

static const bagl_element_t bagl_ui_top_sign_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
		{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* top left bar */
		{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* top right bar */
		{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* center text */
		{ { BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Sign Tx Now", 0, 0, 0, NULL, NULL, NULL, },
		/* left icon is up arrow  */
		{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* right icon is down arrow */
		{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * up on Left button, down on right button, sign on both buttons.
 */
static unsigned int bagl_ui_top_sign_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_approve(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up();
		break;
	}
	return 0;
}

static const bagl_element_t bagl_ui_sign_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
		{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* top left bar */
		{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* top right bar */
		{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* center text */
		{ { BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Sign Tx", 0, 0, 0, NULL, NULL, NULL, },
		/* left icon is up arrow  */
		{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* right icon is down arrow */
		{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * up on Left button, down on right button, sign on both buttons.
 */
static unsigned int bagl_ui_sign_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_approve(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up();
		break;
	}
	return 0;
}

static const bagl_element_t bagl_ui_deny_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
		{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* top left bar */
		{ { BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* top right bar */
		{ { BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* center text */
		{ { BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Deny Tx", 0, 0, 0, NULL, NULL, NULL, },
		/* left icon is up arrow  */
		{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

/**
 * up on Left button, down on right button, deny on both buttons.
 */
static unsigned int bagl_ui_deny_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_deny(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up();
		break;
	}
	return 0;
}

static const bagl_element_t bagl_ui_tx_desc_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
		{ { BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* first line of description of current screen */
		{ { BAGL_LABELINE, 0x02, 10, 10, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, curr_tx_desc[0], 0, 0, 0, NULL, NULL, NULL, },
		/* second line of description of current screen */
		{ { BAGL_LABELINE, 0x02, 10, 20, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, curr_tx_desc[1], 0, 0, 0, NULL, NULL, NULL, },
		/* third line of description of current screen  */
		{ { BAGL_LABELINE, 0x02, 10, 30, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, curr_tx_desc[2], 0, 0, 0, NULL, NULL, NULL, },
		/* left icon is up arrow  */
		{ { BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL, 0, 0, 0, NULL, NULL, NULL, },
		/* right icon is down arrow */
		{ { BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL, 0, 0, 0, NULL, NULL, NULL, },
/* */
};

static unsigned int bagl_ui_tx_desc_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn();
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up();
		break;
	}
	return 0;
}

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
	// Go back to the dashboard
	os_sched_exit(0);
	return NULL; // do not redraw the widget
}

static void copy_tx_desc(void) {
	os_memmove(curr_tx_desc, tx_desc[curr_scr_ix], CURR_TX_DESC_LEN);
}

static void tx_desc_up(void) {
	switch (uiState) {
	case UI_TOP_SIGN:
		ui_deny();
		break;

	case UI_TX_DESC:
		if (curr_scr_ix == 0) {
			ui_top_sign();
		} else {
			curr_scr_ix--;
			copy_tx_desc();
			ui_display_tx_desc();
		}
		break;

	case UI_SIGN:
		curr_scr_ix = max_scr_ix - 1;
		copy_tx_desc();
		ui_display_tx_desc();
		break;

	case UI_DENY:
		ui_sign();
		break;

	default:
		THROW(0x6D02);
		break;
	}
}

static void tx_desc_dn(void) {
	switch (uiState) {
	case UI_TOP_SIGN:
		curr_scr_ix = 0;
		copy_tx_desc();
		ui_display_tx_desc();
		break;

	case UI_TX_DESC:
		if (curr_scr_ix == max_scr_ix - 1) {
			ui_sign();
		} else {
			curr_scr_ix++;
			copy_tx_desc();
			ui_display_tx_desc();
		}
		break;

	case UI_SIGN:
		ui_deny();
		break;

	default:
		THROW(0x6D01);
		break;
	}
}

const bagl_element_t*io_seproxyhal_touch_approve(const bagl_element_t *e) {
	unsigned int tx = 0;
	unsigned int len = get_apdu_buffer_length();
	unsigned char * in = G_io_apdu_buffer + APDU_HEADER_LENGTH;

	// Update the hash
	cx_hash(&hash.header, 0, in, len, NULL);
	if (G_io_apdu_buffer[2] == P1_LAST) {
		// Hash is finalized, send back the signature
		unsigned char result[32];

		cx_hash(&hash.header, CX_LAST, G_io_apdu_buffer, 0, result);
		tx = cx_ecdsa_sign((void*) &N_privateKey, CX_RND_RFC6979 | CX_LAST, CX_SHA256, result, sizeof(result), G_io_apdu_buffer);
		// G_io_apdu_buffer[0] &= 0xF0; // discard the parity information
		hashTainted = 1;
		raw_tx_ix = 0;
		raw_tx_len = 0;

		// add hash to the response, so we can see where the bug is.
		G_io_apdu_buffer[tx++] = 0xFF;
		G_io_apdu_buffer[tx++] = 0xFF;
		for (int ix = 0; ix < 32; ix++) {
			G_io_apdu_buffer[tx++] = result[ix];
		}
	}
	G_io_apdu_buffer[tx++] = 0x90;
	G_io_apdu_buffer[tx++] = 0x00;
	// Send back the response, do not restart the event loop
	io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
	// Display back the original UX
	ui_idle();
	return 0; // do not redraw the widget
}

static const bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e) {
	hashTainted = 1;
	raw_tx_ix = 0;
	raw_tx_len = 0;
	G_io_apdu_buffer[0] = 0x69;
	G_io_apdu_buffer[1] = 0x85;
	// Send back the response, do not restart the event loop
	io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
	// Display back the original UX
	ui_idle();
	return 0; // do not redraw the widget
}

void ui_idle(void) {
	uiState = UI_IDLE;
	UX_DISPLAY(bagl_ui_idle_nanos, NULL);
}

static void ui_display_tx_desc(void) {
	uiState = UI_TX_DESC;
	UX_DISPLAY(bagl_ui_tx_desc_nanos, NULL);
}

static void ui_sign(void) {
	uiState = UI_SIGN;
	UX_DISPLAY(bagl_ui_sign_nanos, NULL);
}

void ui_top_sign(void) {
	uiState = UI_TOP_SIGN;
	UX_DISPLAY(bagl_ui_top_sign_nanos, NULL);
}

static void ui_deny(void) {
	uiState = UI_DENY;
	UX_DISPLAY(bagl_ui_deny_nanos, NULL);
}

unsigned int get_apdu_buffer_length() {
	unsigned int len0 = G_io_apdu_buffer[APDU_BODY_LENGTH_OFFSET];
	return len0 / 2;
}
