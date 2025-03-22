/***************************************************************************
 * Copyright (c) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (c) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ***************************************************************************/

#include "sessiontype.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QSettings>
#include <QLocale>
#include <QRegularExpression>
#include <QtGlobal>
#include <QtCore/QtGlobal>
#include <QtCore/QStringView>

const QString s_entryExtention = QStringLiteral(".desktop");

// QSettings::IniFormat can't be used to read .desktop files due to different
// syntax of values (escape sequences, quoting, automatic QStringList detection).
// So implement yet another .desktop file parser.
class DesktopFileFormat {
    static bool readFunc(QIODevice &device, QSettings::SettingsMap &map)
    {
        QString filename = QStringLiteral("(unknown)");
        if(QFile *file = qobject_cast<QFile*>(&device); file)
            filename = file->fileName();

        QString currentSectionName;
        for(int lineNumber = 1; !device.atEnd(); lineNumber++)
        {
            // Iterate each line, remove line terminators
            const auto line = device.readLine().replace("\r", "").replace("\n", "");
            if(line.isEmpty() || line.startsWith('#'))
                continue; // Ignore empty lines and comments

            if(line.startsWith('[')) // Section header
            {
                const int endOfHeader = line.lastIndexOf(']');
                if(endOfHeader < 0)
                {
                    qWarning() << QStringLiteral("%1:%2: Invalid section header").arg(filename).arg(lineNumber);
                    return false;
                }
                if(endOfHeader != line.length() - 1)
                    qWarning() << QStringLiteral("%1:%2: Section header does not end line with ]").arg(filename).arg(lineNumber);

                currentSectionName = QString::fromUtf8(line.mid(1, endOfHeader - 1));
            }
            else if(int equalsPos = line.indexOf('='); equalsPos > 0) // Key=Value
            {
                const auto key = QString::fromUtf8(line.left(equalsPos));

                       // Read the value, handle escape sequences
                auto valueBytes = line.mid(equalsPos + 1);
                valueBytes.replace("\\s", " ").replace("\\n", "\n");
                valueBytes.replace("\\t", "\t").replace("\\r", "\r");
                valueBytes.replace("\\\\", "\\");

                auto value = QString::fromUtf8(valueBytes);
                map.insert(currentSectionName + QLatin1Char('/') + key, value);
            }
        }

        return true;
    }
public:
    // Register the .desktop file format if necessary, return its id.
    static QSettings::Format format()
    {
        static QSettings::Format s_format = QSettings::InvalidFormat;
        if (s_format == QSettings::InvalidFormat)
            s_format = QSettings::registerFormat(QStringLiteral("desktop"),
                                                 DesktopFileFormat::readFunc, nullptr,
                                                 Qt::CaseSensitive);

        return s_format;
    }
};

WSession::WSession()
    : m_valid(false)
    , m_type(UnknownSession)
    , m_isHidden(false)
    , m_isNoDisplay(false)
{
}

WSession::WSession(Type type, const QString &fileName)
    : WSession()
{
    setTo(type, fileName);
}

bool WSession::isValid() const
{
    return m_valid;
}

WSession::Type WSession::type() const
{
    return m_type;
}

int WSession::vt() const
{
    return m_vt;
}

void WSession::setVt(int vt)
{
    m_vt = vt;
}

QString WSession::xdgSessionType() const
{
    return m_xdgSessionType;
}

QDir WSession::directory() const
{
    return m_dir;
}

QString WSession::fileName() const
{
    return m_fileName;
}

QString WSession::displayName() const
{
    return m_displayName;
}

QString WSession::comment() const
{
    return m_comment;
}

QString WSession::exec() const
{
    return m_exec;
}

QString WSession::tryExec() const
{
    return m_tryExec;
}

QString WSession::desktopSession() const
{
    return QFileInfo(m_fileName).completeBaseName();
}

QString WSession::desktopNames() const
{
    return m_desktopNames;
}

bool WSession::isHidden() const
{
    return m_isHidden;
}

