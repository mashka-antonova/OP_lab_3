#include "draw.h"

#include <QPainter>
#include <QPainterPath>

#define EPS 1e-9

bool isEqual(double left, double right) {
    return std::abs(left - right) <= EPS;
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

QPixmap buildGraphPixmap(const QSize& size, const QVector<GraphPoint>& points, const Metrix& metrix, const QString& yAxisLabel) {
    QPixmap pixmap(size);
    pixmap.fill(QColor("#000000"));
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (points.isEmpty())
        drawNoDataState(painter, size);
    else {
        int minYear = points.first().year;
        int maxYear = points.first().year;
        double minValue = points.first().value;
        double maxValue = points.first().value;

        for (const GraphPoint& point : points) {
            minYear = std::min(point.year, minYear);
            maxYear = std::max(point.year, maxYear);
            minValue = std::min(point.value, minValue);
            maxValue = std::max(point.value, maxValue);
        }

        if (isEqual(minValue, maxValue)) {
            minValue -= 1.0;
            maxValue += 1.0;
        }

        QRect rect(leftPadding, topPadding,
                   size.width() - leftPadding - rightPadding,
                   size.height() - topPadding - bottomPadding);

        painter.setPen(QPen(QColor("#00FF00"), 1));
        painter.drawRect(rect);

        painter.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
        painter.drawLine(rect.left(), rect.bottom(), rect.left(), rect.top());

        painter.drawText(rect.center().x() - 25, size.height() - 20, "Year");
        painter.save();
        painter.translate(20, rect.center().y() + 20);
        painter.rotate(-90);
        painter.drawText(0, 0, yAxisLabel);
        painter.restore();

        painter.drawText(rect.left() - 55, rect.bottom() + 20, QString::number(minYear));
        painter.drawText(rect.right() - 25, rect.bottom() + 20, QString::number(maxYear));
        painter.drawText(rect.left() - 65, rect.top() + 5, QString::number(maxValue, 'f', 2));
        painter.drawText(rect.left() - 65, rect.bottom() + 5, QString::number(minValue, 'f', 2));

        drawMetricLine(painter, rect, metrix.max, minValue, maxValue, QColor("#FF3333"), "Max");
        drawMetricLine(painter, rect, metrix.mediana, minValue, maxValue, QColor("#FFFF00"), "Median");
        drawMetricLine(painter, rect, metrix.min, minValue, maxValue, QColor("#3399FF"), "Min");

        painter.setPen(QPen(QColor("#00FF00"), 2));
        QPainterPath path;
        bool isFirst = true;
        for (const GraphPoint& point : points) {
            const int xCoord = mapToX(point.year, minYear, maxYear, rect);
            const int yCoord = mapToY(point.value, minValue, maxValue, rect);
            if (isFirst) {
                path.moveTo(xCoord, yCoord);
                isFirst = false;
            } else
                path.lineTo(xCoord, yCoord);
        }
        painter.drawPath(path);

        painter.setBrush(QColor("#00FF00"));
        for (const GraphPoint& point : points) {
            const int xCoord = mapToX(point.year, minYear, maxYear, rect);
            const int yCoord = mapToY(point.value, minValue, maxValue, rect);
            painter.drawEllipse(QPoint(xCoord, yCoord), pointRadius, pointRadius);
        }
    }
    return pixmap;
}
