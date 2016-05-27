#include "qmlpreviewplugin.h"

// Qt includes
#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QFileSystemWatcher>
#include <QMenu>
#include <QTemporaryFile>
#include <QToolButton>
#include <QUrl>

// QtCreator includes
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icore.h>
#include <coreplugin/idocument.h>
#include <coreplugin/rightpane.h>

#include <qmljseditor/qmljseditorconstants.h>
#include <qmljseditor/qmljseditordocument.h>

#include <qmljstools/qmljstoolsconstants.h>

#include <texteditor/textdocument.h>

#include <utils/styledbar.h>

// Plugin includes
#include "previewmodel.h"
#include "qmlpreviewconstants.h"
#include "widgets/previewwidget.h"

namespace QmlPreview {
namespace Internal {


////////////////////////// QmlPreviewPlugin //////////////////////////

QmlPreviewPlugin::QmlPreviewPlugin() :
    m_previewFile(new QTemporaryFile(this)),
    m_fileWatcher(new QFileSystemWatcher(this)),
    m_currentConnection(),
    m_previewPane(0),
    m_previewWidget(0),
    m_toolBarLayout(0),
    m_updateModeCBox(0),
    m_openQmlDocumentsCBox(0),
    m_trackCurrentEditorAction(0),
    m_trackCurrentEditorBtn(0),
    m_closeAction(0),
    m_closeBtn(0),
    m_toggleStyleAction(0),
    m_toggleStyleBtn(0),
    m_updateMode(QmlPreviewPlugin::RealtimeUpdate),
    m_trackCurrentEditor(false)
{
    m_previewFile->open();
}

QmlPreviewPlugin::~QmlPreviewPlugin()
{
    m_previewFile->close();

    PreviewModel::destroy();
}

bool QmlPreviewPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorMessage);

    // Register the plugin actions
    Core::ActionContainer *qmlJsToolsContainer = Core::ActionManager::actionContainer(QmlJSTools::Constants::M_TOOLS_QMLJS);

    auto showPreviewAction = new QAction(tr("Show/reload preview"), this);
    Core::Command *command = Core::ActionManager::registerAction(showPreviewAction,
                                                                 Constants::SHOW_PREVIEW);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+X")));

    qmlJsToolsContainer->addAction(command);

    connect(showPreviewAction, &QAction::triggered,
            [=]() {
        useCurrentDocument();
        showPreviewInRightPane(true);
    });

    return true;
}

void QmlPreviewPlugin::extensionsInitialized()
{
    PreviewModel::init();

    m_previewWidget = new PreviewWidget;
    m_previewPane = createPreviewPane();
}

void QmlPreviewPlugin::onCurrentEditorChanged(Core::IEditor *editor)
{
    Q_UNUSED(editor);

    if (!previewIsVisible())
        return;

    if (m_trackCurrentEditor) {
        useCurrentDocument();
    }
}

void QmlPreviewPlugin::updatePreview()
{
    if (!previewIsVisible())
        return;

    updatePreviewFile();

    m_previewWidget->reload();
}

void QmlPreviewPlugin::updatePreviewFile()
{
    if (m_updateMode != OnDocumentSaveUpdate) {
        auto *doc = qobject_cast<TextEditor::TextDocument *>(PreviewModel::document());

        m_previewFile->resize(0);
        m_previewFile->write(doc->plainText().toStdString().c_str());
        m_previewFile->flush();
        m_previewFile->seek(0);
    }
    else {
        m_fileWatcher->addPath(previewedFilePath());
    }
}

void QmlPreviewPlugin::setUpdateMode(int mode)
{
    if (!PreviewModel::document())
        return;

    auto updateMode = QmlPreviewPlugin::UpdateMode(mode);

    if (m_updateMode != updateMode) {
        enableUpdateMode(PreviewModel::document(), PreviewModel::document(), false);

        m_updateMode = updateMode;

        enableUpdateMode(PreviewModel::document(), PreviewModel::document(), true);

        QUrl url = getPreviewedUrl();
        m_previewWidget->setUrl(url);
    }
}

