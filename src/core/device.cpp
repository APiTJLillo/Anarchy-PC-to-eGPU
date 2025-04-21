#include "device.h"
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QTextStream>
#include <QDir>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

Device::Device(QObject *parent)
    : QObject(parent)
{
    // Initialize default configuration
    config.dmaChannels = 8;
    config.ringBufferSize = 256;
    config.pcieLinkSpeed = 0;
    config.tbTimeout = 1000;
}

Device::~Device()
{
    if (state.isConnected) {
        disconnect();
    }
    cleanup();
}

bool Device::connect()
{
    if (state.isConnected) {
        logError("Device already connected");
        return false;
    }

    // Initialize device
    if (!initializeDevice()) {
        return false;
    }

    // Setup components in sequence
    if (!setupThunderbolt()) {
        cleanup();
        return false;
    }

    if (!setupPCIe()) {
        cleanup();
        return false;
    }

    if (!setupDMA()) {
        cleanup();
        return false;
    }

    state.isConnected = true;
    state.stats.connectionTime = QDateTime::currentMSecsSinceEpoch();
    emit connected();
    return true;
}

bool Device::disconnect()
{
    if (!state.isConnected) {
        logError("Device not connected");
        return false;
    }

    cleanup();
    state.isConnected = false;
    emit disconnected();
    return true;
}

bool Device::reset()
{
    if (!state.isConnected) {
        logError("Device not connected");
        return false;
    }

    // Perform soft reset
    if (!writeToSysfs("control", "reset")) {
        return false;
    }

    // Wait for device to stabilize
    QThread::msleep(1000);

    // Reinitialize components
    if (!setupDMA() || !setupPCIe()) {
        cleanup();
        return false;
    }

    return true;
}

bool Device::optimize()
{
    if (!state.isConnected) {
        logError("Device not connected");
        return false;
    }

    // Read current PCIe link status
    QString linkStatus = readFromSysfs("pcie/link_status");
    if (linkStatus.contains("Gen4")) {
        // Already at optimal speed
        return true;
    }

    // Try to optimize PCIe link
    writeToSysfs("pcie/link_speed", "4");
    
    // Optimize DMA settings
    setDMAChannels(12);  // Increased parallelism
    setRingBufferSize(512);  // Larger buffers

    return true;
}

DeviceStats Device::getStats() const
{
    return state.stats;
}

bool Device::isConnected() const
{
    return state.isConnected;
}

QString Device::getLastError() const
{
    return state.lastError;
}

void Device::setDMAChannels(int channels)
{
    if (channels < 1 || channels > 16) {
        logError("Invalid DMA channel count");
        return;
    }
    config.dmaChannels = channels;
    if (state.isConnected) {
        writeToSysfs("dma/channels", QString::number(channels));
    }
}

void Device::setRingBufferSize(int size)
{
    if (size < 16 || size > 1024 || (size & (size - 1))) {
        logError("Invalid ring buffer size");
        return;
    }
    config.ringBufferSize = size;
    if (state.isConnected) {
        writeToSysfs("dma/ring_size", QString::number(size));
    }
}

void Device::setPCIeLinkSpeed(int speed)
{
    if (speed < 0 || speed > 4) {
        logError("Invalid PCIe link speed");
        return;
    }
    config.pcieLinkSpeed = speed;
    if (state.isConnected) {
        writeToSysfs("pcie/link_speed", QString::number(speed));
    }
}

void Device::setThunderboltTimeout(int ms)
{
    if (ms < 100 || ms > 5000) {
        logError("Invalid Thunderbolt timeout");
        return;
    }
    config.tbTimeout = ms;
    if (state.isConnected) {
        writeToSysfs("thunderbolt/timeout", QString::number(ms));
    }
}

bool Device::initializeDevice()
{
    // Open device file
    state.deviceFd = open(DEVICE_PATH.toLocal8Bit().constData(), O_RDWR);
    if (state.deviceFd < 0) {
        logError("Failed to open device");
        return false;
    }

    // Check device status
    if (!readFromSysfs("status").contains("ready")) {
        logError("Device not ready");
        close(state.deviceFd);
        state.deviceFd = -1;
        return false;
    }

    return true;
}

bool Device::setupDMA()
{
    // Allocate DMA buffer
    if (!writeToSysfs("dma/channels", QString::number(config.dmaChannels))) {
        logError("Failed to set DMA channels");
        return false;
    }

    if (!writeToSysfs("dma/ring_size", QString::number(config.ringBufferSize))) {
        logError("Failed to set ring buffer size");
        return false;
    }

    return true;
}

