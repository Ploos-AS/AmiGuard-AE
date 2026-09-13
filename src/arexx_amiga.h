#ifndef AMIGUARD_AE_AREXX_AMIGA_H
#define AMIGUARD_AE_AREXX_AMIGA_H

typedef struct AmiGuardAERexxPort AmiGuardAERexxPort;

AmiGuardAERexxPort *amiguard_ae_arexx_open(void);
void amiguard_ae_arexx_close(AmiGuardAERexxPort *port);
unsigned long amiguard_ae_arexx_signal_mask(const AmiGuardAERexxPort *port);
int amiguard_ae_arexx_process(AmiGuardAERexxPort *port);

#endif
