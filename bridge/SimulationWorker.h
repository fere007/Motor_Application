#pragma once
#include <QObject>
#include <QTimer>
#include <memory>
#include "DC_Motor_Model.h"

class SimulationWorker : public QObject{
    Q_OBJECT

    public:
        explicit SimulationWorker(QObject *parent=nullptr);

    public slots:
        void setMotorType(const QString &type);
        void setInductance(double value);
        void startSimulation();
        void stopSimulation();
    
    signals:
        void simulationUpdated(double speedRadS, double currentA);
    
    private slots:
        void step();
    
    private:
    std::unique_ptr<DC_Motor_Model> m_dcMotor;
    QTimer *m_timer = nullptr;
    double m_voltage = 12.0;
    
};