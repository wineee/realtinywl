/***************************************************************************
* Copyright (c) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#pragma once

#include <QDataStream>
#include <QDir>
#include <QSharedPointer>
#include <QProcessEnvironment>

class WSessionModel;

class WSession {
public:
    enum Type {
        UnknownSession = 0,
        X11Session,
        WaylandSession
    };

    explicit WSession();
    WSession(Type type, const QString &fileName);

    bool isValid() const;

    Type type() const;

    int vt() const;
    void setVt(int vt);

    QString xdgSessionType() const;

    QDir directory() const;
    QString fileName() const;

    QString displayName() const;
    QString comment() const;

    QString exec() const;
    QString tryExec() const;

    QString desktopSession() const;
    QString desktopNames() const;

    bool isHidden() const;
    bool isNoDisplay() const;

    QProcessEnvironment additionalEnv() const;

    void setTo(Type type, const QString &name);

    WSession &operator=(const WSession &other);

private:
    QProcessEnvironment parseEnv(const QString &list);
    bool m_valid;
    Type m_type;
    int m_vt = 0;
    QDir m_dir;
    QString m_name;
    QString m_fileName;
    QString m_displayName;
    QString m_comment;
    QString m_exec;
    QString m_tryExec;
    QString m_xdgSessionType;
    QString m_desktopNames;
    QProcessEnvironment m_additionalEnv;
    bool m_isHidden;
    bool m_isNoDisplay;

    friend class WSessionModel;
};

inline QDataStream &operator<<(QDataStream &stream, const WSession &WSession) {
    stream << quint32(WSession.type()) << WSession.fileName();
    return stream;
}

inline QDataStream &operator>>(QDataStream &stream, WSession &WSession) {
    quint32 type;
    QString fileName;
    stream >> type >> fileName;
    WSession.setTo(static_cast<WSession::Type>(type), fileName);
    return stream;
}
