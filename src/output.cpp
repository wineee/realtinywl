// Copyright (C) 2024 JiDe Zhang <zhangjide@deepin.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "output.h"
#include "surfacewrapper.h"
#include "helper.h"
#include "rootsurfacecontainer.h"

#include <woutputitem.h>
#include <wsurfaceitem.h>
#include <woutputrenderwindow.h>
#include <wxdgsurface.h>
#include <wxdgpopupsurface.h>
#include <winputpopupsurface.h>
#include <woutputlayout.h>
#include <wquicktextureproxy.h>
#include <wxdgpopupsurfaceitem.h>

#include <qwoutputlayout.h>

#include <QQmlEngine>

Q_LOGGING_CATEGORY(qLcLayerShell, "WayGreet.shell.layer", QtWarningMsg)

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

    o->minimizedSurfaces->setFilter([] (SurfaceWrapper *s) {
        return s->isMinimized();
    });

    // Triggering layout updates using a queue helps reduce window jitter.
    // When the screen scaling factor changes, the scale of WOutput is updated first,
    // causing the size of WOutputItem to change. However, at this point, the
    // effectiveDevicePixelRatio of QWindow has not yet been updated.
    // This results in the size of maximized windows being updated prematurely.
    // Xwayland windows use the effectiveDevicePixelRatio to set the surfaceSizeRatio.
    // By updating within a queue, it ensures that the surfaceSizeRatio used when
    // resizing maximized Xwayland windows is accurate, avoiding multiple rapid
    // size changes to Xwayland windows in a short period.
    o->connect(outputItem, &WOutputItem::geometryChanged,
               o, &Output::layoutAllSurfaces, Qt::QueuedConnection);

    auto contentItem = Helper::instance()->window()->contentItem();
    outputItem->setParentItem(contentItem);

    o->m_menuBar = Helper::instance()->qmlEngine()->createMenuBar(outputItem, contentItem);
    o->m_menuBar->setZ(RootSurfaceContainer::MenuBarZOrder);

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
    : SurfaceListModel(parent)
    , m_item(output)
    , minimizedSurfaces(new SurfaceFilterModel(this))
{
    m_outputViewport = output->property("screenViewport").value<WOutputViewport *>();
}

