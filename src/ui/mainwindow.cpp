#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , device(new Device(this))
    , updateTimer(new QTimer(this))
    , isConnected(false)
    , isOptimized(false)
{
    setupUi();
    setupConnections();
    setupGraphs();
    loadSettings();

    // Start update timer
    updateTimer->setInterval(1000);  // 1 second updates
    updateTimer->start();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::setupUi()
{
    ui->setupUi(this);

    // Initialize status bar
    statusLabel = new QLabel(this);
    ui->statusbar->addWidget(statusLabel);
    statusLabel->setText("Disconnected");

    // Initialize control buttons
    connectButton = ui->connectButton;
    disconnectButton = ui->disconnectButton;
    resetButton = ui->resetButton;
    optimizeButton = ui->optimizeButton;

    // Initialize graphs
    throughputGraph = ui->throughputGraph;
    latencyGraph = ui->latencyGraph;

    // Initialize log viewer
    logViewer = ui->logViewer;

    // Set initial button states
    disconnectButton->setEnabled(false);
    resetButton->setEnabled(false);
    optimizeButton->setEnabled(false);
}

void MainWindow::setupConnections()
{
    // Connect button signals
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::connectDevice);
    connect(disconnectButton, &QPushButton::clicked, this, &MainWindow::disconnectDevice);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetConnection);
    connect(optimizeButton, &QPushButton::clicked, this, &MainWindow::optimizePerformance);

    // Connect device signals
    connect(device, &Device::connected, this, [this]() {
        isConnected = true;
        updateDeviceStatus();
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        resetButton->setEnabled(true);
        optimizeButton->setEnabled(true);
        statusLabel->setText("Connected");
    });

    connect(device, &Device::disconnected, this, [this]() {
        isConnected = false;
        updateDeviceStatus();
        connectButton->setEnabled(true);
        disconnectButton->setEnabled(false);
        resetButton->setEnabled(false);
        optimizeButton->setEnabled(false);
        statusLabel->setText("Disconnected");
    });

    connect(device, &Device::error, this, &MainWindow::handleError);
    connect(device, &Device::statsUpdated, this, [this](const DeviceStats& stats) {
        updatePerformanceGraphs();
        updateSystemLog();
    });

    // Connect timer for updates
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        if (isConnected) {
            updateDeviceStatus();
        }
    });
}

void MainWindow::setupGraphs()
{
    // Setup throughput graph
    throughputGraph->addGraph(); // TX
    throughputGraph->addGraph(); // RX
    throughputGraph->graph(0)->setPen(QPen(QColor(0, 188, 212)));  // TX color
    throughputGraph->graph(1)->setPen(QPen(QColor(124, 77, 255))); // RX color
    throughputGraph->xAxis->setLabel("Time (s)");
    throughputGraph->yAxis->setLabel("Throughput (MB/s)");
    throughputGraph->setInteraction(QCP::iRangeDrag, true);
    throughputGraph->setInteraction(QCP::iRangeZoom, true);

    // Setup latency graph
    latencyGraph->addGraph();
    latencyGraph->graph(0)->setPen(QPen(QColor(255, 152, 0)));
    latencyGraph->xAxis->setLabel("Time (s)");
    latencyGraph->yAxis->setLabel("Latency (ns)");
    latencyGraph->setInteraction(QCP::iRangeDrag, true);
    latencyGraph->setInteraction(QCP::iRangeZoom, true);
}

void MainWindow::connectDevice()
{
    if (!device->connect()) {
        showErrorDialog("Failed to connect to device");
        return;
    }
}

void MainWindow::disconnectDevice()
{
    if (!device->disconnect()) {
        showErrorDialog("Failed to disconnect device");
        return;
    }
}

void MainWindow::resetConnection()
{
    if (!device->reset()) {
        showErrorDialog("Failed to reset connection");
        return;
    }
    logViewer->append("Connection reset successful");
}

void MainWindow::optimizePerformance()
{
    if (!device->optimize()) {
        showErrorDialog("Failed to optimize performance");
        return;
    }
    isOptimized = true;
    optimizeButton->setEnabled(false);
    logViewer->append("Performance optimization applied");
}

void MainWindow::updateDeviceStatus()
{
    if (!isConnected) {
        return;
    }

    DeviceStats stats = device->getStats();

    // Update status bar
    QString status = QString("TX: %1 MB/s | RX: %2 MB/s | Latency: %3 ns | PCIe: Gen%4 | Temp: %5°C")
        .arg(stats.txThroughput, 0, 'f', 1)
        .arg(stats.rxThroughput, 0, 'f', 1)
        .arg(stats.latency, 0, 'f', 0)
        .arg(stats.pcieLinkSpeed)
        .arg(stats.temperature);
    statusLabel->setText(status);
}

void MainWindow::updatePerformanceGraphs()
{
    static QVector<double> times;
    static QVector<double> txData;
    static QVector<double> rxData;
    static QVector<double> latencyData;
    static double startTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;

    if (!isConnected) {
        return;
    }

    DeviceStats stats = device->getStats();
    double currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0 - startTime;

    // Add new data points
    times.append(currentTime);
    txData.append(stats.txThroughput);
    rxData.append(stats.rxThroughput);
    latencyData.append(stats.latency);

    // Remove old data points (keep last 60 seconds)
    while (times.size() > 60) {
        times.removeFirst();
        txData.removeFirst();
        rxData.removeFirst();
        latencyData.removeFirst();
    }

    // Update throughput graph
    throughputGraph->graph(0)->setData(times, txData);
    throughputGraph->graph(1)->setData(times, rxData);
    throughputGraph->rescaleAxes();
    throughputGraph->replot();

    // Update latency graph
    latencyGraph->graph(0)->setData(times, latencyData);
    latencyGraph->rescaleAxes();
    latencyGraph->replot();
}

void MainWindow::updateSystemLog()
{
    if (!isConnected) {
        return;
    }

    DeviceStats stats = device->getStats();
    QString logEntry = QString("[%1] %2 | TX: %3 MB/s, RX: %4 MB/s, Latency: %5 ns")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(stats.connectionStatus)
        .arg(stats.txThroughput, 0, 'f', 1)
        .arg(stats.rxThroughput, 0, 'f', 1)
        .arg(stats.latency, 0, 'f', 0);

    logViewer->append(logEntry);
}

void MainWindow::handleError(const QString& error)
{
    logViewer->append(QString("[ERROR] %1").arg(error));
    showErrorDialog(error);
}

void MainWindow::showErrorDialog(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::loadSettings()
{
    QSettings settings("Anarchy", "eGPU");
    
    // Load window geometry
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    // Load device settings
    device->setDMAChannels(settings.value("dma/channels", 8).toInt());
    device->setRingBufferSize(settings.value("dma/ringSize", 256).toInt());
    device->setPCIeLinkSpeed(settings.value("pcie/linkSpeed", 0).toInt());
    device->setThunderboltTimeout(settings.value("thunderbolt/timeout", 1000).toInt());
}

void MainWindow::saveSettings()
{
    QSettings settings("Anarchy", "eGPU");
    
    // Save window geometry
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    // Save device settings
    DeviceStats stats = device->getStats();
    settings.setValue("dma/channels", stats.dmaChannels);
    settings.setValue("dma/ringSize", stats.ringBufferSize);
    settings.setValue("pcie/linkSpeed", stats.pcieLinkSpeed);
    settings.setValue("thunderbolt/timeout", stats.tbTimeout);
} 