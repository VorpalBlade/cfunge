/* -*- mode: C; coding: utf-8; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
 * Copyright (C) 2008-2013 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
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

#include "global.h"
#include "interpreter.h"

#include "diagnostic.h"
#include "division.h"
#include "funge-space/funge-space.h"
#include "input.h"
#include "ip.h"
#include "prng.h"
#include "settings.h"
#include "stack.h"
#include "vector.h"

#include "fingerprints/manager.h"

#include "instructions/execute.h"
#include "instructions/io.h"
#include "instructions/iterate.h"
#include "instructions/sysinfo.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strerror */

#ifdef HAVE_clock_gettime
#  include <time.h>
#else
#  include <sys/time.h>
#endif
#include <assert.h>

#ifdef CFUN_KLEE_TEST
#  include <klee/klee.h>
#endif

/**
 * Either the IP or the IP list.
 */
/*@{*/
#ifdef CONCURRENT_FUNGE
static ipList *IPList = NULL;
#else
static instructionPointer *IP = NULL;
#endif
/*@}*/

/**
 * Print warning on unknown instruction if such warnings are enabled.
 */
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void warn_unknown_instr(funge_cell opcode, instructionPointer * restrict ip)
{
	if (FUNGE_UNLIKELY(setting_enable_warnings))
		diag_warn_format("Unknown instruction at x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ": %c (%" FUNGECELLPRI ")",
		                 ip->position.x, ip->position.y, (char)opcode, opcode);
}

// These two are called from elsewhere. Avoid code duplication.

FUNGE_ATTR_FAST inline void if_east_west(instructionPointer * restrict ip)
{
	if (stack_pop(ip->stack) == 0)
		ip_go_east(ip);
	else
		ip_go_west(ip);
}

FUNGE_ATTR_FAST inline void if_north_south(instructionPointer * restrict ip)
{
	if (stack_pop(ip->stack) == 0)
		ip_go_south(ip);
	else
		ip_go_north(ip);
}

#ifdef CONCURRENT_FUNGE
#  define return_from_execute_instruction(x) return (x)
   /// Return with value if we are concurrent
#  define return_if_con(x) return (x)
#  define CON_RETTYPE bool
#else
#  define return_from_execute_instruction(x) return
#  define CON_RETTYPE void
#  define return_if_con(x) (x); return
#endif

/// Generate a case for use in execute_instruction() that pushes a number on
/// the stack.
#define PUSHVAL(x, y) \
	case (x): \
		stack_push(ip->stack, (funge_cell)y); \
		break;

/// This function handles string mode.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline CON_RETTYPE handle_string_mode(funge_cell opcode, instructionPointer * restrict ip)
{
	if (opcode == '"') {
		ip->mode = ipmCODE;
	} else if (opcode != ' ') {
		ip->stringLastWasSpace = false;
		stack_push(ip->stack, opcode);
	} else {
		// This is a space
		if ((!ip->stringLastWasSpace) || (setting_current_standard == stdver93)) {
			ip->stringLastWasSpace = true;
			stack_push(ip->stack, opcode);
		// More than one space in string mode take no tick in concurrent Funge.
		} else {
			return_from_execute_instruction(true);
		}
	}
	return_from_execute_instruction(false);
}

/// This function handles fingerprint instructions.
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void handle_fprint(funge_cell opcode, instructionPointer * restrict ip)
{
	if (FUNGE_UNLIKELY(setting_disable_fingerprints)) {
		warn_unknown_instr(opcode, ip);
		ip_reverse(ip);
	} else {
		int_fast8_t entry = (int_fast8_t)(opcode - 'A');
		if ((ip->fingerOpcodes[entry].top > 0)
		    && ip->fingerOpcodes[entry].entries[ip->fingerOpcodes[entry].top - 1]) {
			// Call the fingerprint.
			ip->fingerOpcodes[entry].entries[ip->fingerOpcodes[entry].top - 1](ip);
		} else {
			warn_unknown_instr(opcode, ip);
			ip_reverse(ip);
		}
	}
}

