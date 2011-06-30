#include "lockfile.h"

#include <QObject>
#include <QDir>
#include <QString>
#include <QDebug>

McaLockFile::McaLockFile(const QString &filename, qint64 pid)
    : m_file(filename)
{
    m_canContinue = true;

    if(m_file.exists()) {
        m_file.open(QIODevice::ReadOnly);
        QTextStream in(&m_file);
        QString oldPid = in.readLine();
        QDir dir = QDir::current();

        if(dir.exists("/proc/" + oldPid)) {
            // TODO: Check /proc/{pid}/cmdline
            m_canContinue = false;
            return;
        } else {
            m_file.close();
            m_file.remove();
        }
    }

    QString pidString = QString::number(pid);

    m_file.open(QIODevice::Truncate | QIODevice::WriteOnly);
    QTextStream out(&m_file);
    out << pidString;
    m_file.close();
}

McaLockFile::~McaLockFile()
{
    m_file.remove();
}

bool McaLockFile::canContinue()
{
    return m_canContinue;
}

