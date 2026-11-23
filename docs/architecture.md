# Software-Architektur: Motor Application

## 1. Architektur-Übersicht

Die Applikation folgt einer strikten **3-Schichten-Architektur** mit klarer Trennung zwischen Benutzeroberfläche, Thread-Orchestrierung und physikalischer Simulation:

| Schicht | Verzeichnis | Thread | Technologie | Verantwortung |
| :--- | :--- | :--- | :--- | :--- |
| **Frontend** | `frontend/` | GUI-Thread | QML / Qt Quick | Darstellung, Benutzereingaben, Live-Plots, Toolbar |
| **Bridge** | `bridge/` | GUI- & Worker-Thread | Qt 6 C++ (`QObject`, `QThread`) | Property-Bindings, Thread-Trennung, Simulations-Loop, CSV/Preset-Handling |
| **Backend** | `backend/` | Worker-Thread | Reines C++17 (`motor_core`) | Motormodelle (DC, PMSM), Kommutierung/FOC, Lastprofile – **frei von Qt** |

### Zentrale Design-Prinzipien
* **Keine Qt-Abhängigkeit im Backend:** `motor_core` bleibt als eigenständige statische Bibliothek (`lib`) voll testbar (CLI-Tests, spätere Unit-Tests, Wiederverwendung auf Embedded-Targets).
* **Thread-Sicherheit ohne Mutexes:** `MotorBackend` (GUI-Thread) und `SimulationWorker` (Worker-Thread) kommunizieren ausschließlich über asynchrone Qt Signals & Slots (`QueuedConnection`).
* **Polymorphe Motormodelle:** Über ein gemeinsames Interface (`IMotorModel`) kann der Worker beliebige Motortypen (DC, PMSM) austauschen, ohne die Simulationsschleife umzubauen.

---

## 2. UML-Klassendiagramm (Ziel-Architektur)

```mermaid
classDiagram
    direction TB

    class Main_qml {
        <<QML / Frontend GUI-Thread>>
        +ComboBox motorTypeCombo
        +ComboBox connectionCombo
        +ComboBox commutationCombo
        +ComboBox simModeCombo
        +CleanTextField paramFields
        +Slider voltageSlider
        +Slider frictionSlider
        +Slider loadSlider
        +ActionButton startStopBtn
        +ActionButton resetBtn
        +PlotView speedPlot
        +PlotView currentPlot
    }

    class MotorBackend {
        <<QObject / GUI-Thread QML_ELEMENT>>
        -QThread m_workerThread
        -SimulationWorker* m_worker
        -QString m_motorType
        -QString m_connectionType
        -QString m_commutationType
        -QString m_simMode
        -double m_R
        -double m_L
        -double m_Kt
        -double m_Ke
        -double m_J
        -double m_B
        -int m_polePairs
        -double m_voltage
        -double m_loadTorque
        -double m_batchDuration
        -double m_speedRpm
        -double m_ia
        -double m_ib
        -double m_ic
        -bool m_isRunning
        +MotorBackend(QObject* parent)
        +~MotorBackend()
        +startSimulation() void
        +stopSimulation() void
        +resetSimulation() void
        +loadLoadProfile(QString filePath) void
        +exportCsv(QString filePath) void
        +signal_parametersChanged()
        +signal_telemetryUpdated()
        +signal_batchCompleted(QVariantList)
        +signal_requestStartLive()
        +signal_requestStartBatch(double)
        +signal_requestStop()
        +signal_requestReset()
        -onTelemetryReceived(TelemetryFrame) void
    }

    class SimulationWorker {
        <<QObject / Worker-Thread>>
        -unique_ptr~IMotorModel~ m_motor
        -unique_ptr~IController~ m_controller
        -LoadProfile m_loadProfile
        -QTimer* m_liveTimer
        -double m_dtPhys
        -double m_simTime
        -double m_voltage
        -double m_loadTorque
        +SimulationWorker(QObject* parent)
        +setMotorType(QString type) void
        +updateParameters(MotorParams params) void
        +setOperatingPoint(double v, double b, double tLoad) void
        +loadProfileFromCsv(QString path) void
        +startLive() void
        +startBatch(double durationSec) void
        +stop() void
        +reset() void
        -stepLive() void
        +signal_telemetryReady(TelemetryFrame)
        +signal_batchFinished(vector~TelemetryFrame~)
    }

    class IMotorModel {
        <<Interface / C++ Pure>>
        +~IMotorModel()
        +reset() void
        +getSpeed() double
        +getAngle() double
        +getTorque() double
        +setParameters(MotorParams) void
    }

    class DC_Motor_Model {
        <<Pure / C++ motor_core>>
        -double R, L, Kt, Ke, J, B
        -double i, w, theta
        +DC_Motor_Model(R, L, Kt, Ke, J, B)
        +update(double voltage, double loadTorque, double dt) void
        +reset() void
        +getCurrent() double
        +getSpeed() double
        +getAngle() double
    }

    class PMSM_Motor_Model {
        <<Pure / C++ motor_core>>
        -double R, Ld, Lq, psiF, J, B
        -int polePairs
        -ConnectionType connection
        -double ia, ib, ic, w, theta
        +PMSM_Motor_Model(params, polePairs, connection)
        +update(PhaseStates states, double vdc, double loadTorque, double dt) void
        +updateVoltages(double va, double vb, double vc, double loadTorque, double dt) void
        +reset() void
        +getCurrents() PhaseCurrents
        +getSpeed() double
        +getAngle() double
    }

    class IController {
        <<Interface / C++ Pure>>
        +compute(double target, double feedback, double dt) ControlOutput
        +reset() void
    }

    class BlockCommutation {
        <<Pure / C++ motor_core>>
        +compute(double thetaElec) PhaseStates
    }

    class FOC_Controller {
        <<Pure / C++ motor_core>>
        -double kpId, kiId, kpIq, kiIq
        +compute(PhaseCurrents i, double thetaElec, double iqRef, double dt) PhaseVoltages
    }

    class LoadProfile {
        <<Pure / C++ motor_core>>
        -vector~double~ m_angles
        -vector~double~ m_torques
        +loadCsv(string path) bool
        +getTorqueAtAngle(double thetaMech) double
    }

    Main_qml --> MotorBackend : Q_PROPERTY & Q_INVOKABLE
    MotorBackend *-- SimulationWorker : QThread (Signals / Slots)
    SimulationWorker *-- IMotorModel : besitzt (unique_ptr)
    SimulationWorker *-- IController : besitzt (unique_ptr)
    SimulationWorker *-- LoadProfile : nutzt
    IMotorModel <|.. DC_Motor_Model : implementiert
    IMotorModel <|.. PMSM_Motor_Model : implementiert
    IController <|.. BlockCommutation : implementiert
    IController <|.. FOC_Controller : implementiert
```

