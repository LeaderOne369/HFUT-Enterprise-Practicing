#include "drawingwidget.h"
#include <QPainter>
#include <QColorDialog>

DrawingWidget::DrawingWidget(QWidget *parent)
    : QWidget(parent)
    , m_selectedShape(nullptr)
    , m_drawMode(DrawMode::Select)
    , m_editMode(EditMode::None)
    , m_currentColor(Qt::black)
    , m_currentLineWidth(2)
    , m_resizeHandleIndex(-1)
    , m_trianglePoints(0)
    , m_polygonMode(false)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);
    setMinimumSize(400, 300);
}

void DrawingWidget::setDrawMode(DrawMode mode) {
    m_drawMode = mode;
    clearSelection();
    finishCurrentShape();
    m_polygonMode = false;
    setCursor(Qt::ArrowCursor);
}

void DrawingWidget::setCurrentColor(const QColor& color) {
    m_currentColor = color;
    if (m_selectedShape) {
        m_selectedShape->setColor(color);
        update();
    }
}

void DrawingWidget::setLineWidth(int width) {
    m_currentLineWidth = width;
    if (m_selectedShape) {
        m_selectedShape->setLineWidth(width);
        update();
    }
}

void DrawingWidget::clearAll() {
    m_shapes.clear();
    m_currentShape.reset();
    m_selectedShape = nullptr;
    m_editMode = EditMode::None;
    m_polygonMode = false;
    emit selectionChanged(false);
    update();
}

void DrawingWidget::deleteSelected() {
    if (m_selectedShape) {
        for (int i = 0; i < m_shapes.size(); ++i) {
            if (m_shapes[i].get() == m_selectedShape) {
                m_shapes.removeAt(i);
                break;
            }
        }
        m_selectedShape = nullptr;
        emit selectionChanged(false);
        update();
    }
}

bool DrawingWidget::hasSelection() const {
    return m_selectedShape != nullptr;
}

QColor DrawingWidget::getSelectedColor() const {
    return m_selectedShape ? m_selectedShape->getColor() : m_currentColor;
}

int DrawingWidget::getSelectedLineWidth() const {
    return m_selectedShape ? m_selectedShape->getLineWidth() : m_currentLineWidth;
}

void DrawingWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw all shapes
    for (const auto& shape : m_shapes) {
        shape->draw(painter);
    }
    
    // Draw current shape being created
    if (m_currentShape) {
        m_currentShape->draw(painter);
    }
}

void DrawingWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;
    
    QPointF mousePos = event->position();
    
    if (m_drawMode == DrawMode::Select) {
        // Check if clicking on a resize handle first
        if (m_selectedShape) {
            m_resizeHandleIndex = m_selectedShape->getHandleAt(mousePos);
            if (m_resizeHandleIndex != -1) {
                m_editMode = EditMode::Resizing;
                setCursor(Qt::SizeFDiagCursor);
                return;
            }
        }
        
        // Check if clicking on a shape
        Shape* clickedShape = getShapeAt(mousePos);
        if (clickedShape) {
            if (clickedShape != m_selectedShape) {
                clearSelection();
                m_selectedShape = clickedShape;
                m_selectedShape->setSelected(true);
                emit selectionChanged(true);
            }
            m_editMode = EditMode::Moving;
            setCursor(Qt::ClosedHandCursor);
        } else {
            clearSelection();
        }
    } else {
        // Drawing mode
        if (m_drawMode == DrawMode::Polygon && m_polygonMode && m_currentShape) {
            // Continue adding points to polygon
            std::shared_ptr<PolygonShape> polygon = 
                std::dynamic_pointer_cast<PolygonShape>(m_currentShape);
            if (polygon) {
                polygon->addPoint(mousePos);
            }
        } else {
            createShape(mousePos);
        }
    }
    
    m_lastMousePos = mousePos;
    update();
}

