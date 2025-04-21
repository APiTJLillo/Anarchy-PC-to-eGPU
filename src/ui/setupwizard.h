#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include "../core/device.h"

class WelcomePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr);
};

class HardwareCheckPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit HardwareCheckPage(Device *device, QWidget *parent = nullptr);
    virtual bool isComplete() const override;
private:
    QLabel *thunderboltStatus;
    QLabel *gpuStatus;
    QLabel *driverStatus;
    bool hardwareReady;
    void checkHardware();
};

class PerformanceConfigPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit PerformanceConfigPage(Device *device, QWidget *parent = nullptr);
    virtual bool validatePage() override;
private:
    Device *device;
    QSpinBox *dmaChannels;
    QSpinBox *ringBufferSize;
    QComboBox *pcieLinkSpeed;
    QSpinBox *tbTimeout;
};

class MonitoringConfigPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit MonitoringConfigPage(Device *device, QWidget *parent = nullptr);
    virtual bool validatePage() override;
private:
    Device *device;
    QSpinBox *monitoringInterval;
    QCheckBox *metricBoxes[6];
    QSpinBox *maxLatency;
    QSpinBox *minThroughput;
    QSpinBox *maxTemperature;
    QSpinBox *maxPowerUsage;
};

class SetupWizard : public QWizard
{
    Q_OBJECT
public:
    explicit SetupWizard(Device *device, QWidget *parent = nullptr);
    virtual void accept() override;

private:
    Device *device;
};

#endif // SETUPWIZARD_H 