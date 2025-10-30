/*************************************************************************/
/*                                                                       */
/* Copyright(C) 2002-2010 Grape Systems, Inc.                            */
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
/*      cmd_line.c                                         0.03          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      TEST COMMAND ENGINE UTILITY Sample program                       */
/*                                                                       */
/* FUNCTIONS:                                                            */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*   NAME         DATE        REMARKS                                    */
/*                                                                       */
/*   K.Kaneko     2008/06/23  Version 0.02                               */
/*   K.Kaneko     2010/11/17  Version 0.03                               */
/*                            Added GRP_FS_MINIMIZE_LEVEL option for     */
/*                            GR-FILE minimize level                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/* includes                                                              */
/*************************************************************************/
#include <stdarg.h>
#include <string.h>

#include "grp_fs_sysdef.h"
#include "grp_types.h"
#include "fat.h"
#include "grp_stdio.h"

#include "cmd.h"
#include "usb_test.h"


/*************************************************************************/
/* locals                                                                */
/*************************************************************************/
static unsigned char cmd_line_buf[TEST_CMD_LBUF_NUM][TEST_CMD_LBUF_SIZE];   /* line buffer */
static unsigned char cmd_arg_a_buf[TEST_CMD_LBUF_SIZE];                     /* cmd arg buffer */
static unsigned char cmd_arg_b_buf[TEST_CMD_LBUF_SIZE];                     /* cmd arg buffer */
static unsigned char *cmd_arg_list[2][TEST_CMD_MAX_ARGS];                   /* argument list */
static int      cmd_cur_buf = 0;                                            /* current cmd buf */
static int      cmd_bef_buf = 0;                                            /* before cmd buf */
static int      cmd_count = 1;                                              /* command count */


/*************************************************************************/
/* locals                                                                */
/*************************************************************************/
grp_fs_sem_t   g_tMainSem;


/*************************************************************************/
/* prototypes                                                            */
/*************************************************************************/
static int cmd_eval_expr(unsigned char **str, int *eval_val);


/*************************************************************************/
/* FUNCTION   : cmd_output_prompt                                        */
/*                                                                       */
/* DESCRIPTION: output prompt                                            */
/*************************************************************************/
#if(GRP_FS_MINIMIZE_LEVEL < 1)
static void cmd_output_prompt(FILE *fp, char *fname, int cmd_cnt)
{
	fseek(fp, 0, SEEK_END);
	if (fname)
		fprintf(fp, "cmd:%s %d> ", fname, cmd_cnt);
	else
		fprintf(fp, "cmd %d> ",  cmd_cnt);
	fflush(fp);
}
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
static void cmd_output_prompt(char *fname, int cmd_cnt)
{
	if (fname)
		grp_fs_printf("cmd:%s %d> ", fname, cmd_cnt);
	else
		grp_fs_printf("cmd %d> ",	cmd_cnt);
}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */

