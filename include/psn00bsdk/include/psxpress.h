/*
 * PSn00bSDK MDEC library
 * (C) 2022 spicyjpeg - MPL licensed
 */

/**
 * @file psxpress.h
 * @brief MDEC library header
 *
 * @details This is a fully original reimplementation of the official SDK's
 * "data compression" library. This library is made up of two parts, the MDEC
 * API and functions to decompress Huffman-encoded bitstreams (.BS files, or
 * frames in .STR files) into data to be fed to the MDEC. Two different
 * implementations of the latter are provided, one using the GTE and scratchpad
 * region and an older one using a large lookup table in main RAM.
 *
 * FMV playback is not part of this library per se, but can implemented using
 * the APIs defined here alongside some code to stream data from the CD drive.
 *
 * Currently only version 1 and 2 .BS files are supported.
 */

#ifndef __PSXPRESS_H
#define __PSXPRESS_H

#include <stdint.h>
#include <stddef.h>

/* Structure definitions */

typedef struct _DECDCTENV {
	uint8_t iq_y[64];	// Luma quantization table, stored in zigzag order
	uint8_t iq_c[64];	// Chroma quantization table, stored in zigzag order
	int16_t dct[64];	// Inverse DCT matrix (2.14 fixed-point)
} DECDCTENV;

// This is the "small" lookup table used by DecDCTvlc(). It can be copied to
// the scratchpad.
typedef struct _DECDCTTAB {
	uint16_t	lut0[2];
	uint32_t	lut2[8];
	uint32_t	lut3[64];
	uint16_t	lut4[8];
	uint16_t	lut5[8];
	uint16_t	lut7[16];
	uint16_t	lut8[32];
	uint16_t	lut9[32];
	uint16_t	lut10[32];
	uint16_t	lut11[32];
	uint16_t	lut12[32];
} DECDCTTAB;

// This is the "large" table used by DecDCTvlc2().
typedef struct _DECDCTTAB2 {
	uint32_t	lut[8192];
	uint32_t	lut00[512];
} DECDCTTAB2;

typedef enum _DECDCTMODE {
	DECDCT_MODE_24BPP		= 1,
	DECDCT_MODE_16BPP		= 0,
	DECDCT_MODE_16BPP_BIT15	= 2,
	DECDCT_MODE_RAW			= -1
} DECDCTMODE;

typedef struct _VLC_Context {
	const uint32_t	*input;
	uint32_t		window, next_window, remaining;
	uint16_t		quant_scale;
	int8_t			is_v3, bit_offset, block_index, coeff_index;
} VLC_Context;

// Despite what some docs claim, the "number of 32-byte blocks" and "always
// 0x3800" fields are actually a single 32-bit field which is copied over to
// the output buffer, then parsed by DecDCTin() and written to the MDEC0
// register.
typedef struct {
	uint32_t mdec0_header;
	uint16_t quant_scale;
	uint16_t version;
} BS_Header;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resets and optionally initializes the MDEC.
 *
 * @details Resets the MDEC and aborts any MDEC DMA transfers. If mode = 0, the
 * default IDCT matrix and quantization tables are also loaded and the MDEC is
 * put into color output mode, discarding any custom environment previously set
 * with DecDCTPutEnv().
 *
 * DecDCTReset(0) must be called at least once prior to using the MDEC.
 *
 * @param mode
 *
 * @see DecDCTPutEnv()
 */
void DecDCTReset(int mode);

