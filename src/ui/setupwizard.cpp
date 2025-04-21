#include "setupwizard.h"
#include <QMessageBox>

WelcomePage::WelcomePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Welcome to Anarchy eGPU Setup"));
    setSubTitle(tr("This wizard will help you configure your eGPU connection."));

    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *label = new QLabel(tr(
        "This wizard will guide you through:\n\n"
        "• Hardware detection and verification\n"
        "• Performance configuration\n"
        "• Monitoring setup\n\n"
        "Please ensure your eGPU is connected via Thunderbolt/USB4 before proceeding."
    ));
    label->setWordWrap(true);
    layout->addWidget(label);
    setLayout(layout);
}

HardwareCheckPage::HardwareCheckPage(Device *device, QWidget *parent)
    : QWizardPage(parent), hardwareReady(false)
{
    setTitle(tr("Hardware Check"));
    setSubTitle(tr("Verifying connected hardware components..."));

    QVBoxLayout *layout = new QVBoxLayout;
    
    thunderboltStatus = new QLabel(tr("Checking Thunderbolt connection..."));
    gpuStatus = new QLabel(tr("Checking GPU..."));
    driverStatus = new QLabel(tr("Checking NVIDIA drivers..."));

    layout->addWidget(thunderboltStatus);
    layout->addWidget(gpuStatus);
    layout->addWidget(driverStatus);
    layout->addStretch();
    
    setLayout(layout);

    // Start hardware check when page is shown
    QTimer::singleShot(0, this, &HardwareCheckPage::checkHardware);
}

void HardwareCheckPage::checkHardware()
{
    // Check Thunderbolt connection
    QString tbStatus = device->readFromSysfs("thunderbolt/status");
    if (!tbStatus.isEmpty()) {
        thunderboltStatus->setText(tr("✓ Thunderbolt connection detected"));
        thunderboltStatus->setStyleSheet("color: green");
    } else {
        thunderboltStatus->setText(tr("✗ No Thunderbolt connection detected"));
        thunderboltStatus->setStyleSheet("color: red");
        hardwareReady = false;
        emit completeChanged();
        return;
    }

    // Check GPU presence
    QString gpuState = device->readFromSysfs("gpu/power_state");
    if (!gpuState.isEmpty()) {
        gpuStatus->setText(tr("✓ NVIDIA GPU detected"));
        gpuStatus->setStyleSheet("color: green");
    } else {
        gpuStatus->setText(tr("✗ No NVIDIA GPU detected"));
        gpuStatus->setStyleSheet("color: red");
        hardwareReady = false;
        emit completeChanged();
        return;
    }

    // Check driver status
    QString driverVer = device->readFromSysfs("driver_version");
    if (!driverVer.isEmpty()) {
        driverStatus->setText(tr("✓ NVIDIA drivers loaded (version %1)").arg(driverVer));
        driverStatus->setStyleSheet("color: green");
    } else {
        driverStatus->setText(tr("✗ NVIDIA drivers not loaded"));
        driverStatus->setStyleSheet("color: red");
        hardwareReady = false;
        emit completeChanged();
        return;
    }

    hardwareReady = true;
    emit completeChanged();
}

bool HardwareCheckPage::isComplete() const
{
    return hardwareReady;
}