---

## 3. UML-Sequenzdiagramm (Datenfluss & Threading)

Das Sequenzdiagramm zeigt die drei zentralen Laufzeit-Szenarien:
1. **Parameter-Update:** Sofortige, thread-sichere Übernahme von GUI-Eingaben.
2. **Live-Modus (Interaktiv):** Kontinuierliche Echtzeit-Simulation mit 50 Hz GUI-Refresh.
3. **Batch-Modus (Zeitgesteuert):** Schnelle Berechnung über $x$ Sekunden inkl. winkelabhängigem Lastprofil und geschlossenem Plot-Export.

```mermaid
sequenceDiagram
    autonumber
    box rgb(240, 248, 255) GUI-Thread (Main)
        participant QML as Main.qml
        participant Backend as MotorBackend
    end
    box rgb(245, 255, 245) Worker-Thread (QThread)
        participant Worker as SimulationWorker
        participant Ctrl as IController
        participant Model as IMotorModel
    end

    Note over QML, Model: Szenario 1: Parameteränderung (z. B. R, L, Spannung, Last)
    QML->>Backend: Property-Setter (z. B. backend.inductance = 0.002)
    Backend->>Backend: Lokalen Cache aktualisieren
    Backend-->>QML: emit parametersChanged()
    Backend-->>Worker: emit requestUpdateParams(...) [Queued]
    Worker->>Model: setParameters(...)

    Note over QML, Model: Szenario 2: Live-Simulation (Interaktiver Prüfstand)
    QML->>Backend: backend.startSimulation()
    Backend-->>Worker: emit requestStartLive() [Queued]
    Worker->>Worker: m_liveTimer->start(20 ms)

    loop Alle 20 ms (50 Hz GUI-Update)
        Worker->>Worker: stepLive()
        loop N Physik-Schritte (z. B. dt = 10 us - 100 us)
            Worker->>Ctrl: compute(feedback, dt)
            Ctrl-->>Worker: Ansteuerung (U_abc / PhaseStates)
            Worker->>Model: update(controlOut, T_load, dt)
        end
        Worker->>Model: Zustände auslesen (w, i_a, i_b, i_c)
        Worker-->>Backend: emit telemetryReady(frame) [Queued]
        Backend->>Backend: m_speedRpm, m_ia, m_ib, m_ic aktualisieren
        Backend-->>QML: emit telemetryUpdated()
        QML->>QML: Live-Plots & Anzeigen neu zeichnen
    end

    Note over QML, Model: Szenario 3: Batch-Simulation (x Sekunden + Lastprofil)
    QML->>Backend: backend.startSimulation() [Batch-Modus]
    Backend-->>Worker: emit requestStartBatch(durationSec) [Queued]
    Worker->>Model: reset()
    loop t = 0 bis durationSec (mit dt_phys)
        Worker->>Worker: T_load = m_loadProfile.getTorqueAtAngle(theta)
        Worker->>Ctrl: compute(...)
        Worker->>Model: update(..., T_load, dt_phys)
        opt Downsampling (z. B. jeden 100. Punkt)
            Worker->>Worker: Frame in Ergebnis-Vektor speichern
        end
    end
    Worker-->>Backend: emit batchFinished(results) [Queued]
    Backend-->>QML: emit batchCompleted(plotData)
    QML->>QML: Komplette Kurve in Plots darstellen
```

---

## 4. Erweiterungs-Checkliste (Neuer Parameter)

Beim Hinzufügen eines neuen Motor- oder Betriebsparameters werden immer dieselben 4 Stellen angepasst:
1. **Backend (`backend/include/*.h`):** Getter/Setter im Modell ergänzen (inkl. Wertebereichsprüfung, z. B. $L > 0, J > 0$).
2. **Worker (`bridge/SimulationWorker`):** Slot zur Übernahme in `m_motor` anlegen.
3. **Bridge (`bridge/MotorBackend`):** `Q_PROPERTY`, Setter, `request*`-Signal und `connect(...)` im Konstruktor ergänzen.
4. **Frontend (`frontend/Main.qml`):** Textfeld oder Slider an das neue `backend.<property>` binden.