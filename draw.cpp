#include "draw.h"
#include <QPainter>
#include <QPainterPath>
#include <QList>
#include <QLine>
#include <math.h>

extern "C" {
#include "iterator.h"
}

#define EPS 1e-9
#define VALUE_STEP 5.0
#define FALLBACK_VALUE_OFFSET 1.0

struct MetricMarker {
    double value;
    QColor color;
    QString str;
};

double getMinDouble(double a, double b) {
    return (a < b) ? a : b;
}

double getMaxDouble(double a, double b) {
    return (a > b) ? a : b;
}

int getMaxInt(int a, int b) {
    return (a > b) ? a : b;
}

bool isEqual(double a, double b) {
    return fabs(a - b) <= EPS;
}

double roundDownToStep(double value, double step) {
    return floor(value / step) * step;
}

double roundUpToStep(double value, double step) {
    return ceil(value / step) * step;
}

int mapToX(int year, int minYear, int maxYear, int width) {
    double ratio = (maxYear > minYear) ? (double)(year - minYear) / (maxYear - minYear) : 0.0;
    return qRound(ratio * (width - 1));
}

int mapToY(double value, double minValue, double maxValue, int height) {
    double ratio = isEqual(maxValue, minValue) ? 0.0 : (value - minValue) / (maxValue - minValue);
    return (height - 1) - qRound(ratio * (height - 1));
}

GraphBounds calculateGraphBounds(const LinkedList* points) {
    GraphBounds bounds;
    Iterator it = begin((LinkedList*)points);
    GraphPoint* firstPoint = (GraphPoint*)get(&it);
    bounds.minX = firstPoint->x;
    bounds.maxX = firstPoint->x;
    bounds.minY = firstPoint->y;
    bounds.maxY = firstPoint->y;

    while (hasNext(&it)) {
        GraphPoint* point = (GraphPoint*)get(&it);
        bounds.minX = getMinDouble(point->x, bounds.minX);
        bounds.maxX = getMaxDouble(point->x, bounds.maxX);
        bounds.minY = getMinDouble(point->y, bounds.minY);
        bounds.maxY = getMaxDouble(point->y, bounds.maxY);
        next(&it);
    }
    return bounds;
}

void drawNoDataState(QPainter* painter, QSize size) {
    painter->fillRect(QRect(QPoint(0, 0), size), QColor("#000000"));
    painter->setPen(QColor("#00FF00"));
    painter->drawRect(0, 0, size.width() - borderCorrection, size.height() - borderCorrection);
    painter->drawText(QRect(QPoint(0, 0), size), Qt::AlignCenter, "No data to draw");
}

void drawMetricLine(DrawContext* ctx, double value, QColor color, QString label) {
    int yCoord = mapToY(value, ctx->bounds.minY, ctx->bounds.maxY, ctx->height);
    ctx->painter->setPen(QPen(color, thinLineWidth, Qt::DashLine));
    ctx->painter->drawLine(0, yCoord, ctx->width - 1, yCoord);
    ctx->painter->drawText(textOffset, yCoord - textOffset, label);
}

void drawXAxisTicks(DrawContext* ctx) {
    int minX = (int)ctx->bounds.minX;
    int maxX = (int)ctx->bounds.maxX;
    int xRange = (maxX - minX > 0) ? maxX - minX : 1;
    int xTickStep = (int)ceil((double)xRange / maxAxisTicks);

    ctx->painter->setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    for (int year = minX; year <= maxX; year += xTickStep) {
        int xCoord = mapToX(year, minX, maxX, ctx->width);
        ctx->painter->drawLine(xCoord, 0, xCoord, ctx->height);

        ctx->painter->setPen(QColor("#00FF00"));
        ctx->painter->drawText(xCoord - yearLabelXOffset, ctx->height + yearLabelYOffset, QString::number(year));
        ctx->painter->setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    }
}

void drawMetricDecorations(DrawContext* ctx) {
    ctx->painter->setPen(QColor("#00FF00"));
    ctx->painter->drawText(-boundsLabelXOffset, boundsLabelYOffset, QString::number(ctx->bounds.maxY, 'f', 2));
    ctx->painter->drawText(-boundsLabelXOffset, ctx->height + boundsLabelYOffset, QString::number(ctx->bounds.minY, 'f', 2));

    drawMetricLine(ctx, ctx->metrix.max, QColor("#FF3333"), "Max");
    drawMetricLine(ctx, ctx->metrix.mediana, QColor("#FFFF00"), "Median");
    drawMetricLine(ctx, ctx->metrix.min, QColor("#3399FF"), "Min");
}

