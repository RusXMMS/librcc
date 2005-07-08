#ifndef _RCC_OPT_H
#define _RCC_OPT_H

struct rcc_option_defval_t {
    rcc_option option;
    rcc_option_value value;
};
typedef struct rcc_option_defval_t rcc_option_defval;

enum rcc_option_type_t {
    RCC_OPTION_TYPE_BOOLEAN = 0,
    RCC_OPTION_TYPE_RANGE,
    RCC_OPTION_TYPE_FLAGS,
    RCC_OPTION_TYPE_MENU
};
typedef enum rcc_option_type_t rcc_option_type;

struct rcc_option_description_t {
    rcc_option option;
    const char *sn;
    rcc_option_type;
    rcc_option_value min;
    rcc_option_value max;

};
typedef struct rcc_option_description_t rcc_option_description;


#endif /* _RCC_OPT_H */
