#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_SET_SIZE 256

typedef struct node {
    int frequency;
    char symbol;

    struct Node *left, *right;
} Node;

typedef struct heap {
    int size;
    int maxSize;
    Node **array;
} Heap;

FILE *reader, *writer;

char *strRemove(char *str, const char *sub) {
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

void *removeZeros(int *frequency, char *symbols, int size, int *frequencyNonZero, char *symbolsNonZero)
{
    for (int i = 0; i < size; i++) {
        if (frequency[i] > 0) {
            *frequencyNonZero++ = frequency[i];
            *symbolsNonZero++ = symbols[i];
        }
    }
}

void insert(Heap *heap, Node *node) {
    heap->size++;
    int i = heap->size - 1;

    while (i > 0 && heap->array[(i - 1) / 2]->frequency > node->frequency) {
        heap->array[i] = heap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    heap->array[i] = node;
}

void insertNode(Heap *heap, int frequency, char symbol, Node *left, Node *right) {
    Node *node = (Node *) malloc(sizeof(Node));
    node->frequency = frequency;
    node->symbol = symbol;
    node->left = left;
    node->right = right;

    insert(heap, node);
}

void createHeap(Heap *heap, int *frequency, char *symbols, int size) {
    heap->size = 0;
    heap->maxSize = size;
    heap->array = (Node **) malloc(sizeof(Node *) * size);

    for (int i = 0; i < size; i++) {
        insertNode(heap, frequency[i], symbols[i], NULL, NULL);
    }
}

void minHeapify(Heap *heap, int index) {
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int smallest = index;

    if (left < heap->size && heap->array[left]->frequency < heap->array[smallest]->frequency) {
        smallest = left;
    }

    if (right < heap->size && heap->array[right]->frequency < heap->array[smallest]->frequency) {
        smallest = right;
    }

    if (smallest != index) {
        Node *temp = heap->array[index];
        heap->array[index] = heap->array[smallest];
        heap->array[smallest] = temp;

        minHeapify(heap, smallest);
    }
}

void minHeap(Heap *heap) {
    int n = heap->size;
    
    for (int i = (n - 1) / 2; i >= 0; i--) {
        minHeapify(heap, i);
    }
}

void extract(Node *node, Heap *heap) {
    node = heap->array[0];

    heap->array[0] = heap->array[heap->size - 1];

    heap->size--;
    minHeapify(heap, 0);
}


void huffman(Node *root, int *frequency, char *symbols, int size) {
    Node *left = (Node *) malloc(sizeof(Node));
    Node *right = (Node *) malloc(sizeof(Node));

    Heap *heap = (Heap *) malloc(sizeof(Heap));

    createHeap(heap, frequency, symbols, size);

    printf("heap = ");
    for (int i = 0; i < heap->size; i++) {
        printf("%d ", heap->array[i]->frequency);
    }
    printf("\n");

    minHeap(heap);

    printf("min heap = ");
    for (int i = 0; i < heap->size; i++) {
        printf("%d ", heap->array[i]->frequency);
    }
    printf("\n");

    while (heap->size > 1) {
        extract(left, heap);
        extract(right, heap);

        printf("left = %d, right = %d\n", left->frequency, right->frequency);

        insertNode(heap, left->frequency + right->frequency, '\0', left, right);
    }

    printf("after huffman = ");
    for (int i = 0; i < heap->size; i++) {
        printf("%d ", heap->array[i]->frequency);
    }
    printf("\n");

    extract(root, heap);
}

void printCodes(Node *root, int arr[], int top) {
	if (root->left) {
		arr[top] = 0;
		printCodes(root->left, arr, top + 1);
	}

	if (root->right) {
		arr[top] = 1;
		printCodes(root->right, arr, top + 1);
	}

	if (!(root->left) && !(root->right)) {
		printf("%c: ", root->symbol);
        for (int i = 0; i < top; ++i)
            printf("%d", arr[i]);

        printf("\n");
	}
}

void compact(char *filePath) {
    char ch;
    int symbolsUsed = 0;
    int frequency[CHAR_SET_SIZE] = {0};
    char symbols[CHAR_SET_SIZE];
    printf("Compacting file: %s\n", filePath);

    reader = fopen(filePath, "r");
    writer = fopen(strcat(filePath, ".comp"), "w");
    if (reader == NULL) {
        printf("File %s does not exist\n", filePath);
        exit(4);
    }

    while (1) {
        ch = fgetc(reader);
        int index = (int) ch;

        if (ch == EOF) {
            break;
        }

        if (frequency[index] == 0) {
            symbolsUsed++;
        }
        
        symbols[index] = ch;
        frequency[index] = frequency[index] + 1;
    };

    int *frequencyNonZero = (int *) malloc(sizeof(int) * symbolsUsed);
    char *symbolsNonZero = (char *) malloc(sizeof(char) * symbolsUsed);

    removeZeros(frequency, symbols, CHAR_SET_SIZE, frequencyNonZero, symbolsNonZero);

    printf("frequency, symbols = ");
    for (int i = 0; i < symbolsUsed; i++) {
        printf("[%c %d] ", symbolsNonZero[i], frequencyNonZero[i]);
    }

    printf("\n");

    Node *root = (Node *) malloc(sizeof(Node));
    huffman(root, frequencyNonZero, symbolsNonZero, symbolsUsed);

    printf("node = ");
    printf("{ %c %d }\n", root->symbol, root->frequency);

    free(frequencyNonZero);
    free(symbolsNonZero);
    free(root);
}

void descompact(char *filePath) {
    printf("Descompacting file: %s\n", filePath);

    char *extension = strrchr(filePath, '.');
    if (extension == NULL || strcmp(extension, ".comp") != 0) {
        printf("File %s is not compacted\n", filePath);
        exit(5);
    }

    reader = fopen(filePath, "r");
    writer = fopen(strRemove(filePath, ".comp"), "w");
    if (reader == NULL) {
        printf("File %s does not exist\n", filePath);
        exit(6);
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
