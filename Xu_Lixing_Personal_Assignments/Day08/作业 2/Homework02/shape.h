#ifndef SHAPE_H
#define SHAPE_H

#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QColor>
#include <QVector>
#include <memory>

enum class ShapeType {
    Rectangle,
    Circle,
    Triangle,
    Polygon
};

class Shape {
public:
    Shape(ShapeType type, const QPointF& startPoint, const QColor& color = Qt::black);
    virtual ~Shape() = default;

    virtual void draw(QPainter& painter) const = 0;
    virtual bool contains(const QPointF& point) const = 0;
    virtual QRectF boundingRect() const = 0;
    virtual void move(const QPointF& delta) = 0;
    virtual void resize(const QPointF& mousePos, int handleIndex) = 0;
    virtual QVector<QPointF> getResizeHandles() const = 0;
    virtual int getHandleAt(const QPointF& point) const;

    ShapeType getType() const { return m_type; }
    QColor getColor() const { return m_color; }
    void setColor(const QColor& color) { m_color = color; }
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }
    
    void setLineWidth(int width) { m_lineWidth = width; }
    int getLineWidth() const { return m_lineWidth; }

protected:
    ShapeType m_type;
    QColor m_color;
    bool m_selected;
    int m_lineWidth;
    static const int HANDLE_SIZE = 6;
};

class RectangleShape : public Shape {
public:
    RectangleShape(const QPointF& startPoint, const QColor& color = Qt::black);
    
    void draw(QPainter& painter) const override;
    bool contains(const QPointF& point) const override;
    QRectF boundingRect() const override;
    void move(const QPointF& delta) override;
    void resize(const QPointF& mousePos, int handleIndex) override;
    QVector<QPointF> getResizeHandles() const override;
    
    void setEndPoint(const QPointF& endPoint);
    QRectF getRect() const { return QRectF(m_topLeft, m_bottomRight); }

private:
    QPointF m_topLeft;
    QPointF m_bottomRight;
};

class CircleShape : public Shape {
public:
    CircleShape(const QPointF& center, const QColor& color = Qt::black);
    
    void draw(QPainter& painter) const override;
    bool contains(const QPointF& point) const override;
    QRectF boundingRect() const override;
    void move(const QPointF& delta) override;
    void resize(const QPointF& mousePos, int handleIndex) override;
    QVector<QPointF> getResizeHandles() const override;
    
    void setRadius(qreal radius);
    QPointF getCenter() const { return m_center; }
    qreal getRadius() const { return m_radius; }

private:
    QPointF m_center;
    qreal m_radius;
};

class TriangleShape : public Shape {
public:
    TriangleShape(const QPointF& point1, const QColor& color = Qt::black);
    
    void draw(QPainter& painter) const override;
    bool contains(const QPointF& point) const override;
    QRectF boundingRect() const override;
    void move(const QPointF& delta) override;
    void resize(const QPointF& mousePos, int handleIndex) override;
    QVector<QPointF> getResizeHandles() const override;
    
    void setPoint2(const QPointF& point2);
    void setPoint3(const QPointF& point3);
    QVector<QPointF> getPoints() const { return {m_point1, m_point2, m_point3}; }

private:
    QPointF m_point1, m_point2, m_point3;
};

class PolygonShape : public Shape {
public:
    PolygonShape(const QPointF& startPoint, const QColor& color = Qt::black);
    
    void draw(QPainter& painter) const override;
    bool contains(const QPointF& point) const override;
    QRectF boundingRect() const override;
    void move(const QPointF& delta) override;
    void resize(const QPointF& mousePos, int handleIndex) override;
    QVector<QPointF> getResizeHandles() const override;
    
    void addPoint(const QPointF& point);
    void closePolygon();
    bool isClosed() const { return m_closed; }
    QVector<QPointF> getPoints() const { return m_points; }

private:
    QVector<QPointF> m_points;
    bool m_closed;
};

#endif // SHAPE_H 