#include "shape.h"
#include <QPainterPath>
#include <QPolygonF>
#include <cmath>

// Base Shape class
Shape::Shape(ShapeType type, const QPointF& startPoint, const QColor& color)
    : m_type(type), m_color(color), m_selected(false), m_lineWidth(2)
{
}

int Shape::getHandleAt(const QPointF& point) const {
    QVector<QPointF> handles = getResizeHandles();
    for (int i = 0; i < handles.size(); ++i) {
        QRectF handleRect(handles[i].x() - HANDLE_SIZE/2, handles[i].y() - HANDLE_SIZE/2, 
                         HANDLE_SIZE, HANDLE_SIZE);
        if (handleRect.contains(point)) {
            return i;
        }
    }
    return -1;
}

// RectangleShape implementation
RectangleShape::RectangleShape(const QPointF& startPoint, const QColor& color)
    : Shape(ShapeType::Rectangle, startPoint, color), m_topLeft(startPoint), m_bottomRight(startPoint)
{
}

void RectangleShape::draw(QPainter& painter) const {
    painter.setPen(QPen(m_color, m_lineWidth));
    painter.drawRect(QRectF(m_topLeft, m_bottomRight));
    
    if (m_selected) {
        painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter.drawRect(boundingRect());
        
        // Draw resize handles
        painter.setPen(QPen(Qt::red, 1));
        painter.setBrush(Qt::white);
        QVector<QPointF> handles = getResizeHandles();
        for (const QPointF& handle : handles) {
            painter.drawRect(handle.x() - HANDLE_SIZE/2, handle.y() - HANDLE_SIZE/2, 
                           HANDLE_SIZE, HANDLE_SIZE);
        }
    }
}

bool RectangleShape::contains(const QPointF& point) const {
    return QRectF(m_topLeft, m_bottomRight).contains(point);
}

QRectF RectangleShape::boundingRect() const {
    return QRectF(m_topLeft, m_bottomRight).normalized();
}

void RectangleShape::move(const QPointF& delta) {
    m_topLeft += delta;
    m_bottomRight += delta;
}

void RectangleShape::resize(const QPointF& mousePos, int handleIndex) {
    switch (handleIndex) {
        case 0: m_topLeft = mousePos; break;          // Top-left
        case 1: m_topLeft.setY(mousePos.y()); 
                m_bottomRight.setX(mousePos.x()); break; // Top-right
        case 2: m_bottomRight = mousePos; break;      // Bottom-right
        case 3: m_bottomRight.setY(mousePos.y()); 
                m_topLeft.setX(mousePos.x()); break;  // Bottom-left
    }
}

QVector<QPointF> RectangleShape::getResizeHandles() const {
    QRectF rect = boundingRect();
    return {
        rect.topLeft(),
        rect.topRight(),
        rect.bottomRight(),
        rect.bottomLeft()
    };
}

void RectangleShape::setEndPoint(const QPointF& endPoint) {
    m_bottomRight = endPoint;
}

// CircleShape implementation
CircleShape::CircleShape(const QPointF& center, const QColor& color)
    : Shape(ShapeType::Circle, center, color), m_center(center), m_radius(0)
{
}

void CircleShape::draw(QPainter& painter) const {
    painter.setPen(QPen(m_color, m_lineWidth));
    painter.drawEllipse(m_center, m_radius, m_radius);
    
    if (m_selected) {
        painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter.drawRect(boundingRect());
        
        // Draw resize handles
        painter.setPen(QPen(Qt::red, 1));
        painter.setBrush(Qt::white);
        QVector<QPointF> handles = getResizeHandles();
        for (const QPointF& handle : handles) {
            painter.drawRect(handle.x() - HANDLE_SIZE/2, handle.y() - HANDLE_SIZE/2, 
                           HANDLE_SIZE, HANDLE_SIZE);
        }
    }
}

bool CircleShape::contains(const QPointF& point) const {
    qreal distance = std::sqrt(std::pow(point.x() - m_center.x(), 2) + 
                              std::pow(point.y() - m_center.y(), 2));
    return distance <= m_radius;
}

QRectF CircleShape::boundingRect() const {
    return QRectF(m_center.x() - m_radius, m_center.y() - m_radius, 
                  2 * m_radius, 2 * m_radius);
}

void CircleShape::move(const QPointF& delta) {
    m_center += delta;
}

void CircleShape::resize(const QPointF& mousePos, int handleIndex) {
    qreal newRadius = std::sqrt(std::pow(mousePos.x() - m_center.x(), 2) + 
                               std::pow(mousePos.y() - m_center.y(), 2));
    m_radius = newRadius;
}

QVector<QPointF> CircleShape::getResizeHandles() const {
    return {
        QPointF(m_center.x() + m_radius, m_center.y()),
        QPointF(m_center.x() - m_radius, m_center.y()),
        QPointF(m_center.x(), m_center.y() + m_radius),
        QPointF(m_center.x(), m_center.y() - m_radius)
    };
}

