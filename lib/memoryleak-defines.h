#ifndef MEMORYLEAKDEFINES_H
#define MEMORYLEAKDEFINES_H

#ifdef MEMORY_LEAK_DETECTOR
#define __DEBUG_NEW__ new(__FILE__, __LINE__)
#define new __DEBUG_NEW__
#endif

#endif // MEMORYLEAKDEFINES_H
