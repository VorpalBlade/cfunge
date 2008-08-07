/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - a conformant Befunge93/98/08 interpreter in C.
 * Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at the proxy's option) any later version. Arvid Norlander is a
 * proxy who can decide which future versions of the GNU General Public
 * License can be used.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// WARNING THIS FILE IS AUTO GENERATED DON'T CHANGE
// It was generated by tools/gen_fprint_list.sh
// Rerun the tool to update this list
#ifndef MANAGER_INTERNAL
#  error "This is for use in manager.c only"
#endif

#ifndef _HAD_SRC_FINGERPRINTS_FINGERPRINTS_H
#define _HAD_SRC_FINGERPRINTS_FINGERPRINTS_H

#include "../global.h"

#include <stdbool.h>
#include <stdint.h>

#include "BASE/BASE.h"
#include "CPLI/CPLI.h"
#include "DIRF/DIRF.h"
#include "FILE/FILE.h"
#include "FIXP/FIXP.h"
#include "FPDP/FPDP.h"
#include "FPSP/FPSP.h"
#include "FRTH/FRTH.h"
#include "HRTI/HRTI.h"
#include "INDV/INDV.h"
#include "JSTR/JSTR.h"
#include "MODU/MODU.h"
#include "NULL/NULL.h"
#include "ORTH/ORTH.h"
#include "PERL/PERL.h"
#include "REFC/REFC.h"
#include "ROMA/ROMA.h"
#include "STRN/STRN.h"
#include "SUBR/SUBR.h"
#include "TERM/TERM.h"
#include "TIME/TIME.h"
#include "TOYS/TOYS.h"
#include "TURT/TURT.h"

typedef struct s_ImplementedFingerprintEntry {
	const FUNGEDATATYPE     fprint;   /**< Fingerprint. */
	const char            * uri;      /**< URI, used for Funge-108. */
	const fingerprintLoader loader;   /**< Loader function pointer. */
	const char            * opcodes;  /**< Sorted string with all implemented opcodes. */
	const char            * url;      /**< URL, used to show links for more info about fingerprints. */
	const bool              safe:1;   /**< If true, this fingerprint is safe in sandbox mode. */
} ImplementedFingerprintEntry;