PerformanceConfigPage::PerformanceConfigPage(Device *device, QWidget *parent)
    : QWizardPage(parent), device(device)
{
    setTitle(tr("Performance Configuration"));
    setSubTitle(tr("Configure performance settings for optimal operation."));

    QVBoxLayout *layout = new QVBoxLayout;

    // DMA Channels
    QHBoxLayout *dmaLayout = new QHBoxLayout;
    QLabel *dmaLabel = new QLabel(tr("DMA Channels:"));
    dmaChannels = new QSpinBox;
    dmaChannels->setRange(1, 16);
    dmaChannels->setValue(8);
    dmaChannels->setToolTip(tr("Number of DMA channels for parallel data transfer"));
    dmaLayout->addWidget(dmaLabel);
    dmaLayout->addWidget(dmaChannels);
    layout->addLayout(dmaLayout);

    // Ring Buffer Size
    QHBoxLayout *ringLayout = new QHBoxLayout;
    QLabel *ringLabel = new QLabel(tr("Ring Buffer Size:"));
    ringBufferSize = new QSpinBox;
    ringBufferSize->setRange(16, 1024);
    ringBufferSize->setValue(256);
    ringBufferSize->setSingleStep(16);
    ringBufferSize->setToolTip(tr("Size of the DMA ring buffer in pages"));
    ringLayout->addWidget(ringLabel);
    ringLayout->addWidget(ringBufferSize);
    layout->addLayout(ringLayout);

    // PCIe Link Speed
    QHBoxLayout *pcieLayout = new QHBoxLayout;
    QLabel *pcieLabel = new QLabel(tr("PCIe Link Speed:"));
    pcieLinkSpeed = new QComboBox;
    pcieLinkSpeed->addItem(tr("Auto"), 0);
    pcieLinkSpeed->addItem(tr("Gen 1 (2.5 GT/s)"), 1);
    pcieLinkSpeed->addItem(tr("Gen 2 (5.0 GT/s)"), 2);
    pcieLinkSpeed->addItem(tr("Gen 3 (8.0 GT/s)"), 3);
    pcieLinkSpeed->addItem(tr("Gen 4 (16.0 GT/s)"), 4);
    pcieLayout->addWidget(pcieLabel);
    pcieLayout->addWidget(pcieLinkSpeed);
    layout->addLayout(pcieLayout);

    // Thunderbolt Timeout
    QHBoxLayout *tbLayout = new QHBoxLayout;
    QLabel *tbLabel = new QLabel(tr("Thunderbolt Timeout (ms):"));
    tbTimeout = new QSpinBox;
    tbTimeout->setRange(100, 5000);
    tbTimeout->setValue(1000);
    tbTimeout->setSingleStep(100);
    tbLayout->addWidget(tbLabel);
    tbLayout->addWidget(tbTimeout);
    layout->addLayout(tbLayout);

    layout->addStretch();
    setLayout(layout);
}

bool PerformanceConfigPage::validatePage()
{
    device->setDMAChannels(dmaChannels->value());
    device->setRingBufferSize(ringBufferSize->value());
    device->setPCIeLinkSpeed(pcieLinkSpeed->currentData().toInt());
    device->setThunderboltTimeout(tbTimeout->value());
    return true;
}

