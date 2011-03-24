
#include "defines.h"
#ifdef MEMORY_LEAK_DETECTOR
#include <base.h>
#endif

#include <QDebug>
#include "adapter.h"

#ifdef MEMORY_LEAK_DETECTOR
#define __DEBUG_NEW__ new(__FILE__, __LINE__)
#define new __DEBUG_NEW__
#endif

McaAdapter::McaAdapter(QObject *parent):
    QAbstractListModel(parent)
{
#ifdef THREADING_DEBUG
    THREAD_TEST_INIT
#endif
}

#ifdef THREADING_DEBUG
void McaAdapter::moveToThread(QThread *thread)
{
    qDebug() << "Moving thread";
    QAbstractListModel::moveToThread(thread);
    THREAD_TEST_INIT
}
#endif

McaAdapter::~McaAdapter()
{
}

void McaAdapter::triggerSyncUpdate()
{
#if defined(THREADING_DEBUG)
    THREAD_SET_TEST(this);
    emit syncUpdate(this, 0, rowCount() - 1);
    THREAD_UNSET_TEST(this);
#else
    emit syncUpdate(this, 0, rowCount() - 1);
#endif
}

void McaAdapter::triggerSyncRemoval()
{
#if defined(THREADING_DEBUG)
    THREAD_SET_TEST(this);
    emit syncRemoval(this, 0, rowCount() - 1);
    THREAD_UNSET_TEST(this);
#else
    emit syncRemoval(this, 0, rowCount() - 1);
#endif
}

