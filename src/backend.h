#pragma once

#include <QUrl>
#include <QObject>

class Ipc;
class Session;

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl iconsSrc READ iconsSrc CONSTANT)
    Q_PROPERTY(QUrl backgroundSrc READ backgroundSrc CONSTANT)
    Q_PROPERTY(bool sessionInProgress READ sessionInProgress NOTIFY sessionInProgressChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    QUrl iconsSrc() const;
    void setIconsSrc(const QUrl &url);

    QUrl backgroundSrc() const;
    void setBackgroundSrc(const QUrl &url);

    bool sessionInProgress() const;

    void setCommand(const QString &command);

    Q_INVOKABLE bool login(const QString &user, const QString &password);

    static Backend *instance();

Q_SIGNALS:
    void userChanged();
    void sessionInProgressChanged();

    void sessionSuccess();
    void sessionError(const QString &type, const QString &description);
    void infoMessage(const QString &message);
    void errorMessage(const QString &message);

private:
    QUrl m_iconsSrc;
    QUrl m_backgroundSrc;
    QString m_command;
    Ipc *m_ipc = nullptr;
    Session *m_session = nullptr;
};