bool Device::setupPCIe()
{
    // Set PCIe link speed if specified
    if (config.pcieLinkSpeed > 0) {
        if (!writeToSysfs("pcie/link_speed", QString::number(config.pcieLinkSpeed))) {
            logError("Failed to set PCIe link speed");
            return false;
        }
    }

    return true;
}

bool Device::setupThunderbolt()
{
    // Set Thunderbolt timeout
    if (!writeToSysfs("thunderbolt/timeout", QString::number(config.tbTimeout))) {
        logError("Failed to set Thunderbolt timeout");
        return false;
    }

    return true;
}

void Device::cleanup()
{
    if (state.deviceFd >= 0) {
        close(state.deviceFd);
        state.deviceFd = -1;
    }

    if (state.dmaBuffer) {
        // Free DMA buffer
        state.dmaBuffer = nullptr;
    }

    state.stats = DeviceStats();
}

void Device::updateStats()
{
    if (!state.isConnected) {
        return;
    }

    // Read basic performance metrics
    QString perfStats = readFromSysfs("performance/statistics");
    QStringList lines = perfStats.split('\n');

    for (const QString& line : lines) {
        if (line.startsWith("TX:")) {
            state.stats.txThroughput = line.mid(line.indexOf('(') + 1).split(' ').first().toDouble();
        } else if (line.startsWith("RX:")) {
            state.stats.rxThroughput = line.mid(line.indexOf('(') + 1).split(' ').first().toDouble();
        } else if (line.startsWith("Latency:")) {
            state.stats.latency = line.split(' ').at(1).toDouble();
        }
    }

    // Monitor additional components
    monitorPCIeStatus();
    monitorNvidiaGPU();
    monitorThunderboltStatus();

    // Update performance history
    updatePerformanceHistory();

    // Calculate statistics
    calculateStatistics();

    // Check performance thresholds
    checkPerformanceThresholds();

    emit statsUpdated(state.stats);
}

void Device::monitorPCIeStatus()
{
    // Read PCIe link status
    QString pcieStatus = readFromSysfs("pcie/link_status");
    if (pcieStatus.contains("Gen")) {
        state.stats.pcieLinkSpeed = pcieStatus.split("Gen").at(1).split(' ').first().toInt();
        state.stats.pcieLinkWidth = pcieStatus.split("x").at(1).split(' ').first().toInt();
    }

    // Read PCIe errors
    state.stats.pcieErrors = readFromSysfs("pcie/errors").toLongLong();

    // Calculate PCIe utilization
    QString utilStr = readFromSysfs("pcie/utilization");
    state.stats.pcieUtilization = utilStr.split('%').first().toDouble();
}

void Device::monitorNvidiaGPU()
{
    // Read GPU utilization
    state.stats.gpuUtilization = readFromSysfs("gpu/utilization").toInt();

    // Read memory utilization
    state.stats.memoryUtilization = readFromSysfs("gpu/memory_utilization").toInt();

    // Read temperature
    state.stats.temperature = readFromSysfs("gpu/temperature").toInt();

    // Read fan speed
    state.stats.fanSpeed = readFromSysfs("gpu/fan_speed").toInt();

    // Read power usage
    state.stats.powerUsage = readFromSysfs("gpu/power_usage").toInt();

    // Read GPU state
    state.stats.gpuState = readFromSysfs("gpu/power_state");
}

void Device::monitorThunderboltStatus()
{
    // Read Thunderbolt link speed
    QString tbStatus = readFromSysfs("thunderbolt/status");
    QStringList tbLines = tbStatus.split('\n');
    
    for (const QString& line : tbLines) {
        if (line.contains("Speed:")) {
            state.stats.tbLinkSpeed = line.split(':').at(1).trimmed().split(' ').first().toInt();
        } else if (line.contains("Hop count:")) {
            state.stats.tbHopCount = line.split(':').at(1).trimmed().toInt();
        }
    }

    // Read Thunderbolt errors
    state.stats.tbErrors = readFromSysfs("thunderbolt/errors").toLongLong();

    // Read device path and controller status
    state.stats.tbDevicePath = readFromSysfs("thunderbolt/device_path");
    state.stats.tbControllerStatus = readFromSysfs("thunderbolt/controller_status");
}

