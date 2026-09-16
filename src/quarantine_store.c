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

static int hex_digit(unsigned char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}

static int quarantine_id_valid(const char *id)
{
    unsigned long i;
    if (id == 0 || strlen(id) != 16UL || id[0] != 'Q') return 0;
    for (i = 1UL; i < 16UL; ++i) {
        if (!hex_digit((unsigned char)id[i])) return 0;
    }
    return 1;
}

static int parse_crc32_exact(const char *text, unsigned long *value)
{
    unsigned long i;
    unsigned long parsed;
    if (text == 0 || value == 0 || strlen(text) != 8UL) return 0;
    for (i = 0UL; i < 8UL; ++i) {
        if (!hex_digit((unsigned char)text[i])) return 0;
    }
    if (sscanf(text, "%lx", &parsed) != 1) return 0;
    *value = parsed;
    return 1;
}

static int parse_size_exact(const char *text, unsigned long *value)
{
    const unsigned char *p;
    unsigned long parsed;
    char tail;
    if (text == 0 || value == 0 || text[0] == '\0') return 0;
    p = (const unsigned char *)text;
    while (*p != 0U) {
        if (*p < '0' || *p > '9') return 0;
        ++p;
    }
    if (sscanf(text, "%lu%c", &parsed, &tail) != 1) return 0;
    *value = parsed;
    return 1;
}

