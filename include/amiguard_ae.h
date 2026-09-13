#ifndef AMIGUARD_AE_H
#define AMIGUARD_AE_H

#define AMIGUARD_AE_NAME "AmiGuard AE"
#define AMIGUARD_AE_AREXX_PORT "AMIGUARD"
#define AMIGUARD_AE_VERSION_MAJOR 0
#define AMIGUARD_AE_VERSION_MINOR 0
#define AMIGUARD_AE_VERSION_PATCH 0

/*
 * Scanner/backend boundary.
 * M0 intentionally contains no antivirus implementation.  The interface is
 * kept separate from the ARexx transport so the scanner remains usable and
 * testable without RexxMast.
 */
typedef enum AmiGuardAEStatus {
    AMIGUARD_AE_OK = 0,
    AMIGUARD_AE_ERROR = 10,
    AMIGUARD_AE_BAD_ARGUMENT = 20,
    AMIGUARD_AE_NOT_AVAILABLE = 30
} AmiGuardAEStatus;

const char *amiguard_ae_version_string(void);

#endif
