#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#define MIN_POOL_SIZE 1
#define MAX_POOL_SIZE 1000000

#define INITIAL_CAPACITY 1
#define INITIAL_COUNT 1
#define INITIAL_ID 1
#define FREE_BLOCK_ID 0

#define FIRST_FIT 1
#define BEST_FIT 2
#define WORST_FIT 3

struct Block{
    size_t start; //starting offset
    size_t size;
    int id;
    int isAllocated; //1->allocated , 0->free
};

struct MemoryManager{
    unsigned char *pool; //actual memory buffer
    struct Block *blocks;
    size_t poolSize;
    size_t count;
    size_t capacity;
    int nextId;
    int strategy;
};

int getInt(int *value){
    char input[100], extra;
    int ch, n;

    while(1){
        if(fgets(input, sizeof(input), stdin) == NULL){
            return 0;
        }

        if(strchr(input, '\n') == NULL){
            ch = getchar();
            if(ch != '\n' && ch != EOF){
                while(ch != '\n' && ch != EOF){
                    ch = getchar();
                }
                printf("\nInput is too long. Please enter a shorter value.");
                continue;
            }
        }

        if(sscanf(input, "%d %c", &n, &extra) != 1){
            printf("\nEnter only an integer: ");
            continue;
        }

        *value = n;
        return 1;
    }
}

int validPoolSize(int poolSize){
    return poolSize >= MIN_POOL_SIZE && poolSize <= MAX_POOL_SIZE;
}

int validStrategy(int strategy){
    return strategy == FIRST_FIT || strategy == BEST_FIT || strategy == WORST_FIT;
}

int validRequestSize(int requestSize, size_t poolSize){
    return requestSize > 0 && (size_t)requestSize <= poolSize;
}

int initializePool(struct MemoryManager *m){
    int poolSize;

    m->pool = NULL;
    m->blocks = NULL;
    m->poolSize = 0;
    m->count = 0;
    m->capacity = 0;
    m->nextId = INITIAL_ID;
    m->strategy = BEST_FIT;

    printf("\nEnter Pool Size: ");
    if(!getInt(&poolSize)){
        return 0;
    }

    while(!validPoolSize(poolSize)){
        printf("\nPool size must be between %d and %d.",MIN_POOL_SIZE, MAX_POOL_SIZE);
        printf("\nEnter Pool Size: ");
        if(!getInt(&poolSize)){
            return 0;
        }
    }
    m->poolSize = (size_t)poolSize;
    m->pool = malloc(m->poolSize * sizeof(unsigned char));
    if(m->pool == NULL){
        printf("\nMemory Allocation failed!");
        return 0;
    }

    m->blocks = malloc(sizeof(struct Block));
    if(m->blocks == NULL){
        printf("\nMemory Allocation failed!");
        free(m->pool);
        m->pool = NULL;
        return 0;
    }

    m->blocks->start = 0;
    m->blocks->size = m->poolSize;
    m->blocks->id = FREE_BLOCK_ID;
    m->blocks->isAllocated = 0;

    m->count = INITIAL_COUNT;
    m->capacity = INITIAL_CAPACITY;

    return 1;
}

int ensureCapacity(struct MemoryManager *m, size_t requiredCapacity){
    if(requiredCapacity <= m->capacity){
        return 1;
    }

    size_t newCapacity = m->capacity * 2;

    while(newCapacity < requiredCapacity){
        newCapacity *= 2;
    }

    struct Block *t = realloc(m->blocks, newCapacity * sizeof(*t));
    if(t == NULL){
        printf("\nError allocation memory!");
        return 0;
    }

    m->capacity = newCapacity;
    m->blocks = t;

    return 1;
}

int firstFit(struct MemoryManager *m, size_t size){
    for(size_t i=0;i<m->count;i++){
        if(!(m->blocks+i)->isAllocated){
            if((m->blocks+i)->size >= size){
                return i;
            }
        }
    }

    return -1;
}

int bestFit(struct MemoryManager *m, size_t size){
    int minIdx = -1;
    size_t allocatedSize = SIZE_MAX;

    for(size_t i=0;i<m->count;i++){
        if(!(m->blocks+i)->isAllocated){
            if((m->blocks+i)->size >= size){
                if(allocatedSize > (m->blocks+i)->size){
                    allocatedSize = (m->blocks+i)->size;
                    minIdx = i;
                }
            }
        }
    }

    return minIdx;
}

int worstFit(struct MemoryManager *m, size_t size){
    int maxIdx = -1;
    size_t allocatedSize = 0;

    for(size_t i=0;i<m->count;i++){
        if(!(m->blocks+i)->isAllocated){
            if((m->blocks+i)->size >= size){
                if((m->blocks+i)->size > allocatedSize){
                    allocatedSize = (m->blocks+i)->size;
                    maxIdx = i;
                }
            }
        }
    }

    return maxIdx;
}

