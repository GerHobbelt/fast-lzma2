/*
 * Copyright (c) 2017-present, Conor McCarthy
 * All rights reserved.
 * Based on zstd.h copyright Yann Collet
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
*/
#if defined (__cplusplus)
extern "C" {
#endif

#ifndef FAST_LZMA2_H
#define FAST_LZMA2_H

/* ======   Dependency   ======*/
#include <stddef.h>   /* size_t */


/* =====   FL2LIB_API : control library symbols visibility   ===== */
#ifndef FL2LIB_VISIBILITY
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#    define FL2LIB_VISIBILITY __attribute__ ((visibility ("default")))
#  else
#    define FL2LIB_VISIBILITY
#  endif
#endif
#if defined(FL2_DLL_EXPORT) && (FL2_DLL_EXPORT==1)
#  define FL2LIB_API __declspec(dllexport) FL2LIB_VISIBILITY
#elif defined(FL2_DLL_IMPORT) && (FL2_DLL_IMPORT==1)
#  define FL2LIB_API __declspec(dllimport) FL2LIB_VISIBILITY /* It isn't required but allows to generate better code, saving a function pointer load from the IAT and an indirect jump.*/
#else
#  define FL2LIB_API FL2LIB_VISIBILITY
#endif

/* ======   Calling convention   ======*/

#if !defined _WIN32 || defined __x86_64__s || defined _M_X64 || (defined __SIZEOF_POINTER__ && __SIZEOF_POINTER__ == 8)
#  define FL2LIB_CALL
#elif defined(__GNUC__)
#  define FL2LIB_CALL __attribute__((cdecl))
#elif defined(_MSC_VER)
#  define FL2LIB_CALL __cdecl
#else
#  define FL2LIB_CALL
#endif

/*******************************************************************************************************
Introduction

*********************************************************************************************************/

/*------   Version   ------*/
#define FL2_VERSION_MAJOR    1
#define FL2_VERSION_MINOR    0
#define FL2_VERSION_RELEASE  0

#define FL2_VERSION_NUMBER  (FL2_VERSION_MAJOR *100*100 + FL2_VERSION_MINOR *100 + FL2_VERSION_RELEASE)
FL2LIB_API unsigned FL2LIB_CALL FL2_versionNumber(void);   /**< useful to check dll version */

#define FL2_LIB_VERSION FL2_VERSION_MAJOR.FL2_VERSION_MINOR.FL2_VERSION_RELEASE
#define FL2_QUOTE(str) #str
#define FL2_EXPAND_AND_QUOTE(str) FL2_QUOTE(str)
#define FL2_VERSION_STRING FL2_EXPAND_AND_QUOTE(FL2_LIB_VERSION)
FL2LIB_API const char* FL2LIB_CALL FL2_versionString(void);


/***************************************
*  Simple API
***************************************/

#define FL2_MAXTHREADS 200

/*! FL2_compress() :
 *  Compresses `src` content as a single LZMA2 compressed stream into already allocated `dst`.
 *  Call FL2_compressMt() to use > 1 thread. Specify nbThreads = 0 to use all cores.
 *  @return : compressed size written into `dst` (<= `dstCapacity),
 *            or an error code if it fails (which can be tested using FL2_isError()). */
FL2LIB_API size_t FL2LIB_CALL FL2_compress(void* dst, size_t dstCapacity,
    const void* src, size_t srcSize,
    int compressionLevel);

FL2LIB_API size_t FL2LIB_CALL FL2_compressMt(void* dst, size_t dstCapacity,
    const void* src, size_t srcSize,
    int compressionLevel,
    unsigned nbThreads);

/*! FL2_decompress() :
 *  `compressedSize` : must be the _exact_ size of some number of compressed and/or skippable frames.
 *  `dstCapacity` is an upper bound of originalSize to regenerate.
 *  If user cannot imply a maximum upper bound, it's better to use streaming mode to decompress data.
 *  @return : the number of bytes decompressed into `dst` (<= `dstCapacity`),
 *            or an errorCode if it fails (which can be tested using FL2_isError()). */
FL2LIB_API size_t FL2LIB_CALL FL2_decompress(void* dst, size_t dstCapacity,
    const void* src, size_t compressedSize);
FL2LIB_API size_t FL2LIB_CALL FL2_decompressMt(void* dst, size_t dstCapacity,
    const void* src, size_t compressedSize,
    unsigned nbThreads);

/*! FL2_findDecompressedSize()
 *  `src` should point to the start of a LZMA2 encoded stream.
 *  `srcSize` must be at least as large as the LZMA2 stream including end marker.
 *  @return : - decompressed size of the stream in `src`, if known
 *            - FL2_CONTENTSIZE_ERROR if an error occurred (e.g. corruption, srcSize too small)
 *   note 1 : a 0 return value means the frame is valid but "empty".
 *   note 2 : decompressed size can be very large (64-bits value),
 *            potentially larger than what local system can handle as a single memory segment.
 *            In which case, it's necessary to use streaming mode to decompress data.
 *   note 5 : If source is untrusted, decompressed size could be wrong or intentionally modified.
 *            Always ensure return value fits within application's authorized limits.
 *            Each application can set its own limits. */
#define FL2_CONTENTSIZE_ERROR (size_t)-1
FL2LIB_API size_t FL2LIB_CALL FL2_findDecompressedSize(const void *src, size_t srcSize);


/*======  Helper functions  ======*/
#define FL2_COMPRESSBOUND(srcSize)   ((srcSize) + (((srcSize) + 0xFFF) / 0x1000) * 3 + 6)  /* this formula calculates the maximum size of data stored in uncompressed chunks */
FL2LIB_API size_t      FL2LIB_CALL FL2_compressBound(size_t srcSize); /*!< maximum compressed size in worst case scenario */
FL2LIB_API unsigned    FL2LIB_CALL FL2_isError(size_t code);          /*!< tells if a `size_t` function result is an error code */
FL2LIB_API unsigned    FL2LIB_CALL FL2_isTimedOut(size_t code);       /*!< tells if a `size_t` function result is the timeout code */
FL2LIB_API const char* FL2LIB_CALL FL2_getErrorName(size_t code);     /*!< provides readable string from an error code */
FL2LIB_API int         FL2LIB_CALL FL2_maxCLevel(void);               /*!< maximum compression level available */
FL2LIB_API int         FL2LIB_CALL FL2_maxHighCLevel(void);           /*!< maximum compression level available in high mode */

/***************************************
*  Explicit memory management
***************************************/
/*= Compression context
 *  When compressing many times,
 *  it is recommended to allocate a context just once, and re-use it for each successive compression operation.
 *  This will make workload friendlier for system's memory.
 *  The context may not use the number of threads requested if the library is compiled for single-threaded
 *  compression or nbThreads > FL2_MAXTHREADS. Call FL2_CCtx_nbThreads to obtain the actual number. */
typedef struct FL2_CCtx_s FL2_CCtx;
FL2LIB_API FL2_CCtx* FL2LIB_CALL FL2_createCCtx(void);
FL2LIB_API FL2_CCtx* FL2LIB_CALL FL2_createCCtxMt(unsigned nbThreads);
FL2LIB_API void      FL2LIB_CALL FL2_freeCCtx(FL2_CCtx* cctx);

FL2LIB_API unsigned FL2LIB_CALL FL2_CCtx_nbThreads(const FL2_CCtx* ctx);

/*! FL2_compressCCtx() :
 *  Same as FL2_compress(), requires an allocated FL2_CCtx (see FL2_createCCtx()). */
FL2LIB_API size_t FL2LIB_CALL FL2_compressCCtx(FL2_CCtx* ctx,
    void* dst, size_t dstCapacity,
    const void* src, size_t srcSize,
    int compressionLevel);

/*! FL2_getCCtxDictProp() :
 *  Get the dictionary size property.
 *  Intended for use with the FL2_p_omitProperties parameter for creating a
 *  7-zip or XZ compatible LZMA2 stream. */
FL2LIB_API unsigned char FL2LIB_CALL FL2_getCCtxDictProp(FL2_CCtx* ctx);

/****************************
*  Decompression
****************************/


/*= Decompression context
 *  When decompressing many times,
 *  it is recommended to allocate a context only once,
 *  and re-use it for each successive compression operation.
 *  This will make the workload friendlier for the system's memory.
 *  Use one context per thread for parallel execution. */
typedef struct FL2_DCtx_s FL2_DCtx;
FL2LIB_API FL2_DCtx* FL2LIB_CALL FL2_createDCtx(void);
FL2LIB_API FL2_DCtx* FL2LIB_CALL FL2_createDCtxMt(unsigned nbThreads);
FL2LIB_API size_t    FL2LIB_CALL FL2_freeDCtx(FL2_DCtx* dctx);

/*! FL2_initDCtx() :
 *  Use only when a property byte is not present at input byte 0.
 *  The caller must store the result from FL2_getCCtxDictProp() and pass it to this function. */
FL2LIB_API size_t FL2LIB_CALL FL2_initDCtx(FL2_DCtx* dctx, unsigned char prop);

/*! FL2_decompressDCtx() :
 *  Same as FL2_decompress(), requires an allocated FL2_DCtx (see FL2_createDCtx()) */
FL2LIB_API size_t FL2LIB_CALL FL2_decompressDCtx(FL2_DCtx* ctx,
    void* dst, size_t dstCapacity,
    const void* src, size_t srcSize);

/****************************
*  Streaming
****************************/

typedef struct FL2_inBuffer_s {
    const void* src;    /**< start of input buffer */
    size_t size;        /**< size of input buffer */
    size_t pos;         /**< position where reading stopped. Will be updated. Necessarily 0 <= pos <= size */
} FL2_inBuffer;

typedef struct FL2_outBuffer_s {
    void*  dst;         /**< start of output buffer */
    size_t size;        /**< size of output buffer */
    size_t pos;         /**< position where writing stopped. Will be updated. Necessarily 0 <= pos <= size */
} FL2_outBuffer;



/*-***********************************************************************
 *  Streaming compression - HowTo
 *
 *  A FL2_CStream object is required to track streaming operation.
 *  Use FL2_createCStream() and FL2_freeCStream() to create/release resources.
 *  FL2_CStream objects can be reused multiple times on consecutive compression operations.
 *  It is recommended to re-use FL2_CStream in situations where many streaming operations will be achieved consecutively,
 *  since it will play nicer with system's memory, by re-using already allocated memory.
 *
 *  Start a new compression by initializing FL2_CStream.
 *  Use FL2_initCStream() to start a new compression operation.
 *
 *  Use FL2_compressStream() repetitively to consume input stream.
 *  The function will automatically update the `pos` field.
 *  It will always consume the entire input unless an error occurs or the dictionary buffer is filled,
 *  unlike the decompression function.
 *  @return : Nonzero if the CStream can accept more input,
 *            zero if compressed output must be read,
 *            or an error code, which can be tested using FL2_isError().
 *
 *  The radix match finder allows compressed data to be stored in its match table during encoding.
 *  When FL2_compressStream returns zero, the compressed data must be read from
 *  these internal buffers. Call FL2_getNextCStreamBuffer() repeatedly until it returns zero.
 *  Each call returns buffer information in the FL2_inBuffer parameter. Applications typically will 
 *  passed this to an I/O write function or downstream filter.
 *  Alternately, applications may call FL2_getCStreamOutput() to copy all of the compressed data to
 *  a buffer supplied by the application.
 *
 *  At any moment, it's possible, but not recommended, to compress whatever data remains
 *  within internal buffers using FL2_flushStream(). It may be necessary to call it twice if the
 *  CStream was created with dual dictionary buffers.
 *  Note 1 : this will reduce compression ratio because the algorithm is block-based.
 *  Note 2 : compressed content must be read from the CStream object after each call.
 *  @return : nb of bytes still present within internal buffers (0 if they're empty)
 *            or an error code, which can be tested using FL2_isError().
 *
 *  FL2_endStream() instructs to finish a frame.
 *  It will perform a flush and write the LZMA2 termination byte (required).
 *  FL2_endStream() may not be able to flush full data if the CStream was created with dual dictionary
 *  buffers. In which case, read all compressed data from the CStream and call again FL2_endStream().
 *  @return : 0 if stream fully completed and flushed,
 *  or >0 to indicate the nb of bytes still present within the internal buffers,
 *  or an error code, which can be tested using FL2_isError().
 *
 * *******************************************************************/

typedef struct FL2_CCtx_s FL2_CStream;

/*===== FL2_CStream management functions =====*/
FL2LIB_API FL2_CStream* FL2LIB_CALL FL2_createCStream(void);
FL2LIB_API FL2_CStream* FL2LIB_CALL FL2_createCStreamMt(unsigned nbThreads, int dualBuffer);

static void FL2_freeCStream(FL2_CStream * fcs) {
    FL2_freeCCtx(fcs);
}

/*===== Streaming compression functions =====*/
FL2LIB_API size_t FL2LIB_CALL FL2_initCStream(FL2_CStream* fcs, int compressionLevel);

/*! FL2_setCStreamTimeout() :
 *  Sets a timeout in milliseconds. Zero disables the timeout. Functions FL2_compressStream(),
 *  FL2_updateDictionary(), FL2_flushStream(), and FL2_endStream() may return a timeout code
 *  before compression of the current dictionary of data completes. FL2_isError returns true
 *  for the timeout code, so check for it using FL2_isTimedOut() before testing for errors.
 *  Functions EXCEPT FL2_updateDictionary() may be called again after the timeout. A typical
 *  application for timeouts is to update the user on compression progress. */
FL2LIB_API size_t FL2LIB_CALL FL2_setCStreamTimeout(FL2_CStream * fcs, unsigned timeout);

/*! FL2_compressStream() :
 *  Reads data from input into the dictionary buffer. Compression will begin if the buffer fills up.
 *  Streams created for dual buffering will fill the second buffer from input and return if all
 *  input is consumed. A call to FL2_compressStream() will block when all dictionary space is
 *  filled. FL2_compressStream() must not be called again until all compressed data is read.
 *  Returns zero to indicte compressed data must be read, or nonzero otherwise. */
FL2LIB_API size_t FL2LIB_CALL FL2_compressStream(FL2_CStream* fcs, FL2_inBuffer* input);

/*! FL2_getDictionaryBuffer() :
 *  Returns a buffer in the FL2_outBuffer object, which the caller can directly read data into.
 *  Applications will normally pass this buffer to an I/O read function or upstream filter.
 *  Returns the available size (equal to dict.size), or an error or timeout code. */
FL2LIB_API size_t FL2LIB_CALL FL2_getDictionaryBuffer(FL2_CStream* fcs, FL2_outBuffer* dict);

/*! FL2_updateDictionary() :
 *  Informs the CStream how much data was added to the buffer.*/
FL2LIB_API size_t FL2LIB_CALL FL2_updateDictionary(FL2_CStream* fcs, size_t addedSize);

/*! FL2_getCStreamProgress() :
 *  Returns the number of bytes processed since the stream was initialized. This is a synthetic
 *  estimate because the match finder does not proceed sequentially through the data. */
FL2LIB_API unsigned long long FL2LIB_CALL FL2_getCStreamProgress(const FL2_CStream * fcs);

/*! FL2_waitStream() :
 *  Waits for compression to end. This function returns after the timeout set using
 *  FL2_setCStreamTimeout has elapsed. Unnecessary when no timeout is set. */
FL2LIB_API size_t FL2LIB_CALL FL2_waitStream(FL2_CStream * fcs);

/*! FL2_cancelOperation() :
 *  Cancels any compression operation underway. Useful only when dual buffering and/or timeouts
 *  are enabled. */
FL2LIB_API void FL2LIB_CALL FL2_cancelOperation(FL2_CStream *fcs);

/*! FL2_remainingOutputSize() :
 *  The amount of compressed data remaining to be read from the CStream object. */
FL2LIB_API size_t FL2LIB_CALL FL2_remainingOutputSize(const FL2_CStream* fcs);

/*! FL2_getNextCStreamBuffer() :
 *  Returns a buffer containing a slice of the compressed data. Call this function and process the
 *  data until the function returns zero. In most cases it will return a buffer for each compression
 *  thread used. It is sometimes less but never more than that. */
FL2LIB_API size_t FL2LIB_CALL FL2_getNextCStreamBuffer(FL2_CStream* fcs, FL2_inBuffer* cbuf);

/*! FL2_getCStreamOutput() :
 *  Copies the compressed data to a single buffer, which must have a capacity of at least
 *  FL2_remainingOutputSize() bytes. */
FL2LIB_API size_t FL2LIB_CALL FL2_getCStreamOutput(FL2_CStream* fcs, void *dst, size_t dstCapacity);

/*! FL2_flushStream() :
 *  Compress all data remaining in the dictionary buffer(s). With dual buffers it may be necessary
 *  to call FL2_flushStream() twice and read the compressed data in between. Flushing is not
 *  normally useful and produces larger output.
 *  Returns amount of compressed data to be read from the CStream object. */
FL2LIB_API size_t FL2LIB_CALL FL2_flushStream(FL2_CStream* fcs);

/*! FL2_endStream() :
 *  Compress all data remaining in the dictionary buffer(s) and write the stream end marker. With
 *  dual buffers it may be necessary to call FL2_endStream() twice and read the compressed data
 *  each time.
 *  Returns zero when compression is complete and the final output can be read. */
FL2LIB_API size_t FL2LIB_CALL FL2_endStream(FL2_CStream* fcs);

/*-***************************************************************************
 *  Streaming decompression - HowTo
 *
 *  A FL2_DStream object is required to track streaming operations.
 *  Use FL2_createDStream() and FL2_freeDStream() to create/release resources.
 *  FL2_DStream objects can be re-used multiple times.
 *
 *  Use FL2_initDStream() to start a new decompression operation.
 *   @return : recommended first input size
 *
 *  Use FL2_decompressStream() repetitively to consume your input.
 *  The function will update both `pos` fields.
 *  If `input.pos < input.size`, some input has not been consumed.
 *  It's up to the caller to present again remaining data.
 *  More data must be loaded if `input.pos + LZMA_REQUIRED_INPUT_MAX >= input.size`
 *  If `output.pos < output.size`, decoder has flushed everything it could.
 *  @return : 0 when a frame is completely decoded and fully flushed,
 *            an error code, which can be tested using FL2_isError(),
 *            1, which means there is still some decoding to do to complete current frame.
 * *******************************************************************************/

#define LZMA_REQUIRED_INPUT_MAX 20

typedef struct FL2_DStream_s FL2_DStream;

/*===== FL2_DStream management functions =====*/
FL2LIB_API FL2_DStream* FL2LIB_CALL FL2_createDStream(void);
FL2LIB_API FL2_DStream* FL2LIB_CALL FL2_createDStreamMt(unsigned nbThreads);
FL2LIB_API size_t FL2LIB_CALL FL2_freeDStream(FL2_DStream* fds);

FL2LIB_API size_t FL2LIB_CALL FL2_setDStreamTimeout(FL2_DStream * fds, unsigned timeout);
FL2LIB_API size_t FL2LIB_CALL FL2_waitDStream(FL2_DStream * fds);
FL2LIB_API unsigned long long FL2LIB_CALL FL2_getDStreamProgress(const FL2_DStream * fds);

/*===== Streaming decompression functions =====*/
FL2LIB_API size_t FL2LIB_CALL FL2_initDStream(FL2_DStream* fds);
FL2LIB_API size_t FL2LIB_CALL FL2_initDStream_withProp(FL2_DStream* fds, unsigned char prop);
FL2LIB_API size_t FL2LIB_CALL FL2_decompressStream(FL2_DStream* fds, FL2_outBuffer* output, FL2_inBuffer* input);

/*-***************************************************************************
 *  Compression parameters - HowTo
 *
 *  Any function that takes a 'compressionLevel' parameter will replace any
 *  parameters affected by compression level that are already set.
 *  Call FL2_CCtx_setParameter with FL2_p_compressionLevel to set the level,
 *  then call FL2_CCtx_setParameter again with any other settings to change.
 *  Specify compressionLevel=0 when calling a compression function.
 * *******************************************************************************/

#define FL2_DICTLOG_MAX_32   27
#define FL2_DICTLOG_MAX_64   30
#define FL2_DICTLOG_MAX      ((unsigned)(sizeof(size_t) == 4 ? FL2_DICTLOG_MAX_32 : FL2_DICTLOG_MAX_64))
#define FL2_DICTLOG_MIN      20
#define FL2_DICTSIZE_MAX     (1U << FL2_DICTLOG_MAX)
#define FL2_DICTSIZE_MIN     (1U << FL2_DICTLOG_MIN)
#define FL2_CHAINLOG_MAX       14
#define FL2_CHAINLOG_MIN       4
#define FL2_SEARCHLOG_MAX     (FL2_CHAINLOG_MAX-1)
#define FL2_SEARCHLOG_MIN       0
#define FL2_FASTLENGTH_MIN    6   /* only used by optimizer */
#define FL2_FASTLENGTH_MAX  273   /* only used by optimizer */
#define FL2_BLOCK_OVERLAP_MIN 0
#define FL2_BLOCK_OVERLAP_MAX 14
#define FL2_BLOCK_MUL_MIN 2
#define FL2_BLOCK_MUL_MAX 16      /* small enough to fit FL2_DICTSIZE_MAX * FL2_BLOCK_MUL_MAX in 32-bit size_t */
#define FL2_SEARCH_DEPTH_MIN 6
#define FL2_SEARCH_DEPTH_MAX 254
#define FL2_BUFFER_SIZE_LOG_MIN 6
#define FL2_BUFFER_SIZE_LOG_MAX 12
#define FL2_LC_MIN 0
#define FL2_LC_MAX 4
#define FL2_LP_MIN 0
#define FL2_LP_MAX 4
#define FL2_PB_MIN 0
#define FL2_PB_MAX 4
#define FL2_LCLP_MAX 4

typedef enum {
    FL2_fast,
    FL2_opt,
    FL2_ultra
} FL2_strategy;

typedef struct {
    unsigned dictionaryLog;    /* largest match distance : larger == more compression, more memory needed during decompression; >= 27 == more memory, slower */
    unsigned overlapFraction;  /* overlap between consecutive blocks in 1/16 units: larger == more compression, slower */
    unsigned chainLog;         /* fully searched segment : larger == more compression, slower, more memory; hybrid mode only (ultra) */
    unsigned searchLog;        /* nb of searches : larger == more compression, slower; hybrid mode only (ultra) */
    unsigned searchDepth;      /* maximum depth for resolving string matches : larger == more compression, slower; >= 64 == more memory, slower */
    unsigned fastLength;       /* acceptable match size for parser, not less than searchDepth : larger == more compression, slower; fast bytes parameter from 7-zip */
    unsigned divideAndConquer; /* split long chains of 2-byte matches into shorter chains with a small overlap : faster, somewhat less compression; enabled by default */
    unsigned bufferLog;        /* buffer size for processing match chains is (dictionaryLog - bufferLog) : when divideAndConquer enabled, affects compression; */
                               /* when divideAndConquer disabled, affects speed in a hardware-dependent manner */
    FL2_strategy strategy;     /* encoder strategy : fast, optimized or ultra (hybrid) */
} FL2_compressionParameters;

typedef enum {
    /* compression parameters */
    FL2_p_compressionLevel, /* Update all compression parameters according to pre-defined cLevel table
                              * Default level is FL2_CLEVEL_DEFAULT==9.
                              * Setting FL2_p_highCompression to 1 switches to an alternate cLevel table.
                              * Special: value 0 means "do not change cLevel". */
    FL2_p_highCompression,  /* Maximize compression ratio for a given dictionary size.
                              * Has 9 levels instead of 12, with dictionaryLog 20 - 28. */
    FL2_p_dictionaryLog,    /* Maximum allowed back-reference distance, expressed as power of 2.
                              * Must be clamped between FL2_DICTLOG_MIN and FL2_DICTLOG_MAX.
                              * Special: value 0 means "do not change dictionaryLog". */
    FL2_p_dictionarySize,
    FL2_p_overlapFraction,  /* The radix match finder is block-based, so some overlap is retained from
                             * each block to improve compression of the next. This value is expressed
                             * as n / 16 of the block size (dictionary size). Larger values are slower.
                             * Values above 2 mostly yield only a small improvement in compression. */
    FL2_p_blockSizeMultiplier,/* Block size for multithreaded decompression. A dictionary reset will occur
                               after each 2 ^ blockSizeLog bytes of input. */
    FL2_p_bufferLog,        /* Buffering speeds up the matchfinder. Buffer size is 
                             * 2 ^ (dictionaryLog - bufferLog). Lower number = slower, better compression,
                             * higher memory usage. */
    FL2_p_chainLog,         /* Size of the full-search table, as a power of 2.
                              * Resulting table size is (1 << (chainLog+2)).
                              * Larger tables result in better and slower compression.
                              * This parameter is useless when using "fast" strategy.
                              * Special: value 0 means "do not change chainLog". */
    FL2_p_searchLog,        /* Number of search attempts, as a power of 2, made by the HC3 match finder
                              * used only in hybrid mode.
                              * More attempts result in slightly better and slower compression.
                              * This parameter is not used by the "fast" and "optimize" strategies.
                              * Special: value 0 means "do not change searchLog". */
    FL2_p_literalCtxBits,   /* lc value for LZMA2 encoder */
    FL2_p_literalPosBits,   /* lp value for LZMA2 encoder */
    FL2_p_posBits,          /* pb value for LZMA2 encoder */
    FL2_p_searchDepth,      /* Match finder will resolve string matches up to this length. If a longer
                             * match exists further back in the input, it will not be found. */
    FL2_p_fastLength,       /* Only useful for strategies >= opt.
                             * Length of Match considered "good enough" to stop search.
                             * Larger values make compression stronger and slower.
                             * Special: value 0 means "do not change fastLength". */
    FL2_p_divideAndConquer, /* Split long chains of 2-byte matches into shorter chains with a small overlap
                             * during further processing. Allows buffering of all chains at length 2.
                             * Faster, less compression. Generally a good tradeoff. Enabled by default. */
    FL2_p_strategy,         /* 1 = fast; 2 = optimize, 3 = ultra (hybrid mode).
                             * The higher the value of the selected strategy, the more complex it is,
                             * resulting in stronger and slower compression.
                             * Special: value 0 means "do not change strategy". */
#ifndef NO_XXHASH
    FL2_p_doXXHash,         /* Calculate a 32-bit xxhash value from the input data and store it 
                             * after the stream terminator. The value will be checked on decompression.
                             * 0 = do not calculate; 1 = calculate (default) */
#endif
    FL2_p_omitProperties,   /* Omit the property byte at the start of the stream. For use within 7-zip */
                            /* or other containers which store the property byte elsewhere. */
                            /* Cannot be decoded by this library. */
#ifdef RMF_REFERENCE
    FL2_p_useReferenceMF    /* Use the reference matchfinder for development purposes. SLOW. */
#endif
} FL2_cParameter;


/*! FL2_CCtx_setParameter() :
 *  Set one compression parameter, selected by enum FL2_cParameter.
 *  @result : informational value (typically, the one being set, possibly corrected),
 *            or an error code (which can be tested with FL2_isError()). */
FL2LIB_API size_t FL2LIB_CALL FL2_CCtx_setParameter(FL2_CCtx* cctx, FL2_cParameter param, unsigned value);

FL2LIB_API size_t FL2LIB_CALL FL2_CCtx_getParameter(FL2_CCtx* cctx, FL2_cParameter param);

static size_t FL2_CStream_setParameter(FL2_CStream* fcs, FL2_cParameter param, unsigned value) {
    return FL2_CCtx_setParameter(fcs, param, value);
}

static size_t FL2_CStream_getParameter(FL2_CStream* fcs, FL2_cParameter param) {
    return FL2_CCtx_getParameter(fcs, param);
}

FL2LIB_API size_t FL2LIB_CALL FL2_getLevelParameters(int compressionLevel, int high, FL2_compressionParameters *params);

/***************************************
*  Context memory usage
***************************************/

/*! FL2_estimate*() :
*  These functions estimate memory usage of a CCtx before its creation or before any operation has begun.
*  FL2_estimateCCtxSize() will provide a budget large enough for any compression level up to selected one.
*  To use FL2_estimateCCtxSize_usingCCtx, set the compression level and any other settings for the context,
*  then call the function. Some allocation occurs when the context is created, but the large memory buffers
*  used for string matching are allocated only when compression begins. */

FL2LIB_API size_t FL2LIB_CALL FL2_estimateCCtxSize(int compressionLevel, unsigned nbThreads); /*!< memory usage determined by level */
FL2LIB_API size_t FL2LIB_CALL FL2_estimateCCtxSize_byParams(const FL2_compressionParameters *params, unsigned nbThreads); /*!< memory usage determined by params */
FL2LIB_API size_t FL2LIB_CALL FL2_estimateCCtxSize_usingCCtx(const FL2_CCtx* cctx);           /*!< memory usage determined by settings */
FL2LIB_API size_t FL2LIB_CALL FL2_estimateCStreamSize(int compressionLevel, unsigned nbThreads, int dualBuffer);
FL2LIB_API size_t FL2LIB_CALL FL2_estimateCStreamSize_byParams(const FL2_compressionParameters *params, unsigned nbThreads, int dualBuffer);

static size_t FL2_estimateCStreamSize_usingCStream(const FL2_CStream* fcs) {
    return FL2_estimateCCtxSize_usingCCtx(fcs);
}

/*! FL2_getDictSizeFromProp() :
 *  Get the dictionary size from the property byte for a stream. The property byte is the first byte
*   in the stream, unless omitProperties was enabled, in which case the caller must store it. */
FL2LIB_API size_t FL2LIB_CALL FL2_getDictSizeFromProp(unsigned char prop);

/*! FL2_estimateDCtxSize() :
 *  The size of a DCtx does not include a dictionary buffer because the caller must supply one. */
FL2LIB_API size_t FL2LIB_CALL FL2_estimateDCtxSize(unsigned nbThreads);

/*! FL2_estimateDStreamSize() :
 *  Estimate decompression memory use from the dictionary size and number of threads.
 *  For nbThreads == 0 the number of available cores will be used.
 *  Obtain dictSize by passing the property byte to FL2_getDictSizeFromProp. */
FL2LIB_API size_t FL2LIB_CALL FL2_estimateDStreamSize(size_t dictSize, unsigned nbThreads); /*!<  from FL2_getDictSizeFromProp() */

#endif  /* FAST_LZMA2_H */

#if defined (__cplusplus)
}
#endif
