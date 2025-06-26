#include "golden_model.h"

void* golden_model(void* args) {
    gm_args* args_ = (gm_args*) args;
    args_->sort(args_->vector, args_->size);
}