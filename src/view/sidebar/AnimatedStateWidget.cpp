#include "AnimatedStateWidget.h"
#include "global.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>

// ============ 构造函数 ============

AnimatedStateWidget::AnimatedStateWidget(QWidget *parent)
    : AnimatedStateWidget(DisplayMode::QssMode, parent)
{
}

AnimatedStateWidget::AnimatedStateWidget(const QString &iconPath, QWidget *parent)
    : QWidget(parent), _mode(DisplayMode::IconMode), _iconPath(iconPath), _overlayAlpha(0)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    _icon = QPixmap(_iconPath);
    addRedPoint();
}

AnimatedStateWidget::AnimatedStateWidget(DisplayMode mode, QWidget *parent)
    : QWidget(parent), _mode(mode), _overlayAlpha(0)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    if (_mode == DisplayMode::QssMode)
    {
        setState(QStringLiteral("normal"), QStringLiteral("hover"), QStringLiteral("pressed"),
                 QStringLiteral("selected_normal"), QStringLiteral("selected_hover"),
                 QStringLiteral("selected_pressed"));
        setProperty("state", _normal);
        repolish(this);
    }
}

void AnimatedStateWidget::init()
{
    // 预留方法，暂不实现
}

// ============ 红点徽标 ============

void AnimatedStateWidget::addRedPoint()
{
    if (_red_point)
        return;
    _red_point = new QLabel(this);
    _red_point->setObjectName("red_point");
    const QSize badgeSize(50, 50);
    QPixmap pm(QStringLiteral(":/res/red_point.png"));
    if (!pm.isNull())
    {
        _red_point->setPixmap(pm.scaled(badgeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    _red_point->setFixedSize(badgeSize);
    _red_point->setAlignment(Qt::AlignCenter);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(_red_point, 0, Qt::AlignTop | Qt::AlignRight);
    _red_point->setVisible(false);
}

void AnimatedStateWidget::showRedPoint(bool show)
{
    if (show && !_red_point)
        addRedPoint();
    if (_red_point)
        _red_point->setVisible(show);
}

void AnimatedStateWidget::setQssInteraction(QssInteraction interaction)
{
    if (_mode != DisplayMode::QssMode)
        return;
    _qssInteraction = interaction;
    if (interaction == QssInteraction::Momentary)
        _curstate = NORMAL;
}

void AnimatedStateWidget::setCurState(ClickLabelState state)
{
    if (_mode != DisplayMode::QssMode)
        return;
    _curstate = (state == SELECTED) ? SELECTED : NORMAL;
    if (_curstate == NORMAL)
        setProperty("state", _normal);
    else
        setProperty("state", _select_normal.isEmpty() ? _normal : _select_normal);
    repolish(this);
    update();
}

void AnimatedStateWidget::resetNormalState()
{
    setCurState(NORMAL);
}

QString AnimatedStateWidget::normalPressState() const
{
    return _normal_press.isEmpty() ? _normal : _normal_press;
}

QString AnimatedStateWidget::selectPressState() const
{
    if (!_select_press.isEmpty())
        return _select_press;
    return _select_normal.isEmpty() ? normalPressState() : _select_normal;
}

void AnimatedStateWidget::setExclusive(bool exclusive)
{
    _exclusive = exclusive;
}

bool AnimatedStateWidget::isExclusive() const
{
    return _exclusive;
}

void AnimatedStateWidget::setIcon(const QString &iconPath)
{
    _iconPath = iconPath;
    _icon = QPixmap(_iconPath);
    update();
}

QString AnimatedStateWidget::iconPath() const
{
    return _iconPath;
}

void AnimatedStateWidget::setText(const QString &text)
{
    _displayText = text;
    update();
}

// ============ 状态控制 ============

void AnimatedStateWidget::setSelected(bool selected)
{
    if (_mode == DisplayMode::IconMode)
    {
        if (selected)
        {
            _curstate = SELECTED;
            _overlayColor = overlayColorForState();
            animateOverlay(_overlayAlpha, 80, 200);
        }
        else
        {
            _curstate = NORMAL;
            animateOverlay(_overlayAlpha, 0, 200);
        }
    }
    else
    {
        // QssMode
        if (selected)
        {
            _curstate = SELECTED;
            setProperty("state", _select_normal);
        }
        else
        {
            _curstate = NORMAL;
            setProperty("state", _normal);
        }
        repolish(this);
        update();
    }
}

bool AnimatedStateWidget::isSelected() const
{
    return _curstate == SELECTED;
}

void AnimatedStateWidget::setOverlayAlpha(int alpha)
{
    _overlayAlpha = alpha;
    update();
}

// ============ QSS 状态设置 ============

void AnimatedStateWidget::setState(QString normal, QString hover, QString press,
                                    QString select_normal, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;
    _select_normal = select_normal;
    _select_hover = select_hover;
    _select_press = select_press;

    setProperty("state", normal);
    repolish(this);
}

void AnimatedStateWidget::applyQssState()
{
    if (_mode != DisplayMode::QssMode)
        return;

    switch (_curstate)
    {
    case NORMAL:
        setProperty("state", _normal);
        break;
    case SELECTED:
        setProperty("state", _select_normal);
        break;
    }
    repolish(this);
    update();
}

// ============ 渲染 ============

QColor AnimatedStateWidget::overlayColorForState() const
{
    switch (_curstate)
    {
    case SELECTED:
        return QColor(100, 180, 255); // 淡蓝选中色
    case NORMAL:
    default:
        return QColor(255, 255, 255); // 白色高亮
    }
}

void AnimatedStateWidget::animateOverlay(int fromAlpha, int toAlpha, int duration)
{
    if (_anim)
    {
        _anim->stop();
        _anim->deleteLater();
        _anim = nullptr;
    }
    _anim = new QPropertyAnimation(this, "overlayAlpha", this);
    _anim->setStartValue(fromAlpha);
    _anim->setEndValue(toAlpha);
    _anim->setDuration(duration);
    _anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(_anim, &QPropertyAnimation::finished, this, [this]() {
        _anim->deleteLater();
        _anim = nullptr;
    });
    _anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedStateWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (_mode == DisplayMode::IconMode)
    {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setRenderHint(QPainter::Antialiasing);

        // 第一步：绘制缩放后的图标
        if (!_icon.isNull())
        {
            QPixmap scaled = _icon.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int x = (width() - scaled.width()) / 2;
            int y = (height() - scaled.height()) / 2;
            p.drawPixmap(x, y, scaled);
        }

        // 第二步：绘制状态叠加层
        if (_overlayAlpha > 0)
        {
            QColor color = overlayColorForState();
            color.setAlpha(_overlayAlpha);
            p.fillRect(rect(), color);
        }
    }
    else
    {
        // QssMode：只需绘制标准 QWidget 背景（QSS 控制）
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        if (!_displayText.isEmpty())
        {
            style()->drawItemText(&p, rect(), static_cast<int>(Qt::AlignCenter), opt.palette, true,
                                  _displayText, QPalette::WindowText);
        }
    }

    // 红点由 QLabel 子控件自动绘制，无需额外处理
}

// ============ 鼠标事件 ============

void AnimatedStateWidget::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);

    if (_mode == DisplayMode::IconMode)
    {
        if (_curstate == SELECTED)
        {
            _overlayColor = overlayColorForState();
            animateOverlay(80, 120, 100);
        }
        else
        {
            _overlayColor = overlayColorForState();
            animateOverlay(0, 50, 100);
        }
        return;
    }

    if (_qssInteraction == QssInteraction::Momentary)
    {
        setProperty("state", _normal_hover.isEmpty() ? _normal : _normal_hover);
    }
    else if (_curstate == NORMAL)
    {
        setProperty("state", _normal_hover.isEmpty() ? _normal : _normal_hover);
    }
    else
    {
        setProperty("state", _select_hover.isEmpty() ? _select_normal : _select_hover);
    }
    repolish(this);
    update();
}

void AnimatedStateWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (_mode == DisplayMode::IconMode)
    {
        if (_curstate == SELECTED)
        {
            _overlayColor = overlayColorForState();
            animateOverlay(_overlayAlpha, 80, 150);
        }
        else
        {
            animateOverlay(_overlayAlpha, 0, 150);
        }
        return;
    }

    if (_qssInteraction == QssInteraction::Momentary)
    {
        setProperty("state", _normal);
    }
    else if (_curstate == NORMAL)
    {
        setProperty("state", _normal);
    }
    else
    {
        setProperty("state", _select_normal.isEmpty() ? _normal : _select_normal);
    }
    repolish(this);
    update();
}

void AnimatedStateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (_mode == DisplayMode::IconMode)
        {
            _overlayColor = overlayColorForState();
            animateOverlay(_overlayAlpha,
                           _curstate == SELECTED ? 140 : 80, 80);
            return;
        }

        if (_qssInteraction == QssInteraction::Momentary)
        {
            setProperty("state", normalPressState());
        }
        else if (_curstate == NORMAL)
        {
            setProperty("state", normalPressState());
        }
        else
        {
            setProperty("state", selectPressState());
        }
        repolish(this);
        update();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AnimatedStateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (_mode == DisplayMode::IconMode)
        {
            if (_curstate == SELECTED)
            {
                _overlayColor = overlayColorForState();
                animateOverlay(_overlayAlpha, 80, 100);
            }
            else
            {
                animateOverlay(_overlayAlpha, 50, 100);
            }
            emit clicked();
            return;
        }

        if (_qssInteraction == QssInteraction::Momentary)
        {
            setProperty("state", _normal);
            repolish(this);
            update();
            emit clicked();
            return;
        }

        if (_qssInteraction == QssInteraction::ToggleSelection)
        {
            if (_curstate == NORMAL)
            {
                _curstate = SELECTED;
                setProperty("state",
                             _select_hover.isEmpty() ? _select_normal : _select_hover);
            }
            else
            {
                _curstate = NORMAL;
                setProperty("state", _normal_hover.isEmpty() ? _normal : _normal_hover);
            }
            repolish(this);
            update();
            emit clicked();
            return;
        }

        // ExternalSelection
        if (_curstate == NORMAL)
        {
            setProperty("state", _normal_hover.isEmpty() ? _normal : _normal_hover);
        }
        else
        {
            setProperty("state", _select_hover.isEmpty() ? _select_normal : _select_hover);
        }
        repolish(this);
        update();

        emit clicked();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}
