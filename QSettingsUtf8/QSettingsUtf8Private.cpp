#include <QTextStream>
#include <QStack>
#include <QDataStream>
#include <QDebug>
#include <QRect>
#include <QSize>
#include <QTextCodec>

#include "QSettingsUtf8Private.h"

static const char hexDigits[] = "0123456789ABCDEF";

inline static void iniChopTrailingSpaces(QString &str, int limit)
{
    int n = str.size() - 1;
    QChar ch;
    while (n >= limit && ((ch = str.at(n)) == QLatin1Char(' ') || ch == QLatin1Char('\t')))
        str.truncate(n--);
}

QStringList splitArgs(const QString &s, int idx)
{
    int l = s.length();
    Q_ASSERT(l > 0);
    Q_ASSERT(s.at(idx) == QLatin1Char('('));
    Q_ASSERT(s.at(l - 1) == QLatin1Char(')'));

    QStringList result;
    QString item;

    for (++idx; idx < l; ++idx) {
        QChar c = s.at(idx);
        if (c == QLatin1Char(')')) {
            Q_ASSERT(idx == l - 1);
            result.append(item);
        } else if (c == QLatin1Char(' ')) {
            result.append(item);
            item.clear();
        } else {
            item.append(c);
        }
    }

    return result;
}

QString variantToString(const QVariant &v)
{
    QString result;

    switch (v.type()) {
        case QVariant::Invalid:
            result = QLatin1String("@Invalid()");
            break;

        case QVariant::ByteArray: {
            QByteArray a = v.toByteArray();
            result = QLatin1String("@ByteArray(")
                     + QLatin1String(a.constData(), a.size())
                     + QLatin1Char(')');
            break;
        }

        case QVariant::String:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Bool:
        case QVariant::Double:
        case QVariant::KeySequence: {
            result = v.toString();
            if (result.contains(QChar::Null))
                result = QLatin1String("@String(") + result + QLatin1Char(')');
            else if (result.startsWith(QLatin1Char('@')))
                result.prepend(QLatin1Char('@'));
            break;
        }
#ifndef QT_NO_GEOM_VARIANT
        case QVariant::Rect: {
            QRect r = qvariant_cast<QRect>(v);
            result = QString::asprintf("@Rect(%d %d %d %d)", r.x(), r.y(), r.width(), r.height());
            break;
        }
        case QVariant::Size: {
            QSize s = qvariant_cast<QSize>(v);
            result = QString::asprintf("@Size(%d %d)", s.width(), s.height());
            break;
        }
        case QVariant::Point: {
            QPoint p = qvariant_cast<QPoint>(v);
            result = QString::asprintf("@Point(%d %d)", p.x(), p.y());
            break;
        }
#endif // !QT_NO_GEOM_VARIANT

        default: {
#ifndef QT_NO_DATASTREAM
            QDataStream::Version version;
            const char *typeSpec;
            if (v.type() == QVariant::DateTime) {
                version = QDataStream::Qt_5_6;
                typeSpec = "@DateTime(";
            } else {
                version = QDataStream::Qt_4_0;
                typeSpec = "@Variant(";
            }
            QByteArray a;
            {
                QDataStream s(&a, QIODevice::WriteOnly);
                s.setVersion(version);
                s << v;
            }

            result = QLatin1String(typeSpec)
                     + QLatin1String(a.constData(), a.size())
                     + QLatin1Char(')');
#else
            Q_ASSERT(!"QSettings: Cannot save custom types without QDataStream support");
#endif
            break;
        }
    }

    return result;
}

QVariant stringToVariant(const QString &s)
{
    if (s.startsWith(QLatin1Char('@'))) {
        if (s.endsWith(QLatin1Char(')'))) {
            if (s.startsWith(QLatin1String("@ByteArray("))) {
                return QVariant(s.midRef(11, s.size() - 12).toLatin1());
            } else if (s.startsWith(QLatin1String("@String("))) {
                return QVariant(s.midRef(8, s.size() - 9).toString());
            } else if (s.startsWith(QLatin1String("@Variant("))
                       || s.startsWith(QLatin1String("@DateTime("))) {
#ifndef QT_NO_DATASTREAM
                QDataStream::Version version;
                int offset;
                if (s.at(1) == QLatin1Char('D')) {
                    version = QDataStream::Qt_5_6;
                    offset = 10;
                } else {
                    version = QDataStream::Qt_4_0;
                    offset = 9;
                }
                QByteArray a = s.midRef(offset).toLatin1();
                QDataStream stream(&a, QIODevice::ReadOnly);
                stream.setVersion(version);
                QVariant result;
                stream >> result;
                return result;
#else
                Q_ASSERT(!"QSettings: Cannot load custom types without QDataStream support");
#endif
#ifndef QT_NO_GEOM_VARIANT
            } else if (s.startsWith(QLatin1String("@Rect("))) {
                QStringList args = splitArgs(s, 5);
                if (args.size() == 4)
                    return QVariant(QRect(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt()));
            } else if (s.startsWith(QLatin1String("@Size("))) {
                QStringList args = splitArgs(s, 5);
                if (args.size() == 2)
                    return QVariant(QSize(args[0].toInt(), args[1].toInt()));
            } else if (s.startsWith(QLatin1String("@Point("))) {
                QStringList args = splitArgs(s, 6);
                if (args.size() == 2)
                    return QVariant(QPoint(args[0].toInt(), args[1].toInt()));
#endif
            } else if (s == QLatin1String("@Invalid()")) {
                return QVariant();
            }

        }
        if (s.startsWith(QLatin1String("@@")))
            return QVariant(s.mid(1));
    }

    return QVariant(s);
}