/*************************************************************************/
/* FUNCTION   : get_cmd_line                                             */
/*                                                                       */
/* DESCRIPTION: get command line                                         */
/*************************************************************************/
static int get_cmd_line(void)
{
	unsigned char *p, *ep;
	int c, i;
	int fgloop;
	int fgescseq, iescseq;

	if('\0' != cmd_line_buf[0][0]){
		for(i = TEST_CMD_LBUF_NUM - 1;i >= 0;i--){
			strcpy((char *)cmd_line_buf[i], (char *)cmd_line_buf[i-1]);
		}
	}
	strcpy((char *)cmd_line_buf[0], "");

	cmd_bef_buf = cmd_cur_buf = 0;          /* switch command line buffer */
	p = cmd_line_buf[0];                    /* command line buffer */
	ep = &p[TEST_CMD_LBUF_SIZE - 1];        /* end of command line buffer */

	fgloop = 1;
	fgescseq = 0;
	while(fgloop){
#if(GRP_FS_MINIMIZE_LEVEL < 1)
		c = getc(stdin);					/* get char */
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
		c = cons_getchar(0);					/* get char */
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
		
		switch(c){
			case '\b':
				if (p > cmd_line_buf[cmd_cur_buf]){ /* not top */
					p--;                    /* erase previous character */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
					putchar(c);
					putchar(' ');
					putchar(c);
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
					cons_putchar(c);
					cons_putchar(' ');
					cons_putchar(c);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
				}
				break;
			
			case 0x1b:
				fgescseq = 1;
				iescseq = 0;
				break;
				
			case '\a':
			case '\t':
			case '\r':
			case '\f':
			case '\v':
				break;
			
			case '\n':
				fgloop = 0;
#if(GRP_FS_MINIMIZE_LEVEL < 1)
				putchar(c);
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
				cons_putchar(c);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
				break;
			
			default:
				if(fgescseq){
					if( (0 == iescseq) && ('[' == c) ){
						iescseq++;
					} else if( (1 == iescseq) && ('A' == c) ){
						cmd_bef_buf = cmd_cur_buf;
						cmd_cur_buf = (TEST_CMD_LBUF_NUM-1 > cmd_cur_buf) ? cmd_cur_buf + 1: TEST_CMD_LBUF_NUM-1;
						if(0x00 == cmd_line_buf[cmd_cur_buf][0]){
							cmd_cur_buf--;
						}
						iescseq++;
						fgescseq = 0;
					} else if( (1 == iescseq) && ('B' == c) ){
						cmd_bef_buf = cmd_cur_buf;
						cmd_cur_buf = (0 < cmd_cur_buf) ? cmd_cur_buf - 1: 0;
						iescseq++;
						fgescseq = 0;
					} else {
						fgescseq = 0;
					}
					if(2 == iescseq){
						while(p != &cmd_line_buf[cmd_bef_buf][0]){
							p--;                                /* erase previous character */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
							putchar('\b');
							putchar(' ');
							putchar('\b');
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
							cons_putchar(c);
							cons_putchar(' ');
							cons_putchar(c);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
						}
						p = cmd_line_buf[cmd_cur_buf];          /* command line buffer */
						ep = &p[TEST_CMD_LBUF_SIZE - 1];        /* end of command line buffer */
						while(*p){
#if(GRP_FS_MINIMIZE_LEVEL < 1)
							putchar(*p);
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
							cons_putchar(*p);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
							p++;
						}
					}
				} else {
					*p++ = (unsigned char)c;/* put in line buffer */
#if(GRP_FS_MINIMIZE_LEVEL < 1)
					putchar(c);
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
					cons_putchar(c);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
				}
				break;
		}
		if (p >= ep)                        /* over buffer */
			break;                          /* break */
	}
	*p = 0;                                 /* null terminate */
	if (p > cmd_line_buf[cmd_cur_buf] && p[-1] == '\r') /* carrige return */
		*(--p) = 0;                         /* null terminate */
	strcpy((char *)cmd_line_buf[0], (char *)cmd_line_buf[cmd_cur_buf]);
	return(p - cmd_line_buf[cmd_cur_buf]);  /* return command line size */
}

/*************************************************************************/
/* FUNCTION   : get_cmd_arg                                              */
/*                                                                       */
/* DESCRIPTION: get cmd argument                                         */
/*************************************************************************/
static int get_cmd_arg( int tnum )
{
	unsigned char   *p;
	int             i;
	unsigned char   *cmd_arg_buf; /* for common of both tasks */
	int             cur;

	if( tnum == 0 ){
		cmd_arg_buf = cmd_arg_a_buf;
	}
	else{
		cmd_arg_buf = cmd_arg_b_buf;
	}

	cur = cmd_cur_buf;

	strcpy((char *)cmd_arg_buf, (char *)cmd_line_buf[cur]);
											/* copy to arg buffer */
	p = cmd_arg_buf;                        /* argument buffer */
	for (i = 0; i < TEST_CMD_MAX_ARGS - 1; i++) {
		for ( ; *p == ' ' || *p == '\t'; p++);  /* skip white space */
		if (*p == '#')                      /* start of comment */
			break;                          /* break */
		if (*p == 0) {                      /* end of argument */
			cmd_arg_list[tnum][i] = NULL;   /* set end */
			break;                          /* break */
		}
		if (*p == '"') {
			cmd_arg_list[tnum][i] = ++p;    /* set pointer */
			for ( ; *p != '"' && *p; p++);  /* advance to matching '"' */
		} else {
			cmd_arg_list[tnum][i] = p;      /* set pointer */
			for ( ; *p != ' ' && *p != '\t' && *p; p++); /* advance to next */
		}
		if (*p)                             /* not EOL */
			*p++ = 0;                       /* null terminate */
	}
	return(i);                              /* return argument count */
}

