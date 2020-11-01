#include "../filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* path_prefix = NULL;
size_t path_prefix_len = 0;

FILE* handles[FILESYSTEM_MAX_OPEN_FILES];

void filesystem_init(const char* filesystem_path_prefix) {
    path_prefix_len= strlen(filesystem_path_prefix);
    path_prefix = malloc(path_prefix_len + 1);
    strcpy(path_prefix, filesystem_path_prefix);

    memset(handles, 0, sizeof(FILE*) * (FILESYSTEM_MAX_OPEN_FILES));
}

void filesystem_uninit() {
    if (path_prefix) {
        free(path_prefix);
        path_prefix = NULL;
    }
}

int filesystem_open(const char* path) {
    size_t path_len = strlen(path);
    char* filesystem_path = malloc(path_prefix_len + path_len + 1);
    strcpy(filesystem_path, path_prefix);
    strcat(filesystem_path, path);

    int handle;
    for (handle = 0; handle < FILESYSTEM_MAX_OPEN_FILES; handle++) {
        if (handles[handle] == NULL)
            break;
    }

    if (handle < FILESYSTEM_MAX_OPEN_FILES) {
        FILE* file = fopen(filesystem_path, "rb");
        if (file)
            handles[handle] = file;
        else
            handle = -1;
    }
    else {
        handle = -1;
    }

    free(filesystem_path);
    return handle;
}

void filesystem_close(uint32_t handle) {
    if (handles[handle] != NULL) {
        fclose(handles[handle]);
        handles[handle] = NULL;
    }
}

int filesystem_read(void* buf, int size, int count, int handle) {
    int bytes_read = 0;

    if (handles[handle] != NULL)
        bytes_read =  fread(buf, size, count, handles[handle]);

    return bytes_read;
}