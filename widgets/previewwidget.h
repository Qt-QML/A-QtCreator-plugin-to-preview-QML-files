#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

class QAction;
class QHBoxLayout;
class QLabel;
class QQuickWidget;
class QStackedWidget;
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

    QUrl url() const;
    void setUrl(const QUrl &url);

public slots:
    void reload();

signals:
    void trackCurrentEditorClicked(bool);
    void closeButtonClicked();
    void styleToggled(WidgetStyle);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    void rebuildToolBar(WidgetStyle style);

private:
    WidgetStyle m_style;
    QHBoxLayout *m_toolBarLayout;
    /* actions */
    QAction *m_trackCurrentEditorAction;
    QToolButton *m_trackCurrentEditorBtn;
    QAction *m_closeAction;
    QToolButton *m_closeBtn;
    QAction *m_toggleStyleAction;
    QToolButton *m_toggleStyleBtn;

    QStackedWidget *m_stacked;
    QLabel *m_noPreviewLabel;
    QQuickWidget *m_quickView;
};

} // namespace Internal
} // namespace QmlPreview

#endif // PREVIEWWIDGET_H