/**
 * @brief Loads default or custom quantization and IDCT tables into the MDEC.
 *
 * @details Uploads the specified decoding environment's quantization tables
 * and IDCT matrix to the MDEC, or restores the default tables if a null
 * pointer is passed. Calling this function is normally not required as
 * DecDCTReset(0) initializes the MDEC with the default tables, but it may be
 * useful for e.g. decoding JPEG or a format with custom quantization tables.
 *
 * The second argument, not present in the official SDK, specifies whether the
 * MDEC shall be put into color (0) or monochrome (1) output mode. In
 * monochrome mode each DCT block decoded from the input stream is transformed
 * into an 8x8x8bpp bitmap, while in color mode each group of 6 DCT blocks (Cr,
 * Cb, Y1-4) is used to form a 16x16 RGB bitmap.
 *
 * This function uses DecDCTinSync() to wait for the MDEC to become ready and
 * should not be called during decoding or after calling DecDCTin().
 *
 * @param env Pointer to DECDCTENV or 0 for default tables
 * @param mono 0 for color (normal), 1 for monochrome
 */
void DecDCTPutEnv(const DECDCTENV *env, int mono);

/**
 * @brief Feeds the MDEC with a run-length code buffer from the specified
 * location.
 *
 * @details Sets up the MDEC to start fetching and decoding the given buffer.
 * This function is meant to be used with buffers generated by DecDCTvlc(),
 * DecDCTvlc2() or their variants: the first 32-bit word of the buffer is
 * initially copied to the MDEC0 register, then all subsequent data is read in
 * 128-byte (32-word) chunks. The length of the stream (in 32-bit units, minus
 * the first word) is encoded by DecDCTvlc() in the lower 16 bits of the first
 * word.
 *
 * The mode argument optionally specifies the output color depth (0 for 16bpp,
 * 1 for 24bpp) if not already set in the first word. Passing -1 will result in
 * DecDCTin() copying the first word as-is to MDEC0 without manipulating any of
 * its bits.
 *
 * @param data
 * @param mode DECDCT_MODE_* or -1
 *
 * @see DecDCTinRaw(), DecDCTinSync()
 */
void DecDCTin(const uint32_t *data, int mode);

/**
 * @brief Feeds the MDEC with raw data from the specified location.
 *
 * @details Configures the MDEC to automatically fetch data (the input stream,
 * IDCT matrix or quantization tables) in 128-byte (32-word) chunks from the
 * specified address in main RAM. The transfer is stopped, and any callback
 * registered with DMACallback(0) is fired, once a certain number of 32-bit
 * words have been read; usually the length should match the number of input
 * words expected by the MDEC. If the MDEC expects more data its operation will
 * be paused and can be resumed by calling DecDCTinRaw() again.
 *
 * This is a low-level variant of DecDCTin() that only sets up the DMA transfer
 * and does not write anything to the MDEC0 register. The actual transfer won't
 * start until the MDEC is given a valid command.
 * 
 * @param data
 * @param length Number of 32-bit words to read (must be multiple of 32)
 *
 * @see DecDCTin(), DecDCTinSync()
 */
void DecDCTinRaw(const uint32_t *data, size_t length);

/**
 * @brief Waits for an MDEC input transfer to finish or returns its status.
 *
 * @details Waits for the MDEC to finish decoding the input stream (if
 * mode = 0) or returns whether it is busy (if mode = 1). MDEC commands can be
 * issued only when the MDEC isn't busy.
 *
 * WARNING: DecDCTinSync(0) might time out and return -1 if the MDEC can't
 * output decoded data, e.g. if the length passed DecDCTout() was too small and
 * no callback is registered to set up further transfers. DecDCTinSync(0) shall
 * only be used alongside DMACallback(1) or if the entirety of the decoded
 * stream (usually a whole frame) is being written to main RAM.
 *
 * @param mode
 * @return 0 or -1 in case of a timeout (mode = 0), MDEC busy flag (mode = 1)
 */
int DecDCTinSync(int mode);

/**
 * @brief Writes image data decoded by the MDEC to the specified location.
 *
 * @details Configures the MDEC to automatically transfer decoded image data in
 * 128-byte (32-word) chunks to the specified address in main RAM. MDEC
 * operation is paused once a certain number of 32-bit words have been output
 * and can be resumed by calling DecDCTout() again: the MDEC will continue
 * decoding the input stream from where it left off. Any callback registered
 * with DMACallback(1) is also fired whenever the transfer ends.
 *
 * This behavior allows the MDEC's output to be buffered into 16-pixel-wide
 * vertical strips in main RAM, which can then be uploaded to VRAM using
 * LoadImage().
 *
 * @param data
 * @param length Number of 32-bit words to output (must be multiple of 32)
 *
 * @see DecDCToutSync()
 */
