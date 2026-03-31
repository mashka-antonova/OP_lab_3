#ifndef MAINWINDOW_H
#define MAINWINDOW_H

extern "C" {
#include "appcontext.h"
#include "entrypoint.h"
#include "iterator.h"
#include "demography.h"
}

#include <QMainWindow>
#include <QTableWidget>
#include "draw.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    AppContext context;

    void selectFileClicked();
    void loadDataClicked();
    void calculateMetricsClicked();
    void onRegionChanged(const QString &region);
    void tableItemDoubleClicked(QTableWidgetItem *item);

    QString errorText(Status error);
    void showError();
    void updateTable(const QString& region);
    void updateGraph(const LinkedList* points);
    void resizeEvent(QResizeEvent* event);
};

#endif // MAINWINDOW_H