MonitoringConfigPage::MonitoringConfigPage(Device *device, QWidget *parent)
    : QWizardPage(parent), device(device)
{
    setTitle(tr("Monitoring Configuration"));
    setSubTitle(tr("Configure monitoring settings and performance thresholds."));

    QVBoxLayout *layout = new QVBoxLayout;

    // Monitoring Interval
    QHBoxLayout *intervalLayout = new QHBoxLayout;
    QLabel *intervalLabel = new QLabel(tr("Monitoring Interval (ms):"));
    monitoringInterval = new QSpinBox;
    monitoringInterval->setRange(100, 60000);
    monitoringInterval->setValue(1000);
    monitoringInterval->setSingleStep(100);
    intervalLayout->addWidget(intervalLabel);
    intervalLayout->addWidget(monitoringInterval);
    layout->addLayout(intervalLayout);

    // Metrics to Monitor
    QGroupBox *metricsGroup = new QGroupBox(tr("Metrics to Monitor"));
    QVBoxLayout *metricsLayout = new QVBoxLayout;
    const char* metrics[] = {
        "PCIe Status", "GPU Utilization", "Memory Usage",
        "Temperature", "Power Usage", "Error Counts"
    };
    for (int i = 0; i < 6; ++i) {
        metricBoxes[i] = new QCheckBox(tr(metrics[i]));
        metricBoxes[i]->setChecked(true);
        metricsLayout->addWidget(metricBoxes[i]);
    }
    metricsGroup->setLayout(metricsLayout);
    layout->addWidget(metricsGroup);

    // Performance Thresholds
    QGroupBox *thresholdGroup = new QGroupBox(tr("Performance Thresholds"));
    QVBoxLayout *thresholdLayout = new QVBoxLayout;

    // Latency Threshold
    QHBoxLayout *latencyLayout = new QHBoxLayout;
    QLabel *latencyLabel = new QLabel(tr("Max Latency (ns):"));
    maxLatency = new QSpinBox;
    maxLatency->setRange(100, 10000);
    maxLatency->setValue(1000);
    latencyLayout->addWidget(latencyLabel);
    latencyLayout->addWidget(maxLatency);
    thresholdLayout->addLayout(latencyLayout);

    // Throughput Threshold
    QHBoxLayout *throughputLayout = new QHBoxLayout;
    QLabel *throughputLabel = new QLabel(tr("Min Throughput (MB/s):"));
    minThroughput = new QSpinBox;
    minThroughput->setRange(10, 1000);
    minThroughput->setValue(100);
    throughputLayout->addWidget(throughputLabel);
    throughputLayout->addWidget(minThroughput);
    thresholdLayout->addLayout(throughputLayout);

    // Temperature Threshold
    QHBoxLayout *tempLayout = new QHBoxLayout;
    QLabel *tempLabel = new QLabel(tr("Max Temperature (°C):"));
    maxTemperature = new QSpinBox;
    maxTemperature->setRange(50, 100);
    maxTemperature->setValue(85);
    tempLayout->addWidget(tempLabel);
    tempLayout->addWidget(maxTemperature);
    thresholdLayout->addLayout(tempLayout);

    // Power Usage Threshold
    QHBoxLayout *powerLayout = new QHBoxLayout;
    QLabel *powerLabel = new QLabel(tr("Max Power Usage (W):"));
    maxPowerUsage = new QSpinBox;
    maxPowerUsage->setRange(100, 600);
    maxPowerUsage->setValue(450);
    powerLayout->addWidget(powerLabel);
    powerLayout->addWidget(maxPowerUsage);
    thresholdLayout->addLayout(powerLayout);

    thresholdGroup->setLayout(thresholdLayout);
    layout->addWidget(thresholdGroup);

    layout->addStretch();
    setLayout(layout);
}

bool MonitoringConfigPage::validatePage()
{
    device->setMonitoringInterval(monitoringInterval->value());
    
    // Enable/disable metrics
    const char* metricNames[] = {
        "pcie", "gpu", "memory", "temperature", "power", "errors"
    };
    for (int i = 0; i < 6; ++i) {
        device->enableMetric(metricNames[i], metricBoxes[i]->isChecked());
    }

    // Set thresholds
    device->config.thresholds.maxLatency = maxLatency->value();
    device->config.thresholds.minThroughput = minThroughput->value();
    device->config.thresholds.maxTemperature = maxTemperature->value();
    device->config.thresholds.maxPowerUsage = maxPowerUsage->value();

    return true;
}

SetupWizard::SetupWizard(Device *device, QWidget *parent)
    : QWizard(parent), device(device)
{
    setWindowTitle(tr("Anarchy eGPU Setup Wizard"));
    setWizardStyle(ModernStyle);

    // Add pages
    addPage(new WelcomePage);
    addPage(new HardwareCheckPage(device));
    addPage(new PerformanceConfigPage(device));
    addPage(new MonitoringConfigPage(device));

    // Set window properties
    setMinimumSize(600, 400);
}

void SetupWizard::accept()
{
    // Attempt to connect the device
    if (!device->connect()) {
        QMessageBox::critical(this, tr("Connection Error"),
            tr("Failed to connect to the eGPU device:\n%1").arg(device->getLastError()));
        return;
    }

    QMessageBox::information(this, tr("Setup Complete"),
        tr("The eGPU has been successfully configured and connected.\n\n"
           "You can modify these settings later through the main application."));

    QWizard::accept();
} 