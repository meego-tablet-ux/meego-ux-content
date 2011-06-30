#ifndef __mcalockfile_h
#define __maclockfile_h

#include <QObject>
#include <QFile>

class McaLockFile {
public:
    McaLockFile(const QString &filename, qint64 pid);
    ~McaLockFile();

    bool canContinue();
private:
    QFile m_file;
    bool m_canContinue;
};

#endif
