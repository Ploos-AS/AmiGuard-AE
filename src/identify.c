#include <stdio.h>
#include <string.h>

#include "identify.h"

int amiguard_ae_identify_file(const char *path, AmiGuardAEIdentifyResult *out)
{
    FILE *fp;
    unsigned char hdr[4];
    size_t got;

    if (out == 0) return 0;
    out->ok = 0;
    out->type[0] = '\0';
    out->detail[0] = '\0';

    if (path == 0 || path[0] == '\0') {
        strcpy(out->detail, "invalid file path");
        return 0;
    }

    fp = fopen(path, "rb");
    if (fp == 0) {
        strcpy(out->detail, "cannot open file read-only");
        return 0;
    }

    got = fread(hdr, 1U, sizeof(hdr), fp);
    if (ferror(fp)) {
        fclose(fp);
        strcpy(out->detail, "file read failed");
        return 0;
    }
    fclose(fp);

    if (got >= 4U && hdr[0] == 0x00U && hdr[1] == 0x00U && hdr[2] == 0x03U && hdr[3] == 0xF3U) {
        strcpy(out->type, "AMIGA-HUNK");
        strcpy(out->detail, "HUNK_HEADER");
    } else if (got >= 4U && hdr[0] == 'F' && hdr[1] == 'O' && hdr[2] == 'R' && hdr[3] == 'M') {
        strcpy(out->type, "IFF");
        strcpy(out->detail, "FORM container");
    } else {
        strcpy(out->type, "DATA");
        strcpy(out->detail, "unrecognized binary/data");
    }

    out->ok = 1;
    return 1;
}
