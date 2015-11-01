#include "qmlpreviewplugin.h"

// Qt includes
#include <QMenu>

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

QmlPreviewPlugin::QmlPreviewPlugin() :
    m_previewWidget(0)
{

}

QmlPreviewPlugin::~QmlPreviewPlugin()
{

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
            this, &QmlPreviewPlugin::onShowPreviewRequested);

    return true;
}

void QmlPreviewPlugin::extensionsInitialized()
{

}

void QmlPreviewPlugin::onShowPreviewRequested()
{
    Core::IEditor *currentEditor = Core::EditorManager::instance()->currentEditor();
    if (!currentEditor)
        return;

    auto isQmlEditor = qobject_cast<QmlJSEditor::QmlJSEditorDocument *>(currentEditor->document());
    if (!isQmlEditor)
        return;

    if (!m_previewWidget) {
        m_previewWidget = new PreviewWidget(PreviewWidget::SideBarWidget);

        connect(m_previewWidget, &PreviewWidget::closeButtonClicked,
                this, &QmlPreviewPlugin::onCloseButtonClicked);
    }

    Core::RightPaneWidget::instance()->setWidget(m_previewWidget);
    Core::RightPaneWidget::instance()->setShown(true);
}

void QmlPreviewPlugin::onCloseButtonClicked()
{
    Core::RightPaneWidget::instance()->setShown(false);
}

} // namespace Internal
} // namespace QmlPreview
