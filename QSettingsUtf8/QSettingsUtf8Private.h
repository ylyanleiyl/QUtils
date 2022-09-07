#ifndef QSETTING_UTF8PRIVATE_H
#define QSETTING_UTF8PRIVATE_H

#include <QObject>
#include <QSettings>

#include "QSettingsUtf8.h"

class QSettingsUtf8Private
{
public:
    static bool ReadFunc(QIODevice &device, QSettings::SettingsMap &map);
    static bool WriteFunc(QIODevice &device, const QSettings::SettingsMap &map);

};


#endif // QSETTING_UTF8PRIVATE_H
