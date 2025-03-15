// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#pragma once

#include "surfacecontainer.h"
#include "workspacemodel.h"

class SurfaceWrapper;
class Workspace;

class Workspace : public SurfaceContainer
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit Workspace(SurfaceContainer *parent);

    Q_INVOKABLE void addSurface(SurfaceWrapper *surface, int workspaceIndex = -1);
    void removeSurface(SurfaceWrapper *surface) override;
    int containerIndexOfSurface(SurfaceWrapper *surface) const;

    Q_INVOKABLE int createContainer(const QString &name, bool visible = false);
    WorkspaceModel *container(int index) const;

    int count() const;
    int currentIndex() const;

    WorkspaceModel *current() const;
    void setCurrent(WorkspaceModel *container);

signals:
    void countChanged();

private:
    void updateSurfaceOwnsOutput(SurfaceWrapper *surface);
    void updateSurfacesOwnsOutput();

    // Workspace id starts from 1, the WorkspaceModel with id 0 is used to
    // store the surface that is always in the visible workspace.
    int m_currentIndex = 1;
    QList<WorkspaceModel*> m_models;
    QPointer<QQuickItem> m_switcher;
};
