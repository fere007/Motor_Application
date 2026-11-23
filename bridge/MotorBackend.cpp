#include "MotorBackend.h"

MotorBackend::MotorBackend(QObject *parent)
    : QObject(parent) {
    m_worker = new SimulationWorker();
    m_worker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    connect(this, &MotorBackend::requestMotorType,     m_worker, &SimulationWorker::setMotorType);
    connect(this, &MotorBackend::requestSetInductance, m_worker, &SimulationWorker::setInductance);
    connect(this, &MotorBackend::requestStart,         m_worker, &SimulationWorker::startSimulation);
    connect(this, &MotorBackend::requestStop,          m_worker, &SimulationWorker::stopSimulation);

    connect(m_worker, &SimulationWorker::simulationUpdated, this, &MotorBackend::onSimulationUpdated);

    m_workerThread.start();
}

MotorBackend::~MotorBackend() {
    m_workerThread.quit();
    m_workerThread.wait();
}

void MotorBackend::setMotorType(const QString &type) {
    if (m_motorType == type) return;
    m_motorType = type;
    emit motorTypeChanged();
    emit requestMotorType(type);
}

void MotorBackend::setInductance(double value) {
    if (value <= 0.0 || qFuzzyCompare(m_inductance, value)) return;
    m_inductance = value;
    emit inductanceChanged();
    emit requestSetInductance(value);
}

void MotorBackend::startSimulation() { emit requestStart(); }
void MotorBackend::stopSimulation()  { emit requestStop(); }

void MotorBackend::onSimulationUpdated(double speedRadS, double currentA) {
    m_speed = speedRadS;
    m_current = currentA;
    emit telemetryChanged();
}