#include "draw.h"
#include <QPainter>
#include <QPainterPath>
#include <QList>
#include <QPoint>
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
    bounds.minYear = firstPoint->x;
    bounds.maxYear = firstPoint->x;
    bounds.minValue = firstPoint->y;
    bounds.maxValue = firstPoint->y;

    while (hasNext(&it)) {
        GraphPoint* point = (GraphPoint*)get(&it);
        bounds.minYear = getMinDouble(point->x, bounds.minYear);
        bounds.maxYear = getMaxDouble(point->x, bounds.maxYear);
        bounds.minValue = getMinDouble(point->y, bounds.minValue);
        bounds.maxValue = getMaxDouble(point->y, bounds.maxValue);
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
    const int yCoord = mapToY(value, ctx->bounds.minValue, ctx->bounds.maxValue, ctx->rect);
    ctx->painter->setPen(QPen(color, thinLineWidth, Qt::DashLine));
    ctx->painter->drawLine(ctx->rect.left(), yCoord, ctx->rect.right(), yCoord);
    ctx->painter->drawText(ctx->rect.left() + textOffset, yCoord - textOffset, label);
}

void drawXAxisTicks(DrawContext* ctx) {
    int minYear = (int)ctx->bounds.minYear;
    int maxYear = (int)ctx->bounds.maxYear;

    int yearRange = getMaxInt(1, maxYear - minYear);
    int yearTickStep = getMaxInt(1, (int)ceil((double)yearRange / maxAxisTicks));

    ctx->painter->setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    int lastTickYear = minYear;
    for (int year = minYear; year <= maxYear; year += yearTickStep) {
        if (year == maxYear - borderCorrection)
            continue;

        lastTickYear = year;

        int xCoord = mapToX(year, minYear, maxYear, ctx->rect);
        ctx->painter->drawLine(xCoord, ctx->rect.bottom(), xCoord, ctx->rect.top());
        ctx->painter->setPen(QColor("#00FF00"));
        ctx->painter->drawText(xCoord - yearLabelXOffset, ctx->rect.bottom() + yearLabelYOffset, QString::number(year));
        ctx->painter->setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    }

    if (lastTickYear != maxYear) {
        int xCoord = mapToX(maxYear, minYear, maxYear, ctx->rect);
        ctx->painter->drawLine(xCoord, ctx->rect.bottom(), xCoord, ctx->rect.top());
        ctx->painter->setPen(QColor("#00FF00"));
        ctx->painter->drawText(xCoord - yearLabelXOffset, ctx->rect.bottom() + yearLabelYOffset, QString::number(maxYear));
    }
}

void drawMetricDecorations(DrawContext* ctx) {
    ctx->painter->drawText(ctx->rect.left() - boundsLabelXOffset, ctx->rect.top() + boundsLabelYOffset, QString::number(ctx->bounds.maxValue, 'f', 2));
    ctx->painter->drawText(ctx->rect.left() - boundsLabelXOffset, ctx->rect.bottom() + boundsLabelYOffset, QString::number(ctx->bounds.minValue, 'f', 2));

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
        int yCoord = mapToY(metricPoints[i].value, ctx->bounds.minValue, ctx->bounds.maxValue, ctx->rect);
        ctx->painter->setPen(QPen(metricPoints[i].color, thickLineWidth));
        ctx->painter->setBrush(metricPoints[i].color);
        ctx->painter->drawEllipse(QPoint(ctx->rect.left(), yCoord), pointRadius + metricPointExtraRadius, pointRadius + metricPointExtraRadius);
        ctx->painter->drawText(ctx->rect.left() - metricLabelXOffset, yCoord - metricLabelYOffset, metricPoints[i].str);
    }
}

void adjustGraphValueBounds(GraphBounds* bounds) {
    if (isEqual(bounds->minValue, bounds->maxValue)) {
        bounds->minValue -= FALLBACK_VALUE_OFFSET;
        bounds->maxValue += FALLBACK_VALUE_OFFSET;
    }

    bounds->minValue = roundDownToStep(bounds->minValue, VALUE_STEP) - VALUE_STEP;
    bounds->maxValue = roundUpToStep(bounds->maxValue, VALUE_STEP) + VALUE_STEP;
}

void adjustGraphYearBounds(GraphBounds* bounds) {
    if (isEqual(bounds->minYear, bounds->maxYear)) {
        bounds->minYear -= 1.0;
        bounds->maxYear += 1.0;
    } else {
        int offset = getMaxInt(1, (int)((bounds->maxYear - bounds->minYear) * 0.05));
        bounds->minYear -= offset;
        bounds->maxYear += offset;
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

QList<QPoint> buildDrawPoints(const LinkedList* points, DrawContext* ctx) {
    QList<QPoint> drawPoints;
    if (points != nullptr) {
        drawPoints.reserve(points->size);
        Iterator it = begin((LinkedList*)points);
        while (hasNext(&it)) {
            GraphPoint* point = (GraphPoint*)get(&it);
            int x = mapToX((int)point->x, (int)ctx->bounds.minYear, (int)ctx->bounds.maxYear, ctx->rect);
            int y = mapToY(point->y, ctx->bounds.minValue, ctx->bounds.maxValue, ctx->rect);
            drawPoints.append(QPoint(x, y));
            next(&it);
        }
    }
    return drawPoints;
}

void drawDataLine(DrawContext* ctx, const QList<QPoint>* drawPoints) {
    if (drawPoints != nullptr && !drawPoints->isEmpty()) {
        ctx->painter->setBrush(Qt::NoBrush);
        ctx->painter->setPen(QPen(QColor("#00FF00"), thickLineWidth));
        QPainterPath path;
        path.moveTo(drawPoints->first());
        for (int i = 1; i < drawPoints->size(); ++i)
            path.lineTo(drawPoints->at(i));

        ctx->painter->drawPath(path);
    }
}

void drawDataPoints(DrawContext* ctx, const QList<QPoint>* drawPoints) {
    if (drawPoints != nullptr){
        ctx->painter->setBrush(QColor("#00FF00"));
        for (int i = 0; i < drawPoints->size(); ++i)
            ctx->painter->drawEllipse(drawPoints->at(i), pointRadius, pointRadius);
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

        QList<QPoint> drawPoints = buildDrawPoints(points, &ctx);
        drawDataLine(&ctx, &drawPoints);
        drawDataPoints(&ctx, &drawPoints);
    }
    return pixmap;
}
