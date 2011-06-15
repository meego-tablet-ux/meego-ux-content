#ifndef __MEMORYLEAK_H__
#define __MEMORYLEAK_H__

#ifdef MEMORY_LEAK_DETECTOR

#include <exception> // for std::bad_alloc
#include <new>
#include <cstdlib> // for malloc() and free()
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define INCREASE_SIZE 10000

void addToMemLog(void *p, size_t size, const char *file, int line);
void remFromMemLog(void *p);

void* operator new(size_t size, const char *file, int line) throw(std::bad_alloc);
void* operator new[](size_t size, const char *file, int line) throw (std::bad_alloc);
void operator delete (void *p);

void listWhatHappened();

//#define __DEBUG_NEW__ new(__FILE__, __LINE__)
//#define new __DEBUG_NEW__

#endif //MEMORY_LEAK_DETECTOR

#endif // __MEMORYLEAK_H__
