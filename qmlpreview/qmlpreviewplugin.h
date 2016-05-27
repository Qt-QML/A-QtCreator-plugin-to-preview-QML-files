#ifndef QMLPREVIEWPLUGIN_H
#define QMLPREVIEWPLUGIN_H

#include <extensionsystem/iplugin.h>

class QAction;
class QComboBox;
class QFileSystemWatcher;
class QHBoxLayout;
class QTemporaryFile;
class QToolButton;

namespace Core {
class IDocument;
class IEditor;
}

namespace QmlJSEditor {
class QmlJSEditorDocument;
}

namespace QmlPreview {
namespace Internal {

class PreviewWidget;

class QmlPreviewPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "QmlPreview.json")

public:
    enum UpdateMode {
        RealtimeUpdate,
        OnDocumentSaveUpdate,
        ManualUpdate
    };

    QmlPreviewPlugin();
    ~QmlPreviewPlugin();

    virtual bool initialize(const QStringList &arguments, QString *errorMessage);
    virtual void extensionsInitialized();

private slots:
    void onCurrentEditorChanged(Core::IEditor *editor);

    void updatePreview();
    void updatePreviewFile();

    void setUpdateMode(int mode);
    void setPreviewIndex(int index);
    void setTrackCurrentEditor(bool trackCurrentEditor);

private:
    bool previewIsVisible() const;
    QString previewedFilePath() const;
    QUrl getPreviewedUrl() const;

    void enableUpdateMode(Core::IDocument *oldDoc, Core::IDocument *newDoc, bool enable);

    QWidget *createPreviewPane();
    void rebuildToolBar();
    void showPreviewInRightPane(bool show);

    void useCurrentDocument();
    void setDocument(Core::IDocument *document);

private:
    QTemporaryFile *m_previewFile;
    QFileSystemWatcher *m_fileWatcher;
    QMetaObject::Connection m_currentConnection;

    QWidget *m_previewPane;
    PreviewWidget *m_previewWidget;

    /* toolbar */
    QHBoxLayout *m_toolBarLayout;
    QComboBox *m_updateModeCBox;
    QComboBox *m_openQmlDocumentsCBox;
    QAction *m_trackCurrentEditorAction;
    QToolButton *m_trackCurrentEditorBtn;
    QAction *m_closeAction;
    QToolButton *m_closeBtn;
    QAction *m_toggleStyleAction;
    QToolButton *m_toggleStyleBtn;

    /* parameters */
    UpdateMode m_updateMode;
    bool m_trackCurrentEditor;
};

} // namespace Internal
} // namespace QmlPreview

#endif // QMLPREVIEWPLUGIN_H