Output::~Output()
{
    if (m_taskBar) {
        delete m_taskBar;
        m_taskBar = nullptr;
    }

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

void Output::addSurface(SurfaceWrapper *surface)
{
    Q_ASSERT(!hasSurface(surface));
    SurfaceListModel::addSurface(surface);


    auto layoutSurface = [surface, this] {
        layoutNonLayerSurface(surface, {});
    };

    connect(surface, &SurfaceWrapper::widthChanged, this, layoutSurface);
    connect(surface, &SurfaceWrapper::heightChanged, this, layoutSurface);
    layoutSurface();
    if (surface->type() == SurfaceWrapper::Type::XdgPopup) {
        auto xdgPopupSurfaceItem = qobject_cast<WXdgPopupSurfaceItem *>(surface->surfaceItem());
        connect(xdgPopupSurfaceItem, &WXdgPopupSurfaceItem::implicitPositionChanged, this, [surface, this] {
            // Reposition should ignore positionAutomatic
            layoutPopupSurface(surface);
        });
    }
}

void Output::removeSurface(SurfaceWrapper *surface)
{
    Q_ASSERT(hasSurface(surface));
    SurfaceListModel::removeSurface(surface);
    surface->disconnect(this);
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

void Output::layoutNonLayerSurface(SurfaceWrapper *surface, const QSizeF &sizeDiff)
{
    Q_ASSERT(surface->type() != SurfaceWrapper::Type::Layer);
    surface->setFullscreenGeometry(geometry());
    const auto validGeo = this->geometry();
    surface->setMaximizedGeometry(validGeo);

    QRectF normalGeo = surface->normalGeometry();
    do {
        if (surface->positionAutomatic()) {
            if (normalGeo.isEmpty())
                return;

            SurfaceWrapper* parentSurfaceWrapper = surface->parentSurface();
            if (parentSurfaceWrapper) {
                if (surface->type() == SurfaceWrapper::Type::XdgPopup
                    || surface->type() == SurfaceWrapper::Type::InputPopup) {
                    layoutPopupSurface(surface);
                    return;
                }
                QPointF dPos {
                    (parentSurfaceWrapper->width() - surface->width()) / 2,
                    (parentSurfaceWrapper->height() - surface->height()) / 2
                };
                QPointF topLeft;
                topLeft.setX(parentSurfaceWrapper->x() + dPos.x());
                topLeft.setY(parentSurfaceWrapper->y() + dPos.y());
                normalGeo.moveTopLeft(topLeft);
                surface->moveNormalGeometryInOutput(normalGeo.topLeft());
            } else {
                normalGeo.moveCenter(validGeo.center());
                normalGeo.moveTop(qMax(normalGeo.top(), validGeo.top()));
                normalGeo.moveLeft(qMax(normalGeo.left(), validGeo.left()));
                surface->moveNormalGeometryInOutput(normalGeo.topLeft());
            }
        } else if (!sizeDiff.isNull() && sizeDiff.isValid()) {
            const QSizeF outputSize = m_item->size();
            const auto xScale = outputSize.width() / (outputSize.width() - sizeDiff.width());
            const auto yScale = outputSize.height() / (outputSize.height() - sizeDiff.height());
            normalGeo.moveLeft(normalGeo.x() * xScale);
            normalGeo.moveTop(normalGeo.y() * yScale);
            surface->moveNormalGeometryInOutput(normalGeo.topLeft());
        } else {
            break;
        }
    } while (false);
}

void Output::layoutPopupSurface(SurfaceWrapper *surface)
{
    SurfaceWrapper *parentSurfaceWrapper = surface->parentSurface();
    Q_ASSERT(parentSurfaceWrapper);

    QRectF normalGeo = surface->normalGeometry();
    if (normalGeo.isEmpty())
        return;

    auto xdgPopupSurfaceItem = qobject_cast<WXdgPopupSurfaceItem *>(surface->surfaceItem());
    auto inputPopupSurface = qobject_cast<WInputPopupSurface *>(surface->shellSurface());

    QPointF dPos = xdgPopupSurfaceItem ? xdgPopupSurfaceItem->implicitPosition()
                                   : inputPopupSurface->cursorRect().bottomLeft();
    QPointF topLeft;
    // TODO: remove parentSurfaceWrapper->surfaceItem()->x()
    topLeft.setX(parentSurfaceWrapper->x() + parentSurfaceWrapper->surfaceItem()->x() + dPos.x());
    topLeft.setY(parentSurfaceWrapper->y() + parentSurfaceWrapper->surfaceItem()->y() + dPos.y()
                 + parentSurfaceWrapper->titlebarGeometry().height());
    auto output = surface->ownsOutput()->outputItem();

    normalGeo.setWidth(std::min(output->width(), surface->width()));
    normalGeo.setHeight(std::min(output->height(), surface->height()));
    surface->setSize(normalGeo.size());

    if (topLeft.x() + normalGeo.width() > output->x() + output->width())
        topLeft.setX(output->x() + output->width() - normalGeo.width());
    if (topLeft.y() + normalGeo.height() > output->y() + output->height()) {
        if (xdgPopupSurfaceItem)
            topLeft.setY(output->y() + output->height() - normalGeo.height());
        else // input popup
            topLeft.setY(topLeft.y() - inputPopupSurface->cursorRect().height()
                         - normalGeo.height());
    }
    normalGeo.moveTopLeft(topLeft);
    surface->moveNormalGeometryInOutput(normalGeo.topLeft());
}

void Output::layoutNonLayerSurfaces()
{
    const auto currentSize = geometry().size();
    const auto sizeDiff = m_lastSizeOnLayoutNonLayerSurfaces.isValid()
                              ? currentSize - m_lastSizeOnLayoutNonLayerSurfaces
                              : QSizeF(0, 0);
    m_lastSizeOnLayoutNonLayerSurfaces = currentSize;

    for (SurfaceWrapper *surface : surfaces()) {
        layoutNonLayerSurface(surface, sizeDiff);
    }
}

void Output::layoutAllSurfaces()
{
    layoutNonLayerSurfaces();
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