bool WSession::isNoDisplay() const
{
    return m_isNoDisplay;
}

QProcessEnvironment WSession::additionalEnv() const {
    return m_additionalEnv;
}

void WSession::setTo(Type type, const QString &_fileName)
{
    QString fileName(_fileName);
    if (!fileName.endsWith(s_entryExtention))
        fileName += s_entryExtention;

    QFileInfo info(fileName);

    m_type = UnknownSession;
    m_valid = false;
    m_desktopNames.clear();

    QStringList SessionDirs;

    SessionDirs << "/usr/share/wayland-sessions" << "/nix/store/7yn8psywvnl2b54ny1s9sjfkw0barxg2-desktops/share/wayland-sessions";

    switch (type) {
    case WaylandSession:
        //SessionDirs = mainConfig.Wayland.WSessionDir.get();
        m_xdgSessionType = QStringLiteral("wayland");
        break;
    case X11Session:
        //SessionDirs = mainConfig.X11.WSessionDir.get();
        m_xdgSessionType = QStringLiteral("x11");
        break;
    default:
        m_xdgSessionType.clear();
        break;
    }

    QFile file;
    for (const auto &path: std::as_const(SessionDirs)) {
        m_dir.setPath(path);
        m_fileName = m_dir.absoluteFilePath(fileName);

        qDebug() << "Reading from" << m_fileName;

        file.setFileName(m_fileName);
        if (file.open(QIODevice::ReadOnly))
            break;
    }
    if (!file.isOpen())
        return;

    QSettings settings(m_fileName, DesktopFileFormat::format());
    QStringList locales = { QLocale().name() };
    if (auto clean = QLocale().name().remove(QRegularExpression(QLatin1String("_.*"))); clean != locales.constFirst()) {
        locales << clean;
    }

    if (settings.status() != QSettings::NoError)
        return;

    settings.beginGroup(QLatin1String("Desktop Entry"));

    auto localizedValue = [&] (const QLatin1String &key) {
        for (QString locale : std::as_const(locales)) {
            QString localizedValue = settings.value(key + QLatin1Char('[') + locale + QLatin1Char(']'), QString()).toString();
            if (!localizedValue.isEmpty()) {
                return localizedValue;
            }
        }
        return settings.value(key).toString();
    };

    m_displayName = localizedValue(QLatin1String("Name"));
    m_comment = localizedValue(QLatin1String("Comment"));
    m_exec = settings.value(QLatin1String("Exec"), QString()).toString();
    m_tryExec = settings.value(QLatin1String("TryExec"), QString()).toString();
    m_desktopNames = settings.value(QLatin1String("DesktopNames"), QString()).toString().replace(QLatin1Char(';'), QLatin1Char(':'));
    QString hidden = settings.value(QLatin1String("Hidden"), QString()).toString();
    m_isHidden = hidden.toLower() == QLatin1String("true");
    QString noDisplay = settings.value(QLatin1String("NoDisplay"), QString()).toString();
    m_isNoDisplay = noDisplay.toLower() == QLatin1String("true");
    QString additionalEnv = settings.value(QLatin1String("X-SDDM-Env"), QString()).toString();
    m_additionalEnv = parseEnv(additionalEnv);
    settings.endGroup();

    m_type = type;
    m_valid = true;
}

WSession &WSession::operator=(const WSession &other)
{
    setTo(other.type(), other.fileName());
    return *this;
}

QProcessEnvironment WSession::parseEnv(const QString &list)
{
    QProcessEnvironment env;
    const auto entryList = QStringView{list}.split(u',', Qt::SkipEmptyParts);
    for (const auto &entry: entryList) {
        int midPoint = entry.indexOf(QLatin1Char('='));
        if (midPoint < 0) {
            qWarning() << "Malformed entry in" << fileName() << ":" << entry;
            continue;
        }
        env.insert(entry.left(midPoint).toString(), entry.mid(midPoint+1).toString());
    }
    return env;
}