QVariant stringListToVariantList(const QStringList &l)
{
    QStringList outStringList = l;
    for (int i = 0; i < outStringList.count(); ++i) {
        const QString &str = outStringList.at(i);

        if (str.startsWith(QLatin1Char('@'))) {
            if (str.length() >= 2 && str.at(1) == QLatin1Char('@')) {
                outStringList[i].remove(0, 1);
            } else {
                QVariantList variantList;
                const int stringCount = l.count();
                variantList.reserve(stringCount);
                for (int j = 0; j < stringCount; ++j)
                    variantList.append(stringToVariant(l.at(j)));
                return variantList;
            }
        }
    }
    return outStringList;
}

void iniEscapedString(const QString &str, QByteArray &result, QTextCodec *codec)
{
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    bool useCodec = codec && !str.startsWith(QLatin1String("@ByteArray("))
                    && !str.startsWith(QLatin1String("@Variant("));

    int i;
    int startPos = result.size();

    result.reserve(startPos + str.size() * 3 / 2);
    const QChar *unicode = str.unicode();
    for (i = 0; i < str.size(); ++i) {
        uint ch = unicode[i].unicode();
        if (ch == ';' || ch == ',' || ch == '=')
            needsQuotes = true;

        if (escapeNextIfDigit
                && ((ch >= '0' && ch <= '9')
                    || (ch >= 'a' && ch <= 'f')
                    || (ch >= 'A' && ch <= 'F'))) {
            result += "\\x" + QByteArray::number(ch, 16);
            continue;
        }

        escapeNextIfDigit = false;

        switch (ch) {
        case '\0':
            result += "\\0";
            escapeNextIfDigit = true;
            break;
        case '\a':
            result += "\\a";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\v':
            result += "\\v";
            break;
        case '"':
        case '\\':
            result += '\\';
            result += (char)ch;
            break;
        default:
            if (ch <= 0x1F || (ch >= 0x7F && !useCodec)) {
                result += "\\x" + QByteArray::number(ch, 16);
                escapeNextIfDigit = true;
#if QT_CONFIG(textcodec)
            } else if (useCodec) {
                // slow
                result += codec->fromUnicode(&unicode[i], 1);
#endif
            } else {
                result += (char)ch;
            }
        }
    }

    if (needsQuotes
            || (startPos < result.size() && (result.at(startPos) == ' '
                                                || result.at(result.size() - 1) == ' '))) {
        result.insert(startPos, '"');
        result += '"';
    }
}
QStringList variantListToStringList(const QVariantList &l)
{
    QStringList result;
    result.reserve(l.count());
    QVariantList::const_iterator it = l.constBegin();
    for (; it != l.constEnd(); ++it)
        result.append(variantToString(*it));
    return result;
}
void iniEscapedStringList(const QStringList &strs, QByteArray &result, QTextCodec *codec)
{
    if (strs.isEmpty()) {
        /*
            We need to distinguish between empty lists and one-item
            lists that contain an empty string. Ideally, we'd have a
            @EmptyList() symbol but that would break compatibility
            with Qt 4.0. @Invalid() stands for QVariant(), and
            QVariant().toStringList() returns an empty QStringList,
            so we're in good shape.
        */
        result += "@Invalid()";
    } else {
        for (int i = 0; i < strs.size(); ++i) {
            if (i != 0)
                result += ", ";
            iniEscapedString(strs.at(i), result, codec);
        }
    }
}

bool iniUnescapedStringList(const QByteArray &str, int from, int to,
                                              QString &stringResult, QStringList &stringListResult,
                                              QTextCodec *codec)
{
    static const char escapeCodes[][2] =
    {
        { 'a', '\a' },
        { 'b', '\b' },
        { 'f', '\f' },
        { 'n', '\n' },
        { 'r', '\r' },
        { 't', '\t' },
        { 'v', '\v' },
        { '"', '"' },
        { '?', '?' },
        { '\'', '\'' },
        { '\\', '\\' }
    };
    static const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);

    bool isStringList = false;
    bool inQuotedString = false;
    bool currentValueIsQuoted = false;
    int escapeVal = 0;
    int i = from;
    char ch;