int amiguard_ae_quarantine_set_directory(const char *directory,
                                         char *detail,
                                         unsigned long detail_size)
{
    unsigned long length;
    if (directory == 0 || directory[0] == '\0') {
        quarantine_directory[0] = '\0';
        detail_set(detail, detail_size, "quarantine directory cleared; quarantine disabled");
        return 1;
    }
    length = (unsigned long)strlen(directory);
    if (length >= sizeof(quarantine_directory)) {
        detail_set(detail, detail_size, "quarantine directory path too long");
        return 0;
    }
    if (strcmp(directory, "/") == 0 || strcmp(directory, ".") == 0 ||
        strcmp(directory, "..") == 0 || directory[length - 1UL] == ':') {
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

static int read_metadata(const char *path, AmiGuardAEQuarantinePlan *plan)
{
    FILE *fp;
    char line[320];
    unsigned long parsed_crc;
    unsigned long parsed_size;
    int got_id = 0;
    int got_source = 0;
    int got_crc = 0;
    int got_size = 0;

    if (plan == 0) return 0;
    memset(plan, 0, sizeof(*plan));
    fp = fopen(path, "rb");
    if (fp == 0) return 0;
    if (fgets(line, sizeof(line), fp) == 0 || strcmp(line, "AMIGUARD-QUARANTINE 1\n") != 0) {
        fclose(fp);
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != 0) {
        size_t n = strlen(line);
        if (n != 0U && line[n - 1U] == '\n') line[--n] = '\0';
        if (n != 0U && line[n - 1U] == '\r') line[--n] = '\0';
        if (strncmp(line, "ID=", 3) == 0) {
            if (got_id || !quarantine_id_valid(line + 3)) { fclose(fp); return 0; }
            strcpy(plan->id, line + 3); got_id = 1;
        } else if (strncmp(line, "SOURCE=", 7) == 0) {
            if (got_source || strlen(line + 7) >= sizeof(plan->source)) { fclose(fp); return 0; }
            strcpy(plan->source, line + 7); got_source = 1;
        } else if (strncmp(line, "CRC32=", 6) == 0) {
            if (got_crc || !parse_crc32_exact(line + 6, &parsed_crc)) { fclose(fp); return 0; }
            plan->source_crc32 = parsed_crc; got_crc = 1;
        } else if (strncmp(line, "SIZE=", 5) == 0) {
            if (got_size || !parse_size_exact(line + 5, &parsed_size)) { fclose(fp); return 0; }
            plan->source_size = parsed_size; got_size = 1;
        } else {
            fclose(fp); return 0;
        }
    }
    fclose(fp);
    return got_id && got_source && got_crc && got_size;
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
    if (!quarantine_id_valid(plan->id)) {
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
    if (stored_path != 0 && stored_path_size > 0UL) stored_path[0] = '\0';
    if (file_exists(destination)) { detail_set(detail, detail_size, "quarantine destination exists"); return 0; }
    if (file_exists(stage) || file_exists(metadata) || file_exists(metadata_stage)) { detail_set(detail, detail_size, "quarantine staging path busy"); return 0; }
    if (!copy_file(plan->source, stage)) { detail_set(detail, detail_size, "staging copy failed; source preserved"); return 0; }
    if (!amiguard_ae_checksum_crc32(stage, &sum) || !parse_crc32_exact(sum.checksum, &actual_crc) || actual_crc != plan->source_crc32) {
        remove(stage); detail_set(detail, detail_size, "staged verification failed; source preserved"); return 0;
    }
    if (rename(stage, destination) != 0) { remove(stage); detail_set(detail, detail_size, "quarantine commit failed; source preserved"); return 0; }
    if (!write_metadata(plan, metadata_stage) || rename(metadata_stage, metadata) != 0) {
        remove(metadata_stage); remove(destination); detail_set(detail, detail_size, "metadata commit failed; source preserved"); return 0;
    }
    if (remove(plan->source) != 0) {
        if (stored_path != 0 && stored_path_size > 0UL && (unsigned long)strlen(destination) < stored_path_size) strcpy(stored_path, destination);
        detail_set(detail, detail_size, "quarantine committed; source removal pending"); return 1;
    }
    if (stored_path != 0 && stored_path_size > 0UL && (unsigned long)strlen(destination) < stored_path_size) strcpy(stored_path, destination);
    detail_set(detail, detail_size, "quarantine committed and source removed");
    return 1;
}

int amiguard_ae_quarantine_restore(const char *id,
                                   char *restored_path,
                                   unsigned long restored_path_size,
                                   char *detail,
                                   unsigned long detail_size)
{
    AmiGuardAEQuarantinePlan plan;
    AmiGuardAEChecksumResult sum;
    char object[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char metadata[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char stage[AMIGUARD_AE_QUARANTINE_PATH_MAX];
    char name[32];
    char metadata_name[32];
    unsigned long actual_crc;
    FILE *fp;
    long size;

    if (restored_path != 0 && restored_path_size > 0UL) restored_path[0] = '\0';
    if (!amiguard_ae_quarantine_available()) { detail_set(detail, detail_size, "quarantine directory unavailable"); return 0; }
    if (!quarantine_id_valid(id)) { detail_set(detail, detail_size, "invalid quarantine id"); return 0; }
    sprintf(name, "%s.qtn", id);
    sprintf(metadata_name, "%s.meta", id);
    if (!build_path(object, sizeof(object), quarantine_directory, name) || !build_path(metadata, sizeof(metadata), quarantine_directory, metadata_name)) { detail_set(detail, detail_size, "quarantine path too long"); return 0; }
    if (!read_metadata(metadata, &plan) || strcmp(plan.id, id) != 0) { detail_set(detail, detail_size, "invalid quarantine metadata"); return 0; }
    if (!amiguard_ae_quarantine_path_safe(plan.source, detail, detail_size)) return 0;
    if (file_exists(plan.source)) { detail_set(detail, detail_size, "restore destination exists; refusing overwrite"); return 0; }
    if (!amiguard_ae_checksum_crc32(object, &sum) || !parse_crc32_exact(sum.checksum, &actual_crc) || actual_crc != plan.source_crc32) { detail_set(detail, detail_size, "quarantine object verification failed"); return 0; }
    fp = fopen(object, "rb");
    if (fp == 0 || fseek(fp, 0L, SEEK_END) != 0) { if (fp != 0) fclose(fp); detail_set(detail, detail_size, "quarantine object size check failed"); return 0; }
    size = ftell(fp); fclose(fp);
    if (size < 0L || (unsigned long)size != plan.source_size) { detail_set(detail, detail_size, "quarantine object size mismatch"); return 0; }
    if ((unsigned long)strlen(plan.source) + 14UL >= sizeof(stage)) { detail_set(detail, detail_size, "restore staging path too long"); return 0; }
    strcpy(stage, plan.source); strcat(stage, ".amiguard-stage");
    if (file_exists(stage)) { detail_set(detail, detail_size, "restore staging path busy"); return 0; }
    if (!copy_file(object, stage)) { detail_set(detail, detail_size, "restore staging copy failed"); return 0; }
    if (!amiguard_ae_checksum_crc32(stage, &sum) || !parse_crc32_exact(sum.checksum, &actual_crc) || actual_crc != plan.source_crc32) { remove(stage); detail_set(detail, detail_size, "restored staging verification failed"); return 0; }
    if (rename(stage, plan.source) != 0) { remove(stage); detail_set(detail, detail_size, "restore commit failed"); return 0; }
    if (restored_path != 0 && restored_path_size > (unsigned long)strlen(plan.source)) strcpy(restored_path, plan.source);
    detail_set(detail, detail_size, "restore committed; quarantine retained");
    return 1;
}
