/* This header file is in the public domain. */
#ifndef ENCA_H
#define ENCA_H

#include <stdlib.h>
/* According to autoconf stdlib may not be enough for size_t */
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Enumerated types */

/**
 * EncaSurface:
 * @ENCA_SURFACE_EOL_CR: End-of-lines are represented with CR's.
 * @ENCA_SURFACE_EOL_LF: End-of-lines are represented with LF's.
 * @ENCA_SURFACE_EOL_CRLF: End-of-lines are represented with CRLF's.
 * @ENCA_SURFACE_EOL_MIX: Several end-of-line types, mixed.
 * @ENCA_SURFACE_EOL_BIN: End-of-line concept not applicable (binary data).
 * @ENCA_SURFACE_MASK_EOL: Mask for end-of-line surfaces.
 * @ENCA_SURFACE_PERM_21: Odd and even bytes swapped.
 * @ENCA_SURFACE_PERM_4321: Reversed byte sequence in 4byte words.
 * @ENCA_SURFACE_PERM_MIX: Chunks with both endianess, concatenated.
 * @ENCA_SURFACE_MASK_PERM: Mask for permutation surfaces.
 * @ENCA_SURFACE_QP: Quoted printables.
 * @ENCA_SURFACE_REMOVE: Recode `remove' surface.
 * @ENCA_SURFACE_UNKNOWN: Unknown surface.
 * @ENCA_SURFACE_MASK_ALL: Mask for all bits, withnout #ENCA_SURFACE_UNKNOWN.
 *
 * Surface flags.
 **/
typedef enum { /*< flags >*/
  ENCA_SURFACE_EOL_CR    = 1 << 0,
  ENCA_SURFACE_EOL_LF    = 1 << 1,
  ENCA_SURFACE_EOL_CRLF  = 1 << 2,
  ENCA_SURFACE_EOL_MIX   = 1 << 3,
  ENCA_SURFACE_EOL_BIN   = 1 << 4,
  ENCA_SURFACE_MASK_EOL  = (ENCA_SURFACE_EOL_CR
                            | ENCA_SURFACE_EOL_LF
                            | ENCA_SURFACE_EOL_CRLF
                            | ENCA_SURFACE_EOL_MIX
                            | ENCA_SURFACE_EOL_BIN),
  ENCA_SURFACE_PERM_21    = 1 << 5,
  ENCA_SURFACE_PERM_4321  = 1 << 6,
  ENCA_SURFACE_PERM_MIX   = 1 << 7,
  ENCA_SURFACE_MASK_PERM  = (ENCA_SURFACE_PERM_21
                             | ENCA_SURFACE_PERM_4321
                             | ENCA_SURFACE_PERM_MIX),
  ENCA_SURFACE_QP        = 1 << 8,
  ENCA_SURFACE_REMOVE    = 1 << 13,
  ENCA_SURFACE_UNKNOWN   = 1 << 14,
  ENCA_SURFACE_MASK_ALL  = (ENCA_SURFACE_MASK_EOL
                            | ENCA_SURFACE_MASK_PERM
                            | ENCA_SURFACE_QP
                            | ENCA_SURFACE_REMOVE)
} EncaSurface;

/**
 * EncaNameStyle:
 * @ENCA_NAME_STYLE_ENCA: Default, implicit charset name in Enca.
 * @ENCA_NAME_STYLE_RFC1345: RFC 1345 charset name.
 * @ENCA_NAME_STYLE_CSTOCS: Cstocs charset name.
 * @ENCA_NAME_STYLE_ICONV: Iconv charset name.
 * @ENCA_NAME_STYLE_HUMAN: Human comprehensible description.
 *
 * Charset naming styles and conventions.
 **/
typedef enum {
  ENCA_NAME_STYLE_ENCA,
  ENCA_NAME_STYLE_RFC1345,
  ENCA_NAME_STYLE_CSTOCS,
  ENCA_NAME_STYLE_ICONV,
  ENCA_NAME_STYLE_HUMAN
} EncaNameStyle;

