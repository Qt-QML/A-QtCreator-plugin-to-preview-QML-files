#ifndef QMLPREVIEWPLUGIN_H
#define QMLPREVIEWPLUGIN_H

#include <extensionsystem/iplugin.h>

class QTemporaryFile;

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
    QmlPreviewPlugin();
    ~QmlPreviewPlugin();

    virtual bool initialize(const QStringList &arguments, QString *errorMessage);
    virtual void extensionsInitialized();

    bool trackCurrentEditor() const;

private slots:
    void setTrackCurrentEditor(bool trackCurrentEditor);

    void onEditorAboutToClose(Core::IEditor *editor);
    void onCurrentEditorChanged(Core::IEditor *editor);
    void onQmlDocumentContentsChanged();

    void onPreviewedDocumentChangeRequested(int index);

private:
    bool previewIsVisible() const;
    void setPreviewedDocument(Core::IDocument *document);
    void showPreviewWidget(bool show);
    void updatePreviewFile();

    QmlJSEditor::QmlJSEditorDocument *m_previewed;
    QTemporaryFile *m_previewFile;
    PreviewWidget *m_previewWidget;

    bool m_trackCurrentEditor;
};

} // namespace Internal
} // namespace QmlPreview

#endif // QMLPREVIEWPLUGIN_H