void DecDCTout(uint32_t *data, size_t length);

/**
 * @brief Waits for an MDEC output transfer to finish or returns its status.
 *
 * @details Waits until the transfer set up by DecDCTout() finishes (if
 * mode = 0) or returns whether it is still in progress (if mode = 1).
 *
 * WARNING: DecDCToutSync(0) might time out and return -1 if the MDEC is unable
 * to consume enough input data in order to produce the desired amount of data.
 * If the input stream isn't contiguous in memory, DMACallback(0) shall be used
 * to register a callback that calls DecDCTin() to feed the MDEC.
 *
 * @param mode
 * @return 0 or -1 in case of a timeout (mode = 0), DMA busy flag (mode = 1)
 */
int DecDCToutSync(int mode);

/**
 * @brief Decompresses or begins decompressing a .BS file into MDEC codes.
 *
 * @details Begins decompressing the contents of a .BS file (or of a single STR
 * frame) into a buffer that can be passed to DecDCTin(). This function uses a
 * small (<1 KB) lookup table combined with the GTE to accelerate the process;
 * performance is roughly on par with DecDCTvlcStart2() if the lookup table
 * is copied to the scratchpad beforehand by calling DecDCTvlcCopyTable(). The
 * contents of the GTE's LZCR register, if any, will be destroyed.
 *
 * A VLC_Context object must be created and passed to this function, which will
 * then proceed to initialize its fields. The max_size argument sets the
 * maximum number of words that will be written to the output buffer; if more
 * data needs to be written, this function will return 1. To continue decoding
 * call DecDCTvlcContinue() with the same VLC_Context object (the output buffer
 * can be different). If max_size = 0, the entire frame will always be decoded
 * in one shot.
 *
 * Only bitstream version 2 is currently supported.
 *
 * WARNING: InitGeom() must be called prior to using DecDCTvlcStart() for the
 * first time. Attempting to call this function with the GTE disabled will
 * result in a crash.
 *
 * @param ctx Pointer to VLC_Context structure (which will be initialized)
 * @param buf
 * @param max_size Maximum number of 32-bit words to output
 * @param bs
 * @return 0, 1 if more data needs to be output or -1 in case of failure
 *
 * @see DecDCTvlcContinue(), DecDCTvlcCopyTable()
 */
int DecDCTvlcStart(VLC_Context *ctx, uint32_t *buf, size_t max_size, const uint32_t *bs);

/**
 * @brief Resumes or finishes decompressing a .BS file into MDEC codes.
 *
 * @details Resumes the decompression process started by DecDCTvlcStart(). The
 * state of the decompressor is contained entirely in the VLC_Context structure
 * so an arbitrary number of bitstreams can be decoded concurrently (although
 * the limited CPU power makes it impractical to do so) by keeping a separate
 * context for each bitstream.
 *
 * This function behaves like DecDCTvlcStart(), returning 1 if more data has to
 * be written or 0 otherwise. DecDCTvlcContinue() shall not be called after a
 * previous call to DecDCTvlcStart() or DecDCTvlcContinue() with the same
 * context returned 0; in that case the context shall be discarded or reused to
 * decode another bitstream.
 *
 * The contents of the GTE's LZCR register, if any, will be destroyed.
 *
 * See DecDCTvlcStart() for more details.
 *
 * @param ctx Pointer to already initialized VLC_Context structure
 * @param buf
 * @param max_size Maximum number of 32-bit words to output
 * @return 0, 1 if more data needs to be output or -1 in case of failure
 *
 * @see DecDCTvlcStart()
 */
int DecDCTvlcContinue(VLC_Context *ctx, uint32_t *buf, size_t max_size);

