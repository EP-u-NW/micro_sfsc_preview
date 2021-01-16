#ifndef ESP_IDF_PLATFORM
#define ESP_IDF_PLATFORM
#include "esp_idf_config.h"
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef MULTI_TASK
    void lock_init();
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* ESP_IDF_PLATFORM */
