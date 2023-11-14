#include "space.h"
#include <SFML/Graphics.hpp>
#include <fstream>
#include <algorithm>

#define fori(n) for (int i = 0; i < n; i++)
#define forj(n) for (int j = 0; j < n; j++)
#define fork(n) for (int k = 0; k < n; k++)
#include <SFML/Audio.hpp>
const double pi = 3.141592653589;

void rotate_point(num cx, num cy, num angle, num& x, num& y)
{
    num s = sin(angle);
    num c = cos(angle);
    // translate point back to origin:
    x -= cx;
    y -= cy;

    // rotate point
    num xnew = x * c - y * s;
    num ynew = x * s + y * c;

    // translate point back:
    x = xnew + cx;
    y = ynew + cy;
}

num ViewAngle = pi / 2;
num ZFar = 10, ZNear = 1;
num Height = 600, Width = 800;
Point View{ -4, 3, 0 };
Vector U{ 1, 0, 0 }, V{ 0, -1, 0 }, W{ 0, 0, 1 }, normalV, directv;

//original vectors
num Tr = 0.1, Tu = 0.1; //t right (the angle between direction vector and ox vector(at plane xoy)) ,t up (the angle between direction vector and plane xoy)
vector<Point> points;
vector<Point> Triangles;//triangles are points sch that   x ,y and z are indices of points inside the points vector
vector<sf::Color> TriangleColor;
vector<int> sortedTriangles;  //indices of triangles after sorting { we use indices to make sorting faster

vector<object3d> objects;
vector<object3d> dadObjects;
#define sT sortedTriangles

bool sameDirection = 0;

void loadFile(string file, Point scale = { 1, 1, 1 }, Point shift = { 0, 0, 0 }, sf::Color original = { 0, 0, 0, 0 })
{ //not tested
    ifstream in(file);
    int Np = points.size(),
        Nt = Triangles.size(),
        No = objects.size();
    bool globalColor = 0;
    char enter;
    int get, alpha;
    object3d getObj;
    Point get3;
    if (original.a != 0)
        globalColor = 1;
    sf::Color col = original;
    while (in >> enter)
    {
        switch (enter)
        {
        case 's':
        { //set global color
            in >> col.r >> col.b >> col.g >> col.a;
            globalColor = !globalColor;
            break;
        }
        case 'o':
        { //object
            in >> getObj.start >> getObj.len >> getObj.ptStart >> getObj.ptLen >> getObj.dad;
            getObj.start += Nt;
            getObj.ptStart += Np;
            if (getObj.dad != -1 &&
                getObj.dad < objects.size() - No)
            {
                objects[No + getObj.dad].son.pb(objects.size());
            }
            else
            {
                dadObjects.pb(getObj);
            }
            objects.pb(getObj);
            break;
        }
        case 't':
        { //triangle
            in >> get3.x >> get3.y >> get3.z;
            get3.x += Np;
            get3.y += Np;
            get3.z += Np;
            Triangles.pb(get3);
            if (globalColor == 0)
            {
                in >> get3.x >> get3.y >> get3.z >> alpha;
                TriangleColor.pb(sf::Color(get3.x, get3.y, get3.z, alpha));
            }
            else
            {
                TriangleColor.pb(col);
            }
            break;
        }
        case 'p':
        { //point
            in >> get3.x >> get3.y >> get3.z;
            get3 =
                Point{
                    get3.x * scale.x + shift.x,
                    get3.y * scale.y + shift.y,
                    get3.z * scale.z + shift.z };
            points.pb(get3);
            break;
        }
        case 'r':
        { //rectangle
            in >> get3.x >> get3.y >> get3.z;
            get3.x += Np;
            get3.y += Np;
            get3.z += Np;
            Triangles.pb(get3);
            get3.y = get3.z;
            in >> get3.z;
            get3.z += Np;
            Triangles.pb(get3);
            if (globalColor == 0)
            {
                in >> get3.x >> get3.y >> get3.z >> alpha;
                fori(2)
                    TriangleColor.pb(sf::Color(get3.x, get3.y, get3.z, alpha));
            }
            else
            {
                fori(2) TriangleColor.pb(col);
            }
            break;
        }
        case 'c':
        {
            in >> get;
            in >> get3.x >> get3.y >> get3.z;
            get3.x += Np;
            get3.y += Np;
            get3.z += Np;
            Triangles.pb(get3);
            fori(get - 3)
            {
                get3.y = get3.z;
                in >> get3.z;
                get3.z += Np;
                Triangles.pb(get3);
            }
            if (globalColor == 0)
            {
                in >> get3.x >> get3.y >> get3.z >> alpha;
                col = sf::Color(get3.x, get3.y, get3.z, alpha);
            }
            fori(get - 2)
                TriangleColor.pb(col);
            break;
        }
        default:
            break;
        }
    }
}

