#ifndef __threadtest_h
#define __threadtest_h

#include "defines.h"

#ifdef THREADING_DEBUG

#include <QThread>

#define THREAD_SET_TEST(a) { \
         McaThreadTest *t = qobject_cast<McaThreadTest*>(a->thread()); \
        if(t) t->setTestLock(); \
        else qDebug() << "Error, not McaThreadTest: " << a->thread(); }
#define THREAD_UNSET_TEST(a) { \
        McaThreadTest *t = qobject_cast<McaThreadTest*>(a->thread()); \
        if(t) t->unsetTestLock(); \
        else qDebug() << "Error, not McaThreadTest: " << a->thread(); }
#define THREAD_PRINT_TEST(a) if(m_threadTest) { m_threadTest->print(); }
#define THREAD_TEST McaThreadTest *m_threadTest;
#define THREAD_TEST_INIT m_threadTest = qobject_cast<McaThreadTest*>(this->thread());

class McaThreadTest : public QThread
{
    Q_OBJECT

public:
    McaThreadTest(QObject *parent = 0);

    void setTestLock();
    void unsetTestLock();
    void print();
private:
    bool m_lock;
};

#endif
#endif
