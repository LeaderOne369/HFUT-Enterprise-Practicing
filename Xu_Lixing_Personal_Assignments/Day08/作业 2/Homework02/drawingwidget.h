#ifndef DRAWINGWIDGET_H
#define DRAWINGWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QVector>
#include <memory>
#include "shape.h"

enum class DrawMode {
    Select,
    Rectangle,
    Circle,
    Triangle,
    Polygon
};

enum class EditMode {
    None,
    Moving,
    Resizing
};

class DrawingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DrawingWidget(QWidget *parent = nullptr);
    
    void setDrawMode(DrawMode mode);
    void setCurrentColor(const QColor& color);
    void setLineWidth(int width);
    void clearAll();
    void deleteSelected();
    
    bool hasSelection() const;
    QColor getSelectedColor() const;
    int getSelectedLineWidth() const;

signals:
    void selectionChanged(bool hasSelection);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void createShape(const QPointF& point);
    void updateCurrentShape(const QPointF& point);
    void finishCurrentShape();
    void selectShapeAt(const QPointF& point);
    void clearSelection();
    Shape* getShapeAt(const QPointF& point);
    
    QVector<std::shared_ptr<Shape>> m_shapes;
    std::shared_ptr<Shape> m_currentShape;
    Shape* m_selectedShape;
    
    DrawMode m_drawMode;
    EditMode m_editMode;
    QColor m_currentColor;
    int m_currentLineWidth;
    
    // For shape editing
    QPointF m_lastMousePos;
    int m_resizeHandleIndex;
    
    // For triangle drawing
    int m_trianglePoints;
    
    // For polygon drawing
    bool m_polygonMode;
};

#endif // DRAWINGWIDGET_H 