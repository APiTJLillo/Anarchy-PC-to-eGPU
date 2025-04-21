#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QCustomPlot>
#include "../core/device.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Connection operations
    void connectDevice();
    void disconnectDevice();
    void resetConnection();
    void optimizePerformance();

    // Status updates
    void updateDeviceStatus();
    void updatePerformanceGraphs();
    void updateSystemLog();

    // Error handling
    void handleError(const QString& error);
    void showErrorDialog(const QString& message);

private:
    void setupUi();
    void setupConnections();
    void setupGraphs();
    void loadSettings();
    void saveSettings();

    // UI components
    Ui::MainWindow *ui;
    QLabel *statusLabel;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *resetButton;
    QPushButton *optimizeButton;
    QTextEdit *logViewer;
    QCustomPlot *throughputGraph;
    QCustomPlot *latencyGraph;

    // Core components
    Device *device;
    QTimer *updateTimer;
    bool isConnected;
    bool isOptimized;
};

#endif // MAINWINDOW_H 