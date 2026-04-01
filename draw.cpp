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

int mapToX(int year, int minYear, int maxYear, QRect rect) {
    int xCoord = rect.left();
    if (maxYear > minYear) {
        double ratio = (double)(year - minYear) / (double)(maxYear - minYear);
        xCoord = rect.left() + (int)(ratio * rect.width());
    }
    return xCoord;
}

int mapToY(double value, double minValue, double maxValue, QRect rect) {
    int yCoord = rect.bottom();
    if (!isEqual(maxValue, minValue)) {
        double ratio = (value - minValue) / (maxValue - minValue);
        yCoord = rect.bottom() - (int)(ratio * rect.height());
    }
    return yCoord;
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
    const int yCoord = mapToY(value, ctx->bounds.minY, ctx->bounds.maxY, ctx->rect);
    ctx->painter->setPen(QPen(color, thinLineWidth, Qt::DashLine));
    ctx->painter->drawLine(ctx->rect.left(), yCoord, ctx->rect.right(), yCoord);
    ctx->painter->drawText(ctx->rect.left() + textOffset, yCoord - textOffset, label);
}

void drawXAxisTicks(DrawContext* ctx) {
    int minX = (int)ctx->bounds.minX;
    int maxX = (int)ctx->bounds.maxX;

    int xRange = getMaxInt(1, maxX - minX);
    int xTickStep = getMaxInt(1, (int)ceil((double)xRange / maxAxisTicks));

    ctx->painter->setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    int lastTickYear = minX;
    for (int year = minX; year <= maxX; year += xTickStep) {
        if (year == maxX - borderCorrection)
            continue;

        lastTickYear = year;

        int xCoord = mapToX(year, minX, maxX, ctx->rect);
        ctx->painter->drawLine(xCoord, ctx->rect.bottom(), xCoord, ctx->rect.top());
        ctx->painter->setPen(QColor("#00FF00"));
        ctx->painter->drawText(xCoord - yearLabelXOffset, ctx->rect.bottom() + yearLabelYOffset, QString::number(year));
        ctx->painter->setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    }

    if (lastTickYear != maxX) {
        int xCoord = mapToX(maxX, minX, maxX, ctx->rect);
        ctx->painter->drawLine(xCoord, ctx->rect.bottom(), xCoord, ctx->rect.top());
        ctx->painter->setPen(QColor("#00FF00"));
        ctx->painter->drawText(xCoord - yearLabelXOffset, ctx->rect.bottom() + yearLabelYOffset, QString::number(maxX));
    }
}

void drawMetricDecorations(DrawContext* ctx) {
    ctx->painter->drawText(ctx->rect.left() - boundsLabelXOffset, ctx->rect.top() + boundsLabelYOffset, QString::number(ctx->bounds.maxY, 'f', 2));
    ctx->painter->drawText(ctx->rect.left() - boundsLabelXOffset, ctx->rect.bottom() + boundsLabelYOffset, QString::number(ctx->bounds.minY, 'f', 2));

    drawMetricLine(ctx, ctx->metrix.max, QColor("#FF3333"), "Max");
    drawMetricLine(ctx, ctx->metrix.mediana, QColor("#FFFF00"), "Median");
    drawMetricLine(ctx, ctx->metrix.min, QColor("#3399FF"), "Min");

    const MetricMarker metricPoints[] = {
        {ctx->metrix.max, QColor("#FF3333"), QString::number(ctx->metrix.max, 'f', 2)},
        {ctx->metrix.mediana, QColor("#FFFF00"), QString::number(ctx->metrix.mediana, 'f', 2)},
        {ctx->metrix.min, QColor("#3399FF"), QString::number(ctx->metrix.min, 'f', 2)}
    };

    int pointsCount = sizeof(metricPoints) / sizeof(metricPoints[0]);
    for (int i = 0; i < pointsCount; ++i) {
        int yCoord = mapToY(metricPoints[i].value, ctx->bounds.minY, ctx->bounds.maxY, ctx->rect);
        ctx->painter->setPen(QPen(metricPoints[i].color, thickLineWidth));
        ctx->painter->setBrush(metricPoints[i].color);
        ctx->painter->drawEllipse(QPoint(ctx->rect.left(), yCoord), pointRadius + metricPointExtraRadius, pointRadius + metricPointExtraRadius);
        ctx->painter->drawText(ctx->rect.left() - metricLabelXOffset, yCoord - metricLabelYOffset, metricPoints[i].str);
    }
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
    ctx->painter->drawRect(ctx->rect);
    ctx->painter->drawLine(ctx->rect.left(), ctx->rect.bottom(), ctx->rect.right(), ctx->rect.bottom());
    ctx->painter->drawLine(ctx->rect.left(), ctx->rect.bottom(), ctx->rect.left(), ctx->rect.top());

    ctx->painter->drawText(ctx->rect.center().x() - axisTitleXOffset, ctx->size.height() - axisTitleYOffset, "Year");
}

QList<QLine> buildLines(const LinkedList* points, DrawContext* ctx) {
    QList<QLine> lines;
    if (points != nullptr && points->size > 0) {
        lines.reserve(points->size > 1 ? points->size - 1 : 1);
        Iterator it = begin((LinkedList*)points);

        GraphPoint* firstPoint = (GraphPoint*)get(&it);
        int prevX = mapToX((int)firstPoint->x, (int)ctx->bounds.minX, (int)ctx->bounds.maxX, ctx->rect);
        int prevY = mapToY(firstPoint->y, ctx->bounds.minY, ctx->bounds.maxY, ctx->rect);

        next(&it);
        if (!hasNext(&it)) {
            lines.append(QLine(prevX, prevY, prevX, prevY));
        } else {
            while (hasNext(&it)) {
                GraphPoint* point = (GraphPoint*)get(&it);
                int x = mapToX((int)point->x, (int)ctx->bounds.minX, (int)ctx->bounds.maxX, ctx->rect);
                int y = mapToY(point->y, ctx->bounds.minY, ctx->bounds.maxY, ctx->rect);

                lines.append(QLine(prevX, prevY, x, y));

                prevX = x;
                prevY = y;
                next(&it);
            }
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

void drawPoints(DrawContext* ctx, const QList<QLine>* drawLines) {
    if (drawLines != nullptr && !drawLines->isEmpty()){
        ctx->painter->setBrush(QColor("#00FF00"));

        for (int i = 0; i < drawLines->size(); ++i)
            ctx->painter->drawEllipse(drawLines->at(i).p1(), pointRadius, pointRadius);

        if (drawLines->size() > 1 || drawLines->first().p1() != drawLines->first().p2()) {
            ctx->painter->drawEllipse(drawLines->last().p2(), pointRadius, pointRadius);
        }
    }
}

QPixmap buildGraphPixmap(QSize size, const LinkedList* points, Metrix metrix) {
    QPixmap pixmap(size);
    pixmap.fill(QColor("#000000"));
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (points == nullptr || points->size == 0)
        drawNoDataState(&painter, size);
    else {
        GraphBounds bounds = calculateGraphBounds(points);
        adjustGraphValueBounds(&bounds);
        adjustGraphYearBounds(&bounds);

        DrawContext ctx = { &painter, buildGraphRect(size), size, bounds, metrix };

        drawAxesAndTitles(&ctx);
        drawXAxisTicks(&ctx);
        drawMetricDecorations(&ctx);

        QList<QLine> lines = buildLines(points, &ctx);
        drawLines(&ctx, &lines);
        drawPoints(&ctx, &lines);

    }
    return pixmap;
}
