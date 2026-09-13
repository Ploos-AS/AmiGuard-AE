#ifndef AMIGUARD_AE_RESULT_STORE_H
#define AMIGUARD_AE_RESULT_STORE_H

#include "scanner_bridge.h"

#define AMIGUARD_AE_RESULT_PATH_MAX 192
#define AMIGUARD_AE_RESULT_DETAIL_MAX 128

void amiguard_ae_result_clear(void);
void amiguard_ae_result_record(const char *path, const AmiGuardAEScanResult *scan);
int amiguard_ae_result_valid(void);
const char *amiguard_ae_result_status(void);
const char *amiguard_ae_result_path(void);
const char *amiguard_ae_result_detail(void);

#endif
