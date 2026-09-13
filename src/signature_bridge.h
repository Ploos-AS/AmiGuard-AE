#ifndef AMIGUARD_AE_SIGNATURE_BRIDGE_H
#define AMIGUARD_AE_SIGNATURE_BRIDGE_H

#define AMIGUARD_AE_SIGNATURE_TYPE_MAX 16
#define AMIGUARD_AE_SIGNATURE_NAME_MAX 96

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

void amiguard_ae_signature_set_count_provider(AmiGuardAESignatureCountProvider provider);
void amiguard_ae_signature_set_info_provider(AmiGuardAESignatureInfoProvider provider);
int amiguard_ae_signature_available(void);
unsigned long amiguard_ae_signature_count(void);
int amiguard_ae_signature_info(unsigned long index, AmiGuardAESignatureInfo *out);

#endif
