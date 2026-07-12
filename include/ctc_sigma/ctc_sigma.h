#ifndef CTC_SIGMA_H
#define CTC_SIGMA_H

#include "ctc_sigma/arith.h"
#include "ctc_sigma/braid.h"
#include "ctc_sigma/braid_descriptor.h"
#include "ctc_sigma/branch.h"
#include "ctc_sigma/constants.h"
#include "ctc_sigma/dynamic_arith.h"
#include "ctc_sigma/encoder.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/fold.h"
#include "ctc_sigma/lehmer.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/permutation.h"
#include "ctc_sigma/round_schedule.h"
#include "ctc_sigma/sponge.h"
#include "ctc_sigma/status.h"
#include "ctc_sigma/validation.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *ctc_sigma_version(void);

#ifdef __cplusplus
}
#endif

#endif
