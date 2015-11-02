#include "previewwidget.h"

// Qt includes
#include <QAction>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QToolButton>

// QtCreator includes
#include <coreplugin/coreconstants.h>

#include <utils/styledbar.h>

namespace QmlPreview {
namespace Internal {

PreviewWidget::PreviewWidget(WidgetStyle style, QWidget *parent) :
    QWidget(parent),
    m_style(style),
    m_toolBarLayout(0),
    m_closeAction(0),
    m_closeBtn(new QToolButton(this)),
    m_toggleStyleAction(0),
    m_toggleStyleBtn(new QToolButton(this))
{
    auto toolBar = new Utils::StyledBar(this);
    m_toolBarLayout = new QHBoxLayout(toolBar);

    // Create actions
    m_closeAction = new QAction(QIcon(QLatin1String(Core::Constants::ICON_BUTTON_CLOSE)),
                                QString(), toolBar);
    connect(m_closeAction, &QAction::triggered,
            this, &PreviewWidget::closeButtonClicked);

    m_closeBtn->setDefaultAction(m_closeAction);

    m_toggleStyleAction = new QAction(QLatin1String("ToggleStyle"), toolBar);
    connect(m_toggleStyleAction, &QAction::triggered,
            [=]() {
        setWidgetStyle(WidgetStyle(!m_style));
    });

    m_toggleStyleBtn->setDefaultAction(m_toggleStyleAction);

    // Build UI
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    m_toolBarLayout->setMargin(0);
    m_toolBarLayout->setSpacing(0);

    m_toolBarLayout->addStretch();
    m_toolBarLayout->addWidget(m_toggleStyleBtn);
    m_toolBarLayout->addWidget(m_closeBtn);

    mainLayout->addWidget(toolBar);
    mainLayout->addStretch();

    setWidgetStyle(style);
}

PreviewWidget::~PreviewWidget()
{}

PreviewWidget::WidgetStyle PreviewWidget::style() const
{
    return m_style;
}

void PreviewWidget::setWidgetStyle(PreviewWidget::WidgetStyle style)
{
    if (m_style != style) {
        m_style = style;

        rebuildToolBar(style);

        emit styleToggled();
    }
}

void PreviewWidget::closeEvent(QCloseEvent *e)
{
    if (m_style == ExternalWindow) {
        m_style = SideBarWidget;
        rebuildToolBar(m_style);
    }

    QWidget::closeEvent(e);
}

void PreviewWidget::rebuildToolBar(PreviewWidget::WidgetStyle style)
{
    if (style == SideBarWidget) {
        m_toolBarLayout->addWidget(m_closeBtn);
        m_closeBtn->setVisible(true);
    }
    else {
        m_toolBarLayout->removeWidget(m_closeBtn);
        m_closeBtn->setVisible(false);
    }
}

} // namespace Internal
} // namespace QmlPreview