int findSuitableBlock(struct MemoryManager *m, size_t size, int strategy){
    int idx;

    if(strategy == FIRST_FIT){
        idx = firstFit(m,size);
    }else if(strategy == BEST_FIT){
        idx = bestFit(m,size);
    }else{
        idx = worstFit(m,size);
    }

    return idx;
}

int allocateMemory(struct MemoryManager *m, size_t size, int strategy, int *id, size_t *start){
    if(size == 0 || size > m->poolSize){
        printf("\nInvalid size: must be between 1 and %zu.", m->poolSize);
        return 0;
    }

    int idx = findSuitableBlock(m,size,strategy);

    if(idx == -1){
        printf("\nNo suitable block found!");
        return 0;
    }

    int allocationId = m->nextId;
    size_t allocationStart = (m->blocks+idx)->start;

    if((m->blocks+idx)->size == size){
        (m->blocks+idx)->isAllocated = 1;
        (m->blocks+idx)->id = allocationId;

        m->nextId++;
    }else if((m->blocks+idx)->size > size){

        if(!ensureCapacity(m,m->count+1)){
            return 0;
        }

        size_t oldsize = (m->blocks+idx)->size;
        size_t oldstart = (m->blocks+idx)->start;

        for(size_t i=m->count;i>(size_t)idx+1;i--){
            *(m->blocks+i) = *(m->blocks+i-1);
        }

        (m->blocks+idx)->size = size;
        (m->blocks+idx)->isAllocated = 1;
        (m->blocks+idx)->id = allocationId;

        (m->blocks+idx+1)->start = oldstart+size;
        (m->blocks+idx+1)->size = oldsize-size;
        (m->blocks+idx+1)->id = FREE_BLOCK_ID;
        (m->blocks+idx+1)->isAllocated = 0;

        m->count++;
        m->nextId++;
    }

    *id = allocationId;
    *start = allocationStart;

    return 1;
}

void mergeFreeBlock(struct MemoryManager *m){
    size_t i = 1;

    while(i < m->count){
        if(!(m->blocks+i-1)->isAllocated && !(m->blocks+i)->isAllocated){
            (m->blocks+i-1)->size += (m->blocks+i)->size;

            for(size_t j=i+1;j<m->count;j++){
                *(m->blocks+j-1) = *(m->blocks+j);
            }

            m->count--;
        }else{
            i++;
        }
    }
}

int releaseMemory(struct MemoryManager *m, int id){
    if(id <= 0){
        printf("\nId must be greater than 0");
        return 0;
    }

    int found = 0;

    for(size_t i=0;i<m->count;i++){
        if((m->blocks+i)->id == id && (m->blocks+i)->isAllocated){
            (m->blocks+i)->isAllocated = 0;
            (m->blocks+i)->id = FREE_BLOCK_ID;
            found = 1;
            break;
        }
    }

    if(!found){
        printf("\nMemory with allocation id %d not found!", id);
        return 0;
    }

    mergeFreeBlock(m);

    return 1;
}

