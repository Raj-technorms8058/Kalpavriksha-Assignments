#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NUM_BLOCKS 1024
#define BLOCK_SIZE 512
#define MAX_NAME 50
#define MAX_INPUT 8192

typedef struct FreeBlock{
    int index;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode{
    char name[MAX_NAME + 1];
    int isDirectory;
    struct FileNode *parent;
    struct FileNode *next;
    struct FileNode *children;
    int *blockPointers;
    FreeBlock **blockNodes;
    int blockCount;
    size_t contentSize;
} FileNode;

char disk[NUM_BLOCKS][BLOCK_SIZE];
FreeBlock *freeHead = NULL, *freeTail = NULL;
int freeCount = 0;
FileNode *root = NULL, *cwd = NULL;

int readLine(char *buffer, int size){
    if(fgets(buffer, size, stdin) == NULL){
        return 0;
    }
    int len = strlen(buffer);
    if(len > 0 && buffer[len - 1] == '\n'){
        buffer[len - 1] = '\0';
        return 1;
    }
    int ch;
    while((ch = getchar()) != '\n' && ch != EOF){
    }
    return -1;
}

char *trimSpaces(char *str){
    while(isspace(*str)){
        str++;
    }
    char *end = str + strlen(str);
    while(end > str && isspace(*(end - 1))){
        end--;
    }
    *end = '\0';
    return str;
}

int getWord(char **input, char *word){
    int i = 0;
    while(isspace(**input)){
        (*input)++;
    }
    while(**input != '\0' && !isspace(**input)){
        if(i >= MAX_NAME){
            return 0;
        }
        word[i++] = **input;
        (*input)++;
    }
    word[i] = '\0';
    return i > 0;
}

int parseContent(char *input, char **content){
    input = trimSpaces(input);
    if(*input != '"' && *input != '\''){
        return 0;
    }
    char quote = *input;
    *content = ++input;
    while(*input != '\0' && *input != quote){
        input++;
    }
    if(*input != quote){
        return 0;
    }
    *input = '\0';
    input = trimSpaces(input + 1);
    return *input == '\0';
}

int isValidName(const char *name){
    if(name[0] == '\0' || strlen(name) > MAX_NAME){
        return 0;
    }
    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0){
        return 0;
    }
    int i = 0;
    while(name[i] != '\0'){
        if(isspace(name[i]) || name[i] == '/'){
            return 0;
        }
        i++;
    }
    return 1;
}

int initializeFreeBlocks(){
    int i = 0;
    while(i < NUM_BLOCKS){
        FreeBlock *node = malloc(sizeof(FreeBlock));
        if(node == NULL){
            return 0;
        }
        node->index = i;
        node->next = NULL;
        node->prev = freeTail;
        if(freeTail == NULL){
            freeHead = node;
        }
        else{
            freeTail->next = node;
        }
        freeTail = node;
        freeCount++;
        i++;
    }
    return 1;
}

FreeBlock *allocateBlock(){
    if(freeHead == NULL){
        return NULL;
    }
    FreeBlock *node = freeHead;
    freeHead = freeHead->next;
    if(freeHead == NULL){
        freeTail = NULL;
    }
    else{
        freeHead->prev = NULL;
    }
    node->next = NULL;
    node->prev = NULL;
    freeCount--;
    return node;
}

void releaseBlock(FreeBlock *node){
    if(node == NULL){
        return;
    }
    node->next = NULL;
    node->prev = freeTail;
    if(freeTail == NULL){
        freeHead = node;
    }
    else{
        freeTail->next = node;
    }
    freeTail = node;
    freeCount++;
}

void releaseFileBlocks(FileNode *file){
    int i = 0;
    while(i < file->blockCount){
        releaseBlock(file->blockNodes[i]);
        i++;
    }
    free(file->blockPointers);
    free(file->blockNodes);
    file->blockPointers = NULL;
    file->blockNodes = NULL;
    file->blockCount = 0;
    file->contentSize = 0;
}

FileNode *createNode(const char *name, int isDirectory){
    FileNode *node = malloc(sizeof(FileNode));
    if(node == NULL){
        return NULL;
    }
    strcpy(node->name, name);
    node->isDirectory = isDirectory;
    node->parent = NULL;
    node->next = NULL;
    node->children = NULL;
    node->blockPointers = NULL;
    node->blockNodes = NULL;
    node->blockCount = 0;
    node->contentSize = 0;
    return node;
}

int initializeFileSystem(){
    if(!initializeFreeBlocks()){
        return 0;
    }
    root = createNode("/", 1);
    if(root == NULL){
        return 0;
    }
    cwd = root;
    return 1;
}

FileNode *findChild(FileNode *directory, const char *name){
    if(directory->children == NULL){
        return NULL;
    }
    FileNode *current = directory->children;
    while(1){
        if(strcmp(current->name, name) == 0){
            return current;
        }
        current = current->next;
        if(current == directory->children){
            break;
        }
    }
    return NULL;
}

