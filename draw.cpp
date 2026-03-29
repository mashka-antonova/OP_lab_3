#include "draw.h"

#include <QPainter>
#include <QPainterPath>
#include <cmath>

extern "C" {
#include "iterator.h"
}

#define EPS 1e-9

bool isEqual(double left, double right) {
    return std::abs(left - right) <= EPS;
}

double roundDownToStep(double value, double step) {
    return std::floor(value / step) * step;
}

double roundUpToStep(double value, double step) {
    return std::ceil(value / step) * step;
}

int mapToX(int year, int minYear, int maxYear, const QRect& drawRect) {
    int xCoord = drawRect.left();
    if (maxYear > minYear) {
        const double ratio = static_cast<double>(year - minYear) / static_cast<double>(maxYear - minYear);
        xCoord = drawRect.left() + static_cast<int>(ratio * drawRect.width());
    }
    return xCoord;
}

int mapToY(double value, double minValue, double maxValue, const QRect& drawRect) {
    int yCoord = drawRect.bottom();
    if (!isEqual(maxValue, minValue)) {
        const double ratio = (value - minValue) / (maxValue - minValue);
        yCoord = drawRect.bottom() - static_cast<int>(ratio * drawRect.height());
    }
    return yCoord;
}

GraphBounds calculateGraphBounds(const LinkedList* points) {
    GraphBounds bounds;
    Iterator it = begin((LinkedList*)points);
    GraphPoint* firstPoint = (GraphPoint*)get(&it);
    bounds.minYear = firstPoint->year;
    bounds.maxYear = firstPoint->year;
    bounds.minValue = firstPoint->value;
    bounds.maxValue = firstPoint->value;

    while (hasNext(&it)) {
        GraphPoint* point = (GraphPoint*)get(&it);
        bounds.minYear = std::min(point->year, bounds.minYear);
        bounds.maxYear = std::max(point->year, bounds.maxYear);
        bounds.minValue = std::min(point->value, bounds.minValue);
        bounds.maxValue = std::max(point->value, bounds.maxValue);
        next(&it);
    }
    return bounds;
}

void drawNoDataState(QPainter& painter, const QSize& size) {
    painter.fillRect(QRect(QPoint(0, 0), size), QColor("#000000"));
    painter.setPen(QColor("#00FF00"));
    painter.drawRect(0, 0, size.width() - 1, size.height() - 1);
    painter.drawText(QRect(QPoint(0, 0), size), Qt::AlignCenter, "No data to draw");
}

void drawMetricLine(QPainter& painter,
                    const QRect& drawRect,
                    double value,
                    double minValue,
                    double maxValue,
                    const QColor& color,
                    const QString& label) {

    const int yCoord = mapToY(value, minValue, maxValue, drawRect);
    painter.setPen(QPen(color, 1, Qt::DashLine));
    painter.drawLine(drawRect.left(), yCoord, drawRect.right(), yCoord);
    painter.drawText(drawRect.left() + textOffset, yCoord - textOffset, label);
}

void drawXAxisTicks(QPainter& painter, const QRect& rect, int minYear, int maxYear) {
    const int yearRange = std::max(1, maxYear - minYear);
    const int maxTicks = 18;
    int yearTickStep = std::max(1, static_cast<int>(std::ceil(static_cast<double>(yearRange) / maxTicks)));

    painter.setPen(QPen(QColor("#00AA00"), 1, Qt::DashLine));
    int lastTickYear = minYear;
    for (int year = minYear; year <= maxYear; year += yearTickStep) {
        if (year == maxYear - 1)
            continue;

        lastTickYear = year;

        const int xCoord = mapToX(year, minYear, maxYear, rect);
        painter.drawLine(xCoord, rect.bottom(), xCoord, rect.top());
        painter.setPen(QColor("#00FF00"));
        painter.drawText(xCoord - 16, rect.bottom() + 20, QString::number(year));
        painter.setPen(QPen(QColor("#00AA00"), 1, Qt::DashLine));
    }

    if (lastTickYear != maxYear) {
        const int xCoord = mapToX(maxYear, minYear, maxYear, rect);
        painter.drawLine(xCoord, rect.bottom(), xCoord, rect.top());
        painter.setPen(QColor("#00FF00"));
        painter.drawText(xCoord - 16, rect.bottom() + 20, QString::number(maxYear));
    }
}

