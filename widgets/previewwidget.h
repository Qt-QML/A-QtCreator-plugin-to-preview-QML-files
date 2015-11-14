#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

class QLabel;
class QQuickWidget;
class QStackedWidget;

namespace QmlPreview {
namespace Internal {

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();

    QUrl url() const;
    void setUrl(const QUrl &url);

public slots:
    void reload();
    void reset();

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    QStringList retrieveImportPaths() const;

private:
    QStackedWidget *m_stacked;
    QLabel *m_noPreviewLabel;
    QQuickWidget *m_quickView;
};

} // namespace Internal
} // namespace QmlPreview

#endif // PREVIEWWIDGET_H
