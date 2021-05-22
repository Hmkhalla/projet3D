#include "tree.hpp"

using namespace vcl;


mesh create_tree_trunk_cylinder(float radius, float height)
{
    mesh m; 
    const size_t N = 20;
    
    for (int i =0; i<N; i++){
        const float u = i/float(N);
        const vec3 p = {radius*std::cos(2*3.14*u), radius*std::sin(2*3.14*u), 0.0f};
        m.position.push_back(p);
        m.position.push_back(p + vec3(0, 0, height));
    }

    for (int i =0; i<N; i++){
        const unsigned int u00 = 2*i;
        const unsigned int u01 = (u00+1)%(2*N);
        const unsigned int u10 = (u00+2)%(2*N);
        const unsigned int u11 = (u00+3)%(2*N);

        const uint3 t1 = {u00, u01, u10};
        const uint3 t2 = {u01, u11, u10};

        m.connectivity.push_back(t1);
        m.connectivity.push_back(t2);
    }
    m.fill_empty_field();
    return m;
}

mesh create_cone(float radius, float height, float z_offset)
{
    mesh m; 
    const size_t N = 20;
    for (int i =0; i<N; i++){
        const float u = i/float(N);
        const vec3 p = {radius*std::cos(2*3.14*u), radius*std::sin(2*3.14*u), z_offset};
        m.position.push_back(p);
    }
    m.position.push_back(vec3(0.0f, 0.0f, z_offset));
    m.position.push_back(vec3(0.0f, 0.0f, z_offset+height));

    for (int i =0; i<N; i++){
        const uint3 t1 = {i, (i+1)%N, N};
        const uint3 t2 = {i, (i+1)%N, N+1};

        m.connectivity.push_back(t1);
        m.connectivity.push_back(t2);
    }

    m.fill_empty_field();
    return m;
}

mesh create_tree()
{
    float const h = 0.7f; // trunk height
    float const r = 0.1f; // trunk radius

    // Create a brown trunk
    mesh trunk = create_tree_trunk_cylinder(r, h);
    trunk.color.fill({0.4f, 0.3f, 0.3f});


    // Create a green foliage from 3 cones
    mesh foliage = create_cone(4*r, 6*r, 0.0f);      // base-cone
    foliage.push_back(create_cone(4*r, 6*r, 2*r));   // middle-cone
    foliage.push_back(create_cone(4*r, 6*r, 4*r));   // top-cone
    foliage.position += vec3(0,0,h);                 // place foliage at the top of the trunk
    foliage.color.fill({0.4f, 0.6f, 0.3f});
       
    // The tree is composted of the trunk and the foliage
    mesh tree = trunk;
    tree.push_back(foliage);

    return tree;
}


vcl::mesh create_mushroom(){
    float const h = 0.2f;
    float const r = 0.05f;

    mesh trunk = create_tree_trunk_cylinder(r, h);
    trunk.color.fill({179.f/255., 179.f/255., 179.f/255.});
    mesh foliage = create_cone(4*r, 2*r, 0.0f);
    foliage.position += vec3(0,0,h);                 // place foliage at the top of the trunk
    foliage.color.fill({1.f, 45.f/255.,0});

    mesh mushroom = trunk;
    mushroom.push_back(foliage);

    return mushroom;

}

