#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

class QAction;

namespace QmlPreview {
namespace Internal {

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    enum WidgetStyle {
        SideBarWidget,
        ExternalWindow
    };

    PreviewWidget(WidgetStyle style, QWidget *parent = 0);
    ~PreviewWidget();

signals:
    void closeButtonClicked();

private:
    WidgetStyle m_style;
    QAction *m_closeAction;
};

} // namespace Internal
} // namespace QmlPreview

#endif // PREVIEWWIDGET_H