void insertChild(FileNode *directory, FileNode *node){
    node->parent = directory;
    if(directory->children == NULL){
        directory->children = node;
        node->next = node;
        return;
    }
    FileNode *last = directory->children;
    while(last->next != directory->children){
        last = last->next;
    }
    last->next = node;
    node->next = directory->children;
}

int removeChild(FileNode *directory, FileNode *target){
    if(directory->children == NULL){
        return 0;
    }
    FileNode *current = directory->children;
    FileNode *previous = NULL;
    while(1){
        if(current == target){
            if(current->next == current){
                directory->children = NULL;
            }
            else{
                if(previous == NULL){
                    previous = current;
                    while(previous->next != current){
                        previous = previous->next;
                    }
                }
                previous->next = current->next;
                if(directory->children == current){
                    directory->children = current->next;
                }
            }
            current->next = NULL;
            return 1;
        }
        previous = current;
        current = current->next;
        if(current == directory->children){
            break;
        }
    }
    return 0;
}

void printPath(FileNode *directory){
    if(directory->parent != NULL){
        printPath(directory->parent);
        printf("/%s", directory->name);
    }
    else{
        printf("/");
    }
}

void makeDirectory(const char *name){
    if(!isValidName(name)){
        printf("Invalid directory name.\n");
        return;
    }
    if(findChild(cwd, name) != NULL){
        printf("Name already exists.\n");
        return;
    }
    FileNode *node = createNode(name, 1);
    if(node == NULL){
        printf("Memory allocation failed.\n");
        return;
    }
    insertChild(cwd, node);
    printf("Directory '%s' created successfully.\n", name);
}

void changeDirectory(const char *name){
    if(strcmp(name, "..") == 0){
        if(cwd->parent != NULL){
            cwd = cwd->parent;
        }
    }
    else{
        FileNode *node = findChild(cwd, name);
        if(node == NULL || !node->isDirectory){
            printf("Directory not found.\n");
            return;
        }
        cwd = node;
    }
    printf("Moved to ");
    printPath(cwd);
    printf("\n");
}

void listDirectory(){
    if(cwd->children == NULL){
        printf("(empty)\n");
        return;
    }
    FileNode *current = cwd->children;
    while(1){
        printf("%s%s\n", current->name, current->isDirectory ? "/" : "");
        current = current->next;
        if(current == cwd->children){
            break;
        }
    }
}

void removeDirectory(const char *name){
    FileNode *node = findChild(cwd, name);
    if(node == NULL || !node->isDirectory){
        printf("Directory not found.\n");
        return;
    }
    if(node->children != NULL){
        printf("Directory must be empty before deletion.\n");
        return;
    }
    removeChild(cwd, node);
    free(node);
    printf("Directory removed successfully.\n");
}

void createFile(const char *name){
    if(!isValidName(name)){
        printf("Invalid filename.\n");
        return;
    }
    if(findChild(cwd, name) != NULL){
        printf("Name already exists.\n");
        return;
    }
    FileNode *node = createNode(name, 0);
    if(node == NULL){
        printf("Memory allocation failed.\n");
        return;
    }
    insertChild(cwd, node);
    printf("File '%s' created successfully.\n", name);
}

void writeFile(const char *name, const char *content){
    FileNode *file = findChild(cwd, name);
    if(file == NULL || file->isDirectory){
        printf("File not found.\n");
        return;
    }
    size_t length = strlen(content);
    int required = (int)((length + BLOCK_SIZE - 1) / BLOCK_SIZE);
    if(required > freeCount + file->blockCount){
        printf("Disk is full. Cannot write file.\n");
        return;
    }
    int *newPointers = NULL;
    FreeBlock **newNodes = NULL;
    if(required > 0){
        newPointers = malloc(required * sizeof(int));
        newNodes = malloc(required * sizeof(FreeBlock *));
        if(newPointers == NULL || newNodes == NULL){
            free(newPointers);
            free(newNodes);
            printf("Memory allocation failed.\n");
            return;
        }
    }
    releaseFileBlocks(file);
    int i = 0;
    while(i < required){
        FreeBlock *block = allocateBlock();
        newPointers[i] = block->index;
        newNodes[i] = block;
        int j = 0;
        size_t start = (size_t)i * BLOCK_SIZE;
        while(j < BLOCK_SIZE){
            if(start + j < length){
                disk[block->index][j] = content[start + j];
            }
            else{
                disk[block->index][j] = '\0';
            }
            j++;
        }
        i++;
    }
    file->blockPointers = newPointers;
    file->blockNodes = newNodes;
    file->blockCount = required;
    file->contentSize = length;
    printf("Data written successfully (size=%d bytes).\n", (int)length);
}

void readFile(const char *name){
    FileNode *file = findChild(cwd, name);
    if(file == NULL || file->isDirectory){
        printf("File not found.\n");
        return;
    }
    size_t remaining = file->contentSize;
    int i = 0;
    while(i < file->blockCount && remaining > 0){
        int j = 0;
        int blockIndex = file->blockPointers[i];
        while(j < BLOCK_SIZE && remaining > 0){
            putchar(disk[blockIndex][j]);
            j++;
            remaining--;
        }
        i++;
    }
    printf("\n");
}

