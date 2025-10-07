#ifndef TRI_H
#define TRI_H
#include "mesh.h"

static const Mesh tri = {
    .data = (Vect3m[]) {
        {0.0f, 0.0f, 0.0f}, 
        {0.0f, 1.0f, 0.0f}, 
        {1.0f, 0.0f, 0.0f}, 
        
        {1.0f, 0.0f, 0.0f}, 
        {0.0f, 1.0f, 0.0f}, 
        {1.0f, 1.0f, 0.0f}
    },
    .color = (int[]) {1, 2},
    .count = (int) 2,
};

#endif
