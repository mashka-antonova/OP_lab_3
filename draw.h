#ifndef DRAW_H
#define DRAW_H

#include <QPixmap>
#include <QSize>
#include <QString>
#include <QVector>

extern "C" {
#include "metrix.h"
}

typedef enum {
    leftPadding = 70,
    rightPadding = 25,
    topPadding = 25,
    bottomPadding = 70,
    pointRadius = 3,
    textOffset = 6,
} Sizes;

typedef struct {
    int year;
    double value;
} GraphPoint;

QPixmap buildGraphPixmap(const QSize& size, const QVector<GraphPoint>& points, const Metrix& metrix, const QString& yAxisLabel);

#endif // DRAW_H