#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST CON_RETTYPE execute_instruction(funge_cell opcode, instructionPointer * restrict ip, ssize_t * threadindex)
#else
FUNGE_ATTR_FAST CON_RETTYPE execute_instruction(funge_cell opcode, instructionPointer * restrict ip)
#endif
{
	// First check if we are in string mode, and do special stuff then.
	if (ip->mode == ipmSTRING) {
		return_if_con(handle_string_mode(opcode, ip));
	// Next: Is this a fingerprint opcode?
	} else if ((opcode >= 'A') && (opcode <= 'Z')) {
		handle_fprint(opcode, ip);
	// OK a core instruction.
	// Find what one and execute it.
	} else {
		switch (opcode) {
			case ' ': {
#ifdef AFL_FUZZ_TESTING
				long iterations = 500;
#endif
				do {
					ip_forward(ip);
#ifdef AFL_FUZZ_TESTING
					if (!iterations--)
						exit(123);
#endif
				} while (fungespace_get(&ip->position) == ' ');
				ip->needMove = false;
				return_from_execute_instruction(true);
			}
			case 'z':
				break;
			case ';': {
#ifdef AFL_FUZZ_TESTING
				long iterations = 500;
#endif
				do {
					ip_forward(ip);
#ifdef AFL_FUZZ_TESTING
					if (!iterations--)
						exit(123);
#endif
				} while (fungespace_get(&ip->position) != ';');
				return_from_execute_instruction(true);
			}
			case '^':
				ip_go_north(ip);
				break;
			case '>':
				ip_go_east(ip);
				break;
			case 'v':
				ip_go_south(ip);
				break;
			case '<':
				ip_go_west(ip);
				break;
			case 'j': {
				// Currently need to do it like this or wrapping
				// won't work for j.
				funge_cell jumps = stack_pop(ip->stack);
				ip_forward(ip);
				if (jumps != 0) {
					funge_vector tmp;
					tmp.x = ip->delta.x;
					tmp.y = ip->delta.y;
					ip->delta.y *= jumps;
					ip->delta.x *= jumps;
					ip_forward(ip);
					ip->delta.x = tmp.x;
					ip->delta.y = tmp.y;
				}
				ip->needMove = false;
				break;
			}
			case '?': {
				// May not be perfectly uniform.
				// If this matters for you, contact me (with a patch).
				funge_unsigned_cell rnd = prng_generate_unsigned(4);
				switch (rnd) {
					case 0: ip_go_north(ip); break;
					case 1: ip_go_east(ip); break;
					case 2: ip_go_south(ip); break;
					case 3: ip_go_west(ip); break;
				}
				break;
			}
			case 'r':
				ip_reverse(ip);
				break;
			case '[':
				ip_turn_left(ip);
				break;
			case ']':
				ip_turn_right(ip);
				break;
			case 'x': {
				funge_vector pos = stack_pop_vector(ip->stack);
#ifdef AFL_FUZZ_TESTING
				if (pos.x == 0 && pos.y == 0)
					exit(123);
#endif
				ip->delta = pos;
				break;
			}

			PUSHVAL('0', 0)
			PUSHVAL('1', 1)
			PUSHVAL('2', 2)
			PUSHVAL('3', 3)
			PUSHVAL('4', 4)
			PUSHVAL('5', 5)
			PUSHVAL('6', 6)
			PUSHVAL('7', 7)
			PUSHVAL('8', 8)
			PUSHVAL('9', 9)
			PUSHVAL('a', 0xa)
			PUSHVAL('b', 0xb)
			PUSHVAL('c', 0xc)
			PUSHVAL('d', 0xd)
			PUSHVAL('e', 0xe)
			PUSHVAL('f', 0xf)

			case '"':
				ip->mode = ipmSTRING;
				ip->stringLastWasSpace = false;
				break;
			case ':':
				stack_dup_top(ip->stack);
				break;

			case '#':
				ip_forward(ip);
				break;

			case '_':
				if_east_west(ip);
				break;
			case '|':
				if_north_south(ip);
				break;
			case 'w': {
				funge_cell a, b;
				b = stack_pop(ip->stack);
				a = stack_pop(ip->stack);
				if (a < b)
					ip_turn_left(ip);
				else if (a > b)
					ip_turn_right(ip);
				break;
			}
			case 'k':
#ifdef CONCURRENT_FUNGE
				run_iterate(ip, &IPList, threadindex);
#else
				run_iterate(ip);
#endif
				break;

			case '-': {
				funge_cell a, b;
				b = stack_pop(ip->stack);
				a = stack_pop(ip->stack);
				stack_push(ip->stack, a - b);
				break;
			}
			case '+': {
				funge_cell a, b;
				b = stack_pop(ip->stack);
				a = stack_pop(ip->stack);
				stack_push(ip->stack, a + b);
				break;
			}
			case '*': {
				funge_cell a, b;
				b = stack_pop(ip->stack);
				a = stack_pop(ip->stack);
				stack_push(ip->stack, a * b);
				break;
			}
			case '/': {
				funge_cell a, b;
				b = stack_pop(ip->stack);
				a = stack_pop(ip->stack);
				stack_push(ip->stack, funge_division(a, b));
				break;
			}
			case '%': {
				funge_cell a, b;
				b = stack_pop(ip->stack);
				a = stack_pop(ip->stack);
				stack_push(ip->stack, funge_modulo(a, b));
				break;
			}

			case '!':
				stack_push(ip->stack, !stack_pop(ip->stack));
				break;
			case '`': {
				funge_cell a, b;
				b = stack_pop(ip->stack);
				a = stack_pop(ip->stack);
				stack_push(ip->stack, a > b);
				break;
			}

			case 'g': {
				funge_vector pos;
				funge_cell a;
				pos = stack_pop_vector(ip->stack);
				a = fungespace_get_offset(&pos, &ip->storageOffset);
				stack_push(ip->stack, a);
				break;
			}
			case 'p': {
				funge_vector pos;
				funge_cell a;
				pos = stack_pop_vector(ip->stack);
				a = stack_pop(ip->stack);
				fungespace_set_offset(a, &pos, &ip->storageOffset);
				break;
			}

			case '\'':
				ip_forward(ip);
				stack_push(ip->stack, fungespace_get(&ip->position));
				break;
			case 's':
				ip_forward_no_wrap(ip);
				fungespace_set(stack_pop(ip->stack), &ip->position);
				break;

			case '$':
				stack_discard(ip->stack, 1);
				break;
			case '\\':
				stack_swap_top(ip->stack);
				break;
			case 'n':
				stack_clear(ip->stack);
				break;

			case ',': {
				funge_cell a = stack_pop(ip->stack);
				// Reverse on failed output
				if (FUNGE_UNLIKELY(cf_putchar_unlocked((int)a) != (unsigned char)a))
					ip_reverse(ip);
				break;
			}
			case '.':
				// Reverse on failed output
				if (FUNGE_UNLIKELY(printf("%" FUNGECELLPRI " ", stack_pop(ip->stack)) < 0))
					ip_reverse(ip);
				break;

			case '~': {
				funge_cell a;
				if (input_getchar(&a)) {
					stack_push(ip->stack, a);
				} else {
					ip_reverse(ip);
				}
				break;
			}
			case '&': {
				funge_cell a = 0;
				ret_getint gotint = rgi_noint;
				while (gotint == rgi_noint)
					gotint = input_getint(&a, 10);
				if (gotint == rgi_success) {
					stack_push(ip->stack, a);
				} else {
					ip_reverse(ip);
				}
				break;
			}

			case 'y':
				run_sys_info(ip);
				break;

			case '{': {
				funge_cell count;
				funge_vector pos;
				count = stack_pop(ip->stack);
				ip_forward(ip);
				pos.x = ip->position.x;
				pos.y = ip->position.y;
				ip_backward(ip);
				if (!stackstack_begin(ip, count, &pos))
					ip_reverse(ip);
				break;
			}
			case '}':
				if (ip->stackstack->current == 0) {
					ip_reverse(ip);
				} else {
					funge_cell count;
					count = stack_pop(ip->stack);
					if (!stackstack_end(ip, count))
						ip_reverse(ip);
				}
				break;
			case 'u':
				if (ip->stackstack->current == 0) {
					ip_reverse(ip);
				} else {
					funge_cell count;
					count = stack_pop(ip->stack);
					stackstack_transfer(count,
					                    ip->stackstack->stacks[ip->stackstack->current],
					                    ip->stackstack->stacks[ip->stackstack->current - 1]);
				}
				break;

			case 'i':
				run_file_input(ip);
				break;
			case 'o':
				run_file_output(ip);
				break;
			case '=':
				run_system_execute(ip);
				break;

			case '(':
			case ')': {
				// TODO: Handle Funge-109 style too.
				funge_cell fpsize = stack_pop(ip->stack);
				// Check for sanity (because we won't have any fingerprints
				// outside such a range. This prevents long lockups here.
#ifdef AFL_FUZZ_TESTING
				if (fpsize > 500)
					exit(123);
#endif
				if (fpsize < 1) {
					ip_reverse(ip);
				} else if (FUNGE_UNLIKELY(setting_disable_fingerprints)) {
					stack_discard(ip->stack, (size_t)fpsize);
					ip_reverse(ip);
				} else {
					funge_cell fprint = 0;
					if (FUNGE_UNLIKELY((fpsize > 8) && setting_enable_warnings)) {
						diag_warn_format("WARN: %c (x=%" FUNGECELLPRI " y=%"
						                 FUNGECELLPRI "): count is very large(%" FUNGECELLPRI
						                 "), probably a bug.\n", (char)opcode,
						                 ip->position.x, ip->position.y, fpsize);
					}
					while (fpsize--) {
						fprint <<= 8;
						fprint += stack_pop(ip->stack);
					}
					if (opcode == '(') {
						if (!manager_load(ip, fprint))
							ip_reverse(ip);
					} else {
						if (!manager_unload(ip, fprint))
							ip_reverse(ip);
					}
				}
				break;
			}

#ifdef CONCURRENT_FUNGE
			case 't': {
				ssize_t new_index = iplist_duplicate_ip(&IPList, *threadindex);
				// Handle possible failure.
				if (new_index != -1) {
					*threadindex = new_index;
				} else {
					// Yeah this is the same as the child normally,
					// the program should check that the parent still exists.
					ip_reverse(ip);
				}
				break;
			}

#endif /* CONCURRENT_FUNGE */

			case '@':
#ifdef CONCURRENT_FUNGE
				if (IPList->top == 0) {
					fflush(stdout);
					exit(0);
				} else {
					*threadindex = iplist_terminate_ip(&IPList, *threadindex);
#  ifdef LARGE_IPLIST
					IPList->ips[*threadindex]->needMove = false;
#  else
					IPList->ips[*threadindex].needMove = false;
#  endif
				}
#else
				exit(0);
#endif /* CONCURRENT_FUNGE */
				break;

			case 'q':
// We do the wrong thing here when fuzz testing to reduce false positives.
#ifdef FUZZ_TESTING
				exit(0);
#else
				exit((int)stack_pop(ip->stack));
#endif
				break;

			default:
				warn_unknown_instr(opcode, ip);
				ip_reverse(ip);
		}
	}
	return_from_execute_instruction(false);
}


