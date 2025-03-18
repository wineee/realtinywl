// Copyright (C) 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "rootsurfacecontainer.h"
#include "helper.h"
#include "output.h"

#include <woutputlayout.h>
#include <wcursor.h>
#include <woutputitem.h>
#include <woutput.h>
#include <wxdgsurface.h>
#include <wxdgpopupsurface.h>

#include <qwoutputlayout.h>

#include <QQuickWindow>

WAYLIB_SERVER_USE_NAMESPACE


RootSurfaceContainer::RootSurfaceContainer(QQuickItem *parent)
    : QQuickItem(parent)
    , m_cursor(new WCursor(this))
{
    m_cursor->setEventWindow(window());
}

void RootSurfaceContainer::init(WServer *server)
{
    m_outputLayout = new WOutputLayout(server);
    m_cursor->setLayout(m_outputLayout);

    connect(m_outputLayout, &WOutputLayout::implicitWidthChanged, this, [this] {
        const auto width = m_outputLayout->implicitWidth();
        window()->setWidth(width);
        setWidth(width);
    });

    connect(m_outputLayout, &WOutputLayout::implicitHeightChanged, this, [this] {
        const auto height = m_outputLayout->implicitHeight();
        window()->setHeight(height);
        setHeight(height);
    });

    m_outputLayout->safeConnect(&qw_output_layout::notify_change, this, [this] {
        for (auto output : std::as_const(outputs())) {
            output->updatePositionFromLayout();
        }

        ensureCursorVisible();
    });
}

void RootSurfaceContainer::addOutput(Output *output)
{
    m_outputList.append(output);
    m_outputLayout->autoAdd(output->output());
    if (!m_primaryOutput)
        setPrimaryOutput(output);
}

void RootSurfaceContainer::removeOutput(Output *output)
{
    m_outputList.removeOne(output);

    m_outputLayout->remove(output->output());
    if (m_primaryOutput == output) {
        const auto outputs = m_outputLayout->outputs();
        if (!outputs.isEmpty()) {
            auto newPrimaryOutput = Helper::instance()->getOutput(outputs.first());
            setPrimaryOutput(newPrimaryOutput);
        }
    }

    // ensure cursor within output
    const auto outputPos = output->outputItem()->position();
    if (output->geometry().contains(m_cursor->position()) && m_primaryOutput) {
        const auto posInOutput = m_cursor->position() - outputPos;
        const auto newCursorPos = m_primaryOutput->outputItem()->position() + posInOutput;

        if (m_primaryOutput->geometry().contains(newCursorPos))
            Helper::instance()->setCursorPosition(newCursorPos);
        else
            Helper::instance()->setCursorPosition(m_primaryOutput->geometry().center());
    }
}

WOutputLayout *RootSurfaceContainer::outputLayout() const
{
    return m_outputLayout;
}

WCursor *RootSurfaceContainer::cursor() const
{
    return m_cursor;
}

Output *RootSurfaceContainer::cursorOutput() const
{
    Q_ASSERT(m_cursor->layout() == m_outputLayout);
    const auto &pos = m_cursor->position();
    auto o = m_outputLayout->handle()->output_at(pos.x(), pos.y());
    if (!o)
        return nullptr;

    return Helper::instance()->getOutput(WOutput::fromHandle(qw_output::from(o)));
}

Output *RootSurfaceContainer::primaryOutput() const
{
    return m_primaryOutput;
}

void RootSurfaceContainer::setPrimaryOutput(Output *newPrimaryOutput)
{
    if (m_primaryOutput == newPrimaryOutput)
        return;
    m_primaryOutput = newPrimaryOutput;
    emit primaryOutputChanged();
}

const QList<Output*> &RootSurfaceContainer::outputs() const
{
    return m_outputList;
}

void RootSurfaceContainer::ensureCursorVisible()
{
    const auto cursorPos = m_cursor->position();
    if (m_outputLayout->handle()->output_at(cursorPos.x(), cursorPos.y()))
        return;

    if (m_primaryOutput) {
        Helper::instance()->setCursorPosition(m_primaryOutput->geometry().center());
    }
}
