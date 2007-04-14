#ifndef _RCC_EXTERNAL_H
#define _RCC_EXTERNAL_H

typedef enum rcc_external_module_t {
    RCC_EXTERNAL_MODULE_CONTROL = 0,
    RCC_EXTERNAL_MODULE_OPTIONS,
    RCC_EXTERNAL_MODULE_LIBRTRANSLATE,
    RCC_EXTERNAL_MODULE_MAX
} rcc_external_module;

struct rcc_external_info_t {
    int s;
};
typedef struct rcc_external_info_t rcc_external_info_s;
typedef struct rcc_external_info_t *rcc_external_info;

typedef enum rcc_external_option_t {
    RCC_EXTERNAL_OPTION_OFFLINE = 0,
    RCC_EXTERNAL_OPTION_MAX
} rcc_external_option;

struct rcc_external_command_t {
    unsigned long size;
    unsigned char cmd;
};
typedef struct rcc_external_command_t rcc_external_command_s;
typedef struct rcc_external_command_t *rcc_external_command;

#define RCC_EXTERNAL_COMMAND_CLOSE 0

int rccExternalInit();
void rccExternalFree();

int rccExternalAllowOfflineMode();

size_t rccExternalWrite(int s, const char *buffer, ssize_t size, unsigned long timeout);
size_t rccExternalRead(int s, char *buffer, ssize_t size, unsigned long timeout);
int rccExternalConnect(unsigned char module);
void rccExternalClose(int s);

#endif /* _RCC_EXTERNAL_H */
