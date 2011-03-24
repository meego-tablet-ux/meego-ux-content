#ifndef __mcaadapter_h
#define __mcaadapter_h

#include <QAbstractListModel>
#include "defines.h"

#include "threadtest.h"

class McaAdapter : public QAbstractListModel
{
    Q_OBJECT
#if defined(THREADING)
public:
    McaAdapter(QObject *parent);
    virtual ~McaAdapter();

    Q_INVOKABLE void triggerSyncUpdate();
    Q_INVOKABLE void triggerSyncRemoval();

#if defined(THREADING_DEBUG)
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
