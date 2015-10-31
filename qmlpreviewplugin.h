#ifndef QMLPREVIEWPLUGIN_H
#define QMLPREVIEWPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace QmlPreview {
namespace Internal {

class QmlPreviewPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "QmlPreview.json")

public:
    QmlPreviewPlugin();
    ~QmlPreviewPlugin();

    virtual bool initialize(const QStringList &arguments, QString *errorMessage);
    virtual void extensionsInitialized();
};

} // namespace Internal
} // namespace QmlPreview

#endif // QMLPREVIEWPLUGIN_H
