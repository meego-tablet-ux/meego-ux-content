#include "memoryleak.h"
#include <QDebug>
#include "adapter.h"

#include "memoryleak-defines.h"

McaAdapter::McaAdapter(QObject *parent):
    QAbstractListModel(parent)
{
    THREAD_TEST_INIT
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

#ifdef THREADING
void McaAdapter::triggerSyncUpdate()
{
    THREAD_SET_TEST(this);
    emit syncUpdate(this, 0, rowCount() - 1);
    THREAD_UNSET_TEST(this);
}

void McaAdapter::triggerSyncRemoval()
{
    THREAD_SET_TEST(this);
    emit syncRemoval(this, 0, rowCount() - 1);
    THREAD_UNSET_TEST(this);
}
#endif
