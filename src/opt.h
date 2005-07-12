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
