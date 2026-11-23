classDiagram
    direction TB

    class Main_qml {
        <<QML Frontend / GUI-Thread>>
        +ComboBox motorTypeCombo
        +CleanTextField paramL
        +ActionButton startStopBtn
        +Label rpmDisplay
    }

    class MotorBackend {
        <<QObject / QML_ELEMENT / GUI-Thread>>
        -QThread m_workerThread
        -SimulationWorker* m_worker
        -QString m_motorType
        -double m_inductance
        -double m_speed
        -double m_current
        +startSimulation() Q_INVOKABLE
        +stopSimulation() Q_INVOKABLE
        +setInductance(double)
        +setMotorType(QString)
        <<signal>> requestSetInductance(double)
        <<signal>> requestStart()
        <<signal>> telemetryChanged()
    }

    class SimulationWorker {
        <<QObject / Worker-Thread>>
        -unique_ptr~DC_Motor_Model~ m_dcMotor
        -QTimer* m_timer
        -double m_voltage
        +setMotorType(QString) slot
        +setInductance(double) slot
        +startSimulation() slot
        +stopSimulation() slot
        -step() slot
        <<signal>> simulationUpdated(double, double)
    }

    class DC_Motor_Model {
        <<Pure C++ / motor_core>>
        -double R, L, Kt, Ke, J, B
        -double i, w
        +update(double voltage, double dt)
        +getCurrent() double
        +getSpeed() double
        +getL() double
        +setL(double)
    }

    Main_qml --> MotorBackend : nutzt via Q_PROPERTY & Q_INVOKABLE
    MotorBackend *-- SimulationWorker : steuert via Signals/Slots (QThread)
    SimulationWorker *-- DC_Motor_Model : besitzt (unique_ptr)