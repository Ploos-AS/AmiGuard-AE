#ifndef AMIGUARD_AE_QUARANTINE_MODEL_H
#define AMIGUARD_AE_QUARANTINE_MODEL_H

#define AMIGUARD_AE_QUARANTINE_ID_MAX 17
#define AMIGUARD_AE_QUARANTINE_PATH_MAX 256
#define AMIGUARD_AE_QUARANTINE_DETAIL_MAX 128

typedef struct AmiGuardAEQuarantinePlan {
    char id[AMIGUARD_AE_QUARANTINE_ID_MAX];
    char source[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    unsigned long source_crc32;
    unsigned long source_size;
} AmiGuardAEQuarantinePlan;

int amiguard_ae_quarantine_path_safe(const char *path,
                                     char *detail,
                                     unsigned long detail_size);
int amiguard_ae_quarantine_plan(const char *path,
                                AmiGuardAEQuarantinePlan *plan,
                                char *detail,
                                unsigned long detail_size);

/* M4.2 store: stage -> verify -> commit metadata -> remove source. */
int amiguard_ae_quarantine_store(const AmiGuardAEQuarantinePlan *plan,
                                 const char *directory,
                                 char *stored_path,
                                 unsigned long stored_path_size,
                                 char *detail,
                                 unsigned long detail_size);

#endif
