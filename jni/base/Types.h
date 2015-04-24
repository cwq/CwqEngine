#ifndef BASE_TYPES_H
#define BASE_TYPES_H

struct Vertex3F {
    Vertex3F() : x(0), y(0), z(0) {}

    float x;
    float y;
    float z;
};

struct Color4F {
    Color4F() : r(0), g(0), b(0), a(0) {}

    float r;
    float g;
    float b;
    float a;
};

struct Tex2F {
    Tex2F() : u(0), v(0) {}

    float u;
    float v;
};

struct V3F_C4F_T2F {
    Vertex3F vertices;
    Color4F colors;
    Tex2F texCoords;
};

struct V3F_C4F_T2F_Quad {
    //! top left
    V3F_C4F_T2F    tl;
    //! bottom left
    V3F_C4F_T2F    bl;
    //! top right
    V3F_C4F_T2F    tr;
    //! bottom right
    V3F_C4F_T2F    br;
};

#endif //!BASE_TYPES_H
