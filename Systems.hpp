#ifndef SYSTEMS_HPP
#define SYSTEMS_HPP

#include <cmath>

constexpr std::size_t FPS = 200;

constexpr double TimeSpeed = 1.0 / FPS;
constexpr double MotorWeight = 50;

class MovementSystem 
{
private:
    double targetVel = 0;
    double vel = 0;
    double pos = 0;
    
public:
    void setVel(double in)
    { targetVel = std::max(std::min(in, 1.0), -1.0); }

    void setPos(double in)
    { 
        vel = 0;
        targetVel = 0;
        pos = in; 
    }

    void update() 
    {
        vel *= MotorWeight - 1;
        vel += targetVel;
        vel /= MotorWeight;

        pos += vel * TimeSpeed;
    }

    double getPos() const { return pos; }
};

struct PIDLoop 
{
    double P, I, D;
    double integral = 0, previous_error = 0, derivative = 0;
    double target = 0;
    MovementSystem system;

    PIDLoop(double inP, double inI, double inD) 
        : P{inP}, I{inI}, D{inD} { reset(); }

    void setPID(double inP, double inI, double inD) 
    { P = inP; I = inI; D = inD; }

    void reset() 
    {  
        integral = 0; 
        derivative = 0;
        previous_error = 0; 
        target = 0;
        system.setPos(target); 
    }

    void setTarget(double in) { target = in; }
    void setPos(double in) { return system.setPos(in); }
    double getPos() const { return system.getPos(); }

    void update() {
        const double error = target - getPos();

        integral += error * TimeSpeed;

        derivative = (error - previous_error) / TimeSpeed;
        previous_error = error;

        system.setVel(P * error + I * integral + D * derivative);
        system.update();
    }

};

#endif