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
    FreeBlock **blockNodes;
    int blockCount;
    size_t contentSize;
} FileNode;

typedef struct Vfs{
    char disk[NUM_BLOCKS][BLOCK_SIZE];
    FreeBlock *freeHead;
    FreeBlock *freeTail;
    int freeCount;
    FileNode *root;
    FileNode *cwd;
} Vfs;

typedef struct Command{
    const char *name;
    void (*handler)(Vfs *vfs, char *args);
} Command;

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
    while(isspace((unsigned char)*str)){
        str++;
    }
    char *end = str + strlen(str);
    while(end > str && isspace((unsigned char)*(end - 1))){
        end--;
    }
    *end = '\0';
    return str;
}

int getWord(char **input, char *word){
    int i = 0;
    while(isspace((unsigned char)**input)){
        (*input)++;
    }
    while(**input != '\0' && !isspace((unsigned char)**input)){
        if(i >= MAX_NAME){
            return 0;
        }
        word[i++] = **input;
        (*input)++;
    }
    word[i] = '\0';
    return i > 0;
}

char *singleArgument(char *args, const char *usage){
    static char arg[MAX_NAME + 1];
    if(!getWord(&args, arg) || *trimSpaces(args) != '\0'){
        printf("Usage: %s\n", usage);
        return NULL;
    }
    return arg;
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
        if(isspace((unsigned char)name[i]) || name[i] == '/'){
            return 0;
        }
        i++;
    }
    return 1;
}

int initializeFreeBlocks(Vfs *vfs){
    int i = 0;
    while(i < NUM_BLOCKS){
        FreeBlock *node = malloc(sizeof(FreeBlock));
        if(node == NULL){
            return 0;
        }
        node->index = i;
        node->next = NULL;
        node->prev = vfs->freeTail;
        if(vfs->freeTail == NULL){
            vfs->freeHead = node;
        }
        else{
            vfs->freeTail->next = node;
        }
        vfs->freeTail = node;
        vfs->freeCount++;
        i++;
    }
    return 1;
}

FreeBlock *allocateBlock(Vfs *vfs){
    if(vfs->freeHead == NULL){
        return NULL;
    }
    FreeBlock *node = vfs->freeHead;
    vfs->freeHead = vfs->freeHead->next;
    if(vfs->freeHead == NULL){
        vfs->freeTail = NULL;
    }
    else{
        vfs->freeHead->prev = NULL;
    }
    node->next = NULL;
    node->prev = NULL;
    vfs->freeCount--;
    return node;
}

void releaseBlock(Vfs *vfs, FreeBlock *node){
    if(node == NULL){
        return;
    }
    node->next = NULL;
    node->prev = vfs->freeTail;
    if(vfs->freeTail == NULL){
        vfs->freeHead = node;
    }
    else{
        vfs->freeTail->next = node;
    }
    vfs->freeTail = node;
    vfs->freeCount++;
}

void releaseFileBlocks(Vfs *vfs, FileNode *file){
    int i = 0;
    while(i < file->blockCount){
        releaseBlock(vfs, file->blockNodes[i]);
        i++;
    }
    free(file->blockNodes);
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
    node->blockNodes = NULL;
    node->blockCount = 0;
    node->contentSize = 0;
    return node;
}

