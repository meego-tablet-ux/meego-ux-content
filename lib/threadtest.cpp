#include "threadtest.h"
#include "debug.h"

#ifdef THREADING_DEBUG
#include <QDebug>

McaThreadTest::McaThreadTest(QObject *parent):
    QThread(parent)
{
    m_lock = false;
}

void McaThreadTest::setTestLock()
{
    m_lock = true;
    qDebug() << "******** Lock set";
}

void McaThreadTest::unsetTestLock()
{
    m_lock = false;
    qDebug() << "******** Lock unset";
}

void McaThreadTest::print()
{
    if (!m_lock) {
        qDebug() << "Thread is not locked" << this;
        BACKTRACE
    }
}
#endif

