/*
 * testlib.c
 *
 * Unit test interface.
 *
 * Copyright (C) 2009-2010 Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* retrieve type definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testlib.h"
#include <utils.h>
#include <debug_utils.h>

#define TESTLIB_OK          0
#define TESTLIB_FAIL        1

/** Returns TRUE iff str is a whole unsigned int */
#define is_uint(str) \
    E_ { unsigned i; char c; sscanf(str, "%u%c", &i, &c) == 1; } _E

extern int __internal__TestLib_DoList(int id);

/**
 * Prints test result and returns summary result
 * 
 * @author a0194118 (9/7/2009)
 * 
 * @param res    Test result
 * 
 * @return TEST_RESULT_OK on success, TEST_RESULT_FAIL on 
 *         failure, TEST_RESULT_UNAVAILABLE if test is not
 *         available
 */
int __internal__TestLib_Report(int res)
{
    switch (res)
    {
        case TESTLIB_UNAVAILABLE:
            P("==> TEST NOT AVAILABLE\n");
            return TESTLIB_UNAVAILABLE;
        case 0:
            P("==> TEST OK\n");
            return TESTLIB_OK;
        default:
            P("==> TEST FAIL(%d)\n", res);
            return TESTLIB_FAIL;
    }
}

void __internal__TestLib_NullFn(void *ptr)
{
}

int TestLib_Run(int argc, char **argv, void(*init_fn)(void *),
                void(*exit_fn)(void *), void *ptr)
{
    int start, end, res, failed = 0, succeeded = 0, unavailable = 0;

    /* all tests */
    if (argc == 1)
    {
        start = 1; end = -1;
    }
    /* test list */
    else if (argc == 2 && !strcmp(argv[1], "list"))
    {
        __internal__TestLib_DoList(0);
        return -1;
    }
    /* single test */
    else if (argc == 2 && is_uint(argv[1]))
    {
        start = end = atoi(argv[1]);
    }
    /* open range .. b */
    else if (argc == 3 && !strcmp(argv[1], "..") && is_uint(argv[2]))
    {
        start = 1;
        end = atoi(argv[2]);
    }
    /* open range a .. */
    else if (argc == 3 && !strcmp(argv[2], "..") && is_uint(argv[1]))
    {
        start = atoi(argv[1]);
        end = -1;
    }
    else if (argc == 4 && !strcmp(argv[2], "..") && is_uint(argv[1]) && is_uint(argv[3]))
    {
        start = atoi(argv[1]);
        end = atoi(argv[3]);
    }
    else
    {
        P("Usage: %s [<range>], where <range> is\n"
          "   empty:   run all tests\n"
          "   list:    list tests\n"
          "   ix:      run test #ix\n"
          "   a ..:    run tests #a, #a+1, ...\n"
          "   .. b:    run tests #1, #2, .. #b\n"
          "   a .. b:  run tests #a, #a+1, .. #b\n", argv[0]);
        return -1;
    }

    /* execute tests  */
    init_fn(ptr);

    do
    {
        res = __internal__TestLib_DoList(start++);
        if (res == TESTLIB_FAIL) failed++;
        else if (res == TESTLIB_OK) succeeded++;
        else if (res == TESTLIB_UNAVAILABLE) unavailable++;
        P("so far FAILED: %d, SUCCEEDED: %d, UNAVAILABLE: %d\n", failed, succeeded,
           unavailable);
    } while (res != TESTLIB_INVALID && (end < 0 || start <= end));

    P("FAILED: %d, SUCCEEDED: %d, UNAVAILABLE: %d\n", failed, succeeded,
           unavailable);

    /* also execute internal unit tests - this also verifies that we did not
       keep any references */
    exit_fn(ptr);

    return failed;
}

