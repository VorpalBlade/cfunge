/**
 * @file
 * genx - C-callable library for generating XML documents
 */

#ifndef GENX_H
#define GENX_H

/*
 * Copyright (c) 2004 by Tim Bray and Sun Microsystems.  For copying
 *  permission, see http://www.tbray.org/ongoing/genx/COPYING
 */

#include "../../src/global.h"

#if !defined(CFUN_NO_FLOATS) && !defined(CFUN_NO_TURT)

#include <stdio.h>

/**
 * @defgroup genx Genx - XML generation functions
 * A library for writing XML.
 */
/*@{*/

/**
 * Note on error handling: genx routines mostly return
 *  GENX_SUCCESS (guaranteed to be zero) in normal circumstances, one of
 *  these other GENX_ values on a memory allocation or I/O failure or if the
 *  call would result in non-well-formed output.
 * You can associate an error message with one of these codes explicitly
 *  or with the most recent error using genxGetErrorMessage() and
 *  genxLastErrorMessage(); see below.
 */
typedef enum {
	GENX_SUCCESS = 0,
	GENX_BAD_UTF8,
	GENX_NON_XML_CHARACTER,
	GENX_BAD_NAME,
	GENX_ALLOC_FAILED,
	GENX_BAD_NAMESPACE_NAME,
	GENX_INTERNAL_ERROR,
	GENX_DUPLICATE_PREFIX,
	GENX_SEQUENCE_ERROR,
	GENX_NO_START_TAG,
	GENX_IO_ERROR,
	GENX_MISSING_VALUE,
	GENX_MALFORMED_COMMENT,
	GENX_XML_PI_TARGET,
	GENX_MALFORMED_PI,
	GENX_DUPLICATE_ATTRIBUTE,
	GENX_ATTRIBUTE_IN_DEFAULT_NAMESPACE,
	GENX_DUPLICATE_NAMESPACE,
	GENX_BAD_DEFAULT_DECLARATION
} genxStatus;

/** character types */
#define GENX_XML_CHAR 1
/** character types */
#define GENX_LETTER 2
/** character types */
#define GENX_NAMECHAR 4

/**
 * @defgroup genx_types Genx's types
 */
/*@{*/
/** An UTF-8 string */
typedef unsigned char * utf8;
/** A const UTF-8 string */
typedef const unsigned char * constUtf8;

/// A writer, the main struct.
typedef struct genxWriter_rec * genxWriter;
/// A namespace.
typedef struct genxNamespace_rec * genxNamespace;
/// An element.
typedef struct genxElement_rec * genxElement;
/// An attribute.
typedef struct genxAttribute_rec * genxAttribute;
/*@}*/

/*
 * Constructors, set/get
 */

/**
 * Create a new writer.  For generating multiple XML documents, it's most
 *  efficient to re-use the same genx object.  However, you can only write
 *  one document at a time with a writer.
 * @return NULL if it fails, which can only be due to an allocation failure.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_MALLOC FUNGE_ATTR_WARN_UNUSED
genxWriter genxNew(void);

/**
 * Dispose of a writer, freeing all associated memory
 */
FUNGE_ATTR_FAST
void genxDispose(genxWriter w);

/*
 * Set/get
 */

/**
 * Get the prefix associated with a namespace
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
utf8 genxGetNamespacePrefix(genxNamespace ns);

/*
 * Declaration functions
 */

/**
 * Declare a namespace.  The provided prefix is the default but can be
 *  overridden by genxAddNamespace.  If no default prefiix is provided,
 *  genx will generate one of the form g-%d.
 * On error, returns NULL and signals via statusp
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxNamespace genxDeclareNamespace(genxWriter w,
                                   constUtf8 uri, constUtf8 prefix,
                                   genxStatus * statusP);

/**
 * Declare an element
 * If something failed, returns NULL and sets the status code via statusP
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxElement genxDeclareElement(genxWriter w,
                               genxNamespace ns, constUtf8 type,
                               genxStatus * statusP);

/**
 * Declare an attribute
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxAttribute genxDeclareAttribute(genxWriter w,
                                   genxNamespace ns,
                                   constUtf8 name, genxStatus * statusP);

/*
 * Writing XML
 */

/**
 * Start a new document.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxStartDocFile(genxWriter w, FILE * file);

/**
 * Caller-provided I/O package.
 * First form is for a null-terminated string.
 * for second, if you have s="abcdef" and want to send "abc", you'd call
 *  sendBounded(userData, s, s + 3)
 */