void adjustGraphValueBounds(GraphBounds* bounds) {
    if (isEqual(bounds->minY, bounds->maxY)) {
        bounds->minY -= FALLBACK_VALUE_OFFSET;
        bounds->maxY += FALLBACK_VALUE_OFFSET;
    }

    bounds->minY = roundDownToStep(bounds->minY, VALUE_STEP) - VALUE_STEP;
    bounds->maxY = roundUpToStep(bounds->maxY, VALUE_STEP) + VALUE_STEP;
}

void adjustGraphYearBounds(GraphBounds* bounds) {
    if (isEqual(bounds->minX, bounds->maxX)) {
        bounds->minX -= 1.0;
        bounds->maxX += 1.0;
    } else {
        int offset = getMaxInt(1, (int)((bounds->maxX - bounds->minX) * 0.05));
        bounds->minX -= offset;
        bounds->maxX += offset;
    }
}

QRect buildGraphRect(QSize size) {
    return QRect(leftPadding, topPadding,
                 size.width() - leftPadding - rightPadding,
                 size.height() - topPadding - bottomPadding);
}

void drawAxesAndTitles(DrawContext* ctx) {
    ctx->painter->setPen(QPen(QColor("#00FF00"), thinLineWidth));
    ctx->painter->drawRect(0, 0, ctx->width - 1, ctx->height - 1);
    ctx->painter->drawText(ctx->width / 2 - axisTitleXOffset, ctx->height + bottomPadding - axisTitleYOffset, "Year");
}

QList<QLine> buildLines(const LinkedList* points, DrawContext* ctx) {
    QList<QLine> lines;
    if (points != nullptr && points->size > 0) {
        Iterator it = begin((LinkedList*)points);
        GraphPoint* prev = (GraphPoint*)get(&it);
        int prevX = mapToX((int)prev->x, (int)ctx->bounds.minX, (int)ctx->bounds.maxX, ctx->width);
        int prevY = mapToY(prev->y, ctx->bounds.minY, ctx->bounds.maxY, ctx->height);

        next(&it);
        while (hasNext(&it)) {
            GraphPoint* p = (GraphPoint*)get(&it);
            int x = mapToX((int)p->x, (int)ctx->bounds.minX, (int)ctx->bounds.maxX, ctx->width);
            int y = mapToY(p->y, ctx->bounds.minY, ctx->bounds.maxY, ctx->height);
            lines.append(QLine(prevX, prevY, x, y));
            prevX = x;
            prevY = y;
            next(&it);
        }
    }
    return lines;
}

void drawLines(DrawContext* ctx, const QList<QLine>* lines) {
    if (lines != nullptr && !lines->isEmpty()) {
        ctx->painter->setBrush(Qt::NoBrush);
        ctx->painter->setPen(QPen(QColor("#00FF00"), thickLineWidth));
        ctx->painter->drawLines(*lines);
    }
}

void drawPoints(DrawContext* ctx, const QList<QLine>* lines) {
    if (lines != nullptr && !lines->isEmpty()){
        ctx->painter->setBrush(QColor("#00FF00"));

        for (int i = 0; i < lines->size(); ++i)
            ctx->painter->drawEllipse(lines->at(i).p1(), pointRadius, pointRadius);

        if (lines->size() > 1 || lines->first().p1() != lines->first().p2()) {
            ctx->painter->drawEllipse(lines->last().p2(), pointRadius, pointRadius);
        }
    }
}

QPixmap buildGraphPixmap(QSize size, const LinkedList* points, Metrix metrix) {
    QPixmap pixmap(size);
    pixmap.fill(QColor("#000000"));
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (points == nullptr || points->size == 0) {
        drawNoDataState(&painter, size);
    } else {
        GraphBounds bounds = calculateGraphBounds(points);
        adjustGraphValueBounds(&bounds);
        adjustGraphYearBounds(&bounds);

        QRect rect(leftPadding, topPadding,
                   size.width() - leftPadding - rightPadding,
                   size.height() - topPadding - bottomPadding);

        painter.save();
        painter.translate(rect.topLeft());

        DrawContext ctx = { &painter, rect.width(), rect.height(), size, bounds, metrix };

        drawAxesAndTitles(&ctx);
        drawXAxisTicks(&ctx);
        drawMetricDecorations(&ctx);

        QList<QLine> lines = buildLines(points, &ctx);
        ctx.painter->setPen(QPen(QColor("#00FF00"), thickLineWidth));
        ctx.painter->drawLines(lines);
        drawPoints(&ctx, &lines);

        painter.restore();
    }
    return pixmap;
}
