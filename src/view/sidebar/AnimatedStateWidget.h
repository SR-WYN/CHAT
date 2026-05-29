#ifndef ANIMATEDSTATEWIDGET_H
#define ANIMATEDSTATEWIDGET_H

#include "global.h"
#include <QColor>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QWidget>

class AnimatedStateWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int overlayAlpha READ overlayAlpha WRITE setOverlayAlpha)
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    enum class DisplayMode {
        IconMode,   // 绘制 PNG + 叠加层动画
        QssMode     // 通过 QSS 属性切换样式（文字标签/按钮）
    };

    /// QssMode 下鼠标交互策略（默认：仅由外部 setSelected 控制选中态，用于侧栏等）
    enum class QssInteraction {
        ExternalSelection, ///< 不随点击切换 _curstate，由 setSelected / setCurState 控制
        ToggleSelection,   ///< 每次点击在 NORMAL/SELECTED 间切换（原 ClickedLabel）
        Momentary          ///< 仅 normal/hover/press，松手回到 normal（原 ClickedBtn）
    };

    /// 默认 QssMode，供 Qt Designer / .ui 提升控件使用
    explicit AnimatedStateWidget(QWidget *parent = nullptr);
    explicit AnimatedStateWidget(const QString &iconPath, QWidget *parent = nullptr);
    explicit AnimatedStateWidget(DisplayMode mode, QWidget *parent = nullptr);

    void setQssInteraction(QssInteraction interaction);
    QssInteraction qssInteraction() const { return _qssInteraction; }

    // 状态控制
    void setSelected(bool selected);
    bool isSelected() const;

    ClickLabelState getCurState() const { return _curstate; }
    void setCurState(ClickLabelState state);
    void resetNormalState();

    // QSS 模式：设置 6 种状态对应的样式名
    void setState(QString normal = "", QString hover = "", QString press = "",
                  QString select_normal = "", QString select_hover = "", QString select_press = "");

    // 红点徽标
    void showRedPoint(bool show);
    void addRedPoint();

    // 互斥组支持
    void setExclusive(bool exclusive);
    bool isExclusive() const;

    // 覆盖默认图标（仅 IconMode）
    void setIcon(const QString &iconPath);
    QString iconPath() const;

    void setText(const QString &text);
    QString text() const { return _displayText; }

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // 子类可重写以定制叠加颜色（仅 IconMode）
    virtual QColor overlayColorForState() const;

    int overlayAlpha() const { return _overlayAlpha; }
    void setOverlayAlpha(int alpha);

    // QSS 模式：应用当前状态对应的 QSS 属性
    void applyQssState();

    ClickLabelState _curstate = NORMAL;

private:
    void animateOverlay(int fromAlpha, int toAlpha, int duration = 150);
    void init();

    DisplayMode _mode = DisplayMode::IconMode;

    // IconMode 相关
    QString _iconPath;
    QPixmap _icon;
    int _overlayAlpha = 0;
    QColor _overlayColor;
    QPropertyAnimation *_anim = nullptr;

    QString _normal;
    QString _normal_hover;
    QString _normal_press;
    QString _select_normal;
    QString _select_hover;
    QString _select_press;

    QString _displayText;

    QssInteraction _qssInteraction = QssInteraction::ExternalSelection;

    bool _exclusive = false;

    QLabel *_red_point = nullptr;

    QString normalPressState() const;
    QString selectPressState() const;
};

#endif // ANIMATEDSTATEWIDGET_H
