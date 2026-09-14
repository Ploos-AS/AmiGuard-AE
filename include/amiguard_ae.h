#ifndef AMIGUARD_AE_H
#define AMIGUARD_AE_H

#define AMIGUARD_AE_NAME "AmiGuard AE"
#define AMIGUARD_AE_AREXX_PORT "AMIGUARD"
#define AMIGUARD_AE_VERSION_MAJOR 0
#define AMIGUARD_AE_VERSION_MINOR 4
#define AMIGUARD_AE_VERSION_PATCH 0

/* AmigaDOS/ARexx-style return codes used by the public command surface. */
#define AMIGUARD_AE_RC_OK 0
#define AMIGUARD_AE_RC_WARN 5
#define AMIGUARD_AE_RC_ERROR 10
#define AMIGUARD_AE_RC_FAIL 20

/* Scanner/backend boundary remains independent from RexxMast. */
typedef enum AmiGuardAEStatus {
    AMIGUARD_AE_OK = 0,
    AMIGUARD_AE_ERROR = 10,
    AMIGUARD_AE_BAD_ARGUMENT = 20,
    AMIGUARD_AE_NOT_AVAILABLE = 30
} AmiGuardAEStatus;

const char *amiguard_ae_version_string(void);

#endif
