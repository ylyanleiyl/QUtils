#include "QSettingsUtf8.h"
#include "QSettingsUtf8Private.h"

#include <QDebug>
#include <QCoreApplication>
#include <QRegularExpression>

QSettings::Format QSettingsUtf8::m_gformat = QSettings::registerFormat("ini", QSettingsUtf8Private::ReadFunc, QSettingsUtf8Private::WriteFunc);

QSettingsUtf8::QSettingsUtf8(const QString& fileName, Format format, QObject* parent)
    : QSettings(fileName,QSettingsUtf8::m_gformat , parent)
{
    //static int iCount=1;
    //m_iCount = iCount++;
    //qDebug() << "QSettingsUtf8" << m_iCount << "<";
}

QSettingsUtf8::~QSettingsUtf8()
{
    //qDebug() << "~QSettingsUtf8" << m_iCount << ">";
}

QStringList QSettingsUtf8::allKeysOf(const QString &contex)
{
    QStringList keys;
    foreach (const QString &key, allKeys()) {
        QRegularExpression re(contex);
        QRegularExpressionMatch match = re.match(key);
        if (match.hasMatch()) {
            keys.append(key);
        }
    }
    return keys;
}


int main(int argc, char *argv[])
{
    QSettingsUtf8 settings("QSettingsUtf8.ini",QSettings::IniFormat);//settings.setIniCodec("UTF-8");
    settings.beginGroup("CollectForms");
    settings.setValue(QString("中文测试"),"中文测试");
    settings.endGroup();
    settings.setValue(QString("Modbus采集-未命名/容器项目"),"中文测试");

    {
        QSettingsUtf8 settings2("QSettingsUtf8.ini",QSettings::IniFormat);//settings.setIniCodec("UTF-8");
        settings2.setValue(QString("Modbus采集-未命名/类名"),"中文测试2");
    }
}