StSkipSpaces:
    while (i < to && ((ch = str.at(i)) == ' ' || ch == '\t'))
        ++i;
    // fallthrough

StNormal:
    int chopLimit = stringResult.length();
    while (i < to) {
        switch (str.at(i)) {
        case '\\':
            ++i;
            if (i >= to)
                goto end;

            ch = str.at(i++);
            for (int j = 0; j < numEscapeCodes; ++j) {
                if (ch == escapeCodes[j][0]) {
                    stringResult += QLatin1Char(escapeCodes[j][1]);
                    goto StNormal;
                }
            }

            if (ch == 'x') {
                escapeVal = 0;

                if (i >= to)
                    goto end;

                ch = str.at(i);
                if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
                    goto StHexEscape;
            } else if (ch >= '0' && ch <= '7') {
                escapeVal = ch - '0';
                goto StOctEscape;
            } else if (ch == '\n' || ch == '\r') {
                if (i < to) {
                    char ch2 = str.at(i);
                    // \n, \r, \r\n, and \n\r are legitimate line terminators in INI files
                    if ((ch2 == '\n' || ch2 == '\r') && ch2 != ch)
                        ++i;
                }
            } else {
                // the character is skipped
            }
            chopLimit = stringResult.length();
            break;
        case '"':
            ++i;
            currentValueIsQuoted = true;
            inQuotedString = !inQuotedString;
            if (!inQuotedString)
                goto StSkipSpaces;
            break;
        case ',':
            if (!inQuotedString) {
                if (!currentValueIsQuoted)
                    iniChopTrailingSpaces(stringResult, chopLimit);
                if (!isStringList) {
                    isStringList = true;
                    stringListResult.clear();
                    stringResult.squeeze();
                }
                stringListResult.append(stringResult);
                stringResult.clear();
                currentValueIsQuoted = false;
                ++i;
                goto StSkipSpaces;
            }
            Q_FALLTHROUGH();
        default: {
            int j = i + 1;
            while (j < to) {
                ch = str.at(j);
                if (ch == '\\' || ch == '"' || ch == ',')
                    break;
                ++j;
            }

#if !QT_CONFIG(textcodec)
            Q_UNUSED(codec)
#else
            if (codec) {
                stringResult += codec->toUnicode(str.constData() + i, j - i);
            } else
#endif
            {
                int n = stringResult.size();
                stringResult.resize(n + (j - i));
                QChar *resultData = stringResult.data() + n;
                for (int k = i; k < j; ++k)
                    *resultData++ = QLatin1Char(str.at(k));
            }
            i = j;
        }
        }
    }
    if (!currentValueIsQuoted)
        iniChopTrailingSpaces(stringResult, chopLimit);
    goto end;

StHexEscape:
    if (i >= to) {
        stringResult += QChar(escapeVal);
        goto end;
    }

    ch = str.at(i);
    if (ch >= 'a')
        ch -= 'a' - 'A';
    if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
        escapeVal <<= 4;
        escapeVal += strchr(hexDigits, ch) - hexDigits;
        ++i;
        goto StHexEscape;
    } else {
        stringResult += QChar(escapeVal);
        goto StNormal;
    }

StOctEscape:
    if (i >= to) {
        stringResult += QChar(escapeVal);
        goto end;
    }

    ch = str.at(i);
    if (ch >= '0' && ch <= '7') {
        escapeVal <<= 3;
        escapeVal += ch - '0';
        ++i;
        goto StOctEscape;
    } else {
        stringResult += QChar(escapeVal);
        goto StNormal;
    }

end:
    if (isStringList)
        stringListResult.append(stringResult);
    return isStringList;
}