void DrawingWidget::mouseMoveEvent(QMouseEvent *event) {
    QPointF mousePos = event->position();
    
    if (event->buttons() & Qt::LeftButton) {
        if (m_drawMode == DrawMode::Select) {
            if (m_editMode == EditMode::Moving && m_selectedShape) {
                QPointF delta = mousePos - m_lastMousePos;
                m_selectedShape->move(delta);
            } else if (m_editMode == EditMode::Resizing && m_selectedShape) {
                m_selectedShape->resize(mousePos, m_resizeHandleIndex);
            }
        } else if (m_currentShape && !m_polygonMode) {
            updateCurrentShape(mousePos);
        }
        
        m_lastMousePos = mousePos;
        update();
    }
}

void DrawingWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (m_drawMode == DrawMode::Select) {
            m_editMode = EditMode::None;
            setCursor(Qt::ArrowCursor);
        } else if (m_currentShape && !m_polygonMode) {
            finishCurrentShape();
        }
        
        update();
    }
}

void DrawingWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (m_drawMode == DrawMode::Polygon && m_polygonMode && m_currentShape) {
        // Close polygon on double click
        std::shared_ptr<PolygonShape> polygon = 
            std::dynamic_pointer_cast<PolygonShape>(m_currentShape);
        if (polygon) {
            polygon->closePolygon();
        }
        finishCurrentShape();
    }
}

void DrawingWidget::createShape(const QPointF& point) {
    clearSelection();
    
    switch (m_drawMode) {
        case DrawMode::Rectangle:
            m_currentShape = std::make_shared<RectangleShape>(point, m_currentColor);
            break;
        case DrawMode::Circle:
            m_currentShape = std::make_shared<CircleShape>(point, m_currentColor);
            break;
        case DrawMode::Triangle:
            m_currentShape = std::make_shared<TriangleShape>(point, m_currentColor);
            m_trianglePoints = 1;
            break;
        case DrawMode::Polygon:
            m_currentShape = std::make_shared<PolygonShape>(point, m_currentColor);
            m_polygonMode = true;
            break;
        default:
            break;
    }
    
    if (m_currentShape) {
        m_currentShape->setLineWidth(m_currentLineWidth);
    }
}

void DrawingWidget::updateCurrentShape(const QPointF& point) {
    if (!m_currentShape) return;
    
    switch (m_drawMode) {
        case DrawMode::Rectangle: {
            std::shared_ptr<RectangleShape> rect = 
                std::dynamic_pointer_cast<RectangleShape>(m_currentShape);
            if (rect) rect->setEndPoint(point);
            break;
        }
        case DrawMode::Circle: {
            std::shared_ptr<CircleShape> circle = 
                std::dynamic_pointer_cast<CircleShape>(m_currentShape);
            if (circle) {
                qreal radius = std::sqrt(std::pow(point.x() - circle->getCenter().x(), 2) + 
                                       std::pow(point.y() - circle->getCenter().y(), 2));
                circle->setRadius(radius);
            }
            break;
        }
        case DrawMode::Triangle: {
            std::shared_ptr<TriangleShape> triangle = 
                std::dynamic_pointer_cast<TriangleShape>(m_currentShape);
            if (triangle) {
                if (m_trianglePoints == 1) {
                    triangle->setPoint2(point);
                } else if (m_trianglePoints == 2) {
                    triangle->setPoint3(point);
                }
            }
            break;
        }
        default:
            break;
    }
}

void DrawingWidget::finishCurrentShape() {
    if (m_currentShape) {
        if (m_drawMode == DrawMode::Triangle) {
            m_trianglePoints++;
            if (m_trianglePoints < 3) {
                return; // Need more points for triangle
            }
        }
        
        m_shapes.append(m_currentShape);
        m_currentShape.reset();
        m_trianglePoints = 0;
        m_polygonMode = false;
    }
}

void DrawingWidget::selectShapeAt(const QPointF& point) {
    clearSelection();
    Shape* shape = getShapeAt(point);
    if (shape) {
        m_selectedShape = shape;
        m_selectedShape->setSelected(true);
        emit selectionChanged(true);
    }
}

void DrawingWidget::clearSelection() {
    if (m_selectedShape) {
        m_selectedShape->setSelected(false);
        m_selectedShape = nullptr;
        emit selectionChanged(false);
    }
}

Shape* DrawingWidget::getShapeAt(const QPointF& point) {
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        if (m_shapes[i]->contains(point)) {
            return m_shapes[i].get();
        }
    }
    return nullptr;
} 