/**
 * @brief Decompresses a .BS file into MDEC codes.
 *
 * @details A wrapper around DecDCTvlcStart() and DecDCTvlcContinue() for
 * compatibility with the official SDK. This function uses an internal context;
 * additionally, the maximum output buffer size is not passed as an argument
 * but is instead set by calling DecDCTvlcSize().
 *
 * This function behaves identically to DecDCTvlcContinue() if bs = 0 and
 * DecDCTvlcStart() otherwise.
 *
 * See DecDCTvlcStart() for more details.
 *
 * WARNING: InitGeom() must be called prior to using DecDCTvlc() for the first
 * time. Attempting to call this function with the GTE disabled will result in
 * a crash.
 *
 * @param bs Pointer to bitstream data or 0 to resume decoding
 * @param buf
 * @return 0, 1 if more data needs to be output or -1 in case of failure
 *
 * @see DecDCTvlcSize(), DecDCTvlcCopyTable()
 */
int DecDCTvlc(const uint32_t *bs, uint32_t *buf);

/**
 * @brief Sets the maximum amount of data to be decompressed.
 *
 * @details Sets the maximum number of 32-bit words that a single call to
 * DecDCTvlc() will output. If size = 0, the entire frame will always be
 * decoded in one shot.
 *
 * Note that DecDCTvlcStart() and DecDCTvlcContinue() do not use the value set
 * by this function and instead expect the maximum size to be passed as an
 * argument.
 *
 * @param size  Maximum number of 32-bit words to output
 * @return Previously set value
 *
 * @see DecDCTvlc()
 */
size_t DecDCTvlcSize(size_t size);

/**
 * @brief Moves the lookup table used by the .BS decompressor to the scratchpad
 * region.
 *
 * @details Copies the small (<1 KB) lookup table used by DecDCTvlcContinue(),
 * DecDCTvlcStart() and DecDCTvlc() (a DECDCTTAB structure) to the specified
 * address. A copy of this table is always present in main RAM, however this
 * function can be used to copy it to the scratchpad region to boost
 * decompression performance.
 *
 * The address passed to this function is saved. Calls to DecDCTvlcStart(),
 * DecDCTvlcContinue() and DecDCTvlc() will automatically use the last table
 * copied. Call DecDCTvlcCopyTable(0) to revert to using the library's internal
 * table in main RAM.
 *
 * @param addr Pointer to free area in scratchpad region or 0 to reset
 */
void DecDCTvlcCopyTable(DECDCTTAB *addr);

/**
 * @brief Decompresses or begins decompressing a .BS file into MDEC codes
 * (alternate implementation).
 *
 * @details Begins decompressing the contents of a .BS file (or of a single STR
 * frame) into a buffer that can be passed to DecDCTin(). This function uses a
 * large (34 KB) lookup table that must be loaded into main RAM beforehand by
 * calling DecDCTvlcBuild(), but does not use the GTE nor the scratchpad.
 * Depending on the specific bitstream being decoded DecDCTvlcStart2() might be
 * slightly faster or slower than DecDCTvlcStart() with its lookup table copied
 * to the scratchpad (see DecDCTvlcCopyTable()). DecDCTvlcStart() with the
 * table in main RAM tends to be much slower.
 *
 * A VLC_Context object must be created and passed to this function, which will
 * then proceed to initialize its fields. The max_size argument sets the
 * maximum number of words that will be written to the output buffer; if more
 * data needs to be written, this function will return 1. To continue decoding
 * call DecDCTvlcContinue2() with the same VLC_Context object (the output
 * buffer can be different). If max_size = 0, the entire frame will always be
 * decoded in one shot.
 *
 * Only bitstream version 2 is currently supported.
 *
 * @param ctx Pointer to VLC_Context structure (which will be initialized)
 * @param buf
 * @param max_size Maximum number of 32-bit words to output
 * @param bs
 * @return 0, 1 if more data needs to be output or -1 in case of failure
 *
 * @see DecDCTvlcContinue2(), DecDCTvlcBuild()
 */