int initializeFileSystem(Vfs *vfs){
    vfs->freeHead = NULL;
    vfs->freeTail = NULL;
    vfs->freeCount = 0;
    vfs->root = NULL;
    vfs->cwd = NULL;
    
    if(!initializeFreeBlocks(vfs)){
        return 0;
    }
    vfs->root = createNode("/", 1);
    if(vfs->root == NULL){
        return 0;
    }
    vfs->cwd = vfs->root;
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

void makeDirectory(Vfs *vfs, char *args){
    char *name = singleArgument(args, "mkdir directory_name");
    if(name == NULL){
        return;
    }
    if(!isValidName(name)){
        printf("Invalid directory name.\n");
        return;
    }
    if(findChild(vfs->cwd, name) != NULL){
        printf("Name already exists.\n");
        return;
    }
    FileNode *node = createNode(name, 1);
    if(node == NULL){
        printf("Memory allocation failed.\n");
        return;
    }
    insertChild(vfs->cwd, node);
    printf("Directory '%s' created successfully.\n", name);
}

void changeDirectory(Vfs *vfs, char *args){
    char *name = singleArgument(args, "cd directory_name");
    if(name == NULL){
        return;
    }
    if(strcmp(name, "..") == 0){
        if(vfs->cwd->parent != NULL){
            vfs->cwd = vfs->cwd->parent;
        }
    }
    else{
        FileNode *node = findChild(vfs->cwd, name);
        if(node == NULL || !node->isDirectory){
            printf("Directory not found.\n");
            return;
        }
        vfs->cwd = node;
    }
    printf("Moved to ");
    printPath(vfs->cwd);
    printf("\n");
}

void listDirectory(Vfs *vfs, char *args){
    if(*trimSpaces(args) != '\0'){
        printf("Usage: ls\n");
        return;
    }
    if(vfs->cwd->children == NULL){
        printf("(empty)\n");
        return;
    }
    FileNode *current = vfs->cwd->children;
    while(1){
        printf("%s%s\n", current->name, current->isDirectory ? "/" : "");
        current = current->next;
        if(current == vfs->cwd->children){
            break;
        }
    }
}

void removeDirectory(Vfs *vfs, char *args){
    char *name = singleArgument(args, "rmdir directory_name");
    if(name == NULL){
        return;
    }
    FileNode *node = findChild(vfs->cwd, name);
    if(node == NULL || !node->isDirectory){
        printf("Directory not found.\n");
        return;
    }
    if(node->children != NULL){
        printf("Directory must be empty before deletion.\n");
        return;
    }
    removeChild(vfs->cwd, node);
    free(node);
    printf("Directory removed successfully.\n");
}

void createFile(Vfs *vfs, char *args){
    char *name = singleArgument(args, "create filename");
    if(name == NULL){
        return;
    }
    if(!isValidName(name)){
        printf("Invalid filename.\n");
        return;
    }
    if(findChild(vfs->cwd, name) != NULL){
        printf("Name already exists.\n");
        return;
    }
    FileNode *node = createNode(name, 0);
    if(node == NULL){
        printf("Memory allocation failed.\n");
        return;
    }
    insertChild(vfs->cwd, node);
    printf("File '%s' created successfully.\n", name);
}

void writeFile(Vfs *vfs, char *args){
    char name[MAX_NAME + 1];
    char *content;
    
    if(!getWord(&args, name)){
        printf("Usage: write filename \"content\"\n");
        return;
    }
    if(!parseContent(args, &content)){
        printf("Content must be enclosed in quotes.\n");
        return;
    }
    
    FileNode *file = findChild(vfs->cwd, name);
    if(file == NULL || file->isDirectory){
        printf("File not found.\n");
        return;
    }
    size_t length = strlen(content);
    int required = (int)((length + BLOCK_SIZE - 1) / BLOCK_SIZE);
    if(required > vfs->freeCount + file->blockCount){
        printf("Disk is full. Cannot write file.\n");
        return;
    }
    FreeBlock **newNodes = NULL;
    if(required > 0){
        newNodes = malloc(required * sizeof(FreeBlock *));
        if(newNodes == NULL){
            printf("Memory allocation failed.\n");
            return;
        }
    }
    releaseFileBlocks(vfs, file);
    int i = 0;
    while(i < required){
        FreeBlock *block = allocateBlock(vfs);
        newNodes[i] = block;
        int j = 0;
        size_t start = (size_t)i * BLOCK_SIZE;
        while(j < BLOCK_SIZE){
            if(start + j < length){
                vfs->disk[block->index][j] = content[start + j];
            }
            else{
                vfs->disk[block->index][j] = '\0';
            }
            j++;
        }
        i++;
    }
    file->blockNodes = newNodes;
    file->blockCount = required;
    file->contentSize = length;
    printf("Data written successfully (size=%d bytes).\n", (int)length);
}

void readFile(Vfs *vfs, char *args){
    char *name = singleArgument(args, "read filename");
    if(name == NULL){
        return;
    }
    FileNode *file = findChild(vfs->cwd, name);
    if(file == NULL || file->isDirectory){
        printf("File not found.\n");
        return;
    }
    size_t remaining = file->contentSize;
    int i = 0;
    while(i < file->blockCount && remaining > 0){
        int j = 0;
        int blockIndex = file->blockNodes[i]->index;
        while(j < BLOCK_SIZE && remaining > 0){
            putchar(vfs->disk[blockIndex][j]);
            j++;
            remaining--;
        }
        i++;
    }
    printf("\n");
}

void deleteFile(Vfs *vfs, char *args){
    char *name = singleArgument(args, "delete filename");
    if(name == NULL){
        return;
    }
    FileNode *file = findChild(vfs->cwd, name);
    if(file == NULL || file->isDirectory){
        printf("File not found.\n");
        return;
    }
    releaseFileBlocks(vfs, file);
    removeChild(vfs->cwd, file);
    free(file);
    printf("File deleted successfully.\n");
}

void showDiskUsage(Vfs *vfs, char *args){
    if(*trimSpaces(args) != '\0'){
        printf("Usage: df\n");
        return;
    }
    int used = NUM_BLOCKS - vfs->freeCount;
    float percentage = (float)used * 100 / NUM_BLOCKS;
    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", vfs->freeCount);
    printf("Disk Usage: %.2f%%\n", percentage);
}

void freeDirectoryContents(Vfs *vfs, FileNode *directory){
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
            freeDirectoryContents(vfs, current);
        }
        else{
            releaseFileBlocks(vfs, current);
        }
        free(current);
        if(last){
            break;
        }
        current = next;
    }
}

