#ifndef QSETTINGSUTF8_H
#define QSETTINGSUTF8_H

#include <QObject>
#include <QSettings>

#ifdef QSETTINGSUTF8_LIBRARY
#include "QSettingsUtf8_global.h"

class QSETTINGSUTF8_EXPORT QSettingsUtf8 : public QSettings
#else
class QSettingsUtf8 : public QSettings
#endif
{
	Q_OBJECT
    Q_CLASSINFO("作者", "闫磊")
    Q_CLASSINFO("描述", "支持中文字的QSettings,不要和Qt原生的QSettings混合使用")
    Q_CLASSINFO("版本号", "2.0.0.20220308")

public:
    QSettingsUtf8(const QString &fileName, QSettings::Format format,QObject *parent = nullptr);
    ~QSettingsUtf8();

    QStringList allKeysOf(const QString &contex);

private:
    Q_DISABLE_COPY(QSettingsUtf8)
    static QSettings::Format m_gformat;
private:
    //int m_iCount=1;
};

#endif // QSETTINGSUTF8_H
