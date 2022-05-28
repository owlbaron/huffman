#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_SET_SIZE 256
#define MAX_TREE_HEIGHT 100

typedef struct node {
	char symbol;
	unsigned frequency;
	struct node *left, *right;
} Node;

typedef struct heap {
	unsigned size;
	unsigned capacity;
	Node **nodes;
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

void *removeZeros(int *frequency, char *symbols, int size, int *frequencyNonZero, char *symbolsNonZero) {
    for (int i = 0; i < size; i++) {
        if (frequency[i] > 0) {
            *frequencyNonZero++ = frequency[i];
            *symbolsNonZero++ = symbols[i];
        }
    }
}

Node *createNode(char symbol, unsigned frequency) {
    Node *node = (Node *) malloc(sizeof(Node));

	node->left = node->right = NULL;
	node->symbol = symbol;
	node->frequency = frequency;

    return node;
}

Heap* createHeap(unsigned capacity) {
	Heap* heap = (Heap *) malloc(sizeof(Heap));

	heap->size = 0;
	heap->capacity = capacity;
	heap->nodes = (Node **) malloc(heap->capacity * sizeof(Node *));

	return heap;
}

void minHeapify(Heap* heap, int index) {
	int left = 2 * index + 1;
	int right = 2 * index + 2;
	int smallest = index;

	if (left < heap->size && heap->nodes[left]->frequency < heap->nodes[smallest]->frequency) {
		smallest = left;
    }

	if (right < heap->size && heap->nodes[right]->frequency < heap->nodes[smallest]->frequency) {
		smallest = right;
    }

	if (smallest != index) {
        Node *aux = heap->nodes[smallest];
        heap->nodes[smallest] = heap->nodes[index];
        heap->nodes[index] = aux;

		minHeapify(heap, smallest);
	}
}

Node* extract(Heap* heap) {
	Node* node = heap->nodes[0];
	heap->nodes[0] = heap->nodes[heap->size - 1];

	heap->size--;
	minHeapify(heap, 0);

	return node;
}

void insert(Heap* heap, Node* minHeapNode) {
	heap->size++;
	int i = heap->size - 1;

	while (i && minHeapNode->frequency < heap->nodes[(i - 1) / 2]->frequency) {
		heap->nodes[i] = heap->nodes[(i - 1) / 2];
		i = (i - 1) / 2;
	}

	heap->nodes[i] = minHeapNode;
}

void transformHeapToMinHeap(Heap* heap){
	int n = heap->size - 1;

	for (int i = (n - 1) / 2; i >= 0; i--) {
		minHeapify(heap, i);
    }
}

char *creatCode(int arr[], int n) {
    char *code = (char *) malloc(sizeof(char) * n);

	for (int i = 0; i < n; i++) {
        code[i] = arr[i] + '0';
    }

    return code;
}

Heap* buildHeap(char symbols[], int frequency[], int size) {
	Heap* heap = createHeap(size);

	for (int i = 0; i < size; i++) {
		heap->nodes[i] = createNode(symbols[i], frequency[i]);
    }

	heap->size = size;

	return heap;
}

Node* buildHuffmanTree(char symbols[], int frequency[], int size) {
	Node *left, *right, *top;
	Heap *heap = buildHeap(symbols, frequency, size);

    transformHeapToMinHeap(heap);

	while (heap->size > 1) {
		left = extract(heap);
		right = extract(heap);

		top = createNode('\0', left->frequency + right->frequency);

		top->left = left;
		top->right = right;

		insert(heap, top);
	}

	return extract(heap);
}

void createCodes(char **charToCode, Node *root, int acc[], int top) {
	if (root->left) {
		acc[top] = 0;
		createCodes(charToCode, root->left, acc, top + 1);
	}

	if (root->right) {
		acc[top] = 1;
		createCodes(charToCode, root->right, acc, top + 1);
	}

	if (!(root->left) && !(root->right)) {
        int charAsInt = (int) root->symbol;
		charToCode[charAsInt] = creatCode(acc, top);
	}
}

char **getCharToCodeMap(char symbols[], int frequency[], int size) {
	Node *root = buildHuffmanTree(symbols, frequency, size);

    int *acc = (int *) malloc(MAX_TREE_HEIGHT * sizeof(int));

    char **charToCode = (char **) malloc(CHAR_SET_SIZE * sizeof(char *));
    createCodes(charToCode, root, acc, 0);

    return charToCode;
}

void compact(char *filePath) {
    char ch;
    int symbolsUsed = 0, textSize = 0;
    int frequency[CHAR_SET_SIZE] = {0};
    char symbols[CHAR_SET_SIZE];

    char text[9999];

    printf("Compacting file: %s\n", filePath);

    reader = fopen(filePath, "r");
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

        text[textSize++] = ch;

        if (frequency[index] == 0) {
            symbolsUsed++;
        }
        
        symbols[index] = ch;
        frequency[index] = frequency[index] + 1;
    };

    fclose(reader);

    int *frequencyNonZero = (int *) malloc(sizeof(int) * symbolsUsed);
    char *symbolsNonZero = (char *) malloc(sizeof(char) * symbolsUsed);

    removeZeros(frequency, symbols, CHAR_SET_SIZE, frequencyNonZero, symbolsNonZero);

    // printf("frequency, symbols = ");
    // for (int i = 0; i < symbolsUsed; i++) {
    //     printf("[%c %d] ", symbolsNonZero[i], frequencyNonZero[i]);
    // }

    // printf("\n");

    char **charToCodeMap = getCharToCodeMap(symbolsNonZero, frequencyNonZero, symbolsUsed);

    // for (int i = 0; i < CHAR_SET_SIZE; i++) {
    //     if (charToCodeMap[i] != NULL) {
    //         printf("%c: %s\n", symbols[i], charToCodeMap[i]);
    //     }
    // }

    writer = fopen(strcat(filePath, ".comp"), "w");

    char **compacted = (char **) malloc(sizeof(char *) * textSize);

    printf("text size = %d\n", textSize);

    for (int i = 0; i < textSize; i++) {
        compacted[i] = charToCodeMap[(int) text[i]];
    }

    free(frequencyNonZero);
    free(symbolsNonZero);
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
