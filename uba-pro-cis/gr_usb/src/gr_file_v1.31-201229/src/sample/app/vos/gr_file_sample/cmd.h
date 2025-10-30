#ifndef TEST_CMD_H_
#define TEST_CMD_H_
/*************************************************************************/
/*                                                                       */
/* Copyright(C) 2002-2008 Grape Systems, Inc.                            */
/*                        All Rights Reserved                            */
/*                                                                       */
/* This software is furnished under a license and may be used and copied */
/* only in accordance with the terms of such license and with the        */
/* inclusion of the above copyright notice. No title to and ownership of */
/* the software is transferred.                                          */
/* Grape Systems Inc. makes no representation or warranties with respect */
/* to the performance of this computer program, and specifically         */
/* disclaims any responsibility for any damages, special or              */
/* consequential, connected with the use of this program.                */
/*                                                                       */
/*************************************************************************/
/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      cmd.h                                              1.02          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      GR-FILE Sample program                                           */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*   NAME         DATE        REMARKS                                    */
/*                                                                       */
/*   K.Kaneko     2008/06/23  version 1.02                               */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/* test command engine parameters                                        */
/*************************************************************************/
#define TEST_CMD_LBUF_SIZE  256             /* command line buffer */
#define TEST_CMD_LBUF_NUM   8               /* command line buffer */
#define TEST_CMD_MAX_ARGS   16              /* max argument count */
#define TEST_CMD_FILE_NAME  64              /* max length of cmd file name */


/*************************************************************************/
/* defines                                                               */
/*************************************************************************/
typedef struct test_cmd test_cmd_t;         /* command table */
typedef int test_cmd_func_t(
    test_cmd_t *cmd, int ac, unsigned char **av);/* cmd proc func */

struct test_cmd {                           /* command table */
    char            *cmd_name;              /* command name */
    test_cmd_func_t *cmd_func;              /* command function */
    char            *cmd_desc;              /* command description */
    char            *cmd_desc_details;      /* command description details */
};


/*************************************************************************/
/* prototypes                                                            */
/*************************************************************************/
int test_cmd_init(int ac, char **av);       /* command initalize */
int test_cmd_interpret(int id);             /* command interpret loop */
int test_cmd_eval_arg(unsigned char *str, int *eval_val);
                                            /* evaluate argument */


/*************************************************************************/
/* imported variables                                                    */
/*************************************************************************/
extern test_cmd_t test_cmd_tbl[];           /* test command table */


#endif  /* TEST_CMD_H_ */
