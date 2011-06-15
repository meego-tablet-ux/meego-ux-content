#ifndef __mcaadapter_h
#define __mcaadapter_h

#include <QAbstractListModel>

#include "threadtest.h"

class McaAdapter : public QAbstractListModel
{
    Q_OBJECT
public:
    McaAdapter(QObject *parent);
    virtual ~McaAdapter();

#ifdef THREADING
    Q_INVOKABLE void triggerSyncUpdate();
    Q_INVOKABLE void triggerSyncRemoval();

#ifdef THREADING_DEBUG
    void moveToThread(QThread *targetThread);
protected:
    THREAD_TEST
#endif

signals:
    void syncUpdate(McaAdapter *adapter, int start, int end);
    void syncRemoval(McaAdapter *adapter, int start, int end);
#endif
};

#endif
