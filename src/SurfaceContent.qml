// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import Waylib.Server
import WayGreet

Item {
    id: root

    required property SurfaceItem surface
    // surface?.parent maybe is a `SubsurfaceContainer`
    readonly property SurfaceWrapper wrapper: surface?.parent as SurfaceWrapper

    anchors.fill: parent
    SurfaceItemContent {
        id: content
        surface: root.surface?.surface ?? null
        anchors.fill: parent
        live: root.surface && !(root.surface.flags & SurfaceItem.NonLive)
        smooth: root.surface?.smooth ?? true
    }
}
