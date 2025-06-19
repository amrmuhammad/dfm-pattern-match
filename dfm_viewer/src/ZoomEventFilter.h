#ifndef ZOOMEVENTFILTER_H
#define ZOOMEVENTFILTER_H#include <QObject>
#include <QGraphicsView>
#include <QEvent>
#include <QWheelEvent>class ZoomEventFilter : public QObject {
    Q_OBJECT
public:
    ZoomEventFilter(QGraphicsView *view, QObject *parent = nullptr)
        : QObject(parent), graphicsView(view) {
        graphicsView->setProperty("scale", 1.0); // Initialize scale
    }protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Wheel && obj == graphicsView) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            qreal factor = wheelEvent->angleDelta().y() > 0 ? 1.1 : 0.9;
            qreal currentScale = graphicsView->property("scale").toDouble() * factor;
            graphicsView->setProperty("scale", currentScale);
            graphicsView->scale(factor, factor);
            return true; // Event handled
        }
        return QObject::eventFilter(obj, event);
    }private:
    QGraphicsView *graphicsView;
};#endif // ZOOMEVENTFILTER_H