bool QSettingsUtf8Private::ReadFunc(QIODevice& device, QSettings::SettingsMap& map)
{
    QString currentSection;
    QTextStream stream(&device);
    stream.setCodec("UTF-8");
    QString data;
    bool ok = true;

    while (!stream.atEnd())
    {
        data = stream.readLine();
        if (data.trimmed().isEmpty())	continue;

        if (data[0] == QChar('['))
        {
            // INI档 的 Section 部分
            QString iniSection;
            int inx = data.indexOf(QChar(']'));
            if (inx == -1)
            {
                // 沒有对应的 ']' 结尾，但仍然继续处理下去
                ok = false;
                iniSection = data.mid(1);
            }
            else
            {
                iniSection = data.mid(1, inx - 1);
            }

            iniSection = iniSection.trimmed();
            if (iniSection.compare(QString("general"), Qt::CaseInsensitive) == 0)
            {
                // 如果是 [general] 表示這是 Qt 内建 section, 暂存于 INI 档里，
                // 读取后，要复原成 '空' 的 section
                currentSection.clear();
            }
            else
            {
                if (iniSection.compare(QString("%general"), Qt::CaseInsensitive) == 0)
                {
                    // 为了避免跟 Qt 内建的 [general] section，起冲突；
                    // 若是 [%general] 表示这是 原本在 INI 档里的 [general]
                    // 所以，要将复原成 [general] section
                    currentSection = QString("general");
                }
                else
                {
                    currentSection = iniSection;
                }
                currentSection += QChar('/');
            }
        }
        else
        {
            // INI檔 的 Key = Value 部分
            bool inQuotes = false;
            int equalsPos = -1;
            QList<int> commaPos;
            int i = 0;
            while (i < data.size())
            {
                QChar ch = data.at(i);
                if (ch == QChar('='))
                {
                    if (!inQuotes && equalsPos == -1)
                    {
                        equalsPos = i;
                    }
                }
                else if (ch == QChar('"'))
                {
                    inQuotes = !inQuotes;
                }
                else if (ch == QChar(','))
                {
                    if (!inQuotes && equalsPos != -1)
                    {
                        commaPos.append(i);
                    }
                }
                else if (ch == QChar(';') || ch == QChar('#'))
                {
                    if (!inQuotes)
                    {
                        data.resize(i);
                        break;
                    }
                }
                else if (ch == QChar('\\'))
                {
                    if (++i < data.size())
                    {
                    }
                    else
                    {
                        ok = false;
                        break;
                    }
                }
                i++;
            }

            if (equalsPos == -1)
            {
                ok = false;
                break;
            }
            else
            {
                QString key = data.mid(0, equalsPos).trimmed();
                if (key.isEmpty())
                {
                    ok = false;
                    break;
                }
                else
                {
                    key = currentSection + key;
                }

                QStringList strListValue;
                QString strValue;
                QByteArray baData = data.toUtf8();
                int lineLen = baData.size();
                equalsPos = baData.indexOf(' ');

                QTextCodec *iniCodec = QTextCodec::codecForName("UTF-8");
                strValue.reserve(lineLen - (equalsPos+2));
                bool isStringList = iniUnescapedStringList(baData, equalsPos+2, lineLen,
                                                           strValue, strListValue, iniCodec);

                QVariant variant;
                if (isStringList) {
                    variant = stringListToVariantList(strListValue);
                } else {
                    variant = stringToVariant(strValue);
                }
                map[key] = variant;
            }
        }
    }

    return ok;
}


bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
    return s1.left(s1.lastIndexOf('/')) < s2.left(s2.lastIndexOf('/'));
}

bool QSettingsUtf8Private::WriteFunc(QIODevice& device, const QSettings::SettingsMap& map)
{
#ifdef Q_OS_WIN
    const char * const eol = "\r\n";
#else
    const char eol = '\n';
#endif
    bool ok = true;

    QString lastSection;

    QList<QString> listKeys = map.keys();
    qSort(listKeys.begin(), listKeys.end(), [](const QString &s1, const QString &s2)->bool
    {
        return s1.left(s1.lastIndexOf('/')) < s2.left(s2.lastIndexOf('/'));
    });

    foreach (const QString &key_, listKeys) {
		QString key = key_;
        QString section;

        int idx = key.lastIndexOf(QChar('/'));
        if (idx == -1)
        {
            section = QString("[General]");
        }
        else
        {
            section = key.left(idx);
            key = key.mid(idx+1);
            if (section.compare(QString("General"), Qt::CaseInsensitive) == 0)
            {
                section = QString("[%General]");
            }
            else
            {
                section.prepend(QChar('['));
                section.append(QChar(']'));
            }
        }

        if (section.compare(lastSection, Qt::CaseInsensitive))
        {
            if (!lastSection.isEmpty())
            {
                device.write(eol);
            }
            lastSection = section;
            if (device.write(section.toUtf8() + eol) == -1)
            {
                ok = false;
            }
        }

        QByteArray block = key.toUtf8();
        block += " = ";

        //block += variantToString(it.value());
        const QVariant &value = map[key_];

        QTextCodec *iniCodec = QTextCodec::codecForName("UTF-8");
        if (value.type() == QVariant::StringList
                || (value.type() == QVariant::List && value.toList().size() != 1)) {
            iniEscapedStringList(variantListToStringList(value.toList()), block, iniCodec);
        } else {
            iniEscapedString(variantToString(value), block, iniCodec);
        }

        block += eol;
        if (device.write(block) == -1)
        {
            ok = false;
        }
    }

    return ok;
}
