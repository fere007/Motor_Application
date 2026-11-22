#pragma once

struct PhaseCurrents {
    double ia;
    double ib;
    double ic;
};

enum class PhaseState {
    HIGH_SIDE,  // Verbunden mit positiver Versorgungsspannung (z.B. +12V)
    LOW_SIDE,   // Verbunden mit Masse (GND / 0V)
    FLOATING    // Hochohmig (Transistoren gesperrt)
};

enum class ConnectionType {
    STAR,   // Sternschaltung
    DELTA   // Dreieckschaltung
};

struct InverterState {
    PhaseState a, b, c;
};

class PMSM_Motor_Model {
    private:
        ConnectionType connection; // Verbindungstyp (Stern oder Dreieck)
        // Motor-Parameter
        double R;   // Phasen-Widerstand
        double L;   // Phasen-Induktivität (L_s lassen wir weg, L reicht für einfaches Modell)
        double Kt;  // Drehmomentkonstante
        double Ke;  // Induktivitätskonstante (Gegen-EMK Konstante)
        double J;   // Trägheitsmoment
        double B;   // Reibungskonstante
        int pole_pairs; // Anzahl der Polpaare (WICHTIG für Umrechnung Mech <-> Elektr. Winkel)

        // Zustandsgrößen (verändern sich in jedem dt-Schritt)
        double ia, ib, ic;  // Phasenströme
        double omega_m;     // Mechanische Winkelgeschwindigkeit (Drehzahl) - FEHLTE!
        double theta_m;     // Mechanischer Winkel (phi)

    public:
        // Konstruktor (pole_pairs hinzugefügt)
        PMSM_Motor_Model(double R_, double L_, double Kt_, double Ke_, double J_, double B_, int pole_pairs_, ConnectionType conn = ConnectionType::STAR);

        // Die Update-Funktion übergibt nun den Inverterzustand und die Bus-Spannung
        void update(InverterState states, double v_dc, double dt);

        // Getter-Funktionen
        // Rückgabewert auf das Struct geändert!
        PhaseCurrents getCurrents() const; 
        
        double getMechanicalAngle() const;
        double getElectricalAngle() const; // Wichtig für die Hall-Sensor-Logik
        double getSpeed() const;           // Gibt omega_m zurück
};