int resizeMemory(struct MemoryManager *m, int id, size_t newSize){
    if(newSize == 0 || newSize > m->poolSize){
        printf("\nSize is out of range");
        return 0;
    }

    int found = 0;
    size_t i = 0;

    for(i=0;i<m->count;i++){
        if((m->blocks+i)->id == id && (m->blocks+i)->isAllocated){
            found = 1;
            break;
        }
    }

    if(!found){
        printf("\nAllocation Id not Found!");
        return 0;
    }

    if((m->blocks+i)->size > newSize){

        size_t originalSize = (m->blocks+i)->size;
        size_t originalStart = (m->blocks+i)->start;

        if(!ensureCapacity(m,m->count+1)){
            return 0;
        }

        for(size_t j=m->count;j>i;j--){
            *(m->blocks+j) = *(m->blocks+j-1);
        }

        (m->blocks+i)->size = newSize;

        (m->blocks+i+1)->start = originalStart+newSize;
        (m->blocks+i+1)->size = originalSize-newSize;
        (m->blocks+i+1)->id = FREE_BLOCK_ID;
        (m->blocks+i+1)->isAllocated = 0;

        m->count++;

        mergeFreeBlock(m);

    }else if((m->blocks+i)->size < newSize){

        size_t requiredSize = newSize-(m->blocks+i)->size;

        if(i+1 < m->count && !(m->blocks+i+1)->isAllocated &&
           (m->blocks+i+1)->size == requiredSize){

            (m->blocks+i)->size = newSize;

            for(size_t j=i+2;j<m->count;j++){
                *(m->blocks+j-1) = *(m->blocks+j);
            }

            m->count--;

        }
        else if(i+1 < m->count && !(m->blocks+i+1)->isAllocated &&
                 (m->blocks+i+1)->size > requiredSize){

            (m->blocks+i)->size = newSize;
            (m->blocks+i+1)->start += requiredSize;
            (m->blocks+i+1)->size -= requiredSize;
        }
        else{

            size_t oldstart = (m->blocks+i)->start;
            size_t oldsize = (m->blocks+i)->size;
            int oldId = (m->blocks+i)->id;

            int idx = findSuitableBlock(m,newSize,m->strategy);

            if(idx == -1){
                printf("\nNo Suitable block exits for relocation");
                return 0;
            }

            size_t newstart = (m->blocks+idx)->start;
            size_t availableSize = (m->blocks+idx)->size;

            if(availableSize > newSize){
                if(!ensureCapacity(m,m->count+1)){
                    return 0;
                }
            }

            if(availableSize == newSize){
                (m->blocks+idx)->isAllocated = 1;
                (m->blocks+idx)->id = oldId;
            }
            else{
                for(size_t j=m->count;j>(size_t)idx+1;j--){
                    *(m->blocks+j) = *(m->blocks+j-1);
                }

                (m->blocks+idx)->size = newSize;
                (m->blocks+idx)->isAllocated = 1;
                (m->blocks+idx)->id = oldId;

                (m->blocks+idx+1)->start = newstart+newSize;
                (m->blocks+idx+1)->size = availableSize-newSize;
                (m->blocks+idx+1)->isAllocated = 0;
                (m->blocks+idx+1)->id = FREE_BLOCK_ID;

                m->count++;
            }

            memcpy(m->pool+newstart,m->pool+oldstart,oldsize);

            for(size_t j=0;j<m->count;j++){
                if((m->blocks+j)->start == oldstart &&
                   (m->blocks+j)->id == oldId){

                    (m->blocks+j)->isAllocated = 0;
                    (m->blocks+j)->id = FREE_BLOCK_ID;
                    break;
                }
            }

            mergeFreeBlock(m);
        }
    }else{

        printf("\nsize is already equal to requested size");
        return 2;
    }

    return 1;
}

void displayMemoryMap(struct MemoryManager *m){
    for(size_t i=0;i<m->count;i++){
        printf("\nBlock Number: %zu \nStarting Address: %zu \nEnding Address: %zu \nBlock Size: %zu \nStatus: %s \nAllocation ID: %d",
               i+1,
                (m->blocks+i)->start,
                (m->blocks+i)->start+(m->blocks+i)->size-1,
                (m->blocks+i)->size,
                (m->blocks+i)->isAllocated == 1 ? "Allocated" : "Free",
                (m->blocks+i)->id);

        printf("\n------------------");
    }
}

void displayMemoryStatistics(struct MemoryManager *m){
    size_t totalMemory = 0, allocatedMemory = 0, freeMemory = 0;
    int n_allocated = 0, n_free = 0;

    for(size_t i=0;i<m->count;i++){
        totalMemory += (m->blocks+i)->size;

        if((m->blocks+i)->isAllocated){
            allocatedMemory += (m->blocks+i)->size;
            n_allocated++;
        }else{
            freeMemory += (m->blocks+i)->size;
            n_free++;
        }
    }

    printf("\nTotal Pool Size: %zu", totalMemory);
    printf("\nAllocated Memory: %zu", allocatedMemory);
    printf("\nFree Memory: %zu", freeMemory);
    printf("\nNumber of Allocated Blocks: %d", n_allocated);
    printf("\nNumber of Free Blocks: %d", n_free);
    printf("\nMemory Utilization: %0.2lf", ((double)allocatedMemory/totalMemory)*100.0);
    printf("\nFree Memory Percentage: %0.2lf", ((double)freeMemory/totalMemory)*100.0);
}

void detectExternalFragmentation(struct MemoryManager *m, size_t requestedSize){
    size_t totalFreeMemory = 0;
    size_t largestFreeBlock = 0;

    for(size_t i=0;i<m->count;i++){
        if(!(m->blocks+i)->isAllocated){
            totalFreeMemory += (m->blocks+i)->size;

            if((m->blocks+i)->size > largestFreeBlock){
                largestFreeBlock = (m->blocks+i)->size;
            }
        }
    }

    printf("\nTotal Free Memory: %zu bytes", totalFreeMemory);
    printf("\nLargest Free Block: %zu bytes", largestFreeBlock);

    if(requestedSize == 0 || requestedSize > m->poolSize){
        printf("\nInvalid requested size!");
    }else if(totalFreeMemory >= requestedSize && largestFreeBlock < requestedSize){
        printf("\nExternal Fragmentation Detected!");
        printf("\nTotal free memory is sufficient, but no contiguous block is large enough.");
    }else if(largestFreeBlock >= requestedSize){
        printf("\nNo external fragmentation preventing this allocation.");
    }else{
        printf("\nInsufficient total free memory!");
    }
}

