#include <iostream>
#include <fstream>
#include <cmath>
#include "PMSM_Motor_Model.h"

/*
 * Erklärung:
 * - Hauptschleife simuliert hochfrequente Physik (1 µs) und Mikrocontroller-Firmware (50 µs).
 * - "Hall-Sensoren" werden über 60°-Sektoreinteilung des elektrischen Winkels abgebildet.
 * - Klassische 6-Schritt-Kommutierung (B6-Brücke) steuert die Inverter-Zustände.
 * 
 * Darauf achten:
 * - Die mechanische DGL MUSS in der update()-Funktion ergänzt sein, sonst dreht der Motor nicht.
 * - CSV-Logging passiert nur im MCU-Takt, um die Dateigröße der motor_output.csv klein zu halten.
 */

 constexpr double PI = 3.14159265358979323846;

int main() {
    // Motor initialisieren (Sternschaltung, 2.0 Ohm, 0.5mH, 0.1 Kt/Ke, 0.01 J, 0.001 B, 1 Polpaar)
    PMSM_Motor_Model motor(2.0, 0.0005, 0.1, 0.1, 0.01, 0.001, 1, ConnectionType::STAR);
    
    std::ofstream outFile("motor_output.csv");
    outFile << "Time_s,Speed_rad_s,Ia,Ib,Ic\n";

    double t_end = 10.0;         // 10 Sekunden simulieren
    double dt_phys = 1e-6;       // 1 µs Physik-Schritt
    int mcu_prescaler = 50;      // 50 µs MCU-Takt (20 kHz)
    double v_dc = 12.0;          // 12V Batterie/Netzteil

    InverterState inv = {PhaseState::FLOATING, PhaseState::FLOATING, PhaseState::FLOATING};
    int tick = 0;

    for (double t = 0.0; t <= t_end; t += dt_phys) {
        
        // --- 1. MIKROCONTROLLER (läuft nur alle 50 µs) ---
        if (tick % mcu_prescaler == 0) {
            double theta_e = motor.getElectricalAngle();
            
            // Hall-Sensoren "auslesen": Elektrischen Winkel in 6 Sektoren (0 bis 5) einteilen
            int sector = static_cast<int>(theta_e / (PI / 3.0)) % 6;

            // 6-Schritt Blockkommutierungstabelle
            switch (sector) {
                case 0: inv = {PhaseState::HIGH_SIDE, PhaseState::LOW_SIDE,  PhaseState::FLOATING}; break; // A+, B-, C off
                case 1: inv = {PhaseState::HIGH_SIDE, PhaseState::FLOATING,  PhaseState::LOW_SIDE}; break; // A+, B off, C-
                case 2: inv = {PhaseState::FLOATING,  PhaseState::HIGH_SIDE, PhaseState::LOW_SIDE}; break; // A off, B+, C-
                case 3: inv = {PhaseState::LOW_SIDE,  PhaseState::HIGH_SIDE, PhaseState::FLOATING}; break; // A-, B+, C off
                case 4: inv = {PhaseState::LOW_SIDE,  PhaseState::FLOATING,  PhaseState::HIGH_SIDE}; break; // A-, B off, C+
                case 5: inv = {PhaseState::FLOATING,  PhaseState::LOW_SIDE,  PhaseState::HIGH_SIDE}; break; // A off, B-, C+
            }

            // CSV schreiben (nur im MCU-Takt)
            PhaseCurrents i = motor.getCurrents();
            outFile << t << "," << motor.getSpeed() << "," << i.ia << "," << i.ib << "," << i.ic << "\n";
        }

        // --- 2. PHYSIK (läuft jeden 1 µs Tick) ---
        motor.update(inv, v_dc, dt_phys);
        tick++;
    }

    outFile.close();
    std::cout << "Simulation beendet. Daten in motor_output.csv gespeichert.\n";
    
    // Optional: Direkt das Python Plot-Skript aufrufen
    std::system("py tests/plot_pmsm_simu.py");
    
    return 0;
}