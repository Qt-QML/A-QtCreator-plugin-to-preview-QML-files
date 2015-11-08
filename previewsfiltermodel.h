#ifndef PREVIEWSFILTERMODEL_H
#define PREVIEWSFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace QmlPreview {
namespace Internal {

class PreviewsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    PreviewsFilterModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

} // namespace Internal
} // namespace QmlPreview

#endif // PREVIEWSFILTERMODEL_H
