#include "previewwidget.h"

// Qt includes
#include <QBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QStackedWidget>

// QtCreator includes
#include <qmljstools/qmljsmodelmanager.h>

namespace QmlPreview {
namespace Internal {


////////////////////////// PreviewWidget //////////////////////////

PreviewWidget::PreviewWidget(QWidget *parent) :
    QWidget(parent),
    m_stacked(new QStackedWidget(this)),
    m_noPreviewLabel(new QLabel),
    m_quickView(new QQuickWidget)
{
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

    mainLayout->addWidget(m_stacked);
}

PreviewWidget::~PreviewWidget()
{}

void PreviewWidget::setUrl(const QUrl &url)
{
    if (m_quickView->source() != url) {
        m_quickView->engine()->clearComponentCache();

        // Set the import paths
        QStringList importPaths = retrieveImportPaths();
        m_quickView->engine()->setImportPathList(importPaths);
        m_quickView->setSource(url);

        bool showView = !url.isEmpty();
        m_quickView->setVisible(showView);
        m_stacked->setCurrentIndex((showView) ? 1 : 0);
    }
    else {
        reload();
    }
}

QUrl PreviewWidget::url() const
{
    return m_quickView->source();
}

void PreviewWidget::reload()
{
    // http://stackoverflow.com/questions/17337493/how-to-reload-qml-file-to-qquickview
    QUrl tmp = m_quickView->source();

    m_quickView->setSource(QUrl());
    m_quickView->engine()->clearComponentCache();
    m_quickView->setSource(tmp);
}

void PreviewWidget::reset()
{
    if (!m_quickView->source().isEmpty())
        setUrl(QUrl());
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

QStringList PreviewWidget::retrieveImportPaths() const
{
    QStringList ret;

    QmlJS::PathsAndLanguages p = QmlJS::ModelManagerInterface::instance()->importPaths();
    for (int i = 0; i < p.size(); ++i)
        ret << p.at(i).path().toString();

    return ret;
}

} // namespace Internal
} // namespace QmlPreview
