#ifndef AMIGUARD_AE_SIGNATURE_BRIDGE_H
#define AMIGUARD_AE_SIGNATURE_BRIDGE_H

#define AMIGUARD_AE_SIGNATURE_TYPE_MAX 16
#define AMIGUARD_AE_SIGNATURE_NAME_MAX 96
#define AMIGUARD_AE_SIGNATURE_UPDATE_DETAIL_MAX 128

typedef struct AmiGuardAESignatureInfo {
    char type[AMIGUARD_AE_SIGNATURE_TYPE_MAX];
    char name[AMIGUARD_AE_SIGNATURE_NAME_MAX];
    unsigned long offset;
    unsigned long length;
    int test_only;
} AmiGuardAESignatureInfo;

typedef unsigned long (*AmiGuardAESignatureCountProvider)(void);
typedef int (*AmiGuardAESignatureInfoProvider)(unsigned long index,
                                               AmiGuardAESignatureInfo *out);
typedef int (*AmiGuardAESignatureUpdateProvider)(const char *path,
                                                 char *detail,
                                                 unsigned long detail_size);

void amiguard_ae_signature_set_count_provider(AmiGuardAESignatureCountProvider provider);
void amiguard_ae_signature_set_info_provider(AmiGuardAESignatureInfoProvider provider);
void amiguard_ae_signature_set_update_provider(AmiGuardAESignatureUpdateProvider provider);
int amiguard_ae_signature_available(void);
int amiguard_ae_signature_update_available(void);
unsigned long amiguard_ae_signature_count(void);
int amiguard_ae_signature_info(unsigned long index, AmiGuardAESignatureInfo *out);
int amiguard_ae_signature_update(const char *path,
                                 char *detail,
                                 unsigned long detail_size);

#endif