typedef struct {
	genxStatus(* send)(void * userData, constUtf8 s);
	genxStatus(* sendBounded)(void * userData, constUtf8 start, constUtf8 end);
	genxStatus(* flush)(void * userData);
} genxSender;

FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxStartDocSender(genxWriter w, genxSender * sender);

/**
 * End a document.  Calls "flush"
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxEndDocument(genxWriter w);

/**
 * Write a comment
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxComment(genxWriter w, constUtf8 text);

/**
 * Write a PI
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxPI(genxWriter w, constUtf8 target, constUtf8 text);

/**
 * Start an element
 */
FUNGE_ATTR_FAST
genxStatus genxStartElementLiteral(genxWriter w,
                                   constUtf8 xmlns, constUtf8 type);

/**
 * Start a predeclared element
 * - element must have been declared
 */
FUNGE_ATTR_FAST
genxStatus genxStartElement(genxElement e);

/**
 * Write an attribute
 */
FUNGE_ATTR_FAST
genxStatus genxAddAttributeLiteral(genxWriter w, constUtf8 xmlns,
                                   constUtf8 name, constUtf8 value);

/**
 * Write a predeclared attribute
 */
FUNGE_ATTR_FAST
genxStatus genxAddAttribute(genxAttribute a, constUtf8 value);

/**
 * Add a namespace declaration
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxAddNamespace(genxNamespace ns, utf8 prefix);

/**
 * Clear default namespace declaration
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxUnsetDefaultNamespace(genxWriter w);

/**
 * Write an end tag
 */
FUNGE_ATTR_FAST
genxStatus genxEndElement(genxWriter w);

/**
 * Write some text
 * You can't write any text outside the root element, except with
 *  genxComment and genxPI
 */
FUNGE_ATTR_FAST
genxStatus genxAddText(genxWriter w, constUtf8 start);
FUNGE_ATTR_FAST
genxStatus genxAddCountedText(genxWriter w, constUtf8 start, int byteCount);
FUNGE_ATTR_FAST
genxStatus genxAddBoundedText(genxWriter w, constUtf8 start, constUtf8 end);

/**
 * Write one character.  The integer value is the Unicode character
 *  value, as usually expressed in U+XXXX notation.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxAddCharacter(genxWriter w, int c);

/*
 * Utility routines
 */

/**
 * Return the Unicode character encoded by the UTF-8 pointed-to by the
 *  argument, and advance the argument past the encoding of the character.
 * @returns
 *  -1 if the UTF-8 is malformed, in which case advances the
 *  argument to point at the first byte past the point past the malformed
 *  ones.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
int genxNextUnicodeChar(constUtf8 * sp);

/**
 * Scan a buffer allegedly full of UTF-8 encoded XML characters; return
 *  one of GENX_SUCCESS, GENX_BAD_UTF8, or GENX_NON_XML_CHARACTER
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
genxStatus genxCheckText(const genxWriter restrict w, constUtf8 s);

/**
 * Return character status, the OR of GENX_XML_CHAR,
 *  GENX_LETTER, and GENX_NAMECHAR
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
int genxCharClass(const genxWriter restrict w, int c);

/**
 * Silently wipe any non-XML characters out of a chunk of text.
 * If you call this on a string before you pass it addText or
 *  addAttribute, you will never get an error from genx unless
 *  (a) there's a bug in your software, e.g. a malformed element name, or
 *  (b) there's a memory allocation or I/O error
 * The output can never be longer than the input.
 * @returns true if any changes were made.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
int genxScrubText(const genxWriter restrict w, constUtf8 in, utf8 out);

/**
 * Return a specific error message.
 * @param w The genxWriter in question.
 * @param status What status to look up.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
const char * genxGetErrorMessage(const genxWriter restrict w, genxStatus status);
/**
 * Return last error message.
 * @param w The genxWriter in question.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
const char * genxLastErrorMessage(const genxWriter restrict w);

/**
 * Return version of genx.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_WARN_UNUSED
const char * genxGetVersion(void);

/*@}*/

#ifdef GENX_INTERNAL
FUNGE_ATTR_FAST void genxSetCharProps(char * restrict p);
#endif

#endif /* !defined(CFUN_NO_FLOATS) && !defined(CFUN_NO_TURT) */

#endif /* GENX_H */
