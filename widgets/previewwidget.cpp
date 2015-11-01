#include "previewwidget.h"

// Qt includes
#include <QAction>
#include <QBoxLayout>
#include <QToolButton>

// QtCreator includes
#include <coreplugin/coreconstants.h>

#include <utils/styledbar.h>

namespace QmlPreview {
namespace Internal {

PreviewWidget::PreviewWidget(WidgetStyle style, QWidget *parent) :
    QWidget(parent),
    m_style(style),
    m_closeAction(0)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    auto toolBar = new Utils::StyledBar;
    auto toolBarLayout = new QHBoxLayout;
    toolBarLayout->setMargin(0);
    toolBarLayout->setSpacing(0);

    toolBar->setLayout(toolBarLayout);

    toolBarLayout->addStretch();

    if (style != ExternalWindow) {
        m_closeAction = new QAction(QIcon(QLatin1String(Core::Constants::ICON_BUTTON_CLOSE)),
                                    QString(), toolBar);

        connect(m_closeAction, &QAction::triggered,
                this, &PreviewWidget::closeButtonClicked);

        auto closeBtn = new QToolButton;
        closeBtn->setDefaultAction(m_closeAction);
        toolBarLayout->addWidget(closeBtn);
    }

    mainLayout->addWidget(toolBar);
    mainLayout->addStretch();
}

PreviewWidget::~PreviewWidget()
{}

} // namespace Internal
} // namespace QmlPreview
