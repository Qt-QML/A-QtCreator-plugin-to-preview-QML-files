#include "previewmodel.h"

// Qt includes
#include <QDebug>
#include <QRegularExpression>
#include <QSortFilterProxyModel>

// QtCreator includes
#include <coreplugin/editormanager/documentmodel.h>
#include <coreplugin/idocument.h>

#include <qmljseditor/qmljseditordocument.h>

namespace QmlPreview {
namespace Internal {


////////////////////////// PreviewsFilterModel //////////////////////////

static const QRegularExpression regex(QLatin1String(".+\\.qml$"),
                                      QRegularExpression::CaseInsensitiveOption |
                                      QRegularExpression::MultilineOption);

class PreviewsFilterModel : public QSortFilterProxyModel
{
public:
    PreviewsFilterModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

PreviewsFilterModel::PreviewsFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(false);
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


////////////////////////// PreviewModelPrivate //////////////////////////

class PreviewModelPrivate
{
public:
    PreviewModelPrivate() :
        model(new PreviewsFilterModel),
        previewed(nullptr)
    {}

    ~PreviewModelPrivate()
    {
        delete model;
    }

    /* functions */

    /* variables */
    PreviewsFilterModel *model;
    Core::IDocument *previewed;
};


////////////////////////// PreviewModel //////////////////////////

static PreviewModelPrivate *d;

PreviewModel::PreviewModel()
{}

void PreviewModel::init()
{
    d = new PreviewModelPrivate;
}

void PreviewModel::destroy()
{
    delete d;
}

QAbstractItemModel *PreviewModel::model()
{
    return d->model;
}

int PreviewModel::rowOfDocument(Core::IDocument *document)
{
    int rowOfDoc = Core::DocumentModel::rowOfDocument(document);
    QModelIndex index = Core::DocumentModel::model()->index(rowOfDoc, 0);

    int ret = d->model->mapFromSource(index).row();
    if (ret == -1)
        ret = 0;

    return (ret);
}

Core::IDocument *PreviewModel::documentFromRow(int row)
{
    Core::IDocument *ret = nullptr;

    QModelIndex mIndex = d->model->index(row, 0);
    int rowInDom = d->model->mapToSource(mIndex).row();

    Core::DocumentModel::Entry *documentEntry = Core::DocumentModel::entryAtRow(rowInDom);
    if (documentEntry)
        ret = documentEntry->document;

    return ret;
}

Core::IDocument *PreviewModel::document()
{
    return d->previewed;
}

void PreviewModel::setDocument(Core::IDocument *document)
{
    if (d->previewed != document) {
        d->previewed = document;
    }
}

} // namespace Internal
} // namespace QmlPreview