#ifdef CONCURRENT_FUNGE
FUNGE_ATTR_FAST FUNGE_ATTR_NONNULL
static inline void thread_forward(instructionPointer * restrict ip)
{
	assert(ip != NULL);

	if (ip->needMove)
		ip_forward(ip);
	else
		ip->needMove = true;
}
#endif


FUNGE_ATTR_NORET
static inline void interpreter_main_loop(void)
{
#ifdef AFL_FUZZ_TESTING
	long iterations = 1000;
#endif
#ifdef CONCURRENT_FUNGE
	while (true) {
		ssize_t i = IPList->top;
#    ifdef AFL_FUZZ_TESTING
		long thread_iterations = 1000;
		// Give up after too many instructions
		if (!iterations--)
			exit(123);
#    endif
		while (i >= 0) {
			bool retval;
			funge_cell opcode;
#    ifdef AFL_FUZZ_TESTING
			if (!thread_iterations--)
				exit(123);
#    endif

#    ifdef LARGE_IPLIST
			opcode = fungespace_get(&IPList->ips[i]->position);
#    else
			opcode = fungespace_get(&IPList->ips[i].position);
#    endif

#    if !defined(DISABLE_TRACE) && defined(LARGE_IPLIST)
			if (FUNGE_UNLIKELY(setting_trace_level != 0)) {
				if (setting_trace_level > 8) {
					fprintf(stderr, "tix=%zd tid=%" FUNGECELLPRI " x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ": %c (%" FUNGECELLPRI ")\n",
					        i, IPList->ips[i]->ID, IPList->ips[i]->position.x,
					        IPList->ips[i]->position.y, (char)opcode, opcode);
					stack_print_top(IPList->ips[i]->stack);
				} else if (setting_trace_level > 3) {
					fprintf(stderr, "tix=%zd tid=%" FUNGECELLPRI " x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ": %c (%" FUNGECELLPRI ")\n",
					        i, IPList->ips[i]->ID, IPList->ips[i]->position.x,
					        IPList->ips[i]->position.y, (char)opcode, opcode);
				} else if (setting_trace_level > 2)
					fprintf(stderr, "%c", (char)opcode);
			}
#    elif !defined(DISABLE_TRACE) && !defined(LARGE_IPLIST)
			if (FUNGE_UNLIKELY(setting_trace_level != 0)) {
				if (setting_trace_level > 8) {
					fprintf(stderr, "tix=%zd tid=%" FUNGECELLPRI " x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ": %c (%" FUNGECELLPRI ")\n",
					        i, IPList->ips[i].ID, IPList->ips[i].position.x,
					        IPList->ips[i].position.y, (char)opcode, opcode);
					stack_print_top(IPList->ips[i].stack);
				} else if (setting_trace_level > 3) {
					fprintf(stderr, "tix=%zd tid=%" FUNGECELLPRI " x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ": %c (%" FUNGECELLPRI ")\n",
					        i, IPList->ips[i].ID, IPList->ips[i].position.x,
					        IPList->ips[i].position.y, (char)opcode, opcode);
				} else if (setting_trace_level > 2)
					fprintf(stderr, "%c", (char)opcode);
			}
#    endif /* DISABLE_TRACE */

#    ifdef LARGE_IPLIST
			retval = execute_instruction(opcode, IPList->ips[i], &i);
			thread_forward(IPList->ips[i]);
#    else
			retval = execute_instruction(opcode, &IPList->ips[i], &i);
			thread_forward(&IPList->ips[i]);
#    endif
			if (!retval)
				i--;
		}
	}
#else /* CONCURRENT_FUNGE */
	while (true) {
		funge_cell opcode;
#    ifdef AFL_FUZZ_TESTING
		// Give up after too many instructions
		if (!iterations--)
			exit(123);
#    endif
		opcode = fungespace_get(&IP->position);
#    ifndef DISABLE_TRACE
		if (FUNGE_UNLIKELY(setting_trace_level != 0)) {
			if (setting_trace_level > 8) {
				fprintf(stderr, "x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ": %c (%" FUNGECELLPRI ")\n",
				        IP->position.x, IP->position.y, (char)opcode, opcode);
				stack_print_top(IP->stack);
			} else if (setting_trace_level > 3) {
				fprintf(stderr, "x=%" FUNGECELLPRI " y=%" FUNGECELLPRI ": %c (%" FUNGECELLPRI ")\n",
				        IP->position.x, IP->position.y, (char)opcode, opcode);
			} else if (setting_trace_level > 2)
				fprintf(stderr, "%c", (char)opcode);
		}
#    endif /* DISABLE_TRACE */

		execute_instruction(opcode, IP);
		if (IP->needMove)
			ip_forward(IP);
		else
			IP->needMove = true;
	}
#endif /* CONCURRENT_FUNGE */
}


