
#define EXT

#include "../common/global.h"
#include "outlier_checker.h"

#define ERR_FAILURE_NEW_NN_OUTPUT_LACK	        0x0c0c	//　発火値がおかしい最大発火が閾値以下、もしくは、第2発火値が閾値以上
#define ERR_FAILURE_NEW_NN_MISMATCH_CONDITIONS	0x0c0d	//　

u16 check_is_outlier(NN_Tensor *x)
{
    if (x->data[0] > FMAX_MIN && x->data[4] < TMAX_MAX)
    {
        if ((u16)(x->data[1] - x->data[3]) % 4 == 0)
        {
            return 0;
        }
        else if (x->data[0] > FMAX_MAX)
        {
            return 0;
        }
        else
        {
            return ERR_FAILURE_NEW_NN_MISMATCH_CONDITIONS;
        }
    }
    else
    {
        return ERR_FAILURE_NEW_NN_OUTPUT_LACK;
    }
}
