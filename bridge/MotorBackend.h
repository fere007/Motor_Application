#pragma once
#include <QObject>
#include <QThread>
#include <QtQml/qqmlregistration.h>
#include "SimulationWorker.h"

class MotorBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString motorType READ motorType WRITE setMotorType NOTIFY motorTypeChanged)
    Q_PROPERTY(double inductance READ inductance WRITE setInductance NOTIFY inductanceChanged)
    Q_PROPERTY(double currentSpeed READ currentSpeed NOTIFY telemetryChanged)
    Q_PROPERTY(double currentAmpere READ currentAmpere NOTIFY telemetryChanged)

public:
    explicit MotorBackend(QObject *parent = nullptr);
    ~MotorBackend() override;

    QString motorType() const { return m_motorType; }
    void setMotorType(const QString &type);

    double inductance() const { return m_inductance; }
    void setInductance(double value);

    double currentSpeed() const { return m_speed; }
    double currentAmpere() const { return m_current; }

    Q_INVOKABLE void startSimulation();
    Q_INVOKABLE void stopSimulation();

signals:
    // Signale für QML
    void motorTypeChanged();
    void inductanceChanged();
    void telemetryChanged();

    // Interne Signale an den Worker-Thread
    void requestMotorType(const QString &type);
    void requestSetInductance(double value);
    void requestStart();
    void requestStop();

private slots:
    void onSimulationUpdated(double speedRadS, double currentA);

private:
    QThread m_workerThread;
    SimulationWorker *m_worker = nullptr;

    QString m_motorType = "DC";
    double m_inductance = 0.0005;
    double m_speed = 0.0;
    double m_current = 0.0;
};