#ifndef NDEBUG
// Used with debugging for freeing stuff at end of the program.
// Not needed, but useful to check that free functions works,
// and for detecting real memory leaks.
static void debug_free(void)
{
# ifdef CONCURRENT_FUNGE
	iplist_free(IPList);
# else
	ip_free(IP);
# endif
	sysinfo_cleanup();
	fungespace_free();
}
#endif


#ifdef CFUN_KLEE_TEST_PROGRAM
void klee_generate_program(void)
{
	unsigned char program_code[10];
	klee_make_symbolic(program_code, sizeof(program_code), "program_code");

	for (int i = 0; i < sizeof(program_code); i++) {
		klee_assume(program_code[i] < 128);
		klee_assume((program_code[i] > 31)
		            | (program_code[i] == '\n')
		            | (program_code[i] == '\r'));
		klee_assume(program_code[i] != 'm');
		klee_assume(program_code[i] != 'l');
		klee_assume(program_code[i] != 'h');
#ifndef CONCURRENT_FUNGE
		klee_assume(program_code[i] != 't');
#endif
	}
	fungespace_load_string(program_code, sizeof(program_code));
}
#endif


FUNGE_ATTR_NORET FUNGE_ATTR_FAST
void interpreter_run(const char *filename)
{
	if (FUNGE_UNLIKELY(!fungespace_create())) {
		DIAG_FATAL_FORMAT_LOC("Couldn't create funge space: %s", strerror(errno));
	}
#if !defined(NDEBUG) && !defined(CFUN_KLEE_TEST)
	atexit(&debug_free);
#endif
	prng_init();
#ifdef CFUN_KLEE_TEST_PROGRAM
	klee_generate_program();
#else
	if (FUNGE_UNLIKELY(!fungespace_load(filename))) {
		diag_fatal_format("Failed to process file \"%s\": %s", filename, strerror(errno));
	}
#endif
#ifdef CONCURRENT_FUNGE
	IPList = iplist_create();
	if (FUNGE_UNLIKELY(IPList == NULL)) {
		DIAG_FATAL_LOC("Couldn't create instruction pointer list!?");
	}
#else
	IP = ip_create();
	if (FUNGE_UNLIKELY(IP == NULL)) {
		DIAG_FATAL_LOC("Couldn't create instruction pointer!?");
	}
#endif
	interpreter_main_loop();
}
