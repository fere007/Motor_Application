#include "DC_Motor_Model.h"

DC_Motor_Model::DC_Motor_Model(double R_, double L_, double Kt_, double Ke_, double J_, double B_) {
    R = R_; L = L_; Kt = Kt_; Ke = Ke_; J = J_; B = B_;
    // Initialisierung der Zustandsgrössen
    i = 0.0; w = 0.0;
}

void DC_Motor_Model::update(double voltage, double dt) {
    // 1. Ableitungen berechnen (di/dt und domega/dt)
    double di_dt = (voltage - R * i - Ke * w) / L;
    double dw_dt = (Kt * i - B * w) / J;

    // 2. Numerische Integration (Euler-Schritt)
    i += di_dt * dt;
    w += dw_dt * dt;
}

double DC_Motor_Model::getCurrent() const {
    return i;
}

double DC_Motor_Model::getSpeed() const {
    return w;
}