void deleteFile(const char *name){
    FileNode *file = findChild(cwd, name);
    if(file == NULL || file->isDirectory){
        printf("File not found.\n");
        return;
    }
    releaseFileBlocks(file);
    removeChild(cwd, file);
    free(file);
    printf("File deleted successfully.\n");
}

void showDiskUsage(){
    int used = NUM_BLOCKS - freeCount;
    float percentage = (float)used * 100 / NUM_BLOCKS;
    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", freeCount);
    printf("Disk Usage: %.2f%%\n", percentage);
}

void freeDirectoryContents(FileNode *directory){
    if(directory->children == NULL){
        return;
    }
    FileNode *head = directory->children;
    FileNode *current = head;
    directory->children = NULL;
    while(current != NULL){
        FileNode *next = current->next;
        int last = (next == head);
        current->next = NULL;
        if(current->isDirectory){
            freeDirectoryContents(current);
        }
        else{
            releaseFileBlocks(current);
        }
        free(current);
        if(last){
            break;
        }
        current = next;
    }
}

void cleanupFileSystem(){
    if(root != NULL){
        freeDirectoryContents(root);
        free(root);
        root = NULL;
        cwd = NULL;
    }
    while(freeHead != NULL){
        FreeBlock *next = freeHead->next;
        free(freeHead);
        freeHead = next;
    }
    freeTail = NULL;
    freeCount = 0;
}

int processCommand(char *input){
    char command[MAX_NAME + 1];
    char name[MAX_NAME + 1];
    char *content;
    char *args = input;
    
    if(!getWord(&args, command)){
        return 1;
    }
    
    args = trimSpaces(args);
    
    if(strcmp(command, "mkdir") == 0){
        if(!getWord(&args, name) || *trimSpaces(args) != '\0'){
            printf("Usage: mkdir directory_name\n");
        }
        else{
            makeDirectory(name);
        }
    }
    else if(strcmp(command, "cd") == 0){
        if(!getWord(&args, name) || *trimSpaces(args) != '\0'){
            printf("Usage: cd directory_name\n");
        }
        else{
            changeDirectory(name);
        }
    }
    else if(strcmp(command, "create") == 0){
        if(!getWord(&args, name) || *trimSpaces(args) != '\0'){
            printf("Usage: create filename\n");
        }
        else{
            createFile(name);
        }
    }
    else if(strcmp(command, "write") == 0){
        if(!getWord(&args, name)){
            printf("Usage: write filename \"content\"\n");
        }
        else if(!parseContent(args, &content)){
            printf("Content must be enclosed in quotes.\n");
        }
        else{
            writeFile(name, content);
        }
    }
    else if(strcmp(command, "read") == 0){
        if(!getWord(&args, name) || *trimSpaces(args) != '\0'){
            printf("Usage: read filename\n");
        }
        else{
            readFile(name);
        }
    }
    else if(strcmp(command, "delete") == 0){
        if(!getWord(&args, name) || *trimSpaces(args) != '\0'){
            printf("Usage: delete filename\n");
        }
        else{
            deleteFile(name);
        }
    }
    else if(strcmp(command, "rmdir") == 0){
        if(!getWord(&args, name) || *trimSpaces(args) != '\0'){
            printf("Usage: rmdir directory_name\n");
        }
        else{
            removeDirectory(name);
        }
    }
    else if(strcmp(command, "ls") == 0){
        if(*args != '\0'){
            printf("Usage: ls\n");
        }
        else{
            listDirectory();
        }
    }
    else if(strcmp(command, "pwd") == 0){
        if(*args != '\0'){
            printf("Usage: pwd\n");
        }
        else{
            printPath(cwd);
            printf("\n");
        }
    }
    else if(strcmp(command, "df") == 0){
        if(*args != '\0'){
            printf("Usage: df\n");
        }
        else{
            showDiskUsage();
        }
    }
    else if(strcmp(command, "exit") == 0){
        if(*args != '\0'){
            printf("Usage: exit\n");
        }
        else{
            cleanupFileSystem();
            printf("Memory released. Exiting program...\n");
            return 0;
        }
    }
    else{
        printf("Unknown command: %s\n", command);
    }
    return 1;
}

int main(){
    char input[MAX_INPUT];
    if(!initializeFileSystem()){
        printf("Filesystem initialization failed.\n");
        cleanupFileSystem();
        return 1;
    }
    
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    
    while(1){
        printPath(cwd);
        printf(" > ");
        int result = readLine(input, MAX_INPUT);
        
        if(result == 0){
            printf("\n");
            break;
        }
        
        if(result == -1){
            printf("Input too long. Command rejected.\n");
            continue;
        }
        
        if(!processCommand(input)){
            return 0;
        }
    }
    
    cleanupFileSystem();
    return 0;
}