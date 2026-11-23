#pragma once

class DC_Motor_Model {
    private:
        double R;   //Widerstand
        double L;   //Induktivität
        double Kt;  //Drehmomentkonstante
        double Ke;  //Induktivitätskonstante
        double J;   //Trägheitsmoment
        double B;   //Reibungskonstante

        //Zustandsgrössen
        double i;   //Strom
        double w;   //Drehzahl
    public:
        DC_Motor_Model(double R, double L, double Kt, double Ke, double J, double B);

        void update(double voltage, double dt);
        //Getter fuer Zustandsgroessen
        double getCurrent() const;
        double getSpeed() const;
        //Getter/Setter fuer Motorparameter
        double getL() const { return L; }
        void setL(double value) { if (value > 0.0) L = value; }
};