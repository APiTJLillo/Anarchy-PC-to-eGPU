#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>

struct DeviceStats {
    // Basic metrics
    double txThroughput;
    double rxThroughput;
    double latency;
    int pcieLinkSpeed;
    int temperature;
    QString driverVersion;
    QString connectionStatus;
    qint64 connectionTime;

    // Extended PCIe metrics
    int pcieLinkWidth;
    qint64 pcieErrors;
    double pcieUtilization;
    
    // DMA metrics
    int activeChannels;
    int ringBufferSize;
    double ringBufferUtilization;
    qint64 totalBytesTransferred;
    qint64 transferErrors;
    
    // GPU metrics
    int gpuUtilization;
    int memoryUtilization;
    int fanSpeed;
    int powerUsage;
    QString gpuState;
    
    // Thunderbolt metrics
    int tbLinkSpeed;
    int tbHopCount;
    qint64 tbErrors;
    QString tbDevicePath;
    QString tbControllerStatus;

    // Performance history
    struct PerformancePoint {
        QDateTime timestamp;
        double value;
    };
    QVector<PerformancePoint> txHistory;
    QVector<PerformancePoint> rxHistory;
    QVector<PerformancePoint> latencyHistory;
    QVector<PerformancePoint> temperatureHistory;

    // Statistics
    struct Stats {
        double min;
        double max;
        double avg;
        double stdDev;
    };
    Stats txStats;
    Stats rxStats;
    Stats latencyStats;
    Stats temperatureStats;
};

class Device : public QObject
{
    Q_OBJECT

public:
    explicit Device(QObject *parent = nullptr);
    ~Device();

    // Connection management
    bool connect();
    bool disconnect();
    bool reset();
    bool optimize();

    // Status queries
    DeviceStats getStats() const;
    bool isConnected() const;
    QString getLastError() const;

    // Configuration
    void setDMAChannels(int channels);
    void setRingBufferSize(int size);
    void setPCIeLinkSpeed(int speed);
    void setThunderboltTimeout(int ms);

    // Monitoring control
    void setMonitoringInterval(int ms);
    void enableMetric(const QString& metric, bool enable);
    void clearHistory();
    void exportStats(const QString& filename) const;

    // Performance analysis
    DeviceStats::Stats calculateStats(const QVector<DeviceStats::PerformancePoint>& history) const;
    QString generatePerformanceReport() const;
    bool detectPerformanceIssues(QStringList& issues) const;

signals:
    void connected();
    void disconnected();
    void error(const QString& message);
    void statsUpdated(const DeviceStats& stats);
    void performanceAlert(const QString& message, int severity);
    void performanceThresholdExceeded(const QString& metric, double value, double threshold);

private:
    bool initializeDevice();
    bool setupDMA();
    bool setupPCIe();
    bool setupThunderbolt();
    void cleanup();
    void updateStats();
    void updatePerformanceHistory();
    void checkPerformanceThresholds();
    void calculateStatistics();

    // Device configuration
    struct {
        int dmaChannels = 8;
        int ringBufferSize = 256;
        int pcieLinkSpeed = 0;  // 0 = auto
        int tbTimeout = 1000;
        int monitoringInterval = 1000;
        QSet<QString> enabledMetrics;
        struct {
            double maxLatency = 1000.0;  // ns
            double minThroughput = 100.0; // MB/s
            int maxTemperature = 85;      // °C
            double maxPowerUsage = 450.0; // W
        } thresholds;
    } config;

    // Device state
    struct {
        bool isConnected = false;
        QString lastError;
        DeviceStats stats;
        int deviceFd = -1;
        void* dmaBuffer = nullptr;
        QDateTime monitoringStartTime;
    } state;

    // System paths
    const QString SYSFS_PATH = "/sys/kernel/debug/anarchy-egpu/";
    const QString DEVICE_PATH = "/dev/anarchy-egpu";
    const QString NVIDIA_SYSFS_PATH = "/sys/class/nvidia/";

    // Helper functions
    bool writeToSysfs(const QString& file, const QString& value);
    QString readFromSysfs(const QString& file);
    void logError(const QString& error);
    double calculateStdDev(const QVector<double>& values, double mean) const;
    void monitorNvidiaGPU();
    void monitorPCIeStatus();
    void monitorThunderboltStatus();
};

#endif // DEVICE_H 