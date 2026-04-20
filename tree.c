#include "tree.h"
#include "index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int object_write(int type, const void *data, size_t len, ObjectID *id_out);

// ─── PARSE ───────────────────────────────────

int tree_parse(const void *data, size_t len, Tree *tree_out) {
    tree_out->count = 0;

    const unsigned char *ptr = data;
    const unsigned char *end = ptr + len;

    while (ptr < end) {
        TreeEntry *e = &tree_out->entries[tree_out->count++];

        sscanf((const char*)ptr, "%o %s", &e->mode, e->name);

        while (*ptr != '\0') ptr++;
        ptr++;

        memcpy(e->hash.hash, ptr, HASH_SIZE);
        ptr += HASH_SIZE;
    }

    return 0;
}

// ─── SERIALIZE ───────────────────────────────

static int cmp_tree(const void *a, const void *b) {
    return strcmp(((TreeEntry*)a)->name, ((TreeEntry*)b)->name);
}

int tree_serialize(const Tree *tree, void **data_out, size_t *len_out) {
    Tree temp = *tree;
    qsort(temp.entries, temp.count, sizeof(TreeEntry), cmp_tree);

    size_t size = temp.count * 300;
    unsigned char *buf = malloc(size);

    size_t offset = 0;

    for (int i = 0; i < temp.count; i++) {
        offset += sprintf((char*)buf + offset, "%o %s", temp.entries[i].mode, temp.entries[i].name) + 1;

        memcpy(buf + offset, temp.entries[i].hash.hash, HASH_SIZE);
        offset += HASH_SIZE;
    }

    *data_out = buf;
    *len_out = offset;
    return 0;
}

// ─── FROM INDEX ──────────────────────────────

static int cmp_index(const void *a, const void *b) {
    return strcmp(((IndexEntry*)a)->path, ((IndexEntry*)b)->path);
}

int tree_from_index(ObjectID *id_out) {
    Index index;

    if (index_load(&index) != 0) return -1;

    qsort(index.entries, index.count, sizeof(IndexEntry), cmp_index);

    Tree tree;
    tree.count = 0;

    for (int i = 0; i < index.count; i++) {
        TreeEntry *te = &tree.entries[tree.count++];

        const char *name = strrchr(index.entries[i].path, '/');
        name = name ? name + 1 : index.entries[i].path;

        strcpy(te->name, name);
        te->mode = index.entries[i].mode;
        te->hash = index.entries[i].hash;
    }

    void *data;
    size_t len;

    if (tree_serialize(&tree, &data, &len) != 0) return -1;

    int rc = object_write(OBJ_TREE, data, len, id_out);

    free(data);
    return rc;
}
