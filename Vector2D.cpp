#include "Vector2D.hpp"

Vector2D::Vector2D(double X, double Y) : x(X), y(Y) {
    norme = sqrt(X * X + Y * Y);
}

void Vector2D::redef(double X, double Y) {
    // Redéfinie le vecteur et calcul sa nouvelle norme
    x = X;
    y = Y;
    norme = sqrt(X * X + Y * Y);
}

void Vector2D::plus(double X, double Y) {
    x += X;
    y += Y;
    norme = sqrt(x * x + y * y);
}

Vector2D& Vector2D::operator+=(const Vector2D& o) {
    // Ajoute le vecteur de droite au vecteur de gauche
    x += o.x;
    y += o.y;
    norme = sqrt(x * x + y * y);
    return *this;
}

Vector2D Vector2D::operator+(const Vector2D& o) {
    // Ajoute le vecteur de droite au vecteur de gauche
    Vector2D v;
    v.x = x + o.x;
    v.y = y + o.y;
    v.norme = sqrt(v.x * v.x + v.y * v.y);
    return v;
}

Vector2D& Vector2D::operator-=(const Vector2D& o) {
    // Ajoute le vecteur de droite au vecteur de gauche
    x -= o.x;
    y -= o.y;
    norme = sqrt(x * x + y * y);
    return *this;
}

Vector2D Vector2D::operator-(const Vector2D& o) {
    // Ajoute le vecteur de droite au vecteur de gauche
    Vector2D v;
    v.x = x - o.x;
    v.y = y - o.y;
    v.norme = sqrt(v.x * v.x + v.y * v.y);
    return v;
}

Vector2D& Vector2D::operator*=(const double a) {
    // Multiplie le vecteur par le scalaire à gauche
    x *= a;
    y *= a;
    norme *= std::abs(a);
    return *this;
}

Vector2D Vector2D::operator*(const double a) {
    // Multiplie le vecteur par le scalaire à gauche
    Vector2D v;
    v.x = x * a;
    v.y = y * a;
    v.norme = norme*std::abs(a);
    return v;
}

Vector2D& Vector2D::operator/=(const double a) {
    // Divise le vecteur par le scalaire à gauche
    if (!a) return *this;
    x /= a;
    y /= a;
    norme /= std::abs(a);
    return *this;
}

Vector2D Vector2D::operator/(const double a) {
    // Divise le vecteur par le scalaire à gauche
    if (!a) return Vector2D(x, y);
    Vector2D v;
    v.x = x / a;
    v.y = y / a;
    v.norme = norme / std::abs(a);
    return v;
}

void Vector2D::normeToV(double newNorme) {
    // Redéfinie la longueur du vecteur à newNorme sans changer sa direction
    if (norme) {
        *this *= newNorme / norme;
    }
}

void Vector2D::printSelf() {
    std::cout << "x=" << x << ", y=" << y << ", norme=" << norme << std::endl;
}