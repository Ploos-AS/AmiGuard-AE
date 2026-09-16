#ifndef AMIGUARD_AE_AUDIT_LOG_H
#define AMIGUARD_AE_AUDIT_LOG_H

#define AMIGUARD_AE_AUDIT_PATH_MAX 256
#define AMIGUARD_AE_AUDIT_DETAIL_MAX 128

int amiguard_ae_audit_set_path(const char *path, char *detail,
                               unsigned long detail_size);
const char *amiguard_ae_audit_path(void);
int amiguard_ae_audit_available(void);
int amiguard_ae_audit_append(const char *event, const char *status,
                             const char *subject, const char *detail,
                             char *error, unsigned long error_size);

#endif