/**
 * EncaCharsetFlags:
 * @ENCA_CHARSET_7BIT: Characters are represented with 7bit characters.
 * @ENCA_CHARSET_8BIT: Characters are represented with bytes.
 * @ENCA_CHARSET_16BIT: Characters are represented with 2byte words.
 * @ENCA_CHARSET_32BIT: Characters are represented with 4byte words.
 * @ENCA_CHARSET_FIXED: One characters consists of one fundamental piece.
 * @ENCA_CHARSET_VARIABLE: One character consists of variable number of
 * fundamental pieces.
 * @ENCA_CHARSET_BINARY: Charset is binary from ASCII viewpoint.
 * @ENCA_CHARSET_REGULAR: Language dependent (8bit) charset.
 * @ENCA_CHARSET_MULTIBYTE: Multibyte charset.
 *
 * Charset properties.
 *
 * Flags %ENCA_CHARSET_7BIT, %ENCA_CHARSET_8BIT, %ENCA_CHARSET_16BIT,
 * %ENCA_CHARSET_32BIT tell how many bits a `fundamental piece' consists of.
 * This is different from bits per character; r.g. UTF-8 consists of 8bit
 * pieces (bytes), but character can be composed from 1 to 6 of them.
 **/
typedef enum { /*< flags >*/
  ENCA_CHARSET_7BIT      = 1 << 0,
  ENCA_CHARSET_8BIT      = 1 << 1,
  ENCA_CHARSET_16BIT     = 1 << 2,
  ENCA_CHARSET_32BIT     = 1 << 3,
  ENCA_CHARSET_FIXED     = 1 << 4,
  ENCA_CHARSET_VARIABLE  = 1 << 5,
  ENCA_CHARSET_BINARY    = 1 << 6,
  ENCA_CHARSET_REGULAR   = 1 << 7,
  ENCA_CHARSET_MULTIBYTE = 1 << 8
} EncaCharsetFlags;

/**
 * EncaErrno:
 * @ENCA_EOK: OK.
 * @ENCA_EINVALUE: Invalid value (usually of an option).
 * @ENCA_EEMPTY: Sample is empty.
 * @ENCA_EFILTERED: After filtering, (almost) nothing remained.
 * @ENCA_ENOCS8: Mulitibyte tests failed and language contains no 8bit charsets.
 * @ENCA_ESIGNIF: Too few significant characters.
 * @ENCA_EWINNER: No clear winner.
 * @ENCA_EGARBAGE: Sample is garbage.
 *
 * Error codes.
 **/
typedef enum {
  ENCA_EOK = 0,
  ENCA_EINVALUE,
  ENCA_EEMPTY,
  ENCA_EFILTERED,
  ENCA_ENOCS8,
  ENCA_ESIGNIF,
  ENCA_EWINNER,
  ENCA_EGARBAGE
} EncaErrno;

/**
 * ENCA_CS_UNKNOWN:
 *
 * Unknown character set id.
 *
 * Use enca_charset_is_known() to check for unknown charset instead of direct
 * comparsion.
 **/
#define ENCA_CS_UNKNOWN (-1)

/**
 * ENCA_NOT_A_CHAR:
 *
 * Not-a-character in unicode tables.
 **/
#define ENCA_NOT_A_CHAR 0xffff

/* Published (opaque) typedefs  */
typedef struct _EncaAnalyserState *EncaAnalyser;

/* Public (transparent) typedefs */
typedef struct _EncaEncoding EncaEncoding;

