/*
  LibRCC - abstraction for numerical and boolean configuration options

  Copyright (C) 2005-2018 Suren A. Chilingaryan <csa@suren.me>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <string.h>

#include "rccconfig.h"
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
    rcc_option_description *desc;
    rcc_option_value min, max;
    
    
    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return -1;
	
    
    desc = rccGetOptionDescription(option);
    if (desc) {
	// DS: More checks for different range types
	min = desc->range.min;
	max = desc->range.max;
	if ((min)&&(min!=max)) {
	    if ((option<min)||(option>max)) return -1;
	}
    }
    
    ctx->default_options[option] = 0;

    if (ctx->options[option] != value) {
	rccMutexLock(ctx->mutex);
	ctx->configure = 1;
	ctx->options[option]=value;
	rccMutexUnLock(ctx->mutex);
    }
    
    return 0;
}

int rccOptionSetDefault(rcc_context ctx, rcc_option option) {
    rcc_option_description *desc;
    rcc_option_value value;

    if (!ctx) {
	if (rcc_default_ctx) ctx = rcc_default_ctx;
	else return -1;
    }
    if ((option<0)||(option>=RCC_MAX_OPTIONS)) return -1;

    ctx->default_options[option] = 1;

    desc = rccGetOptionDescription(option);
    if (desc) value = desc->value;
    else value = 0;

    if (ctx->options[option] != value) {
	ctx->configure = 1;
	ctx->options[option]=value;
    }
    
    return 0;
}

rcc_option_type rccOptionGetType(rcc_context ctx, rcc_option option) {
    rcc_option_description *desc;

    desc = rccGetOptionDescription(option);
    if (desc) return desc->type;
    return 0;
}

rcc_option_range *rccOptionGetRange(rcc_context ctx, rcc_option option) {
    rcc_option_description *desc;

    desc = rccGetOptionDescription(option);
    if (desc) return &desc->range;
    return 0;
}

const char *rccOptionDescriptionGetName(rcc_option_description *desc) {
    if (desc) return desc->sn;
    return NULL;
}

rcc_option rccOptionDescriptionGetOption(rcc_option_description *desc) {
    if (desc) return desc->option;
    return (rcc_option)-1;
}

const char *rccOptionDescriptionGetValueName(rcc_option_description *desc, rcc_option_value value) {
    unsigned int i;
    
    if ((desc)&&(desc->vsn)) {
	for (i=0;desc->vsn[i];i++) {
	    if (i == value) return desc->vsn[i];
	}
    }
    return NULL;
}

rcc_option_value rccOptionDescriptionGetValueByName(rcc_option_description *desc, const char *name) {
    unsigned int i;

    if ((desc)&&(desc->vsn)&&(name)) {
	for (i=0;desc->vsn[i];i++) {
	    if (!strcasecmp(desc->vsn[i], name)) return (rcc_option_value)i;
	}
    }

    return (rcc_option_value)-1;
}


const char *rccGetOptionName(rcc_option option) {
    rcc_option_description *desc;
    
    desc = rccGetOptionDescription(option);
    return rccOptionDescriptionGetName(desc);
}

const char *rccGetOptionValueName(rcc_option option, rcc_option_value value) {
    rcc_option_description *desc;
    
    desc = rccGetOptionDescription(option);
    return rccOptionDescriptionGetValueName(desc, value);
}

rcc_option rccGetOptionByName(const char *name) {
    rcc_option_description *desc;
    
    desc = rccGetOptionDescriptionByName(name);
    return rccOptionDescriptionGetOption(desc);
}

rcc_option_value rccGetOptionValueByName(rcc_option option, const char *name) {
    rcc_option_description *desc;
    
    desc = rccGetOptionDescription(option);
    return rccOptionDescriptionGetValueByName(desc, name);
}
