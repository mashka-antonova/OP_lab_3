#include "draw.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>
#include <QList>
#include <QPoint>

extern "C" {
#include "iterator.h"
}

#define EPS 1e-9
#define VALUE_STEP 5.0
#define FALLBACK_VALUE_OFFSET 1.0

struct MetricMarker {
    double value;
    QColor color;
    QString str; //
};

bool isEqual(double a, double b) {
    return std::abs(a - b) <= EPS;
}

double roundDownToStep(double value, double step) {
    return std::floor(value / step) * step;
}

double roundUpToStep(double value, double step) {
    return std::ceil(value / step) * step;
}

int mapToX(int year, int minYear, int maxYear, const QRect& rect) {
    int xCoord = rect.left();
    if (maxYear > minYear) {
        double ratio = static_cast<double>(year - minYear) / static_cast<double>(maxYear - minYear);
        xCoord = rect.left() + static_cast<int>(ratio * rect.width());
    }
    return xCoord;
}

int mapToY(double value, double minValue, double maxValue, const QRect& rect) {
    int yCoord = rect.bottom();
    if (!isEqual(maxValue, minValue)) {
        double ratio = (value - minValue) / (maxValue - minValue);
        yCoord = rect.bottom() - static_cast<int>(ratio * rect.height());
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
        bounds.minYear = std::min(point->x, bounds.minYear);
        bounds.maxYear = std::max(point->x, bounds.maxYear);
        bounds.minValue = std::min(point->y, bounds.minValue);
        bounds.maxValue = std::max(point->y, bounds.maxValue);
        next(&it);
    }
    return bounds;
}

void drawNoDataState(QPainter& painter, const QSize& size) {
    painter.fillRect(QRect(QPoint(0, 0), size), QColor("#000000"));
    painter.setPen(QColor("#00FF00"));
    painter.drawRect(0, 0, size.width() - borderCorrection, size.height() - borderCorrection);
    painter.drawText(QRect(QPoint(0, 0), size), Qt::AlignCenter, "No data to draw");
}

void drawMetricLine(QPainter& painter,
                    const QRect& rect,
                    double value,
                    double minValue,
                    double maxValue,
                    const QColor& color,
                    const QString& label) {

    const int yCoord = mapToY(value, minValue, maxValue, rect);
    painter.setPen(QPen(color, thinLineWidth, Qt::DashLine));
    painter.drawLine(rect.left(), yCoord, rect.right(), yCoord);
    painter.drawText(rect.left() + textOffset, yCoord - textOffset, label);
}

void drawXAxisTicks(QPainter& painter, const QRect& rect, int minYear, int maxYear) {
    int yearRange = std::max(1, maxYear - minYear);
    int yearTickStep = std::max(1, static_cast<int>(std::ceil(static_cast<double>(yearRange) / maxAxisTicks)));

    painter.setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    int lastTickYear = minYear;
    for (int year = minYear; year <= maxYear; year += yearTickStep) {
        if (year == maxYear - borderCorrection)
            continue;

        lastTickYear = year;

        int xCoord = mapToX(year, minYear, maxYear, rect);
        painter.drawLine(xCoord, rect.bottom(), xCoord, rect.top());
        painter.setPen(QColor("#00FF00"));
        painter.drawText(xCoord - yearLabelXOffset, rect.bottom() + yearLabelYOffset, QString::number(year));
        painter.setPen(QPen(QColor("#00AA00"), thinLineWidth, Qt::DashLine));
    }

    if (lastTickYear != maxYear) {
        int xCoord = mapToX(maxYear, minYear, maxYear, rect);
        painter.drawLine(xCoord, rect.bottom(), xCoord, rect.top());
        painter.setPen(QColor("#00FF00"));
        painter.drawText(xCoord - yearLabelXOffset, rect.bottom() + yearLabelYOffset, QString::number(maxYear));
    }
}