/**
 * EncaEncoding:
 * @charset: Numeric charset identifier.
 * @surface: Surface flags.
 *
 * Encoding, i.e. charset and surface.
 *
 * This is what enca_analyse() and enca_analyse_const() return.
 *
 * The @charset field is an opaque numerical charset identifier, which has no
 * meaning outside Enca library.
 * You will probably want to use it only as enca_charset_name() argument.
 * It is only guaranteed not to change meaning
 * during program execution time; change of its interpretation (e.g. due to
 * addition of new charsets) is not considered API change.
 *
 * The @surface field is a combination of #EncaSurface flags.  You may want
 * to ignore it completely; you should use enca_set_interpreted_surfaces()
 * to disable weird surfaces then.
 **/
struct _EncaEncoding { int charset; EncaSurface surface; };

void          (*enca_set_multibyte)              (EncaAnalyser analyser, int multibyte);
void          (*enca_set_interpreted_surfaces)   (EncaAnalyser analyser, int interpreted_surfaces);
void          (*enca_set_ambiguity)              (EncaAnalyser analyser, int ambiguity);
void          (*enca_set_filtering)              (EncaAnalyser analyser, int filtering);
void          (*enca_set_garbage_test)           (EncaAnalyser analyser, int garabage_test);
void          (*enca_set_termination_strictness) (EncaAnalyser analyser, int termination_strictness);
int           (*enca_set_significant)            (EncaAnalyser analyser, size_t significant);
int           (*enca_set_threshold)              (EncaAnalyser analyser, double threshold);
const char*   (*enca_charset_name)               (int charset, EncaNameStyle whatname);
int*          (*enca_get_language_charsets)      (const char *langname, size_t *n);
EncaAnalyser  (*enca_analyser_alloc)             (const char *langname);
void          (*enca_analyser_free)              (EncaAnalyser analyser);
EncaEncoding  (*enca_analyse_const)              (EncaAnalyser analyser,const unsigned char *buffer, size_t size);

/**
 * enca_charset_is_known:
 * @cs: Charset id.
 *
 * Expands to nonzero when the charset is known (i.e. it's not
 * ENCA_CS_UNKNOWN).
 **/
#define enca_charset_is_known(cs) \
  ((cs) != ENCA_CS_UNKNOWN)

/**
 * enca_charset_is_7bit:
 * @cs: Charset id.
 *
 * Expands to nonzero when characters are represented with 7bit characters.
 **/
#define enca_charset_is_7bit(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_7BIT)

/**
 * enca_charset_is_8bit:
 * @cs: Charset id.
 *
 * Expands to nonzero when characters are represented with bytes.
 **/
#define enca_charset_is_8bit(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_8BIT)

/**
 * enca_charset_is_16bit:
 * @cs: Charset id.
 *
 * Expands to nonzero when characters are represented with 2byte words.
 **/
#define enca_charset_is_16bit(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_16BIT)

/**
 * enca_charset_is_32bit:
 * @cs: Charset id.
 *
 * Expands to nonzero when characters are represented with 4byte words.
 **/
#define enca_charset_is_32bit(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_32BIT)

/**
 * enca_charset_is_fixed:
 * @cs: Charset id.
 *
 * Expands to nonzero when one characters consists of one fundamental piece.
 **/
#define enca_charset_is_fixed(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_FIXED)

/**
 * enca_charset_is_variable:
 * @cs: Charset id.
 *
 * Expands to nonzero when one character consists of variable number of
 * fundamental pieces.
 **/
#define enca_charset_is_variable(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_VARIABLE)

/**
 * enca_charset_is_binary:
 * @cs: Charset id.
 *
 * Expands to nonzero when charset is binary from ASCII viewpoint.
 **/
#define enca_charset_is_binary(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_BINARY)

/**
 * enca_charset_is_regular:
 * @cs: Charset id.
 *
 * Expands to nonzero when charset is language dependent (8bit) charset.
 **/
#define enca_charset_is_regular(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_REGULAR)

/**
 * enca_charset_is_multibyte:
 * @cs: Charset id.
 *
 * Expands to nonzero when charset is multibyte.
 **/
#define enca_charset_is_multibyte(cs) \
  (enca_charset_properties(cs) & ENCA_CHARSET_MULTIBYTE)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
