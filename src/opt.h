/*
  LibRCC - abstraction for numerical and boolean configuration options

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as published
  by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
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
