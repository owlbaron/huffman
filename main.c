#include <stdio.h>
#include <string.h>

FILE *reader, *writer;

char *strremove(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        size_t size = 0;
        while ((p = strstr(p, sub)) != NULL) {
            size = (size == 0) ? (p - str) + strlen(p + len) + 1 : size - len;
            memmove(p, p + len, size - (p - str));
        }
    }
    return str;
}

void compact(char *filePath) {
    printf("Compacting file: %s\n", filePath);

    reader = fopen(filePath, "r");
    writer = fopen(strcat(filePath, ".comp"), "w");
    if (reader == NULL) {
        printf("File %s does not exist\n", filePath);
        EXIT(4);
    }
}

void descompact(char *filePath) {
    printf("Descompacting file: %s\n", filePath);

    char *extension = strrchr(filePath, '.');

    if (extension == NULL || strcmp(extension, ".comp") != 0) {
        printf("File %s is not compacted\n", filePath);
        EXIT(5);
    }

    reader = fopen(filePath, "r");
    writer = fopen(strremove(filePath, ".comp"), "w");
    if (reader == NULL) {
        printf("File %s does not exist\n", filePath);
        EXIT(6);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("A <command>, <file_path>  is expected\n");
        return 1;
    } else if (argc < 3) {
        printf("A <file_path> is expected\n");
        return 2;
    }

    char *command = argv[1];
    char *filePath = argv[2];

    if (strcmp(command, "compact") == 0) {
        compact(filePath);
    } else if (strcmp(command, "descompact") == 0) {
        descompact(filePath);
    } else {
        printf("Command not implemented. Use compact or descompact\n");
        return 3;
    }

    return 0;
}