int changeAllocationStrategy(struct MemoryManager *m){
    printf("\n1 -> First Fit");
    printf("\n2 -> Best Fit");
    printf("\n3 -> Worst Fit");

    while(1){
        printf("\nEnter your choice: ");
        int n;
        if(!getInt(&n)){
            return 0;
        }

        if(!validStrategy(n)){
            printf("\nEnter a choice between %d and %d.", FIRST_FIT, WORST_FIT);
            continue;
        }

        switch(n){
            case FIRST_FIT:
                m->strategy = FIRST_FIT;
                break;

            case BEST_FIT:
                m->strategy = BEST_FIT;
                break;

            case WORST_FIT:
                m->strategy = WORST_FIT;
                break;
        }

        break;
    }

    switch(m->strategy){
        case FIRST_FIT:
            printf("First Fit");
            break;

        case BEST_FIT:
            printf("Best Fit");
            break;

        case WORST_FIT:
            printf("Worst Fit");
            break;
    }

    return 1;
}

void printMenu(){
    printf("\n\n========== SMART MEMORY MANAGER ==========");
    printf("\n1. Allocate Memory");
    printf("\n2. Release Memory");
    printf("\n3. Resize Memory");
    printf("\n4. Display Memory Map");
    printf("\n5. Display Memory Statistics");
    printf("\n6. Detect External Fragmentation");
    printf("\n7. Change Allocation Strategy");
    printf("\n8. Exit");
}

void cleanupMemoryManager(struct MemoryManager *m){
    if(m == NULL){
        return;
    }

    free(m->pool);
    free(m->blocks);
    free(m);
}

int main(){
    struct MemoryManager *m = malloc(sizeof(*m));

    if(m == NULL){
        printf("\nMemory Allocation failed!");
        return 1;
    }

    int a = initializePool(m);

    if(!a){
        cleanupMemoryManager(m);
        return 1;
    }

    int choice;

    while(1){
        printMenu();
        printf("\nEnter your choice: ");

        if(!getInt(&choice)){
            cleanupMemoryManager(m);
            return 0;
        }

        switch(choice){

            case 1:{
                size_t size, start;
                int id;

                printf("\nEnter allocation size: ");
                int requestedSize;
                if(!getInt(&requestedSize)){
                    cleanupMemoryManager(m);
                    return 0;
                }

                size = (size_t)requestedSize;

                if(allocateMemory(m,size,m->strategy,&id,&start)){
                    printf("\nAllocation successful. ID: %d, Start: %zu, Size: %zu", id, start, size);
                }else if(requestedSize > 0 && size <= m->poolSize){
                    detectExternalFragmentation(m,size);
                }

                break;
            }

            case 2:{
                int id;

                printf("\nEnter Allocation ID: ");
                if(!getInt(&id)){
                    cleanupMemoryManager(m);
                    return 0;
                }

                if(releaseMemory(m,id)){
                    printf("\nMemory released successfully!");
                }

                break;
            }

            case 3:{
                int id;
                size_t newSize;

                printf("\nEnter Allocation ID: ");
                if(!getInt(&id)){
                    cleanupMemoryManager(m);
                    return 0;
                }

                printf("\nEnter New Size: ");
                int requestedSize;
                if(!getInt(&requestedSize)){
                    cleanupMemoryManager(m);
                    return 0;
                }

                newSize = (size_t)requestedSize;

                if(!validRequestSize(requestedSize,m->poolSize)){
                    printf("\nInvalid resize size.");
                    break;
                }

                int resizeStatus = resizeMemory(m,id,newSize);

                if(resizeStatus == 1){
                    printf("\nMemory resized successfully!");
                }else if(resizeStatus == 0){
                    detectExternalFragmentation(m,newSize);
                }

                break;
            }

            case 4:
                displayMemoryMap(m);
                break;

            case 5:
                displayMemoryStatistics(m);
                break;

            case 6:{
                printf("\nEnter requested size: ");
                int inputSize;
                if(!getInt(&inputSize)){
                    cleanupMemoryManager(m);
                    return 0;
                }

                if(inputSize < 0){
                    printf("\nInvalid requested size.");
                    break;
                }

                detectExternalFragmentation(m,(size_t)inputSize);
                break;
            }

            case 7:
                if(!changeAllocationStrategy(m)){
                    cleanupMemoryManager(m);
                    return 0;
                }
                break;

            case 8:
                printf("\nExiting Memory Manager...");
                cleanupMemoryManager(m);
                return 0;

            default:
                printf("\nInvalid choice! Enter a number between 1 and 8.");
        }
    }

    return 0;
}