// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#pragma once

#include <wglobal.h>
#include <QMargins>
#include <QObject>
#include <QQmlComponent>
#include <woutputviewport.h>

Q_MOC_INCLUDE(<woutputitem.h>)

WAYLIB_SERVER_BEGIN_NAMESPACE
class WOutput;
class WOutputItem;
class WOutputViewport;
class WOutputLayout;
class WOutputLayer;
class WQuickTextureProxy;
class WSeat;
WAYLIB_SERVER_END_NAMESPACE

WAYLIB_SERVER_USE_NAMESPACE

class Output : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(WOutputItem* outputItem MEMBER m_item CONSTANT)
    Q_PROPERTY(WOutputViewport* screenViewport MEMBER m_outputViewport CONSTANT)

public:
    enum class Type {
        Primary,
        Proxy
    };

    static Output *createPrimary(WOutput *output, QQmlEngine *engine, QObject *parent = nullptr);
    static Output *createCopy(WOutput *output, Output *proxy, QQmlEngine *engine, QObject *parent = nullptr);

    explicit Output(WOutputItem *output, QObject *parent = nullptr);
    ~Output();

    bool isPrimary() const;

    WOutput *output() const;
    WOutputItem *outputItem() const;

    QRectF rect() const;
    QRectF geometry() const;
    WOutputViewport *screenViewport() const;
    void updatePositionFromLayout();

signals:
    void exclusiveZoneChanged();
    void moveResizeFinised();

public Q_SLOTS:
    void updatePrimaryOutputHardwareLayers();

private:
    std::pair<WOutputViewport*, QQuickItem*> getOutputItemProperty();

    Type m_type;
    WOutputItem *m_item;
    Output *m_proxy = nullptr;
    QPointer<QQuickItem> m_menuBar;
    WOutputViewport *m_outputViewport;

    QList<WOutputLayer *> m_hardwareLayersOfPrimaryOutput;
};

Q_DECLARE_OPAQUE_POINTER(WAYLIB_SERVER_NAMESPACE::WOutputItem*)
