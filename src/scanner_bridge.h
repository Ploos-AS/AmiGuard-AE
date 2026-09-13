#ifndef AMIGUARD_AE_SCANNER_BRIDGE_H
#define AMIGUARD_AE_SCANNER_BRIDGE_H

#define AMIGUARD_AE_SCAN_PATH_MAX 192
#define AMIGUARD_AE_SCAN_DETAIL_MAX 128

typedef enum AmiGuardAEScanStatus {
    AMIGUARD_AE_SCAN_ERROR = -1,
    AMIGUARD_AE_SCAN_CLEAN = 0,
    AMIGUARD_AE_SCAN_INFECTED = 1,
    AMIGUARD_AE_SCAN_SUSPICIOUS = 2
} AmiGuardAEScanStatus;

typedef struct AmiGuardAEScanResult {
    AmiGuardAEScanStatus status;
    char detail[AMIGUARD_AE_SCAN_DETAIL_MAX];
} AmiGuardAEScanResult;

typedef int (*AmiGuardAEScanFileProvider)(const char *path, AmiGuardAEScanResult *out);

void amiguard_ae_scanner_set_provider(AmiGuardAEScanFileProvider provider);
int amiguard_ae_scanner_available(void);
int amiguard_ae_scanner_scan_file(const char *path, AmiGuardAEScanResult *out);

#endif
