#include <stdio.h>

#include "internal.h"
#include "opt.h"

rcc_option_value rccGetOption(rcc_context ctx, rcc_option option) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return (rcc_option_value)0;
    }
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return 0;
    
    return ctx->options[option];
}

int rccOptionIsDefault(rcc_context ctx, rcc_option option) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return -1;

    return ctx->default_options[option];
}

int rccSetOption(rcc_context ctx, rcc_option option, rcc_option_value value) {
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return -1;

    ctx->default_options[option] = 0;

    if (ctx->options[option] != value) {
	ctx->configure = 1;
	ctx->options[option]=value;
    }
    
    return 0;
}

int rccOptionSetDefault(rcc_context ctx, rcc_option option) {
    rcc_option_value value;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return -1;

    ctx->default_options[option] = 1;
    value = rccGetOptionDefaultValue(option);

    if (ctx->options[option] != value) {
	ctx->configure = 1;
	ctx->options[option]=value;
    }
    
    return 0;
}
