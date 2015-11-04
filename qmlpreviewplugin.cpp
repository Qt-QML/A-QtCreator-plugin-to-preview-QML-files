#include "qmlpreviewplugin.h"

// Qt includes
#include <QMenu>
#include <QTemporaryFile>

// QtCreator includes
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icore.h>
#include <coreplugin/rightpane.h>

#include <qmljseditor/qmljseditorconstants.h>
#include <qmljseditor/qmljseditordocument.h>

#include <qmljstools/qmljstoolsconstants.h>

// Plugin includes
#include "qmlpreviewconstants.h"
#include "widgets/previewwidget.h"

namespace QmlPreview {
namespace Internal {


////////////////////////// QmlPreviewPlugin //////////////////////////

QmlPreviewPlugin::QmlPreviewPlugin() :
    m_qmlDocument(0),
    m_previewFile(new QTemporaryFile(this)),
    m_previewWidget(new PreviewWidget(PreviewWidget::SideBarWidget))
{
    connect(m_previewWidget, &PreviewWidget::closeButtonClicked,
            [=]() {
        Core::RightPaneWidget::instance()->setShown(false);
    });

    connect(m_previewWidget, &PreviewWidget::styleToggled,
            this, &QmlPreviewPlugin::onPreviewStyleToggled);

    m_previewFile->open();
}

QmlPreviewPlugin::~QmlPreviewPlugin()
{
    m_previewFile->close();
}

bool QmlPreviewPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorMessage);

    // Register the plugin actions
    Core::ActionContainer *qmlJsToolsContainer = Core::ActionManager::actionContainer(QmlJSTools::Constants::M_TOOLS_QMLJS);

    auto showPreviewAction = new QAction(tr("Show preview"), this);
    Core::Command *command = Core::ActionManager::registerAction(showPreviewAction,
                                                                 Constants::SHOW_PREVIEW);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+X")));

    qmlJsToolsContainer->addAction(command);

    connect(showPreviewAction, &QAction::triggered,
            [=]() {
        showPreviewWidget(true);
    });

    // Connect with the other plugins
    connect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged,
            this, &QmlPreviewPlugin::onCurrentEditorChanged);

    return true;
}

void QmlPreviewPlugin::extensionsInitialized()
{

}

void QmlPreviewPlugin::onCurrentEditorChanged(Core::IEditor *editor)
{
    QmlJSEditor::QmlJSEditorDocument *qmlDoc = nullptr;

    if (editor)
        qmlDoc = qobject_cast<QmlJSEditor::QmlJSEditorDocument *>(editor->document());

    if (m_qmlDocument != qmlDoc) {
        if (m_qmlDocument) {
            disconnect(m_qmlDocument, &QmlJSEditor::QmlJSEditorDocument::contentsChanged,
                       this, &QmlPreviewPlugin::onQmlDocumentContentsChanged);
        }

        m_qmlDocument = qmlDoc;

        if (qmlDoc) {
            connect(qmlDoc, &QmlJSEditor::QmlJSEditorDocument::contentsChanged,
                    this, &QmlPreviewPlugin::onQmlDocumentContentsChanged);

            if (m_previewWidget->url().isEmpty()) {
                updatePreviewFile();
                m_previewWidget->setUrl(QUrl(m_previewFile->fileName()));
            }
        }
        else {
            m_previewWidget->setUrl(QUrl());
        }
    }
}

void QmlPreviewPlugin::onQmlDocumentContentsChanged()
{
    if (!Core::RightPaneWidget::instance()->isShown() || !m_previewWidget->isVisible())
        return;

    updatePreviewFile();

    m_previewWidget->reload();
}

void QmlPreviewPlugin::onPreviewStyleToggled()
{
    if (m_previewWidget->style() == PreviewWidget::ExternalWindow) {
        showPreviewWidget(false);
        m_previewWidget->show();
    }
    else {
        m_previewWidget->close();
        showPreviewWidget(true);
    }
}

void QmlPreviewPlugin::showPreviewWidget(bool show)
{
    if (show) {
        updatePreviewFile();

        m_previewWidget->reload();

        Core::RightPaneWidget::instance()->setWidget(m_previewWidget);
        Core::RightPaneWidget::instance()->setShown(true);
    }
    else {
        Core::RightPaneWidget::instance()->setWidget(0);
        Core::RightPaneWidget::instance()->setShown(false);
    }
}

void QmlPreviewPlugin::updatePreviewFile()
{
    m_previewFile->resize(0);
    m_previewFile->write(m_qmlDocument->plainText().toStdString().c_str());
    m_previewFile->flush();
    m_previewFile->seek(0);
}

} // namespace Internal
} // namespace QmlPreview