void Device::updatePerformanceHistory()
{
    QDateTime now = QDateTime::currentDateTime();
    DeviceStats::PerformancePoint point;
    point.timestamp = now;

    // Add TX throughput
    point.value = state.stats.txThroughput;
    state.stats.txHistory.append(point);

    // Add RX throughput
    point.value = state.stats.rxThroughput;
    state.stats.rxHistory.append(point);

    // Add latency
    point.value = state.stats.latency;
    state.stats.latencyHistory.append(point);

    // Add temperature
    point.value = state.stats.temperature;
    state.stats.temperatureHistory.append(point);

    // Keep history size limited (last hour)
    const int maxPoints = 3600000 / config.monitoringInterval;
    while (state.stats.txHistory.size() > maxPoints) {
        state.stats.txHistory.removeFirst();
        state.stats.rxHistory.removeFirst();
        state.stats.latencyHistory.removeFirst();
        state.stats.temperatureHistory.removeFirst();
    }
}

void Device::calculateStatistics()
{
    // Helper lambda for calculating statistics
    auto calcStats = [this](const QVector<DeviceStats::PerformancePoint>& history) -> DeviceStats::Stats {
        if (history.isEmpty()) return DeviceStats::Stats();

        QVector<double> values;
        values.reserve(history.size());
        for (const auto& point : history) {
            values.append(point.value);
        }

        DeviceStats::Stats stats;
        stats.min = *std::min_element(values.begin(), values.end());
        stats.max = *std::max_element(values.begin(), values.end());
        
        // Calculate average
        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        stats.avg = sum / values.size();
        
        // Calculate standard deviation
        stats.stdDev = calculateStdDev(values, stats.avg);
        
        return stats;
    };

    // Calculate statistics for each metric
    state.stats.txStats = calcStats(state.stats.txHistory);
    state.stats.rxStats = calcStats(state.stats.rxHistory);
    state.stats.latencyStats = calcStats(state.stats.latencyHistory);
    state.stats.temperatureStats = calcStats(state.stats.temperatureHistory);
}

void Device::checkPerformanceThresholds()
{
    // Check latency threshold
    if (state.stats.latency > config.thresholds.maxLatency) {
        emit performanceThresholdExceeded("latency", state.stats.latency, config.thresholds.maxLatency);
    }

    // Check throughput threshold
    if (state.stats.txThroughput < config.thresholds.minThroughput ||
        state.stats.rxThroughput < config.thresholds.minThroughput) {
        emit performanceThresholdExceeded("throughput",
            std::min(state.stats.txThroughput, state.stats.rxThroughput),
            config.thresholds.minThroughput);
    }

    // Check temperature threshold
    if (state.stats.temperature > config.thresholds.maxTemperature) {
        emit performanceThresholdExceeded("temperature",
            state.stats.temperature,
            config.thresholds.maxTemperature);
    }

    // Check power usage threshold
    if (state.stats.powerUsage > config.thresholds.maxPowerUsage) {
        emit performanceThresholdExceeded("power",
            state.stats.powerUsage,
            config.thresholds.maxPowerUsage);
    }
}

double Device::calculateStdDev(const QVector<double>& values, double mean) const
{
    if (values.isEmpty()) return 0.0;

    double sumSquares = 0.0;
    for (double value : values) {
        double diff = value - mean;
        sumSquares += diff * diff;
    }
    return std::sqrt(sumSquares / values.size());
}

void Device::setMonitoringInterval(int ms)
{
    if (ms < 100 || ms > 60000) {
        logError("Invalid monitoring interval");
        return;
    }
    config.monitoringInterval = ms;
}

void Device::enableMetric(const QString& metric, bool enable)
{
    if (enable) {
        config.enabledMetrics.insert(metric);
    } else {
        config.enabledMetrics.remove(metric);
    }
}

void Device::clearHistory()
{
    state.stats.txHistory.clear();
    state.stats.rxHistory.clear();
    state.stats.latencyHistory.clear();
    state.stats.temperatureHistory.clear();
    state.monitoringStartTime = QDateTime::currentDateTime();
}

