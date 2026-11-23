#include "SimulationWorker.h"
#include <QDebug>

SimulationWorker::SimulationWorker(QObject *parent):QObject(parent){
    setMotorType("DC");
}

void SimulationWorker::setMotorType(const QString &type){
    if (type == "DC") {
        m_dcMotor = std::make_unique<DC_Motor_Model>(2.0, 0.0005, 0.1, 0.1, 0.01, 0.001);
        qDebug() << "[Worker] DC_Motor_Model erstellt.";
    } else {
        m_dcMotor.reset(); // Platzhalter für späteres PMSM_Motor_Model
        qDebug() << "[Worker] Anderer Motortyp gewählt:" << type;
    }
}

void SimulationWorker::setInductance(double value) {
    if (m_dcMotor && value > 0.0) {
        m_dcMotor->setL(value);
        qDebug() << "[Worker] Neue Induktivität L:" << m_dcMotor->getL() << "H";
    }
}

void SimulationWorker::startSimulation() {
    if (!m_timer) {
        m_timer = new QTimer(this); // Lebt jetzt garantiert im Worker-Thread
        connect(m_timer, &QTimer::timeout, this, &SimulationWorker::step);
    }
    if (!m_timer->isActive()) {
        m_timer->start(20); // 50 Hz GUI-Update-Rate
    }
}

void SimulationWorker::stopSimulation() {
    if (m_timer) {
        m_timer->stop();
    }
}

void SimulationWorker::step() {
    if (!m_dcMotor) return;

    // 20 ms Echtzeit in 200 kleinen Physik-Schritten (dt = 0.1 ms) berechnen
    constexpr double dt = 0.0001;
    for (int k = 0; k < 200; ++k) {
        m_dcMotor->update(m_voltage, dt);
    }

    emit simulationUpdated(m_dcMotor->getSpeed(), m_dcMotor->getCurrent());
}