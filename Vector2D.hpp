#pragma once
//#ifndef VECTOR2D_HPP
//#define VECTOR2D_HPP

#include <math.h>
#include <iostream>

class Vector2D {
public:
    double x;
    double y;
    double norme;

    Vector2D() = default; // Doesn't initialise values
    Vector2D(double X, double Y);
    void redef(double X, double Y);
    void plus(double X, double Y);
    Vector2D& operator+=(const Vector2D& o);
    Vector2D operator+(const Vector2D& o);
    Vector2D& operator-=(const Vector2D& o);
    Vector2D operator-(const Vector2D& o);
    Vector2D& operator*=(const double a);
    Vector2D operator*(const double a);
    Vector2D& operator/=(const double a); // Doesn't check if a != 0
    Vector2D operator/(const double a); // Doesn't check if a != 0
    
    

    inline static double dot(const Vector2D& v1, const Vector2D& v2) { return v1.x * v2.x + v1.y * v2.y; };
    inline static double vectorial(const Vector2D& v1, const Vector2D& v2) { return v1.x * v2.y - v1.y * v2.x; }; // In 2D we consider the result of a vectorial product to be a scalar

    void normeToV(double newNorme);
    inline void calcNorm() { norme = sqrt(x * x + y * y); };

    void printSelf();
};

//#endif