#include "index.h"
#include <stdio.h>

int main() {
    Index index;

    // IMPORTANT: initialize index
    if (index_load(&index) != 0) {
        printf("index load failed\n");
        return 1;
    }

    if (index_add(&index, "file.txt") != 0) {
        printf("index add failed\n");
        return 1;
    }

    printf("Added file.txt successfully\n");
    return 0;
}