/*************************************************************************/
/* FUNCTION   : cmd_eval_factor                                          */
/*                                                                       */
/* DESCRIPTION: evaluate factor                                          */
/*************************************************************************/
static int cmd_eval_factor(unsigned char **str, int *eval_val)
{
	unsigned char *p;                       /* string pointer */
	int val = 0;                            /* evaluated value */
	int ret;                                /* return value */
	int base;                               /* base value */

	*eval_val = 0;                          /* set default value */
	for (p = *str; *p == ' ' || *p == '\t'; p++);   /* skip blank */
	*str = p;                               /* set pointer */
	if (*p == 0)                            /* end of expression */ 
		return(-1);                         /* error */
	switch(*p) {
	case '-':                               /* minus */
		p++;                                /* advance */
		ret = cmd_eval_factor(&p, &val);    /* eval factor */
		if (ret == 0)                       /* success */
			val = -val;                     /* - value */
		break;
	case '~':                               /* tilda */
		p++;                                /* advance */
		ret = cmd_eval_factor(&p, &val);    /* eval factor */
		if (ret == 0)                       /* success */
			val = ~val;                     /* bit negate value */
		break;
	case '(':                               /* left parenthesis */
		p++;
		ret = cmd_eval_expr(&p, &val);      /* evaluate expression */
		if (*p != ')')                      /* not right parenthesis */
			ret = -1;                       /* error */
		else                                /* right parenthesis */
			p++;                            /* advance it */
		break;
	default:
		ret = 0;                            /* return value */
		if ('0' <= *p && *p <= '9') {       /* digit */
			if (*p == '0') {
				if (p[1] == 'x') {          /* hex */
					p += 2;                 /* advance 2 char */
					base = 16;              /* base 16 */
				} else {                    /* octal */
					p++;                    /* advance 1 char */
					base = 8;               /* base 8 */
				}
			} else {
				base = 10;                  /* base 10 */
			}
			val = 0;                        /* init value */
			for ( ; ('0' <= *p && *p <= '9') || ('a' <= *p && *p <= 'f'); p++) {
				if ('0' <= *p && *p <= '9')             /* digit */
					val = val * base + *p - '0';        /* digit value */
				else                                    /* hex */
					val = val * base + *p - 'a' + 10;   /* hex value */
			}
		} 
		break;
	}
	*str = p;                               /* set current pointer */
	*eval_val = val;                        /* set evaluated value */
	return(ret);                            /* return success/error */
}

/*************************************************************************/
/* FUNCTION   : cmd_eval_term                                            */
/*                                                                       */
/* DESCRIPTION: evaluate term                                            */
/*************************************************************************/
static int cmd_eval_term(unsigned char **str, int *eval_val)
{
	unsigned char *p;                       /* string pointer */
	int val1, val2;                         /* evaluated value */
	int ret;                                /* return value */
	int op;                                 /* operator */

	for (p = *str; *p == ' ' || *p == '\t'; p++);/* skip blank */
	ret = cmd_eval_factor(&p, &val1);           /* evaluate left term */
	while (ret == 0
		 && (*p == '*' || *p == '/' || *p == '%' || *p == '&' || *p == '^'
			 || (*p == '>' && p[1] == '>')
			 || (*p == '<' && p[1] == '<'))) {  /* term op */
		op = *p++;                          /* operator */
		if (op == '>' || op == '<')         /* shift */
			p++;                            /* advance one more char */
		ret = cmd_eval_factor(&p, &val2);   /* evaluate right term */
		if (ret != 0)                       /* error */
			break;                          /* break */
		switch(op) {
		case '*':                           /* star */
			val1 *= val2;                   /* multiply value */
			break;                          /* break */
		case '/':                           /* slash */
			if (val2 == 0) {                /* divide by 0 */
				ret = -1;                   /* return error */
			} else {
				val1 /= val2;               /* divide value */
			}
			break;
		case '%':                           /* percent */
			if (val2 == 0) {                /* divide by 0 */
				ret = -1;                   /* return error */
			} else {
				val1 %= val2;               /* modulus */
			}
			break;
		case '&':                           /* and */
			val1 &= val2;                   /* and value */
			break;
		case '^':                           /* circonflex */
			val1 ^= val2;                   /* exclusive or */
			break;
		case '>':                           /* right bracket */
			val1 >>= val2;                  /* shift right */
			break;
		case '<':                           /* left bracket */
			val1 <<= val2;                  /* shift left */
			break;
		}
		for ( ; *p == ' ' || *p == '\t'; p++);/* skip blank */
	}
	*str = p;                               /* set current pointer */
	*eval_val = val1;                       /* set evaluated value */
	return(ret);                            /* return success/error */
}

