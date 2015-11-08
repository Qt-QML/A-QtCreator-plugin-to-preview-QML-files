#include "previewwidget.h"

// Qt includes
#include <QAction>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QStackedWidget>
#include <QToolButton>

// QtCreator includes
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/documentmodel.h>
#include <coreplugin/idocument.h>

#include <utils/styledbar.h>

// Plugin includes
#include "../previewsfiltermodel.h"

namespace QmlPreview {
namespace Internal {


////////////////////////// PreviewWidget //////////////////////////

PreviewWidget::PreviewWidget(WidgetStyle style, QWidget *parent) :
    QWidget(parent),
    m_style(style),
    m_toolBarLayout(0),
    m_openQmlDocumentsModel(new PreviewsFilterModel(this)),
    m_openQmlDocumentsCBox(new QComboBox(this)),
    m_trackCurrentEditorAction(0),
    m_trackCurrentEditorBtn(new QToolButton(this)),
    m_closeAction(0),
    m_closeBtn(new QToolButton(this)),
    m_toggleStyleAction(0),
    m_toggleStyleBtn(new QToolButton(this)),
    m_stacked(new QStackedWidget(this)),
    m_noPreviewLabel(new QLabel),
    m_quickView(new QQuickWidget)
{
    auto toolBar = new Utils::StyledBar(this);
    m_toolBarLayout = new QHBoxLayout(toolBar);

    // Create actions
    m_trackCurrentEditorAction = new QAction(QIcon(QLatin1String(Core::Constants::ICON_LINK)),
                                             QString(), toolBar);
    m_trackCurrentEditorAction->setCheckable(true);

    connect(m_trackCurrentEditorAction, &QAction::toggled,
            this, &PreviewWidget::trackEditorButtonClicked);

    m_trackCurrentEditorBtn->setDefaultAction(m_trackCurrentEditorAction);

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
    m_openQmlDocumentsModel->setDynamicSortFilter(false);
    m_openQmlDocumentsCBox->setModel(m_openQmlDocumentsModel);

    connect(m_openQmlDocumentsCBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int row) {
        QModelIndex modelIndex = m_openQmlDocumentsModel->index(row, 0);
        int realIndex = m_openQmlDocumentsModel->mapToSource(modelIndex).row();

        emit documentChangeRequested(realIndex);
    });

    m_quickView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickView->setVisible(false);

    const QString placeholderText = tr("<html><body style=\"color:#909090; font-size:14px\">"
                                       "<div align='center'>"
                                       "<div style=\"font-size:20px\">No preview available</div>"
                                       "<table><tr><td>"
                                       "<hr/>"
                                       "<div style=\"margin-top: 5px\">&bull; Select a qml file</div>"
                                       "<div style=\"margin-top: 5px\">&bull; Enable editor tracking</div>"
                                       "</td></tr></table>"
                                       "</div>"
                                       "</body></html>");

    m_noPreviewLabel->setText(placeholderText);

    m_stacked->addWidget(m_noPreviewLabel);
    m_stacked->addWidget(m_quickView);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    m_toolBarLayout->setMargin(0);
    m_toolBarLayout->setSpacing(0);

    m_toolBarLayout->addStretch();
    m_toolBarLayout->addWidget(m_openQmlDocumentsCBox);
    m_toolBarLayout->addWidget(m_trackCurrentEditorBtn);
    m_toolBarLayout->addWidget(m_toggleStyleBtn);
    m_toolBarLayout->addWidget(m_closeBtn);

    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(m_stacked);

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

        emit styleToggled(style);
    }
}

QUrl PreviewWidget::url() const
{
    return m_quickView->source();
}

void PreviewWidget::setUrl(const QUrl &url)
{
    if (m_quickView->source() != url) {
        m_quickView->engine()->clearComponentCache();
        m_quickView->setSource(url);

        if (url.isEmpty()) {
            m_quickView->setVisible(false);
            m_stacked->setCurrentWidget(m_noPreviewLabel);
        }
        else {
            m_quickView->setVisible(true);
            m_stacked->setCurrentWidget(m_quickView);
        }
    }
}

void PreviewWidget::setDocument(Core::IDocument *document)
{
    int rowOfDoc = Core::DocumentModel::rowOfDocument(document);

    QModelIndex index = Core::DocumentModel::model()->index(rowOfDoc, 0);
    int mappedRow = m_openQmlDocumentsModel->mapFromSource(index).row();

    if (m_openQmlDocumentsCBox->currentIndex() != mappedRow) {
        m_openQmlDocumentsCBox->setCurrentIndex(mappedRow);
    }
}

void PreviewWidget::reload()
{
    // http://stackoverflow.com/questions/17337493/how-to-reload-qml-file-to-qquickview
    QUrl tmp = m_quickView->source();

    m_quickView->setSource(QUrl());
    m_quickView->engine()->clearComponentCache();
    m_quickView->setSource(tmp);
}

void PreviewWidget::resizeEvent(QResizeEvent *e)
{
    // Necessary to avoid crashes.
    // One of the ugliest tricks so far.
    // Waiting for Qt 5.6 -> https://bugreports.qt.io/browse/QTBUG-47588
    bool v = (e->size().width() > 0 && e->size().height() > 0);

    if (m_stacked->currentWidget() == m_noPreviewLabel) {
        m_quickView->setVisible(false);
    }
    else {
        m_quickView->setVisible(v);
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
