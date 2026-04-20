#include "tree.h"
#include <stdio.h>

int main() {
    ObjectID id;

    if (tree_from_index(&id) != 0) {
        printf("tree creation failed\n");
        return 1;
    }

    printf("Tree created successfully\n");
    return 0;
}