Point transform1(Point A, Point B)
{ //view from A to B
    Point res;
    num t = U.a * (B.x - A.x) + U.b * (B.y - A.y) + U.c * (B.z - A.z);
    t /= (U.a * U.a + U.b * U.b + U.c * U.c);
    Point C{ U.a * t + A.x, U.b * t + A.y, U.c * t + A.z };
    res.z = A.length(C);
    Vector CB{ B.x - C.x, B.y - C.y, B.z - C.z },
        AB{ B.x - A.x, B.y - A.y, B.z - A.z };
    res.x = (W.a * CB.b - CB.a * W.b) / (W.a * V.b - V.a * W.b);
    res.y = (V.a * CB.b - CB.a * V.b) / (W.a * V.b - V.a * W.b);
    sameDirection = CosAngle(AB, U) >= cos(3 * ViewAngle / 4);
    directv = AB;

    return res;
}

Point transform2(Point ob)
{
    num w = ob.z;
    ob.x *= (Height / Width) / tan(ViewAngle / 2);
    ob.y /= tan(ViewAngle / 2);
    ob.z = (ob.z - ZNear) * (ZFar / (ZFar - ZNear));
    if (w == 0)
    {
        w = 1;
    }

    ob.z /= w;
    ob.x = (ob.x * Width) / (2.0 * w) + Width / 2;
    ob.y = (ob.y * Height) / (2.0 * w) + Height / 2;

    return ob;
}

bool closer(int a1, int b1)
{
    //if center of triangle a1 is closer than center of triangle b1

    Point a = Triangles[a1],
        b = Triangles[b1];

    Point ca{ (points[a.x].x + points[a.y].x + points[a.z].x) / 3,
             (points[a.x].y + points[a.y].y + points[a.z].y) / 3,
             (points[a.x].z + points[a.y].z + points[a.z].z) / 3 },
        cb{ (points[b.x].x + points[b.y].x + points[b.z].x) / 3,
           (points[b.x].y + points[b.y].y + points[b.z].y) / 3,
           (points[b.x].z + points[b.y].z + points[b.z].z) / 3 };
    return (View.length(ca) > View.length(cb));
}

num fix(num a)
{//get only 6 digits after fixed point
    return a;
    a *= 1000000;
    a = int(a);
    a /= 1000000;
    return a;
}
void setUVW()
{
    U = Vector{//direction vector
        fix(cos(Tu) * cos(Tr)),
        fix(cos(Tu) * sin(Tr)),
        fix(sin(Tu)) };

    V = Vector{//the vector to your right
        fix( cos(Tr - num(pi / 2))),
        fix( sin(Tr - num(pi / 2))),
        fix(0) };
    W = Vector{//the vector to up
        fix(cos(Tu + num(pi / 2)) * cos(Tr)),
        fix(cos(Tu + num(pi / 2)) * sin(Tr)),
        fix(sin(Tu + num(pi / 2))) };
}