void QmlPreviewPlugin::setPreviewIndex(int index)
{
    Core::IDocument *document = PreviewModel::documentFromRow(index);
    setDocument(document);
}

void QmlPreviewPlugin::setTrackCurrentEditor(bool track)
{
    if (m_trackCurrentEditor != track) {
        m_trackCurrentEditor = track;

        if (m_trackCurrentEditor) {
            connect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged,
                    this, &QmlPreviewPlugin::onCurrentEditorChanged);

            useCurrentDocument();
        }
        else {
            disconnect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged,
                       this, &QmlPreviewPlugin::onCurrentEditorChanged);
        }
    }
}

bool QmlPreviewPlugin::previewIsVisible() const
{
    bool isVisibleInRightPane = (Core::RightPaneWidget::instance()->isShown()
                                 && !m_previewPane->isWindow()
                                 && m_previewPane->isVisible());

    bool isVisibleAsWindow = (m_previewPane->isVisible()
                              && m_previewPane->isWindow());

    return (isVisibleInRightPane || isVisibleAsWindow);
}

QString QmlPreviewPlugin::previewedFilePath() const
{
    Core::IDocument *doc = PreviewModel::document();
    Q_ASSERT(doc);

    return doc->filePath().toString();
}

QUrl QmlPreviewPlugin::getPreviewedUrl() const
{
    QUrl ret;

    switch (m_updateMode) {
    case RealtimeUpdate:
    case ManualUpdate:
        ret = m_previewFile->fileName();
        break;
    case OnDocumentSaveUpdate:
        ret = previewedFilePath();
        break;
    default:
        break;
    }

    return ret;
}

void QmlPreviewPlugin::enableUpdateMode(Core::IDocument *oldDoc, Core::IDocument *newDoc, bool enable)
{
    switch (m_updateMode) {
    case RealtimeUpdate: {
        if (!oldDoc || !newDoc)
            return;

        auto *concreteDoc = qobject_cast<TextEditor::TextDocument *>(newDoc);

        if (enable) {
            m_currentConnection = connect(concreteDoc, &TextEditor::TextDocument::contentsChanged,
                                          this, &QmlPreviewPlugin::updatePreview);
        }
        else {
            disconnect(m_currentConnection);
        }
    }
        break;
    case OnDocumentSaveUpdate: {
        QString filePath = previewedFilePath();

        if (enable) {
            m_fileWatcher->addPath(filePath);

            m_currentConnection = connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
                                          this, &QmlPreviewPlugin::updatePreview);
        }
        else {
            m_fileWatcher->removePath(filePath);

            disconnect(m_currentConnection);
        }
    }
        break;
    default:
        break;
    }
}

