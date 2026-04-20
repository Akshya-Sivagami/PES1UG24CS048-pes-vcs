#include "index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// ─── REQUIRED FUNCTION (MISSING BEFORE) ─────

IndexEntry* index_find(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0)
            return &index->entries[i];
    }
    return NULL;
}

// ─── LOAD ───────────────────────────────────

int index_load(Index *index) {
    index->count = 0;

    FILE *f = fopen(".pes/index", "r");
    if (!f) return 0;

    while (index->count < MAX_INDEX_ENTRIES) {
        IndexEntry *e = &index->entries[index->count];

        char hex[65];
        unsigned long mtime, size;

        int read = fscanf(f, "%o %64s %lu %lu %255s\n",
            &e->mode, hex, &mtime, &size, e->path);

        if (read != 5) break;

        e->mtime_sec = mtime;
        e->size = size;

        hex_to_hash(hex, &e->hash);

        index->count++;
    }

    fclose(f);
    return 0;
}

// ─── SAVE ───────────────────────────────────

static int cmp_entries(const void *a, const void *b) {
    return strcmp(((IndexEntry*)a)->path, ((IndexEntry*)b)->path);
}

int index_save(const Index *index) {
    mkdir(".pes", 0755);

    FILE *f = fopen(".pes/index.tmp", "w");
    if (!f) return -1;

    Index sorted = *index;
    qsort(sorted.entries, sorted.count, sizeof(IndexEntry), cmp_entries);

    for (int i = 0; i < sorted.count; i++) {
        char hex[65];
        hash_to_hex(&sorted.entries[i].hash, hex);

        fprintf(f, "%o %s %lu %lu %s\n",
            sorted.entries[i].mode,
            hex,
            sorted.entries[i].mtime_sec,
            sorted.entries[i].size,
            sorted.entries[i].path);
    }

    fclose(f);
    rename(".pes/index.tmp", ".pes/index");

    return 0;
}

// ─── ADD ───────────────────────────────────

int index_add(Index *index, const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;

    IndexEntry *e = index_find(index, path);

    if (!e) {
        e = &index->entries[index->count++];
    }

    strcpy(e->path, path);
    e->mode = 0100644;
    e->mtime_sec = st.st_mtime;
    e->size = st.st_size;

    memset(e->hash.hash, 0, 32);

    return index_save(index);
}
