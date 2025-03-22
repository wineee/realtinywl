// Copyright (C) 2024 JiDe Zhang <zhangjide@deepin.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "output.h"
#include "helper.h"
#include "rootsurfacecontainer.h"

#include <woutputitem.h>
#include <woutputrenderwindow.h>
#include <woutputlayout.h>
#include <wquicktextureproxy.h>

#include <qwoutputlayout.h>

#include <QQmlEngine>

Output *Output::createPrimary(WOutput *output, QQmlEngine *engine, QObject *parent)
{
    QQmlComponent delegate(engine, "WayGreet", "PrimaryOutput");
    QObject *obj = delegate.beginCreate(engine->rootContext());
    delegate.setInitialProperties(obj, {
        {"forceSoftwareCursor", output->handle()->is_x11()}
    });
    delegate.completeCreate();
    WOutputItem *outputItem = qobject_cast<WOutputItem *>(obj);
    Q_ASSERT(outputItem);
    QQmlEngine::setObjectOwnership(outputItem, QQmlEngine::CppOwnership);
    outputItem->setOutput(output);

    auto o = new Output(outputItem, parent);
    o->m_type = Type::Primary;
    obj->setParent(o);

    auto contentItem = Helper::instance()->window()->contentItem();
    outputItem->setParentItem(contentItem);

    //o->m_menuBar = Helper::instance()->qmlEngine()->createMenuBar(outputItem, contentItem);
    //o->m_menuBar->setZ(RootSurfaceContainer::MenuBarZOrder);

    return o;
}

Output *Output::createCopy(WOutput *output, Output *proxy, QQmlEngine *engine, QObject *parent)
{
    QQmlComponent delegate(engine, "WayGreet", "CopyOutput");
    QObject *obj = delegate.createWithInitialProperties({
                                                         {"targetOutputItem", QVariant::fromValue(proxy->outputItem())},
                                                         }, engine->rootContext());

    WOutputItem *outputItem = qobject_cast<WOutputItem *>(obj);
    Q_ASSERT(outputItem);
    QQmlEngine::setObjectOwnership(outputItem, QQmlEngine::CppOwnership);
    outputItem->setOutput(output);

    auto o = new Output(outputItem, parent);
    o->m_type = Type::Proxy;
    o->m_proxy = proxy;
    obj->setParent(o);

    auto contentItem = Helper::instance()->window()->contentItem();
    outputItem->setParentItem(contentItem);
    o->updatePrimaryOutputHardwareLayers();
    connect(o->m_outputViewport, &WOutputViewport::hardwareLayersChanged,
            o, &Output::updatePrimaryOutputHardwareLayers);

    return o;
}

Output::Output(WOutputItem *output, QObject *parent)
    : QObject(parent)
    , m_item(output)
{
    m_outputViewport = output->property("screenViewport").value<WOutputViewport *>();
}

Output::~Output()
{
    if (m_menuBar) {
        delete m_menuBar;
        m_menuBar = nullptr;
    }

    if (m_item) {
        delete m_item;
        m_item = nullptr;
    }
}

bool Output::isPrimary() const
{
    return m_type == Type::Primary;
}

WOutput *Output::output() const
{
    auto o = m_item->output();
    Q_ASSERT(o);
    return o;
}

WOutputItem *Output::outputItem() const
{
    return m_item;
}

void Output::updatePositionFromLayout()
{
    WOutputLayout * layout = output()->layout();
    Q_ASSERT(layout);

    auto *layoutOutput = layout->handle()->get(output()->nativeHandle());
    QPointF pos(layoutOutput->x, layoutOutput->y);
    m_item->setPosition(pos);
}

std::pair<WOutputViewport*, QQuickItem*> Output::getOutputItemProperty()
{
    WOutputViewport *viewportCopy = outputItem()->findChild<WOutputViewport*>({}, Qt::FindDirectChildrenOnly);
    Q_ASSERT(viewportCopy);
    auto textureProxy = outputItem()->findChild<WQuickTextureProxy*>();
    Q_ASSERT(textureProxy);

    return std::make_pair(viewportCopy, textureProxy);
}

void Output::updatePrimaryOutputHardwareLayers()
{
    WOutputViewport *viewportPrimary = screenViewport();
    std::pair<WOutputViewport*, QQuickItem*> copyOutput = getOutputItemProperty();
    const auto layers = viewportPrimary->hardwareLayers();
    for (auto layer : layers) {
        if (m_hardwareLayersOfPrimaryOutput.removeOne(layer))
            continue;
        Helper::instance()->window()->attach(layer, copyOutput.first, viewportPrimary, copyOutput.second);
    }

    for (auto oldLayer : std::as_const(m_hardwareLayersOfPrimaryOutput)) {
        Helper::instance()->window()->detach(oldLayer, copyOutput.first);
    }

    m_hardwareLayersOfPrimaryOutput = layers;
}

QRectF Output::rect() const
{
    return QRectF(QPointF(0, 0), m_item->size());
}

QRectF Output::geometry() const
{
    return QRectF(m_item->position(), m_item->size());
}

WOutputViewport *Output::screenViewport() const
{
    return m_outputViewport;
}
