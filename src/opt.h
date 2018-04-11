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

#ifndef _RCC_OPT_H
#define _RCC_OPT_H

#include "internal.h"

typedef const char *rcc_option_value_name;
typedef const rcc_option_value_name *rcc_option_value_names;

struct rcc_option_description_t {
    rcc_option option;
    rcc_option_value value;
    rcc_option_range range;
    rcc_option_type type;
    const char *sn;
    rcc_option_value_names vsn;

};
typedef struct rcc_option_description_t rcc_option_description;

const char *rccOptionDescriptionGetName(rcc_option_description *desc);
rcc_option rccOptionDescriptionGetOption(rcc_option_description *desc);
const char *rccOptionDescriptionGetValueName(rcc_option_description *desc, rcc_option_value value);
rcc_option_value rccOptionDescriptionGetValueByName(rcc_option_description *desc, const char *name);

#endif /* _RCC_OPT_H */
