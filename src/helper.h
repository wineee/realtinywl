// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "qmlengine.h"
#include "backend.h"

#include <wglobal.h>
#include <wqmlcreator.h>
#include <wseat.h>

#include <QObject>
#include <QQmlApplicationEngine>

QT_BEGIN_NAMESPACE
class QQuickItem;
QT_END_NAMESPACE

WAYLIB_SERVER_BEGIN_NAMESPACE
class WServer;
class WOutputRenderWindow;
class WOutputLayout;
class WCursor;
class WBackend;
class WOutputItem;
class WOutputViewport;
class WOutputLayer;
class WOutput;
class WInputMethodHelper;
class WSocket;
class WSurface;
class WToplevelSurface;
class WSurfaceItem;
WAYLIB_SERVER_END_NAMESPACE

QW_BEGIN_NAMESPACE
class qw_renderer;
class qw_allocator;
class qw_compositor;
QW_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE
QW_USE_NAMESPACE

class Output;
class SurfaceWrapper;
class RootSurfaceContainer;
class LayerSurfaceContainer;
class Helper : public WSeatEventFilter
{
    friend class RootSurfaceContainer;
    Q_OBJECT
    Q_PROPERTY(RootSurfaceContainer* rootContainer READ rootContainer CONSTANT FINAL)
    Q_PROPERTY(OutputMode outputMode READ outputMode WRITE setOutputMode NOTIFY outputModeChanged FINAL)
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Helper(QObject *parent = nullptr);
    ~Helper();

    enum class OutputMode {
        Copy,
        Extension
    };
    Q_ENUM(OutputMode)

    static Helper *instance();

    QmlEngine *qmlEngine() const;
    WOutputRenderWindow *window() const;
    Output* output() const;
    void init();

    RootSurfaceContainer *rootContainer() const;
    Output *getOutput(WOutput *output) const;

    OutputMode outputMode() const;
    void setOutputMode(OutputMode mode);

    Q_INVOKABLE void addOutput();

signals:
    void socketEnabledChanged();
    void keyboardFocusSurfaceChanged();
    void activatedSurfaceChanged();
    void primaryOutputChanged();
    void currentUserIdChanged();

    void animationSpeedChanged();
    void outputModeChanged();

private:
    void allowNonDrmOutputAutoChangeMode(WOutput *output);
    void enableOutput(WOutput *output);

    int indexOfOutput(WOutput *output) const;

    void setOutputProxy(Output *output);

    void setCursorPosition(const QPointF &position);

    bool beforeDisposeEvent(WSeat *seat, QWindow *watched, QInputEvent *event) override;
    bool afterHandleEvent(WSeat *seat, WSurface *watched, QObject *surfaceItem, QObject *, QInputEvent *event) override;

    static Helper *m_instance;

    // Greeter Backends
    Backend *m_greetd = nullptr;

    // qtquick helper
    WOutputRenderWindow *m_renderWindow = nullptr;

    // wayland helper
    WServer *m_server = nullptr;
    WSocket *m_socket = nullptr;
    WSeat *m_seat = nullptr;
    WBackend *m_backend = nullptr;
    qw_renderer *m_renderer = nullptr;
    qw_allocator *m_allocator = nullptr;

    // protocols
    qw_compositor *m_compositor = nullptr;

    // privaet data
    QList<Output*> m_outputList;

    RootSurfaceContainer *m_surfaceContainer = nullptr;
    int m_currentUserId = -1;
    OutputMode m_mode = OutputMode::Extension;
};

Q_DECLARE_OPAQUE_POINTER(RootSurfaceContainer*)
