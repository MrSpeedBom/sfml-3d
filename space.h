#include <iostream>
using namespace std;
#include <vector>
#include <map>
#include <math.h>
#define ll long long
#define pb push_back
#define num float
#include <SFML/Graphics.hpp>
#include <iostream>
struct Point
{
    num x, y, z;
    num length(Point pt) {
        pt.x -= x; pt.y -= y; pt.z -= z;
        return sqrt(pt.x * pt.x + pt.y * pt.y + pt.z * pt.z);
    }
};

struct Vector
{
    num a, b, c;
    num length() { return sqrt(a * a + b * b + c * c); }
    num dot(Vector x)
    {
        return (a * x.a + b * x.b + c * x.c);
    }
    Vector cross(Vector x) {
        Vector res;
        res.a = x.c * b - x.b * c;
        res.b = -(x.c * a - x.a * c);
        res.c = x.b * a - x.a * b;
        return res;
    }
    void normalize()
    {
        num len = length();
        if (len == 0)
            return;
        a /= length();
        b /= length();
        c /= length();
    }
};
Vector createVector(Point a, Point b) {
    b.x -= a.x; b.y -= a.y; b.z -= a.z;
    return Vector{ b.x,b.y,b.z };
}

struct Line
{
    Point pt;
    Vector dir;
    Point getPoint(num t) {
        return Point{ dir.a * t + pt.x,
                               dir.b * t + pt.y,
                               dir.c * t + pt.z };
    }
};

struct Plane
{
    num a, b, c, d;
};

struct object3d {
    int start, len, ptStart, ptLen; bool draw = 1;
    int dad;
    vector<int> son;
};

num CosAngle(Vector a, Vector b)
{
    num result;
    result = a.dot(b) / (a.length() * b.length());
    return result;
}
num SinAngle(Vector a, Vector b) {
    num result;
    result = a.cross(b).length() / (a.length() * b.length());
    return result;
}

Point pointOnLine(Point a, Line d) {
    num t;
    t = ((a.x - d.pt.x) * d.dir.a + (a.y - d.pt.y) * d.dir.b + (a.z - d.pt.z) * d.dir.c)
        / (d.dir.a * d.dir.a + d.dir.b * d.dir.b + d.dir.c * d.dir.c);
    return d.getPoint(t);
}



