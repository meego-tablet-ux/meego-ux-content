
#include <memoryleak.h>

struct Memory {
    int index;
    int pointer;
    size_t size;
    int lineNo;
    char *fileName;
    bool deleted;
};

static size_t global_size = 0;
static size_t global_current = 0;
static Memory **allocList = NULL;
static pthread_mutex_t __mutex1__ = PTHREAD_MUTEX_INITIALIZER;

void addToMemLog(void *p, size_t size, const char *file, int line)
{
    if(0 != pthread_mutex_lock( &__mutex1__ )) {
        fprintf(stderr, "LOCK ERROR");
    }
    if(!allocList) {
        allocList = (Memory**)calloc(INCREASE_SIZE, sizeof(Memory*));
        global_size = INCREASE_SIZE;
    } else if(global_current == global_size) {
        global_size += INCREASE_SIZE;
        allocList = (Memory**)realloc(allocList, global_size * sizeof(Memory*));
    }
//    fprintf(stderr, "current %d %d\n", global_current, global_size);
    Memory *__memory = (Memory*)malloc(sizeof(Memory));
    __memory->index = global_current;
    __memory->pointer = (int)p;
    __memory->size = size;
    __memory->lineNo = line;
    __memory->deleted = false;
    if(file) {
        __memory->fileName = strdup(file);
    }

    allocList[global_current] = __memory;
    global_current++;
    pthread_mutex_unlock( &__mutex1__ );
}

void remFromMemLog(void *p)
{
    if(!allocList) return;
    pthread_mutex_lock(&__mutex1__);
    for(size_t i = 0; i < global_current; i++) {
        if(allocList[i] && allocList[i]->pointer == (int)p) {
            if(allocList[i]->deleted) {
                continue;
            }
            allocList[i]->deleted = true;
//            free(allocList[i]->fileName);
//            free(allocList[i]);
//            allocList[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&__mutex1__);
}

void listWhatHappened()
{
    fprintf(stderr, "listWhatHappened() %p\n", allocList);
    if(0 == allocList) {
        return;
    }
    int count = 0;
    int size = 0;
    for(size_t i = 0; i < global_current; i++) {
        if(allocList[i]->deleted == false) {
            fprintf(stderr,"%s: pointer %p, size %d, line %d\n", allocList[i]->fileName, (void*)allocList[i]->pointer, allocList[i]->size, allocList[i]->lineNo);
            size += allocList[i]->size;
            count++;
        }
    }
    fprintf(stderr, "TOTAL: %d, %d\n", count, size);
}

void* operator new(size_t size, const char *file, int line) throw(std::bad_alloc)
{
    void *p=malloc(size);
    addToMemLog(p, size, file, line);
    if (p==0) {// did malloc succeed?
        throw std::bad_alloc();
    }
    return p;
}

void* operator new[](size_t size, const char *file, int line) throw (std::bad_alloc)
{
    void *p=malloc(size);
    addToMemLog(p, size, file, line);

    if (p==0) {// did malloc succeed?
        throw std::bad_alloc();
    }
    return p;
}

void operator delete (void *p)
{
    remFromMemLog(p);
    free(p);
}

#include "memoryleak-defines.h"