/*************************************************************************/
/* FUNCTION   : cmd_eval_expr                                            */
/*                                                                       */
/* DESCRIPTION: evaluate expression                                      */
/*************************************************************************/
static int cmd_eval_expr(unsigned char **str, int *eval_val)
{
	unsigned char *p;                       /* string pointer */
	int val1, val2;                         /* evaluated value */
	int ret;                                /* return value */
	int op;                                 /* operator */

	for (p = *str; *p == ' ' || *p == '\t'; p++);   /* skip blank */
	ret = cmd_eval_term(&p, &val1);         /* evaluate left term */
	while (ret == 0 && (*p == '+' || *p == '-' || *p == '|')) { /* expr op */
		op = *p++;                          /* operator */
		ret = cmd_eval_term(&p, &val2);     /* evaluate right term */
		if (ret != 0)                       /* error */
			break;                          /* break */
		switch(op) {
		case '+':                           /* add */
			val1 += val2;                   /* add value */
			break;                          /* break */
		case '-':                           /* minus */
			val1 -= val2;                   /* substruct value */
			break;
		case '|':                           /* or */
			val1 |= val2;                   /* bit or value */
			break;
		}
		for ( ; *p == ' ' || *p == '\t'; p++);/* skip blank */
	}
	*str = p;                               /* set current pointer */
	*eval_val = val1;                       /* set evaluated value */
	return(ret);                            /* return success/error */
}

/*************************************************************************/
/* FUNCTION   : test_cmd_eval_arg                                        */
/*                                                                       */
/* DESCRIPTION: evaluate argument                                        */
/*************************************************************************/
int test_cmd_eval_arg(unsigned char *str, int *eval_val)
{
	int ret;                                /* return value */

	ret = cmd_eval_expr(&str, eval_val);    /* evaluate expression */
	if (ret == 0 && *str)                   /* non evaluated string left */
		ret = -1;                           /* error */
	return(ret);                            /* return success/error */
}

/*************************************************************************/
/* FUNCTION   : test_cmd_interpret                                       */
/*                                                                       */
/* DESCRIPTION: input and exec command                                   */
/*************************************************************************/
int test_cmd_interpret(int id)
{
	test_cmd_t  *cmd;                       /* command */
	int         cmd_ac;                     /* cmd arg count */
	int         task_num;
	char        name[8];
#if(GRP_FS_MINIMIZE_LEVEL >= 1)
	int	sprintf(char *pcBuf, const char *pcFormat, ...);
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */

	task_num = (int)id;
	sprintf( name, "%s%d", "main", id );

	if( grp_fs_get_sem( g_tMainSem ) != 0 )
		grp_fs_printf("!!cmd error\n");

#if(GRP_FS_MINIMIZE_LEVEL < 1)
	cmd_output_prompt(stdout, name, cmd_count); /* output prompt */
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
	cmd_output_prompt(name, cmd_count);			/* output prompt */
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
	if (get_cmd_line() < 0)                 /* get command line */
		goto err_interpret;                 /* return EOF */
	cmd_ac = get_cmd_arg(task_num);         /* parse argument */
	grp_fs_release_sem( g_tMainSem );

	if (cmd_ac == 0)                        /* no command */
		return(0);                          /* continue to next */

	cmd = lookup_cmd(cmd_arg_list[task_num][0]);/* lookup command */

	if (cmd == NULL) {                      /* no such command */
		grp_fs_printf("%s: not such command\n", cmd_arg_list[task_num][0]);
											/* output msg */
	} else {
		cmd->cmd_func(cmd, cmd_ac, &cmd_arg_list[task_num][0]);
											/* call cmd proc func */
	}
	cmd_count++;                            /* increment command count */

	return(0);

err_interpret:
	grp_fs_release_sem( g_tMainSem );
	return(-1);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_init                                            */
/*                                                                       */
/* DESCRIPTION: init test command                                        */
/*************************************************************************/
int test_cmd_init(int ac, char **av)
{
	int     ret;                        /* return value */

	ret = grp_fs_create_sem(&g_tMainSem, "FT_ST", 0, 1);

	return(ret);                        /* return */
}
