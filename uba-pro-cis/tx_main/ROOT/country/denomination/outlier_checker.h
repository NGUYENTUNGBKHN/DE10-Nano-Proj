#ifndef _OUTLIER_CHECKER_H_
#define _OUTLIER_CHECKER_H_

//#include "MLPModule.h"

#define FMAX_MIN    0.7
#define FMAX_MAX    0.95
#define TMAX_MAX    0.02


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

u16 check_is_outlier(NN_Tensor *x);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif/*_OUTLIER_CHECKER_H_*/
