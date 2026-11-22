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
        // Konstruktor (wird beim Erstellen eines Objekts aufgerufen)
        DC_Motor_Model(double R, double L, double Kt, double Ke, double J, double B);

        // Die Update-Funktion (berechnet einen Zeitschritt)
        void update(double voltage, double dt);

        // Getter-Funktionen (um die Werte später im main.cpp auszulesen)
        // Das "const" am Ende sagt C++: "Diese Funktion verändert keine Variablen"
        double getCurrent() const;
        double getSpeed() const;
};