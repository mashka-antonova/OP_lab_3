#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>
#include <QClipboard>
#include <QResizeEvent>
#include <QStandardPaths>

#define defaultGraphWidth 480
#define defaultGraphHeight 320

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icon/photo.png"));
    this->setWindowTitle("ROBCO: DEMOGRAPHIC MODULE");

    connect(ui->selectFile, &QToolButton::clicked, this, &MainWindow::selectFileClicked);
    connect(ui->loadData, &QPushButton::clicked, this, &MainWindow::loadDataClicked);
    connect(ui->calculateMetrix, &QPushButton::clicked, this, &MainWindow::calculateMetricsClicked);
    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &MainWindow::tableItemDoubleClicked);
    connect(ui->regionInput, &QComboBox::currentTextChanged, this, &MainWindow::updateTable);

    doOperation(INITIALIZATION, &context, NULL);

    ui->columnInput->clear();
    ui->columnInput->addItem("Year", COL_YEAR);
    ui->columnInput->addItem("Natural Population Growth", COL_NPG);
    ui->columnInput->addItem("Birth Rate", COL_BIRTH_RATE);
    ui->columnInput->addItem("Death Rate", COL_DEATH_RATE);
    ui->columnInput->addItem("General Demographic Weight", COL_GDW);
    ui->columnInput->addItem("Urbanization", COL_URBANIZATION);

    ui->tableWidget->setColumnCount(COL_COUNT);
    ui->tableWidget->setHorizontalHeaderLabels({
        "Year", "Region", "Nat.Growth", "Birth Rate",
        "Death Rate", "Dem.Weight", "Urbanization"
    });
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->graph->setMinimumSize(defaultGraphWidth, defaultGraphHeight);
    ui->graph->setPixmap(buildGraphPixmap(ui->graph->size(), context.graphPoints, context.metrix));
}

MainWindow::~MainWindow()
{
    doOperation(DISPOSE_CONTEXT, &context, NULL);
    delete ui;
}

QString MainWindow::errorText(Status error){
    QString str;
    switch(error){
    case STATUS_OK:
        str = "";
        break;
    case ERR_MALLOC_FAILED:
        str = "Memory allocation failed";
        break;
    case ERR_FILE_OPEN:
        str = "Failed to open file";
        break;
    case ERR_EMPTY_DATA:
        str = "The loaded file is empty";
        break;
    case ERR_INVALID_REGION:
        str = "Invalid region";
        break;
    case ERR_INVALID_COLUMN:
        str = "Invalid column";
        break;
    case ERR_INVALID_HEADER:
        str = "Invalid header row";
        break;
    default:
        str = "Unknown error";
        break;
    }
    return str;
}

void MainWindow::showError(){
    ui->outputErrorLabel->setText(errorText(context.programmStatus));
}

void MainWindow::selectFileClicked()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString initialDir = desktopPath + "/OP";
    QString fileName = QFileDialog::getOpenFileName(this, "Choose CSV file", initialDir, "CSV Files (*.csv);;All Files (*)");

    if (!fileName.isEmpty())
        ui->filePath->setText(fileName);
    else
        context.programmStatus = ERR_EMPTY_DATA;
}

void MainWindow::loadDataClicked()
{
    std::string str = ui->filePath->text().toStdString();
    const char* cStr = str.c_str();

    if (str.empty())
        ui->outputErrorLabel->setText("File not uploaded");
    else {
        AppParams params;
        params.str = cStr;
        doOperation(LOAD_DATA, &context, &params);
        showError();

        if (context.programmStatus == STATUS_OK) {
            ui->regionInput->blockSignals(true);
            ui->regionInput->clear();
            ui->regionInput->addItem("");

            if (context.list != nullptr) {
                Iterator it = begin(context.list);
                QString lastRegion = "";
                while (hasNext(&it)) {
                    DemographicRecord* record = (DemographicRecord*)get(&it);
                    QString currentRegion = QString::fromUtf8(record->region);
                    if (currentRegion != lastRegion) {
                        ui->regionInput->addItem(currentRegion);
                        lastRegion = currentRegion;
                    }
                    next(&it);
                }
            }
            ui->regionInput->blockSignals(false);
        }

        int successRows = context.stats.totalRows - context.stats.errorRows;
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("TERMINAL NOTIFICATION");
        msgBox.setText(QString("FILE ANALYSIS COMPLETE:\n\n"
                               "TOTAL RECORDS: %1\n"
                               "SUCCESSFULLY READ: %2\n"
                               "ERRORS: %3")
                           .arg(context.stats.totalRows)
                           .arg(successRows)
                           .arg(context.stats.errorRows));

        msgBox.setStyleSheet(
            "QMessageBox {"
            "   background-color: #000000;"
            "   border: 2px solid #00FF00;"
            "}"
            "QLabel {"
            "   color: #00FF00;"
            "   font-family: 'Courier New';"
            "   font-size: 11pt;"
            "}"
            "QPushButton {"
            "   background-color: #000000;"
            "   color: #00FF00;"
            "   border: 1px solid #00FF00;"
            "   padding: 5px 15px;"
            "   min-width: 80px;"
            "   font-family: 'Courier New';"
            "   font-weight: bold;"
            "   text-transform: uppercase;"
            "}"
            "QPushButton:hover {"
            "   background-color: #00FF00;"
            "   color: #000000;"
            "}"
            );
        msgBox.exec();
        updateTable(ui->regionInput->currentText().trimmed());
    }
}

