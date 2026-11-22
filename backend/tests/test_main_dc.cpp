#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include "DC_Motor_Model.h"

int main() {
    // R = 2.0 Ohm, L = 0.5 mH, Kt = 0.1 Nm/A, Ke = 0.1 V/(rad/s), J = 0.01 kg*m^2, B = 0.001 Nms
    DC_Motor_Model mymotor(2.0, 0.0005, 0.1, 0.1, 0.01, 0.001);
    std::ofstream outputFile("motor_output.csv");
    if (!outputFile.is_open()) {
        std::cerr << "Fehler beim Öffnen der Datei!" << std::endl;
        return 1;
    }

    outputFile << "Time_s,Current_A,Speed_rad_per_s\n";
    double t_end = 10.0;   // Simulationszeit in Sekunden
    double dt = 0.00005;   // Zeitschritt in Sekunden
    double voltage = 12.0; // Anzulegende Spannung in Volt
    auto start = std::chrono::high_resolution_clock::now();

    for (double t = 0.0; t <= t_end; t += dt) {
        mymotor.update(voltage, dt);
        outputFile << t << "," << mymotor.getCurrent() << "," << mymotor.getSpeed() << "\n";
    }

    outputFile.close();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Simulation completed in " << elapsed.count() << " seconds." << std::endl;

    std::system("py \"" TESTS_DIR "/plot_simulation.py\"");    

    return 0;
}