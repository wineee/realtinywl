// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "workspace.h"
#include "surfacewrapper.h"
#include "output.h"
#include "helper.h"
#include "rootsurfacecontainer.h"

Workspace::Workspace(SurfaceContainer *parent)
    : SurfaceContainer(parent)
{
    // TODO: save and restore from local storage
    static int workspaceGlobalIndex = 0;

    // TODO: save and restore workpsace's name from local storage
    createContainer(QStringLiteral("show-on-all-workspace"), true);
    createContainer(QStringLiteral("workspace-%1").arg(++workspaceGlobalIndex), true);
}

void Workspace::addSurface(SurfaceWrapper *surface, int workspaceIndex)
{
    doAddSurface(surface, true);

    if (workspaceIndex < 0)
        workspaceIndex = m_currentIndex;

    auto container = m_models.at(workspaceIndex);

    if (container->hasSurface(surface))
        return;

    for (auto c : std::as_const(m_models)) {
        if (c == container)
            continue;
        if (c->surfaces().contains(surface)) {
            c->removeSurface(surface);
            break;
        }
    }

    container->addSurface(surface);
    if (!surface->ownsOutput())
        surface->setOwnsOutput(rootContainer()->primaryOutput());
}

void Workspace::removeSurface(SurfaceWrapper *surface)
{
    if (!doRemoveSurface(surface, true))
        return;

    for (auto container : std::as_const(m_models)) {
        if (container->surfaces().contains(surface)) {
            container->removeSurface(surface);
            break;
        }
    }
}

int Workspace::containerIndexOfSurface(SurfaceWrapper *surface) const
{
    for (int i = 0; i < m_models.size(); ++i) {
        if (m_models.at(i)->hasSurface(surface))
            return i;
    }

    return -1;
}

int Workspace::createContainer(const QString &name, bool visible)
{
    m_models.append(new WorkspaceModel(this, m_models.size()));
    auto newContainer = m_models.last();
    newContainer->setName(name);
    newContainer->setVisible(visible);
    return newContainer->index();
}

WorkspaceModel *Workspace::container(int index) const
{
    if (index < 0 || index >= m_models.size())
        return nullptr;
    return m_models.at(index);
}

int Workspace::count() const
{
    return m_models.size();
}

int Workspace::currentIndex() const
{
    return m_currentIndex;
}

WorkspaceModel *Workspace::current() const
{
    if (m_currentIndex <= 0 || m_currentIndex >= m_models.size())
        return nullptr;

    return m_models.at(m_currentIndex);
}

void Workspace::updateSurfaceOwnsOutput(SurfaceWrapper *surface)
{
    auto outputs = surface->surface()->outputs();
    if (surface->ownsOutput() && outputs.contains(surface->ownsOutput()->output()))
        return;

    Output *output = nullptr;
    if (!outputs.isEmpty())
        output = Helper::instance()->getOutput(outputs.first());
    if (!output)
        output = rootContainer()->cursorOutput();
    if (!output)
        output = rootContainer()->primaryOutput();
    if (output)
        surface->setOwnsOutput(output);
}

void Workspace::updateSurfacesOwnsOutput()
{
    const auto surfaces = this->surfaces();
    for (auto surface : surfaces) {
        updateSurfaceOwnsOutput(surface);
    }
}

