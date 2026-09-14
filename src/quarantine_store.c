#include "quarantine_model.h"
#include "checksum.h"

#include <stdio.h>
#include <string.h>

static char quarantine_directory[AMIGUARD_AE_QUARANTINE_PATH_MAX];

static void detail_set(char *detail, unsigned long size, const char *text)
{
    if (detail == 0 || size == 0UL) return;
    if (text == 0) text = "";
    strncpy(detail, text, (size_t)(size - 1UL));
    detail[size - 1UL] = '\0';
}

int amiguard_ae_quarantine_set_directory(const char *directory,
                                         char *detail,
                                         unsigned long detail_size)
{
    if (directory == 0 || directory[0] == '\0') {
        quarantine_directory[0] = '\0';
        detail_set(detail, detail_size, "quarantine directory cleared; quarantine disabled");
        return 1;
    }
    if (strlen(directory) >= sizeof(quarantine_directory)) {
        detail_set(detail, detail_size, "quarantine directory path too long");
        return 0;
    }
    if (strcmp(directory, "/") == 0 || strcmp(directory, ".") == 0 ||
        strcmp(directory, "..") == 0) {
        detail_set(detail, detail_size, "unsafe quarantine directory");
        return 0;
    }
    strcpy(quarantine_directory, directory);
    detail_set(detail, detail_size, "quarantine directory configured");
    return 1;
}

const char *amiguard_ae_quarantine_directory(void)
{
    return quarantine_directory;
}

int amiguard_ae_quarantine_available(void)
{
    return quarantine_directory[0] != '\0';
}

static int file_exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == 0) return 0;
    fclose(fp);
    return 1;
}

static int build_path(char *out, unsigned long size, const char *dir,
                      const char *name)
{
    unsigned long dl;
    unsigned long nl;
    if (out == 0 || size == 0UL || dir == 0 || name == 0) return 0;
    dl = (unsigned long)strlen(dir);
    nl = (unsigned long)strlen(name);
    if (dl + nl + 2UL > size) return 0;
    strcpy(out, dir);
    if (dl != 0UL && dir[dl - 1UL] != '/' && dir[dl - 1UL] != ':')
        strcat(out, "/");
    strcat(out, name);
    return 1;
}

static int copy_file(const char *source, const char *stage)
{
    FILE *in;
    FILE *out;
    unsigned char buffer[4096];
    size_t n;

    in = fopen(source, "rb");
    if (in == 0) return 0;
    out = fopen(stage, "wb");
    if (out == 0) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buffer, 1U, sizeof(buffer), in)) != 0U) {
        if (fwrite(buffer, 1U, n, out) != n) {
            fclose(out);
            fclose(in);
            remove(stage);
            return 0;
        }
    }
    if (ferror(in) != 0) {
        fclose(out);
        fclose(in);
        remove(stage);
        return 0;
    }
    if (fclose(out) != 0) {
        fclose(in);
        remove(stage);
        return 0;
    }
    fclose(in);
    return 1;
}

static int write_metadata(const AmiGuardAEQuarantinePlan *plan,
                          const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (fp == 0) return 0;
    if (fprintf(fp, "AMIGUARD-QUARANTINE 1\nID=%s\nSOURCE=%s\nCRC32=%08lX\nSIZE=%lu\n",
                plan->id, plan->source, plan->source_crc32,
                plan->source_size) < 0) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

int amiguard_ae_quarantine_store(const AmiGuardAEQuarantinePlan *plan,
                                 const char *directory,
                                 char *stored_path,
                                 unsigned long stored_path_size,
                                 char *detail,
                                 unsigned long detail_size)
{
    char destination[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char stage[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char metadata_stage[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char metadata[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char name[32];
    char metadata_name[32];
    char stage_name[32];
    char metadata_stage_name[32];
    AmiGuardAEChecksumResult sum;
    unsigned long actual_crc;

    if (plan == 0 || directory == 0 || directory[0] == '\0') {
        detail_set(detail, detail_size, "plan and quarantine directory required");
        return 0;
    }
    if (!amiguard_ae_quarantine_path_safe(plan->source, detail, detail_size))
        return 0;
    if (strlen(plan->id) == 0UL || strlen(plan->id) >= sizeof(name)) {
        detail_set(detail, detail_size, "invalid quarantine id");
        return 0;
    }

    sprintf(name, "%s.qtn", plan->id);
    sprintf(metadata_name, "%s.meta", plan->id);
    sprintf(stage_name, ".%s.stage", plan->id);
    sprintf(metadata_stage_name, ".%s.meta.stage", plan->id);
    if (!build_path(destination, sizeof(destination), directory, name) ||
        !build_path(stage, sizeof(stage), directory, stage_name) ||
        !build_path(metadata, sizeof(metadata), directory, metadata_name) ||
        !build_path(metadata_stage, sizeof(metadata_stage), directory, metadata_stage_name)) {
        detail_set(detail, detail_size, "quarantine path too long");
        return 0;
    }
    if (stored_path != 0 && stored_path_size > 0UL)
        stored_path[0] = '\0';

    if (file_exists(destination)) {
        detail_set(detail, detail_size, "quarantine destination exists");
        return 0;
    }
    if (file_exists(stage) || file_exists(metadata) || file_exists(metadata_stage)) {
        detail_set(detail, detail_size, "quarantine staging path busy");
        return 0;
    }

    if (!copy_file(plan->source, stage)) {
        detail_set(detail, detail_size, "staging copy failed; source preserved");
        return 0;
    }
    if (!amiguard_ae_checksum_crc32(stage, &sum) ||
        sscanf(sum.checksum, "%lx", &actual_crc) != 1 ||
        actual_crc != plan->source_crc32) {
        remove(stage);
        detail_set(detail, detail_size, "staged verification failed; source preserved");
        return 0;
    }
    if (rename(stage, destination) != 0) {
        remove(stage);
        detail_set(detail, detail_size, "quarantine commit failed; source preserved");
        return 0;
    }

    if (!write_metadata(plan, metadata_stage) || rename(metadata_stage, metadata) != 0) {
        remove(metadata_stage);
        remove(destination);
        detail_set(detail, detail_size, "metadata commit failed; source preserved");
        return 0;
    }

    if (remove(plan->source) != 0) {
        if (stored_path != 0 && stored_path_size > 0UL &&
            (unsigned long)strlen(destination) < stored_path_size)
            strcpy(stored_path, destination);
        detail_set(detail, detail_size, "quarantine committed; source removal pending");
        return 1;
    }

    if (stored_path != 0 && stored_path_size > 0UL &&
        (unsigned long)strlen(destination) < stored_path_size)
        strcpy(stored_path, destination);
    detail_set(detail, detail_size, "quarantine committed and source removed");
    return 1;
}
