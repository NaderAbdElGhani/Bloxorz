#include <glad/glad.h>
#include "vertex.h"

template<typename T>
inline void defineAttributePointer(){
    static_assert(false, "Type has no attribute pointer definition");
}

//The follow pointers assume that the layout is:
//Location 0: Position
//Location 1: Color
//Location 2: UV
//Location 3: Normal

template<>
inline void defineAttributePointer<ColoredVertex>(){
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)offsetof(ColoredVertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)offsetof(ColoredVertex, color));
    glEnableVertexAttribArray(1);
}

template<>
inline void defineAttributePointer<TexturedVertex>(){
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, uv));
    glEnableVertexAttribArray(2);
}

template<>
inline void defineAttributePointer<Vertex>(){
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(3);
}