void Device::exportStats(const QString& filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        logError("Failed to open export file");
        return;
    }

    QTextStream out(&file);
    
    // Write header
    out << "Timestamp,TX_Throughput,RX_Throughput,Latency,Temperature,GPU_Util,Mem_Util,Power\n";

    // Write data points
    for (int i = 0; i < state.stats.txHistory.size(); ++i) {
        out << state.stats.txHistory[i].timestamp.toString(Qt::ISODate) << ","
            << state.stats.txHistory[i].value << ","
            << state.stats.rxHistory[i].value << ","
            << state.stats.latencyHistory[i].value << ","
            << state.stats.temperatureHistory[i].value << ","
            << state.stats.gpuUtilization << ","
            << state.stats.memoryUtilization << ","
            << state.stats.powerUsage << "\n";
    }

    file.close();
}

QString Device::generatePerformanceReport() const
{
    QString report;
    QTextStream stream(&report);

    stream << "Performance Report\n";
    stream << "=================\n\n";

    // Connection Information
    stream << "Connection Status: " << state.stats.connectionStatus << "\n";
    stream << "Connection Time: " << (QDateTime::fromMSecsSinceEpoch(state.stats.connectionTime)
        .toString(Qt::ISODate)) << "\n\n";

    // Current Performance
    stream << "Current Performance\n";
    stream << "-----------------\n";
    stream << QString("TX Throughput: %1 MB/s\n").arg(state.stats.txThroughput, 0, 'f', 2);
    stream << QString("RX Throughput: %1 MB/s\n").arg(state.stats.rxThroughput, 0, 'f', 2);
    stream << QString("Latency: %1 ns\n").arg(state.stats.latency, 0, 'f', 2);
    stream << QString("Temperature: %1°C\n\n").arg(state.stats.temperature);

    // Statistics
    auto printStats = [&stream](const QString& name, const DeviceStats::Stats& stats) {
        stream << name << " Statistics:\n";
        stream << QString("  Min: %1\n").arg(stats.min, 0, 'f', 2);
        stream << QString("  Max: %1\n").arg(stats.max, 0, 'f', 2);
        stream << QString("  Avg: %1\n").arg(stats.avg, 0, 'f', 2);
        stream << QString("  StdDev: %1\n").arg(stats.stdDev, 0, 'f', 2);
        stream << "\n";
    };

    printStats("TX Throughput", state.stats.txStats);
    printStats("RX Throughput", state.stats.rxStats);
    printStats("Latency", state.stats.latencyStats);
    printStats("Temperature", state.stats.temperatureStats);

    return report;
}

bool Device::detectPerformanceIssues(QStringList& issues) const
{
    bool hasIssues = false;

    // Check for throughput issues
    if (state.stats.txThroughput < config.thresholds.minThroughput) {
        issues << "Low TX throughput detected";
        hasIssues = true;
    }
    if (state.stats.rxThroughput < config.thresholds.minThroughput) {
        issues << "Low RX throughput detected";
        hasIssues = true;
    }

    // Check for latency issues
    if (state.stats.latency > config.thresholds.maxLatency) {
        issues << "High latency detected";
        hasIssues = true;
    }

    // Check for temperature issues
    if (state.stats.temperature > config.thresholds.maxTemperature) {
        issues << "High temperature detected";
        hasIssues = true;
    }

    // Check for PCIe issues
    if (state.stats.pcieErrors > 0) {
        issues << QString("PCIe errors detected: %1").arg(state.stats.pcieErrors);
        hasIssues = true;
    }

    // Check for Thunderbolt issues
    if (state.stats.tbErrors > 0) {
        issues << QString("Thunderbolt errors detected: %1").arg(state.stats.tbErrors);
        hasIssues = true;
    }

    // Check for GPU issues
    if (state.stats.powerUsage > config.thresholds.maxPowerUsage) {
        issues << "High power usage detected";
        hasIssues = true;
    }

    return hasIssues;
}

bool Device::writeToSysfs(const QString& file, const QString& value)
{
    QFile f(SYSFS_PATH + file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        logError("Failed to open " + file + " for writing");
        return false;
    }

    if (f.write(value.toLocal8Bit()) < 0) {
        logError("Failed to write to " + file);
        return false;
    }

    return true;
}

QString Device::readFromSysfs(const QString& file)
{
    QFile f(SYSFS_PATH + file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logError("Failed to open " + file + " for reading");
        return QString();
    }

    return QString::fromLocal8Bit(f.readAll()).trimmed();
}

void Device::logError(const QString& error)
{
    state.lastError = error;
    qDebug() << "Device error:" << error;
    emit this->error(error);
} 