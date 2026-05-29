#include "BubbleFrame.h"
#include <QPainter>
#include <QPainterPath>

constexpr double WIDTH_SANJIAO = 8;

BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent) : QFrame(parent), _role(role), _margin(3)
{
    _p_hlayout = new QHBoxLayout();
    if (_role == ChatRole::SELF)
    {
        _p_hlayout->setContentsMargins(_margin, _margin, WIDTH_SANJIAO + _margin, _margin);
    }
    else
    {
        _p_hlayout->setContentsMargins(WIDTH_SANJIAO + _margin, _margin, _margin, _margin);
    }
    this->setLayout(_p_hlayout);
}

void BubbleFrame::setWidget(QWidget *w)
{
    if (_p_hlayout->count() > 0)
    {
        return;
    }
    else
    {
        _p_hlayout->addWidget(w);
    }
}

void BubbleFrame::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    
    // --- 1. 基础设置 ---
    // 开启抗锯齿，这是让圆角和斜线丝滑的关键
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    // --- 2. 参数计算 ---
    const int triangleWidth = 8;  // 三角形宽度
    const int triangleHeight = 10; // 三角形高度
    const int triangleOffset = 12; // 三角形距离顶部的偏移
    const int borderRadius = 6;    // 气泡圆角

    QPainterPath mainPath;
    QColor baseColor;

    if (_role == ChatRole::OTHER) {
        // 【对方的消息：白色气泡，箭头在左】
        baseColor = QColor(255, 255, 255);
        QRect rect(triangleWidth, 0, width() - triangleWidth, height());
        
        // 绘制圆角矩形路径
        mainPath.addRoundedRect(rect, borderRadius, borderRadius);
        
        // 绘制左侧三角形路径
        QPainterPath trianglePath;
        trianglePath.moveTo(rect.x(), triangleOffset);
        trianglePath.lineTo(rect.x() - triangleWidth, triangleOffset + triangleHeight / 2);
        trianglePath.lineTo(rect.x(), triangleOffset + triangleHeight);
        
        // 将三角形与矩形路径合并（Union）
        mainPath = mainPath.united(trianglePath);
    } 
    else {
        // 【自己的消息：微信绿气泡，箭头在右】
        baseColor = QColor(158, 234, 106);
        QRect rect(0, 0, width() - triangleWidth, height());
        
        // 绘制圆角矩形路径
        mainPath.addRoundedRect(rect, borderRadius, borderRadius);
        
        // 绘制右侧三角形路径
        QPainterPath trianglePath;
        trianglePath.moveTo(rect.right(), triangleOffset);
        trianglePath.lineTo(rect.right() + triangleWidth, triangleOffset + triangleHeight / 2);
        trianglePath.lineTo(rect.right(), triangleOffset + triangleHeight);
        
        mainPath = mainPath.united(trianglePath);
    }

    // --- 3. 渲染气泡 ---
    // 使用微弱的渐变，增加一点点空间感（可选）
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, baseColor.lighter(103)); 
    gradient.setColorAt(1, baseColor);

    painter.setBrush(gradient);
    painter.drawPath(mainPath);

    // --- 4. 细节修饰（可选边框） ---
    // 如果想要更精致，可以画一层极淡的描边
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(baseColor.darker(110), 0.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(mainPath);
}

