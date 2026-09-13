/* AmiGuard AE ARexx example; becomes runnable in M1. */
ADDRESS AMIGUARD
'PING'
IF RC ~= 0 THEN EXIT RC
'VERSION'
EXIT RC
