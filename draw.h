#ifndef DRAW_H
#define DRAW_H

#include <QPixmap>
#include <QSize>
#include <QString>

extern "C" {
#include "metrix.h"
#include "logic.h"
}

typedef enum {
    leftPadding = 70,
    rightPadding = 25,
    topPadding = 25,
    bottomPadding = 70,
    pointRadius = 3,
    textOffset = 6,
    maxAxisTicks = 18,           // Максимальное количество делений по оси X
    yearLabelXOffset = 16,       // Смещение года по X для центрирования
    yearLabelYOffset = 20,       // Смещение года по Y от оси
    boundsLabelXOffset = 65,     // Смещение подписей границ графика (Min/Max) по X
    boundsLabelYOffset = 5,      // Смещение подписей границ по Y
    metricLabelXOffset = 50,     // Смещение текстовых значений метрик по X
    metricLabelYOffset = 8,      // Смещение текстовых значений метрик по Y
    metricPointExtraRadius = 1,  // Дополнительный радиус для точек метрик
    axisTitleXOffset = 25,       // Смещение заголовка оси "Year" по X
    axisTitleYOffset = 20,       // Смещение заголовка оси "Year" по Y от края
    borderCorrection = 1,        // Корректировка рамки (размер - 1)
    thinLineWidth = 1,           // Тонкая линия
    thickLineWidth = 2           // Толстая линия
} Sizes;

typedef struct {
    GraphPoint max;
    GraphPoint min;
} GraphBounds;

struct DrawContext {
    QPainter* painter;
    int width;
    int height;
    QSize size;
    GraphBounds bounds;
    Metrix metrix;
};

QPixmap buildGraphPixmap(QSize& size, const LinkedList* points, Metrix& metrix);

#endif // DRAW_H