void CircleShape::setRadius(qreal radius) {
    m_radius = radius;
}

// TriangleShape implementation
TriangleShape::TriangleShape(const QPointF& point1, const QColor& color)
    : Shape(ShapeType::Triangle, point1, color), m_point1(point1), m_point2(point1), m_point3(point1)
{
}

void TriangleShape::draw(QPainter& painter) const {
    painter.setPen(QPen(m_color, m_lineWidth));
    QPolygonF triangle;
    triangle << m_point1 << m_point2 << m_point3;
    painter.drawPolygon(triangle);
    
    if (m_selected) {
        painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter.drawRect(boundingRect());
        
        // Draw resize handles
        painter.setPen(QPen(Qt::red, 1));
        painter.setBrush(Qt::white);
        QVector<QPointF> handles = getResizeHandles();
        for (const QPointF& handle : handles) {
            painter.drawRect(handle.x() - HANDLE_SIZE/2, handle.y() - HANDLE_SIZE/2, 
                           HANDLE_SIZE, HANDLE_SIZE);
        }
    }
}

bool TriangleShape::contains(const QPointF& point) const {
    QPolygonF triangle;
    triangle << m_point1 << m_point2 << m_point3;
    return triangle.containsPoint(point, Qt::OddEvenFill);
}

QRectF TriangleShape::boundingRect() const {
    qreal minX = std::min({m_point1.x(), m_point2.x(), m_point3.x()});
    qreal maxX = std::max({m_point1.x(), m_point2.x(), m_point3.x()});
    qreal minY = std::min({m_point1.y(), m_point2.y(), m_point3.y()});
    qreal maxY = std::max({m_point1.y(), m_point2.y(), m_point3.y()});
    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

void TriangleShape::move(const QPointF& delta) {
    m_point1 += delta;
    m_point2 += delta;
    m_point3 += delta;
}

void TriangleShape::resize(const QPointF& mousePos, int handleIndex) {
    switch (handleIndex) {
        case 0: m_point1 = mousePos; break;
        case 1: m_point2 = mousePos; break;
        case 2: m_point3 = mousePos; break;
    }
}

QVector<QPointF> TriangleShape::getResizeHandles() const {
    return {m_point1, m_point2, m_point3};
}

void TriangleShape::setPoint2(const QPointF& point2) {
    m_point2 = point2;
}

void TriangleShape::setPoint3(const QPointF& point3) {
    m_point3 = point3;
}

// PolygonShape implementation
PolygonShape::PolygonShape(const QPointF& startPoint, const QColor& color)
    : Shape(ShapeType::Polygon, startPoint, color), m_closed(false)
{
    m_points.append(startPoint);
}

void PolygonShape::draw(QPainter& painter) const {
    if (m_points.size() < 2) return;
    
    painter.setPen(QPen(m_color, m_lineWidth));
    QPolygonF polygon(m_points);
    if (m_closed) {
        painter.drawPolygon(polygon);
    } else {
        painter.drawPolyline(polygon);
    }
    
    if (m_selected) {
        painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter.drawRect(boundingRect());
        
        // Draw resize handles
        painter.setPen(QPen(Qt::red, 1));
        painter.setBrush(Qt::white);
        QVector<QPointF> handles = getResizeHandles();
        for (const QPointF& handle : handles) {
            painter.drawRect(handle.x() - HANDLE_SIZE/2, handle.y() - HANDLE_SIZE/2, 
                           HANDLE_SIZE, HANDLE_SIZE);
        }
    }
}

bool PolygonShape::contains(const QPointF& point) const {
    if (!m_closed || m_points.size() < 3) return false;
    QPolygonF polygon(m_points);
    return polygon.containsPoint(point, Qt::OddEvenFill);
}

QRectF PolygonShape::boundingRect() const {
    if (m_points.isEmpty()) return QRectF();
    
    qreal minX = m_points[0].x(), maxX = m_points[0].x();
    qreal minY = m_points[0].y(), maxY = m_points[0].y();
    
    for (const QPointF& point : m_points) {
        minX = std::min(minX, point.x());
        maxX = std::max(maxX, point.x());
        minY = std::min(minY, point.y());
        maxY = std::max(maxY, point.y());
    }
    
    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

void PolygonShape::move(const QPointF& delta) {
    for (QPointF& point : m_points) {
        point += delta;
    }
}

void PolygonShape::resize(const QPointF& mousePos, int handleIndex) {
    if (handleIndex >= 0 && handleIndex < m_points.size()) {
        m_points[handleIndex] = mousePos;
    }
}

QVector<QPointF> PolygonShape::getResizeHandles() const {
    return m_points;
}

void PolygonShape::addPoint(const QPointF& point) {
    if (!m_closed) {
        m_points.append(point);
    }
}

void PolygonShape::closePolygon() {
    m_closed = true;
} 