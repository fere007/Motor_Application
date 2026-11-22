#include "PMSM_Motor_Model.h"
#include <cmath>
#include <stdexcept>

constexpr double PI = 3.14159265358979323846;
constexpr double TWO_PI_OVER_THREE = 2.0 * PI / 3.0; // 2π/3 für die Umrechnung von elektrischen zu mechanischen Winkeln

PMSM_Motor_Model::PMSM_Motor_Model(double R_, double L_, double Kt_, double Ke_, double J_, double B_, int pole_pairs_, ConnectionType conn) {
    R = R_; 
    L = L_; 
    Kt = Kt_; 
    Ke = Ke_; 
    J = J_; 
    B = B_; 
    pole_pairs = pole_pairs_;
    connection=conn;
    
    ia = 0.0; ib = 0.0; ic = 0.0;
    omega_m = 0.0; 
    theta_m = 0.0;
}

void PMSM_Motor_Model::update(InverterState states, double v_dc, double dt){
    double theta_e = getElectricalAngle();

    // Gegen-EMK-Spannungen für jede Phase berechnen
    double ea = Ke * omega_m * std::sin(theta_e);
    double eb = Ke * omega_m * std::sin(theta_e - TWO_PI_OVER_THREE);
    double ec = Ke * omega_m * std::sin(theta_e - 2 * TWO_PI_OVER_THREE);

    // Terminal-Spannungen initialisieren (auf LOW = 0V)
    double va_term = 0.0, vb_term = 0.0, vc_term = 0.0;
    if (states.a == PhaseState::HIGH_SIDE) va_term = v_dc;
    if (states.b == PhaseState::HIGH_SIDE) vb_term = v_dc;
    if (states.c == PhaseState::HIGH_SIDE) vc_term = v_dc;

    double T_elec = 0.0; // Elektrisches Drehmoment

    if (connection==ConnectionType::STAR)
    {
        int active_phases = 0;
        double sum_v = 0.0, sum_e = 0.0;

        // nur aktive Phasen berücksichtigen
        if (states.a != PhaseState::FLOATING) { active_phases++; sum_v += va_term; sum_e += ea; }
        if (states.b != PhaseState::FLOATING) { active_phases++; sum_v += vb_term; sum_e += eb; }
        if (states.c != PhaseState::FLOATING) { active_phases++; sum_v += vc_term; sum_e += ec; }

        double v_neutral = (active_phases > 0) ? (sum_v - sum_e) / active_phases : 0.0;

        // DGL für Leiterströme in Sternschaltung
        if (states.a != PhaseState::FLOATING) ia += ((va_term - v_neutral - ea - R * ia) / L) * dt; else ia = 0.0;
        if (states.b != PhaseState::FLOATING) ib += ((vb_term - v_neutral - eb - R * ib) / L) * dt; else ib = 0.0;
        if (states.c != PhaseState::FLOATING) ic += ((vc_term - v_neutral - ec - R * ic) / L) * dt; else ic = 0.0;

        // Drehmoment über mechanische Leistung berechnen: T = Kt * (ia + ib + ic)
        if (omega_m != 0.0){
            T_elec = (ea*ia + eb*ib + ec*ic) / omega_m; // mechanisches Drehmoment
        }
        else{
            // Fallback für Stillstand (0 durch 0 verhindern)
            T_elec = Ke * (ia * std::sin(theta_e) + ib * std::sin(theta_e - TWO_PI_OVER_THREE) + ic * std::sin(theta_e - 2.0 * TWO_PI_OVER_THREE));
        }
    }
    else if (connection==ConnectionType::DELTA)
    {
        // EMK über den Wicklungen (Leiterspannung = Differenz der Phasen-EMKs)
        double e_ab = ea - eb;
        double e_bc = eb - ec;
        double e_ca = ec - ea;

        // Wenn ein Terminal FLOATING ist, stellt sich seine Spannung als Spannungsteiler 
        // zwischen den beiden anderen Terminals ein (damit der zufließende Leiterstrom 0 wird).
        if (states.a == PhaseState::FLOATING) va_term = 0.5 * (vb_term + vc_term + e_ab - e_ca);
        if (states.b == PhaseState::FLOATING) vb_term = 0.5 * (vc_term + va_term + e_bc - e_ab);
        if (states.c == PhaseState::FLOATING) vc_term = 0.5 * (va_term + vb_term + e_ca - e_bc);

        // Spannungen über den drei Dreieckswicklungen
        double v_ab = va_term - vb_term;
        double v_bc = vb_term - vc_term;
        double v_ca = vc_term - va_term;

        // DGL für Wicklungsströme (hier sind ia, ib, ic die Ströme IN der Dreieckswicklung)
        ia += ((v_ab - R * ia - e_ab) / L) * dt; // ia = i_ab
        ib += ((v_bc - R * ib - e_bc) / L) * dt; // ib = i_bc
        ic += ((v_ca - R * ic - e_ca) / L) * dt; // ic = i_ca

        // Drift-Korrektur: Wenn ein Terminal offen ist, liegen zwei Wicklungen exakt in Reihe. 
        // Der Strom MUSS gleich sein. Das korrigiert kleine numerische Integrationsfehler des Euler-Verfahrens.
        if (states.a == PhaseState::FLOATING) { double avg = (ia + ic) / 2.0; ia = avg; ic = avg; }
        if (states.b == PhaseState::FLOATING) { double avg = (ib + ia) / 2.0; ib = avg; ia = avg; }
        if (states.c == PhaseState::FLOATING) { double avg = (ic + ib) / 2.0; ic = avg; ib = avg; }

        if (omega_m != 0.0) {
            T_elec = (e_ab * ia + e_bc * ib + e_ca * ic) / omega_m;
        } else {
            T_elec = Ke * (ia * (std::sin(theta_e) - std::sin(theta_e - TWO_PI_OVER_THREE)) + 
                           ib * (std::sin(theta_e - TWO_PI_OVER_THREE) - std::sin(theta_e - 2.0 * TWO_PI_OVER_THREE)) + 
                           ic * (std::sin(theta_e - 2.0 * TWO_PI_OVER_THREE) - std::sin(theta_e)));
        }
    }
    else
    {
        // Fehlerbehandlung für unbekannten Verbindungstyp
        throw std::invalid_argument("Unbekannter Verbindungstyp");
    }

    // Mechanische Bewegungsgleichung: J * d(omega_m)/dt = T_elec - B * omega_m
    double domega_dt = (T_elec - B * omega_m) / J;
    omega_m += domega_dt * dt;
    theta_m += omega_m * dt;

    // Winkelüberlauf auf [0, 2π] begrenzen
    theta_m = std::fmod(theta_m, 2.0 * PI);
    if (theta_m < 0.0) theta_m += 2.0 * PI;
}

// Gibt die messbaren LEITERSTRÖME zurück (wichtig für den Controller)
PhaseCurrents PMSM_Motor_Model::getCurrents() const {
    if (connection == ConnectionType::STAR) {
        return {ia, ib, ic};
    } else {
        // Bei Dreieck: Kirchhoffscher Knotenpunkt an den Klemmen
        // I_Leiter_A = i_ab - i_ca
        // I_Leiter_B = i_bc - i_ab
        // I_Leiter_C = i_ca - i_bc
        return {ia - ic, ib - ia, ic - ib};
    }
}

double PMSM_Motor_Model::getMechanicalAngle() const {
    return theta_m;
}

double PMSM_Motor_Model::getElectricalAngle() const {
    double theta_e = std::fmod(theta_m * pole_pairs, 2.0 * PI);
    if (theta_e < 0.0) {
        theta_e += 2.0 * PI;
    }
    return theta_e;
}

double PMSM_Motor_Model::getSpeed() const {
    return omega_m;
}