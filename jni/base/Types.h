#ifndef BASE_TYPES_H
#define BASE_TYPES_H

struct V3F_C4F_T2F {
    float vertices[3] = {0.0f};
    float colors[4] = {0.0f};
    float texCoords[2] = {0.0f};
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