void drawMetricDecorations(QPainter& painter, const QRect& rect, const Metrix& metrix, double minValue, double maxValue) {
    painter.drawText(rect.left() - 65, rect.top() + 5, QString::number(maxValue, 'f', 2));
    painter.drawText(rect.left() - 65, rect.bottom() + 5, QString::number(minValue, 'f', 2));

    drawMetricLine(painter, rect, metrix.max, minValue, maxValue, QColor("#FF3333"), "Max");
    drawMetricLine(painter, rect, metrix.mediana, minValue, maxValue, QColor("#FFFF00"), "Median");
    drawMetricLine(painter, rect, metrix.min, minValue, maxValue, QColor("#3399FF"), "Min");

    const struct {
        double value;
        QColor color;
        QString label;
    } metricPoints[] = {
        {metrix.max, QColor("#FF3333"), QString::number(metrix.max, 'f', 2)},
        {metrix.mediana, QColor("#FFFF00"), QString::number(metrix.mediana, 'f', 2)},
        {metrix.min, QColor("#3399FF"), QString::number(metrix.min, 'f', 2)}
    };

    for (const auto& metricPoint : metricPoints) {
        const int yCoord = mapToY(metricPoint.value, minValue, maxValue, rect);
        painter.setPen(QPen(metricPoint.color, 2));
        painter.setBrush(metricPoint.color);
        painter.drawEllipse(QPoint(rect.left(), yCoord), pointRadius + 1, pointRadius + 1);
        painter.drawText(rect.left() - 50, yCoord - 8, metricPoint.label);
    }
}

void adjustGraphValueBounds(GraphBounds& bounds) {
    if (isEqual(bounds.minValue, bounds.maxValue)) {
        bounds.minValue -= 1.0;
        bounds.maxValue += 1.0;
    }

    constexpr double yStep = 5.0;
    bounds.minValue = roundDownToStep(bounds.minValue, yStep) - yStep;
    bounds.maxValue = roundUpToStep(bounds.maxValue, yStep) + yStep;
    if (isEqual(bounds.minValue, bounds.maxValue))
        bounds.maxValue += yStep;
}

QRect buildGraphRect(const QSize& size) {
    return QRect(leftPadding, topPadding,
                 size.width() - leftPadding - rightPadding,
                 size.height() - topPadding - bottomPadding);
}

void drawAxesAndTitles(QPainter& painter, const QRect& rect, const QSize& size) {
    painter.setPen(QPen(QColor("#00FF00"), 1));
    painter.drawRect(rect);
    painter.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    painter.drawLine(rect.left(), rect.bottom(), rect.left(), rect.top());

    painter.drawText(rect.center().x() - 25, size.height() - 20, "Year");
    painter.save();
}

void drawDataLine(QPainter& painter, const LinkedList* points, const GraphBounds& bounds, const QRect& rect) {
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#00FF00"), 2));
    QPainterPath path;
    bool isFirst = true;
    Iterator it = begin((LinkedList*)points);
    while (hasNext(&it)) {
        GraphPoint* point = (GraphPoint*)get(&it);
        const int xCoord = mapToX(point->year, bounds.minYear, bounds.maxYear, rect);
        const int yCoord = mapToY(point->value, bounds.minValue, bounds.maxValue, rect);
        if (isFirst) {
            path.moveTo(xCoord, yCoord);
            isFirst = false;
        } else
            path.lineTo(xCoord, yCoord);
        next(&it);
    }
    painter.drawPath(path);
}

void drawDataPoints(QPainter& painter, const LinkedList* points, const GraphBounds& bounds, const QRect& rect) {
    painter.setBrush(QColor("#00FF00"));
    Iterator it = begin((LinkedList*)points);
    while (hasNext(&it)) {
        GraphPoint* point = (GraphPoint*)get(&it);
        const int xCoord = mapToX(point->year, bounds.minYear, bounds.maxYear, rect);
        const int yCoord = mapToY(point->value, bounds.minValue, bounds.maxValue, rect);
        painter.drawEllipse(QPoint(xCoord, yCoord), pointRadius, pointRadius);
        next(&it);
    }
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
        drawDataLine(painter, points, bounds, rect);
        drawDataPoints(painter, points, bounds, rect);
    }
    return pixmap;
}