void drawMetricDecorations(QPainter& painter, const QRect& rect, const Metrix& metrix, double minValue, double maxValue) {
    painter.drawText(rect.left() - boundsLabelXOffset, rect.top() + boundsLabelYOffset, QString::number(maxValue, 'f', 2));
    painter.drawText(rect.left() - boundsLabelXOffset, rect.bottom() + boundsLabelYOffset, QString::number(minValue, 'f', 2));

    drawMetricLine(painter, rect, metrix.max, minValue, maxValue, QColor("#FF3333"), "Max");
    drawMetricLine(painter, rect, metrix.mediana, minValue, maxValue, QColor("#FFFF00"), "Median");
    drawMetricLine(painter, rect, metrix.min, minValue, maxValue, QColor("#3399FF"), "Min");

    const MetricMarker metricPoints[] = {
        {metrix.max, QColor("#FF3333"), QString::number(metrix.max, 'f', 2)},
        {metrix.mediana, QColor("#FFFF00"), QString::number(metrix.mediana, 'f', 2)},
        {metrix.min, QColor("#3399FF"), QString::number(metrix.min, 'f', 2)}
    };

    for (const auto& metricPoint : metricPoints) {
        int yCoord = mapToY(metricPoint.value, minValue, maxValue, rect);
        painter.setPen(QPen(metricPoint.color, thickLineWidth));
        painter.setBrush(metricPoint.color);
        painter.drawEllipse(QPoint(rect.left(), yCoord), pointRadius + metricPointExtraRadius, pointRadius + metricPointExtraRadius);
        painter.drawText(rect.left() - metricLabelXOffset, yCoord - metricLabelYOffset, metricPoint.str);
    }
}

void adjustGraphValueBounds(GraphBounds& bounds) {
    if (isEqual(bounds.minValue, bounds.maxValue)) {
        bounds.minValue -= FALLBACK_VALUE_OFFSET;
        bounds.maxValue += FALLBACK_VALUE_OFFSET;
    }

    bounds.minValue = roundDownToStep(bounds.minValue, VALUE_STEP) - VALUE_STEP;
    bounds.maxValue = roundUpToStep(bounds.maxValue, VALUE_STEP) + VALUE_STEP;
}

QRect buildGraphRect(const QSize& size) {
    return QRect(leftPadding, topPadding,
                 size.width() - leftPadding - rightPadding,
                 size.height() - topPadding - bottomPadding);
}

void drawAxesAndTitles(QPainter& painter, const QRect& rect, const QSize& size) {
    painter.setPen(QPen(QColor("#00FF00"), thinLineWidth));
    painter.drawRect(rect);
    painter.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    painter.drawLine(rect.left(), rect.bottom(), rect.left(), rect.top());

    painter.drawText(rect.center().x() - axisTitleXOffset, size.height() - axisTitleYOffset, "Year");
    painter.save();
}

QList<QPoint> buildDrawPoints(const LinkedList* points, const GraphBounds& bounds, const QRect& rect) {
    QList<QPoint> drawPoints;
    if (points != nullptr) {
        drawPoints.reserve(points->size);
        Iterator it = begin((LinkedList*)points);
        while (hasNext(&it)) {
            GraphPoint* point = (GraphPoint*)get(&it);
            int x = mapToX(point->x, bounds.minYear, bounds.maxYear, rect);
            int y = mapToY(point->y, bounds.minValue, bounds.maxValue, rect);
            drawPoints.append(QPoint(x, y));
            next(&it);
        }
    }
    return drawPoints;
}

void drawDataLine(QPainter& painter, const QList<QPoint>& drawPoints) {
    if (drawPoints.isEmpty())
        return;

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#00FF00"), thickLineWidth));
    QPainterPath path;
    path.moveTo(drawPoints.first());
    for (int i = 1; i < drawPoints.size(); ++i)
        path.lineTo(drawPoints[i]);

    painter.drawPath(path);
}

void drawDataPoints(QPainter& painter, const QList<QPoint>& drawPoints) {
    painter.setBrush(QColor("#00FF00"));
    for (const QPoint& drawPoint : drawPoints)
        painter.drawEllipse(drawPoint, pointRadius, pointRadius);
}

QPixmap buildGraphPixmap(const QSize& size, const LinkedList* points, const Metrix& metrix) {
    QPixmap pixmap(size);
    pixmap.fill(QColor("#000000"));
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (points == nullptr || points->size == 0)
        drawNoDataState(painter, size);
    else {
        GraphBounds bounds = calculateGraphBounds(points);
        adjustGraphValueBounds(bounds);

        const QRect rect = buildGraphRect(size);
        drawAxesAndTitles(painter, rect, size);
        drawXAxisTicks(painter, rect, bounds.minYear, bounds.maxYear);
        drawMetricDecorations(painter, rect, metrix, bounds.minValue, bounds.maxValue);
        const QList<QPoint> drawPoints = buildDrawPoints(points, bounds, rect);
        drawDataLine(painter, drawPoints);
        drawDataPoints(painter, drawPoints);
    }
    return pixmap;
}
