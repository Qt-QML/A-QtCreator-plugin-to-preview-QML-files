#ifndef PREVIEWMODEL_H
#define PREVIEWMODEL_H

#include <QString>

class QAbstractItemModel;

namespace Core {
class IDocument;
}

namespace QmlPreview {
namespace Internal {

class PreviewModel
{
public:
    static void init();
    static void destroy();

    static QAbstractItemModel *model();

    static int rowOfDocument(Core::IDocument *document);
    static Core::IDocument *documentFromRow(int row);

    static Core::IDocument *document();
    static void setDocument(Core::IDocument *document);

private:
    PreviewModel();
};

} // namespace Internal
} // namespace QmlPreview

#endif // PREVIEWMODEL_H
