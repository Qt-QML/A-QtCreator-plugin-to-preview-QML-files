#include "previewsfiltermodel.h"

#include <QRegularExpression>

// QtCreator includes
#include <coreplugin/editormanager/documentmodel.h>

namespace QmlPreview {
namespace Internal {

static const QRegularExpression regex(QLatin1String(".+\\.qml$"),
                                      QRegularExpression::CaseInsensitiveOption |
                                      QRegularExpression::MultilineOption);


////////////////////////// PreviewsFilterModel //////////////////////////

PreviewsFilterModel::PreviewsFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    setSourceModel(Core::DocumentModel::model());
}

bool PreviewsFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QString text = index.data().toString();

    bool ret = (text == QLatin1String("<no document>"))
            || (text.contains(regex));

    return ret;
}

} // namespace Internal
} // namespace QmlPreview


