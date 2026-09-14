#ifndef AMIGUARD_AE_SIGNATURE_AUTH_ED25519_H
#define AMIGUARD_AE_SIGNATURE_AUTH_ED25519_H

int amiguard_ae_ed25519_auth_available(void);
int amiguard_ae_ed25519_authenticate(const char *manifest_path,
                                     const char *database_path,
                                     const char *crc32,
                                     char *detail,
                                     unsigned long detail_size);
void amiguard_ae_ed25519_auth_commit(void);
unsigned long amiguard_ae_ed25519_committed_sequence(void);

#endif