int DecDCTvlcStart2(VLC_Context *ctx, uint32_t *buf, size_t max_size, const uint32_t *bs);

/**
 * @brief Resumes or finishes decompressing a .BS file into MDEC codes
 * (alternate implementation).
 *
 * @details Resumes the decompression process started by DecDCTvlcStart2(). The
 * state of the decompressor is contained entirely in the VLC_Context structure
 * so an arbitrary number of bitstreams can be decoded concurrently (although
 * the limited CPU power makes it impractical to do so) by keeping a separate
 * context for each bitstream.
 *
 * This function behaves like DecDCTvlcStart2(), returning 1 if more data has
 * to be written or 0 otherwise. DecDCTvlcContinue2() shall not be called after
 * a previous call to DecDCTvlcStart2() or DecDCTvlcContinue2() with the same
 * context returned 0; in that case the context shall be discarded or reused to
 * decode another bitstream.
 *
 * See DecDCTvlcStart2() for more details.
 *
 * @param ctx Pointer to already initialized VLC_Context structure
 * @param buf
 * @param max_size Maximum number of 32-bit words to output
 * @return 0, 1 if more data needs to be output or -1 in case of failure
 *
 * @see DecDCTvlcStart2()
 */
int DecDCTvlcContinue2(VLC_Context *ctx, uint32_t *buf, size_t max_size);

/**
 * @brief Decompresses a .BS file into MDEC codes (alternate implementation).
 *
 * @details A wrapper around DecDCTvlcStart2() and DecDCTvlcContinue2() for
 * compatibility with the official SDK. This function uses an internal context;
 * additionally, the maximum output buffer size is not passed as an argument
 * but is instead set by calling DecDCTvlcSize2().
 *
 * This function behaves identically to DecDCTvlcContinue() if bs = 0 and
 * DecDCTvlcStart() otherwise. The table argument can optionally be passed to
 * use a custom lookup table. If zero, the last pointer passed to
 * DecDCTvlcBuild() will be used.
 *
 * See DecDCTvlcStart2() for more details.
 *
 * @param bs Pointer to bitstream data or 0 to resume decoding
 * @param buf
 * @param table Pointer to decompressed table or 0 to use last table used
 * @return 0, 1 if more data needs to be output or -1 in case of failure
 *
 * @see DecDCTvlcSize2(), DecDCTvlcBuild()
 */
int DecDCTvlc2(const uint32_t *bs, uint32_t *buf, DECDCTTAB2 *table);

/**
 * @brief Sets the maximum amount of data to be decompressed (alternate
 * implementation).
 *
 * @details Sets the maximum number of 32-bit words that a single call to
 * DecDCTvlc2() will output. If size = 0, the entire frame will always be
 * decoded in one shot.
 *
 * Note that DecDCTvlcStart2() and DecDCTvlcContinue2() do not use the value
 * set by this function and instead expect the maximum size to be passed as an
 * argument.
 *
 * @param size Maximum number of 32-bit words to output
 * @return Previously set value
 *
 * @see DecDCTvlc2()
 */
size_t DecDCTvlcSize2(size_t size);

/**
 * @brief Generates the lookup table used by the alternate implementation of
 * the .BS decompressor.
 *
 * @details Generates the lookup table required by DecDCTvlcStart2(),
 * DecDCTvlcContinue2() and DecDCTvlc2() (a DECDCTTAB2 structure) into the
 * specified buffer. Since the table is relatively large (34 KB), it is
 * recommended to only generate it in a dynamically-allocated buffer when
 * needed and deallocate the buffer afterwards.
 *
 * The address passed to this function is saved. Calls to DecDCTvlcStart2() and
 * DecDCTvlcContinue2() will automatically use the last table decompressed.
 *
 * @param table
 */
void DecDCTvlcBuild(DECDCTTAB2 *table);

#ifdef __cplusplus
}
#endif

#endif