QWidget *QmlPreviewPlugin::createPreviewPane()
{
    QWidget *ret = new QWidget;
    {
        QVBoxLayout *layout = new QVBoxLayout(ret);
        layout->setMargin(0);
        layout->setSpacing(0);

        auto toolBar = new Utils::StyledBar(ret);
        m_toolBarLayout = new QHBoxLayout(toolBar);

        // Create widgets
        m_updateModeCBox = new QComboBox(ret);
        m_updateModeCBox->addItem(QLatin1String("Realtime"));
        m_updateModeCBox->addItem(QLatin1String("On save"));
        m_updateModeCBox->addItem(QLatin1String("Manual"));

        connect(m_updateModeCBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &QmlPreviewPlugin::setUpdateMode);

        m_openQmlDocumentsCBox = new QComboBox(ret);
        m_openQmlDocumentsCBox->setModel(PreviewModel::model());

        connect(m_openQmlDocumentsCBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
                this, &QmlPreviewPlugin::setPreviewIndex);

        // Create actions and related
        m_trackCurrentEditorAction = new QAction(QIcon(QLatin1String(Core::Constants::ICON_LINK)),
                                                 QString(), toolBar);
        m_trackCurrentEditorAction->setCheckable(true);
        m_trackCurrentEditorBtn = new QToolButton(ret);
        m_trackCurrentEditorBtn->setDefaultAction(m_trackCurrentEditorAction);

        connect(m_trackCurrentEditorAction, &QAction::toggled,
                this, &QmlPreviewPlugin::setTrackCurrentEditor);

        m_toggleStyleAction = new QAction(QLatin1String("ToggleStyle"), toolBar);
        m_toggleStyleBtn = new QToolButton(ret);
        m_toggleStyleBtn->setDefaultAction(m_toggleStyleAction);

        connect(m_toggleStyleAction, &QAction::triggered,
                [=]() {
            if (!m_previewPane->isWindow()) {
                showPreviewInRightPane(false);

                m_previewPane->show();
                m_previewPane->setVisible(true);

                Core::ICore::raiseWindow(m_previewPane);
            }
            else {
                m_previewPane->close();
                showPreviewInRightPane(true);
            }

            rebuildToolBar();
        });

        m_closeAction = new QAction(QIcon(QLatin1String(Core::Constants::ICON_BUTTON_CLOSE)),
                                    QString(), toolBar);
        m_closeBtn = new QToolButton(ret);
        m_closeBtn->setDefaultAction(m_closeAction);

        connect(m_closeAction, &QAction::triggered,
                [=]() {
            Core::RightPaneWidget::instance()->setShown(false);
        });

        // Build toolbar
        m_toolBarLayout->setMargin(0);
        m_toolBarLayout->setSpacing(0);

        m_toolBarLayout->addStretch();
        m_toolBarLayout->addWidget(m_updateModeCBox);
        m_toolBarLayout->addWidget(m_openQmlDocumentsCBox);
        m_toolBarLayout->addWidget(m_trackCurrentEditorBtn);
        m_toolBarLayout->addWidget(m_toggleStyleBtn);
        m_toolBarLayout->addWidget(m_closeBtn);

        // Build main layout
        layout->addWidget(toolBar);
        layout->addWidget(m_previewWidget);
    }

    return ret;
}

void QmlPreviewPlugin::rebuildToolBar()
{
    if (m_previewPane->parentWidget()) {
        m_toolBarLayout->addWidget(m_closeBtn);
        m_closeBtn->setVisible(true);
    }
    else {
        m_toolBarLayout->removeWidget(m_closeBtn);
        m_closeBtn->setVisible(false);
    }
}

void QmlPreviewPlugin::showPreviewInRightPane(bool show)
{
    if (Core::RightPaneWidget::instance()->isShown() != show) {
        if (show) {
            Core::RightPaneWidget::instance()->setWidget(m_previewPane);
            Core::RightPaneWidget::instance()->setShown(true);
        }
        else {
            Core::RightPaneWidget::instance()->setWidget(0);
            Core::RightPaneWidget::instance()->setShown(false);
        }
    }
}

void QmlPreviewPlugin::useCurrentDocument()
{
    Core::IDocument *doc = nullptr;

    Core::IEditor *currentEditor = Core::EditorManager::instance()->currentEditor();
    if (currentEditor)
        doc = currentEditor->document();

    setDocument(doc);
}

void QmlPreviewPlugin::setDocument(Core::IDocument *document)
{
    Core::IDocument *current = PreviewModel::document();

    /* 0 - Do nothing
     * 1 - Update view
     * 2 - Reload view
     * 3 - Reset view
     */
    int action = 0;

    if (current != document) {
        // Update the preview model
        enableUpdateMode(current, document, false);

        PreviewModel::setDocument(document);

        int row = PreviewModel::rowOfDocument(document);
        m_openQmlDocumentsCBox->setCurrentIndex(row);

        enableUpdateMode(current, document, true);

        bool isQmlFile = bool(qobject_cast<QmlJSEditor::QmlJSEditorDocument *>(document));

        action = (isQmlFile) ? 1 : 3; /* Update view otherwise Reset View*/
    }
    else if (current) {
        action = 2; /* Reload view */
    }

    switch (action) {
    case 1:
    case 2:
    {
        updatePreviewFile();

        QUrl url = getPreviewedUrl();
        m_previewWidget->setUrl(url);
        break;
    }
    case 3:
        m_previewWidget->reset();
        break;
    default:
        break;
    }
}

} // namespace Internal
} // namespace QmlPreview
