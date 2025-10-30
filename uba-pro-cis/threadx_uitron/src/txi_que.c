/********************************************************************************/
/*                Copyright (C) 2002-2017 Grape Systems, Inc.                   */
/*                     All Rights Reserved.                                     */
/*                                                                              */
/*  This software is furnished under a license and may be used and copied only  */
/*  in accordance with the terms of such license and with the inclusion of the  */
/*  above copyright notice. No title to and ownership of the software is        */
/*  transferred.                                                                */
/*  Grape Systems Inc. makes no warranties with respect to the performance of   */
/*  this computer program, and specifically disclaims any responsibility for    */
/*  any damages, special or consequential, connected with the use of this       */
/*  program.                                                                    */
/*                                                                              */
/********************************************************************************/

/********************************************************************************/
/*  txi_que.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Queue control for internal                                                  */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

#ifdef TXI_DISABLE_INLINE
void _kernel_enque(_KERNEL_UITRON_OBJ** ppTop, _KERNEL_UITRON_OBJ* pNode)
{
    if(*ppTop==NULL)
    {
        /* Insert tail node. */
        pNode->prev = pNode;
        pNode->next = pNode;
        *ppTop = pNode;
    }
    else
    {
        /* pNode is inserted at the tail. */
        pNode->prev = (*ppTop)->prev;
        pNode->next = *ppTop;
        (*ppTop)->prev->next = pNode;
        (*ppTop)->prev = pNode;
    }
    pNode->plist = ppTop;
}

void _kernel_deque(_KERNEL_UITRON_OBJ* pNode)
{
    _KERNEL_UITRON_OBJ** ppTop;

    if(pNode->plist!=NULL)
    {
        ppTop = pNode->plist;
        if(pNode->next==pNode)
        {
            /* Remove all. */
            (*ppTop) = NULL;
        }
        else
        {
            /* A node remove from queue. */
            (pNode->next)->prev = pNode->prev;
            (pNode->prev)->next = pNode->next;

            if((*ppTop)==pNode)
            {
                /* A top node is exchanged. */
                (*ppTop) = pNode->next;
            }
        }
        pNode->plist = NULL;
    }
}
#endif

void _kernel_enque_task_pri(_KERNEL_UITRON_OBJ** ppTop, _KERNEL_UITRON_OBJ* pNode)
{
    _KERNEL_UITRON_OBJ* pFindNode;
    PRI pri;
    _KERNEL_UITRON_TSK* pTcb;

    if(*ppTop==NULL)
    {
        /* Insert tail node. */
        pNode->prev = pNode;
        pNode->next = pNode;
        *ppTop = pNode;
    }
    else
    {
        /* pNode is inserted by priority. */
        pFindNode = *ppTop;
        pri = (PRI)(((_KERNEL_UITRON_TSK*)pNode->data)->thread.tx_thread_priority);
        do
        {
            pTcb = (_KERNEL_UITRON_TSK*)pFindNode->data;
            if(pri<(PRI)pTcb->thread.tx_thread_priority)
            {
                if(pFindNode==*ppTop)
                {
                    *ppTop = pNode;
                }
                break;
            }
            pFindNode = pFindNode->next;
        } while(pFindNode!=*ppTop);

        /* pNode is inserted before pFindNode. */
        pNode->prev = pFindNode->prev;
        pNode->next = pFindNode;
        pFindNode->prev->next = pNode;
        pFindNode->prev = pNode;
    }
    pNode->plist = ppTop;
}