void moveObject(int id, Point direct, Point scale = { 1, 1, 1 })
{
    fori(objects[id].ptLen)
    {
        points[objects[id].ptStart + i].x *= scale.x;
        points[objects[id].ptStart + i].y *= scale.y;
        points[objects[id].ptStart + i].z *= scale.z;
        points[objects[id].ptStart + i].x += direct.x;
        points[objects[id].ptStart + i].y += direct.y;
        points[objects[id].ptStart + i].z += direct.z;
    }
}

int main()
{
    num speed = 0.1;

    sf::Keyboard::Key butt[10] = { 
        sf::Keyboard::Key::A,
        sf::Keyboard::Key::W,
        sf::Keyboard::Key::D,
        sf::Keyboard::Key::S,
        sf::Keyboard::Key::Left,
        sf::Keyboard::Key::Up,
        sf::Keyboard::Key::Right,
        sf::Keyboard::Key::Down,
        sf::Keyboard::Key::Space,
        sf::Keyboard::Key::LAlt
         };

    ofstream out("output.txt");
    bool get;
    Point a, b, c;
    sf::Color blackAndWhite;
    loadFile("diamond.txt", { 1,1,1 }, { 0,0,2 });
    fori(8)forj(8)
        loadFile("cube.txt", { 1, 1, 1 }, { 2 * num(i), 2 * num(j), 0 });

    sf::ConvexShape ver;

    fori(Triangles.size()) sT.pb(i);

    sf::RenderWindow win(sf::VideoMode(int(Height), int(Width)), "window");

#define sT sortedTriangles
    //music.play();
    while (win.isOpen())
    {
        sf::Event ev;
        while (win.pollEvent(ev))
        {
        }
        
            fori(10)
            {
                if (sf::Keyboard::isKeyPressed(butt[i]))
                {
                    if (i == 0)
                    {
                        Tr += 0.01;
                    }
                    else if (i == 1)
                    {
                        Tu += 0.01;
                    }
                    else if (i == 2)
                    {
                        Tr -= 0.01;
                    }
                    else if (i == 3)
                    {
                        Tu -= 0.01;
                    }
                    else if (i == 4)
                    {
                        View = Point{
                            View.x - speed * V.a,
                            View.y - speed * V.b,
                            View.z };
                    }
                    else if (i == 5)
                    {
                        View = Point{
                            View.x + speed * U.a,
                            View.y + speed * U.b,
                            View.z };
                    }
                    else if (i == 6)
                    {
                        View = Point{
                            View.x + speed * V.a,
                            View.y + speed * V.b,
                            View.z };
                    }
                    else if (i == 7)
                    {
                        View = Point{
                            View.x - speed * U.a,
                            View.y - speed * U.b,
                            View.z };
                    }
                    else if (i == 8)
                    {
                        View = Point{
                            View.x,
                            View.y,
                            View.z + speed };
                    }
                    else if (i == 9)
                    {
                        View = Point{
                            View.x,
                            View.y,
                            View.z - speed };
                    }
                }
            }
        
        sort(sT.begin(), sT.end(), &closer);

        win.clear();
        ver.setPointCount(3);
        fori(sT.size())
        { //draw triangles

            a = points[Triangles[sT[i]].x];
            b = points[Triangles[sT[i]].y];
            c = points[Triangles[sT[i]].z];
            normalV = createVector(a, b).cross(createVector(a, c));
            get = 0;
            a = transform1(View, a);
            a = transform2(a);
            get = (get) || (directv.dot(normalV) <= 0 && (sameDirection));
            b = transform1(View, b);
            b = transform2(b);
            get = (get) || (directv.dot(normalV) <= 0 && (sameDirection));
            c = transform1(View, c);
            c = transform2(c);
            get = (get) || (directv.dot(normalV) <= 0 && (sameDirection));

            if ((get))
            {


                ver.setPoint(0, sf::Vector2f(a.x, a.y));
                ver.setPoint(1, sf::Vector2f(b.x, b.y));
                ver.setPoint(2, sf::Vector2f(c.x, c.y));
                ver.setFillColor(TriangleColor[sT[i]]);
                win.draw(ver);
            }
        }

        win.display();
        setUVW();
    }
}