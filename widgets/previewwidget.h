#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

class QAction;
class QHBoxLayout;
class QToolButton;

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

    WidgetStyle style() const;
    void setWidgetStyle(WidgetStyle style);

signals:
    void closeButtonClicked();
    void styleToggled();

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    void rebuildToolBar(WidgetStyle style);

private:
    WidgetStyle m_style;
    QHBoxLayout *m_toolBarLayout;
    QAction *m_closeAction;
    QToolButton *m_closeBtn;
    QAction *m_toggleStyleAction;
    QToolButton *m_toggleStyleBtn;
};

} // namespace Internal
} // namespace QmlPreview

#endif // PREVIEWWIDGET_H
