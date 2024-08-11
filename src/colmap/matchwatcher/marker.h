#pragma once

#include <QGraphicsObject>

#include "imagescene.h"

class Marker : public QGraphicsObject {
  Q_OBJECT
 public:
  explicit Marker(int row);
  QRectF boundingRect() const Q_DECL_OVERRIDE;
  QPainterPath shape() const Q_DECL_OVERRIDE;
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget*) Q_DECL_OVERRIDE;

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
    emit clicked(id);
    QGraphicsObject::mousePressEvent(event);
  }

 signals:
  void clicked(int id);

 private:
  QColor idToColor(int id);
  double radius;
  double innerRadius;
  QColor color;
  const int id;
};