void MainWindow::updateTable(const QString& region) {
    ui->tableWidget->setRowCount(0);

    if (context.list == nullptr)
        return;

    bool isRegionFound = false;
    bool isRegionEmpty = region.isEmpty();

    Iterator it = begin(context.list);
    int row = 0;
    while (hasNext(&it)) {
        DemographicRecord* record = (DemographicRecord*)get(&it);
        QString recordRegion = QString::fromUtf8(record->region);

        if (isRegionEmpty || recordRegion.compare(region, Qt::CaseInsensitive) == 0) {
            if (!isRegionEmpty)
                isRegionFound = true;
            ui->tableWidget->insertRow(row);
            ui->tableWidget->setItem(row, COL_YEAR, new QTableWidgetItem(QString::number(record->year)));
            ui->tableWidget->setItem(row, COL_REGION, new QTableWidgetItem(recordRegion));
            ui->tableWidget->setItem(row, COL_NPG, new QTableWidgetItem(QString::number(record->natural_population_growth)));
            ui->tableWidget->setItem(row, COL_BIRTH_RATE, new QTableWidgetItem(QString::number(record->birth_rate)));
            ui->tableWidget->setItem(row, COL_DEATH_RATE, new QTableWidgetItem(QString::number(record->death_rate)));
            ui->tableWidget->setItem(row, COL_GDW, new QTableWidgetItem(QString::number(record->general_demographic_weight)));
            ui->tableWidget->setItem(row, COL_URBANIZATION, new QTableWidgetItem(QString::number(record->urbanization)));
            row++;
        }
        next(&it);
    }
    if (!isRegionEmpty && !isRegionFound)
        ui->outputErrorLabel->setText("This region does not exist");
    else
        ui->outputErrorLabel->clear();
}

void MainWindow::calculateMetricsClicked() {
    std::string str = ui->regionInput->currentText().trimmed().toStdString();
    const char* cStr = str.c_str();
    Column column = static_cast<Column>(ui->columnInput->currentData().toInt());

    if (str.empty())
        ui->outputErrorLabel->setText("Empty region. To calculate metrix need region");
    else {
        AppParams params = {.str = cStr, .column = column};
        doOperation(CALCULATE_METRICS, &context, &params);
        showError();

        if (context.programmStatus == STATUS_OK) {
            ui->minimum->setText(QString::number(context.metrix.min));
            ui->maximum->setText(QString::number(context.metrix.max));
            ui->mediana->setText(QString::number(context.metrix.mediana));
            updateGraph(context.graphPoints);
        } else {
            ui->minimum->clear();
            ui->maximum->clear();
            ui->mediana->clear();
            ui->graph->setPixmap(buildGraphPixmap(ui->graph->size(), context.graphPoints, context.metrix));
        }
    }
}

void MainWindow::tableItemDoubleClicked(QTableWidgetItem *item) {
    if (item && item->column() == COL_REGION)
        QGuiApplication::clipboard()->setText(item->text());
}

void MainWindow::updateGraph(const LinkedList* points) {
    ui->graph->setPixmap(buildGraphPixmap(ui->graph->size(), points, context.metrix));
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateGraph(context.graphPoints);
}
