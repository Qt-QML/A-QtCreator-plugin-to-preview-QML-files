#ifndef QMLPREVIEWPLUGIN_H
#define QMLPREVIEWPLUGIN_H

#include <extensionsystem/iplugin.h>

class QTemporaryFile;

namespace Core {
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

private slots:
    void onCurrentEditorChanged(Core::IEditor *editor);
    void onQmlDocumentContentsChanged();
    void onPreviewStyleToggled();

private:
    void showPreviewWidget(bool show);
    void updatePreviewFile();

    QmlJSEditor::QmlJSEditorDocument *m_qmlDocument;
    QTemporaryFile *m_previewFile;
    PreviewWidget *m_previewWidget;
};

} // namespace Internal
} // namespace QmlPreview

#endif // QMLPREVIEWPLUGIN_H
