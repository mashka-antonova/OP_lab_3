#ifndef DRAW_H
#define DRAW_H

#include <QPixmap>
#include <QSize>
#include <QString>

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
    int minYear;
    int maxYear;
    double minValue;
    double maxValue;
} GraphBounds;

QPixmap buildGraphPixmap(const QSize& size, const LinkedList* points, const Metrix& metrix);

#endif // DRAW_H
