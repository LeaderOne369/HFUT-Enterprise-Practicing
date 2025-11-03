#include "progressbardelegate.h"
#include <QPainter>
#include <QApplication>
#include <QStyleOptionProgressBar>

ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, 
                                const QModelIndex &index) const
{
    // 获取进度值
    int progress = index.data(Qt::UserRole).toInt();
    
    if (progress >= 0 && progress <= 100) {
        drawProgressBar(painter, option, progress);
    } else {
        // 如果不是有效的进度值，使用默认绘制
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option, 
                                   const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QSize(option.rect.width(), 30);
}

void ProgressBarDelegate::drawProgressBar(QPainter *painter, const QStyleOptionViewItem &option, 
                                         int progress) const
{
    painter->save();
    
    // 设置抗锯齿
    painter->setRenderHint(QPainter::Antialiasing);
    
    // 计算进度条区域
    QRect progressRect = option.rect.adjusted(5, 5, -5, -5);
    
    // 绘制背景
    painter->setPen(QPen(QColor(200, 200, 200), 1));
    painter->setBrush(QColor(240, 240, 240));
    painter->drawRoundedRect(progressRect, 3, 3);
    
    // 绘制进度条
    if (progress > 0) {
        int progressWidth = (progressRect.width() * progress) / 100;
        QRect filledRect = progressRect;
        filledRect.setWidth(progressWidth);
        
        // 根据进度选择颜色
        QColor progressColor;
        if (progress < 30) {
            progressColor = QColor(255, 99, 71);  // 红色
        } else if (progress < 70) {
            progressColor = QColor(255, 165, 0);  // 橙色
        } else {
            progressColor = QColor(50, 205, 50);  // 绿色
        }
        
        // 创建渐变效果
        QLinearGradient gradient(filledRect.topLeft(), filledRect.bottomLeft());
        gradient.setColorAt(0, progressColor.lighter(120));
        gradient.setColorAt(1, progressColor.darker(110));
        
        painter->setBrush(gradient);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(filledRect, 3, 3);
    }
    
    // 绘制百分比文字
    painter->setPen(QColor(60, 60, 60));
    painter->setFont(option.font);
    QString text = QString("%1%").arg(progress);
    painter->drawText(progressRect, Qt::AlignCenter, text);
    
    painter->restore();
} 