// Implemented fingerprints
// NOTE: Keep sorted (apart from ending 0 entry).
// Also note that this table is processed by scripts, so keep the .loader and
// .opcodes entries on the same line! As well as in current format.
static const ImplementedFingerprintEntry ImplementedFingerprints[] = {
	// BASE - I/O for numbers in other bases
	{ .fprint = 0x42415345, .uri = NULL, .loader = &FingerBASEload, .opcodes = "BHINO",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// CPLI - Complex Integer extension
	{ .fprint = 0x43504c49, .uri = NULL, .loader = &FingerCPLIload, .opcodes = "ADMOSV",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// DIRF - Directory functions extension
	{ .fprint = 0x44495246, .uri = NULL, .loader = &FingerDIRFload, .opcodes = "CMR",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = false },
	// FILE - File I/O functions
	{ .fprint = 0x46494c45, .uri = NULL, .loader = &FingerFILEload, .opcodes = "CDGLOPRSW",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = false },
	// FIXP - Some useful math functions
	{ .fprint = 0x46495850, .uri = NULL, .loader = &FingerFIXPload, .opcodes = "ABCDIJNOPQRSTUVX",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// FPDP - Double precision floating point
	{ .fprint = 0x46504450, .uri = NULL, .loader = &FingerFPDPload, .opcodes = "ABCDEFGHIKLMNPQRSTVXY",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// FPSP - Single precision floating point
	{ .fprint = 0x46505350, .uri = NULL, .loader = &FingerFPSPload, .opcodes = "ABCDEFGHIKLMNPQRSTVXY",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// FRTH - Some common forth commands
	{ .fprint = 0x46525448, .uri = NULL, .loader = &FingerFRTHload, .opcodes = "DLOPR",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// HRTI - High-Resolution Timer Interface
	{ .fprint = 0x48525449, .uri = NULL, .loader = &FingerHRTIload, .opcodes = "EGMST",
	  .url = "http://catseye.tc/projects/funge98/library/HRTI.html", .safe = true },
	// INDV - Pointer functions
	{ .fprint = 0x494e4456, .uri = NULL, .loader = &FingerINDVload, .opcodes = "GPVW",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// JSTR - Read and write strings in Funge-Space.
	{ .fprint = 0x4a535452, .uri = NULL, .loader = &FingerJSTRload, .opcodes = "GP",
	  .url = "http://www.imaginaryrobots.net/projects/funge/myexts.txt", .safe = true },
	// MODU - Modulo Arithmetic
	{ .fprint = 0x4d4f4455, .uri = NULL, .loader = &FingerMODUload, .opcodes = "MRU",
	  .url = "http://catseye.tc/projects/funge98/library/MODU.html", .safe = true },
	// NULL - Null
	{ .fprint = 0x4e554c4c, .uri = NULL, .loader = &FingerNULLload, .opcodes = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	  .url = "http://catseye.tc/projects/funge98/library/NULL.html", .safe = true },
	// ORTH - Orthogonal Easement Library
	{ .fprint = 0x4f525448, .uri = NULL, .loader = &FingerORTHload, .opcodes = "AEGOPSVWXYZ",
	  .url = "http://catseye.tc/projects/funge98/library/ORTH.html", .safe = true },
	// PERL - Generic Interface to the Perl Language
	{ .fprint = 0x5045524c, .uri = NULL, .loader = &FingerPERLload, .opcodes = "EIS",
	  .url = "http://catseye.tc/projects/funge98/library/PERL.html", .safe = false },
	// PNTR - Alias for INDV - Pointer functions
	{ .fprint = 0x504e5452, .uri = NULL, .loader = &FingerPNTRload, .opcodes = "GPVW",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// REFC - Referenced Cells Extension
	{ .fprint = 0x52454643, .uri = NULL, .loader = &FingerREFCload, .opcodes = "DR",
	  .url = "http://catseye.tc/projects/funge98/library/REFC.html", .safe = true },
	// ROMA - Roman Numerals
	{ .fprint = 0x524f4d41, .uri = NULL, .loader = &FingerROMAload, .opcodes = "CDILMVX",
	  .url = "http://catseye.tc/projects/funge98/library/ROMA.html", .safe = true },
	// STRN - String functions
	{ .fprint = 0x5354524e, .uri = NULL, .loader = &FingerSTRNload, .opcodes = "ACDFGILMNPRSV",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// SUBR - Subroutine extension
	{ .fprint = 0x53554252, .uri = NULL, .loader = &FingerSUBRload, .opcodes = "CJR",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// TERM - Terminal control functions
	{ .fprint = 0x5445524d, .uri = NULL, .loader = &FingerTERMload, .opcodes = "CDGHLSU",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// TIME - Time and Date functions
	{ .fprint = 0x54494d45, .uri = NULL, .loader = &FingerTIMEload, .opcodes = "DFGHLMOSWY",
	  .url = "http://www.elf-emulation.com/funge/rcfunge_manual.html", .safe = true },
	// TOYS - Funge-98 Standard Toys
	{ .fprint = 0x544f5953, .uri = NULL, .loader = &FingerTOYSload, .opcodes = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	  .url = "http://catseye.tc/projects/funge98/library/TOYS.html", .safe = true },
	// TURT - Simple Turtle Graphics Library
	{ .fprint = 0x54555254, .uri = NULL, .loader = &FingerTURTload, .opcodes = "ABCDEFHILNPQRTU",
	  .url = "http://catseye.tc/projects/funge98/library/TURT.html", .safe = true },
	// Last should be 0
	{ .fprint = 0, .uri = NULL, .loader = NULL, .opcodes = NULL, .url = NULL, .safe = true }
};

#endif
