/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2007-2020 Grape Systems, Inc.                          */
/*                                       All Rights Reserved                                    */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      grp_std_tools.h                                                         2.02            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Common standard function definition for GrapeWare                                       */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/09/10  2.00                                                            */
/*                              Created initial version 2.00                                    */
/*   K.Kaneko       2020/03/19  2.02                                                            */
/*                              Add parentheses to macro arguments.                             */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_STD_TOOLS_H_
#define _GRP_STD_TOOLS_H_

#include    <string.h>

#define     grp_std_memcmp(x,y,z)       memcmp((const void *)(x),(const void *)(y),(size_t)(z))
#define     grp_std_memcpy(x,y,z)       memcpy((void *)(x),(const void *)(y),(size_t)(z))
#define     grp_std_memset(x,y,z)       memset((void *)(x),(int)(y),(size_t)(z))
#define     grp_std_sprintf             sprintf
#define     grp_std_sscanf              sscanf
#define     grp_std_strcat(x,y)         strcat((char *)(x), (const char *)(y))
#define     grp_std_strlen(x)           strlen((const char *)(x))
#define     grp_std_strncpy(x,y,z)      strncpy((char *)(x),(const char *)(y),(size_t)(z))
#define     grp_std_strncmp(x,y,z)      strncmp((const char *)(x),(const char *)(y),(size_t)(z))
#define     grp_std_strstr(x,y)         strstr((const char *)(x),(const char *)(y))
#define     grp_std_strchr(x,y)         strchr((const char *)(x),(int)(y))
#define     grp_std_strcpy(x,y)         strcpy((char *)(x),(const char *)(y))

#define     grp_std_swap16(x)                                               \
                (((x) & 0xff00) >> 0x08 | ((x) & 0x00ff) << 0x08)

#define     grp_std_swap32(x)                                               \
                (((x) & 0xff000000) >> 0x18 | ((x) & 0x00ff0000) >> 0x08    \
               | ((x) & 0x0000ff00) << 0x08 | ((x) & 0x000000ff) << 0x18)


#endif  /* _GRP_STD_TOOLS_H_ */

