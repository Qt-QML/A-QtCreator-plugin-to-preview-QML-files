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
    m_previewWidget(new PreviewWidget(PreviewWidget::SideBarWidget)),
    m_trackCurrentEditor(false)
{
    connect(m_previewWidget, &PreviewWidget::trackCurrentEditorClicked,
            this, &QmlPreviewPlugin::setTrackCurrentEditor);

    // onPreviewWidgetCloseButtonClicked
    connect(m_previewWidget, &PreviewWidget::closeButtonClicked,
            [=]() {
        Core::RightPaneWidget::instance()->setShown(false);
    });

    // onPreviewWidgetStyleToggled
    connect(m_previewWidget, &PreviewWidget::styleToggled,
            [=](PreviewWidget::WidgetStyle style) {
        if (style == PreviewWidget::ExternalWindow) {
            showPreviewWidget(false);
            m_previewWidget->show();
            Core::ICore::raiseWindow(m_previewWidget);
        }
        else {
            m_previewWidget->close();
            showPreviewWidget(true);
        }

    });

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
        processEditor(Core::EditorManager::currentEditor());
        showPreviewWidget(true);
    });

    // Connect with the other plugins
    connect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged,
            this, &QmlPreviewPlugin::onCurrentEditorChanged);

    connect(Core::EditorManager::instance(), &Core::EditorManager::editorAboutToClose,
            this, &QmlPreviewPlugin::onEditorAboutToClose);

    return true;
}

void QmlPreviewPlugin::extensionsInitialized()
{

}

bool QmlPreviewPlugin::trackCurrentEditor() const
{
    return m_trackCurrentEditor;
}

void QmlPreviewPlugin::setTrackCurrentEditor(bool track)
{
    if (m_trackCurrentEditor != track) {
        m_trackCurrentEditor = track;

        if (m_trackCurrentEditor)
            processEditor(Core::EditorManager::instance()->currentEditor());
    }
}

void QmlPreviewPlugin::onEditorAboutToClose(Core::IEditor *editor)
{
    Q_ASSERT(editor);

    if (m_qmlDocument) {
        disconnect(m_qmlDocument, &QmlJSEditor::QmlJSEditorDocument::contentsChanged,
                   this, &QmlPreviewPlugin::onQmlDocumentContentsChanged);

        m_qmlDocument = nullptr;
    }
}

void QmlPreviewPlugin::onCurrentEditorChanged(Core::IEditor *editor)
{
    if (!previewIsVisible())
        return;

    if (m_trackCurrentEditor)
        processEditor(editor);
}

void QmlPreviewPlugin::onQmlDocumentContentsChanged()
{
    if (!previewIsVisible())
        return;

    updatePreviewFile();

    m_previewWidget->reload();
}

bool QmlPreviewPlugin::previewIsVisible() const
{
    bool isVisibleInRightPane = (Core::RightPaneWidget::instance()->isShown()
                                 && !m_previewWidget->style()
                                 && m_previewWidget->isVisible());

    bool isVisibleAsWindow = (m_previewWidget->style()
                              && m_previewWidget->isWindow());

    return (isVisibleInRightPane || isVisibleAsWindow);
}

void QmlPreviewPlugin::processEditor(Core::IEditor *editor)
{
    QmlJSEditor::QmlJSEditorDocument *qmlDoc = (!editor) ? nullptr
                                                         : qobject_cast<QmlJSEditor::QmlJSEditorDocument *>(editor->document());

    if (m_qmlDocument != qmlDoc) {
        if (m_qmlDocument) {
            disconnect(m_qmlDocument, &QmlJSEditor::QmlJSEditorDocument::contentsChanged,
                       this, &QmlPreviewPlugin::onQmlDocumentContentsChanged);
        }

        m_qmlDocument = qmlDoc;

        if (m_qmlDocument) {
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
    else if (!qmlDoc) {
        m_previewWidget->setUrl(QUrl());
    }
}

void QmlPreviewPlugin::showPreviewWidget(bool show)
{
    if (Core::RightPaneWidget::instance()->isShown() != show) {
        if (show) {
            Core::RightPaneWidget::instance()->setWidget(m_previewWidget);
            Core::RightPaneWidget::instance()->setShown(true);
        }
        else {
            Core::RightPaneWidget::instance()->setWidget(0);
            Core::RightPaneWidget::instance()->setShown(false);
        }
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
