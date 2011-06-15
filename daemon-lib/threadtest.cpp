#include "memoryleak.h"

#include "threadtest.h"
#include <QDebug>

#include "memoryleak-defines.h"

#ifdef THREADING_DEBUG

McaThreadTest::McaThreadTest(QObject *parent):
    QThread(parent)
{
    m_lock = false;
}

void McaThreadTest::setTestLock()
{
    m_lock = true;
    qDebug() << "******** Lock set" << this;
}

void McaThreadTest::unsetTestLock()
{
    m_lock = false;
    qDebug() << "******** Lock unset" << this;
}

void McaThreadTest::print()
{
    if (!m_lock) {
        qDebug() << "Thread is not locked" << this;
    }
}
#endif

