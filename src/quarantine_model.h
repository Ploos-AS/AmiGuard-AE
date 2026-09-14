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

/* M4.1 is planning-only: these APIs never move, delete or overwrite files. */
int amiguard_ae_quarantine_path_safe(const char *path,
                                     char *detail,
                                     unsigned long detail_size);
int amiguard_ae_quarantine_plan(const char *path,
                                AmiGuardAEQuarantinePlan *plan,
                                char *detail,
                                unsigned long detail_size);

#endif