void cleanupFileSystem(Vfs *vfs){
    if(vfs->root != NULL){
        freeDirectoryContents(vfs, vfs->root);
        free(vfs->root);
        vfs->root = NULL;
        vfs->cwd = NULL;
    }
    while(vfs->freeHead != NULL){
        FreeBlock *next = vfs->freeHead->next;
        free(vfs->freeHead);
        vfs->freeHead = next;
    }
    vfs->freeTail = NULL;
    vfs->freeCount = 0;
}

void exitShell(Vfs *vfs, char *args){
    if(*trimSpaces(args) != '\0'){
        printf("Usage: exit\n");
        return;
    }
    cleanupFileSystem(vfs);
    printf("Memory released. Exiting program...\n");
    exit(0);
}

Command commands[] = {
    {"mkdir", makeDirectory},
    {"cd", changeDirectory},
    {"create", createFile},
    {"write", writeFile},
    {"read", readFile},
    {"delete", deleteFile},
    {"rmdir", removeDirectory},
    {"ls", listDirectory},
    {"df", showDiskUsage},
    {"exit", exitShell}
};

void showPwd(Vfs *vfs, char *args){
    if(*trimSpaces(args) != '\0'){
        printf("Usage: pwd\n");
        return;
    }
    printPath(vfs->cwd);
    printf("\n");
}

int processCommand(Vfs *vfs, char *input){
    char command[MAX_NAME + 1];
    char *args = input;
    
    if(!getWord(&args, command)){
        return 1;
    }
    
    args = trimSpaces(args);
    
    if(strcmp(command, "pwd") == 0){
        showPwd(vfs, args);
        return 1;
    }
    
    int numCommands = sizeof(commands) / sizeof(commands[0]);
    int i = 0;
    while(i < numCommands){
        if(strcmp(command, commands[i].name) == 0){
            commands[i].handler(vfs, args);
            return 1;
        }
        i++;
    }
    
    printf("Unknown command: %s\n", command);
    return 1;
}

int main(){
    char input[MAX_INPUT];
    Vfs vfs;
    
    if(!initializeFileSystem(&vfs)){
        printf("Filesystem initialization failed.\n");
        cleanupFileSystem(&vfs);
        return 1;
    }
    
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    
    while(1){
        printPath(vfs.cwd);
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
        
        processCommand(&vfs, input);
    }
    
    cleanupFileSystem(&vfs);
    return 0;
}