
#include "terrain.hpp"

using namespace vcl;

// Evaluate 3D position of the terrain for any (u,v) \in [0,1]
vec3 evaluate_terrain(float u, float v)
{
    float const x = 20*(u-0.5f);
    float const y = 20*(v-0.5f);

    //vec2 const u0 = {0.5f, 0.5f};
    //float const h0 = 2.0f;
    //float const sigma0 = 0.15f;
    int const n = 4;
    vec2 const pi [n] = {{0.66f, 1.0f}, {0.5f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};
    float const hi[n] = {3.0f, 1.5f, 4.0f, 2.0f};
    float const sigmai[n] = {0.15f, 0.1f, 0.2f, 0.2f};

    float z = 0;
    for (int i=0; i<n; i++){
        float const d = norm(vec2(u,v)-pi[i])/sigmai[i];
        z += hi[i]*std::exp(-d*d);
    }
    
    return {x,y,z};
}

mesh create_terrain()
{
    // Number of samples of the terrain is N x N
    const unsigned int N = 100;

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.uv.resize(N*N);

    // Fill terrain geometry
    for(unsigned int ku=0; ku<N; ++ku)
    {
        for(unsigned int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);

            // Compute the local surface function
            vec3 const p = evaluate_terrain(u,v);

            // Store vertex coordinates
            terrain.position[kv+N*ku] = p;
            terrain.uv[kv+N*ku] = {5*u, 5*v};
        }
    }

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    for(size_t ku=0; ku<N-1; ++ku)
    {
        for(size_t kv=0; kv<N-1; ++kv)
        {
            const unsigned int idx = kv + N*ku; // current vertex offset

            const uint3 triangle_1 = {idx, idx+1+N, idx+1};
            const uint3 triangle_2 = {idx, idx+N, idx+1+N};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

	terrain.fill_empty_field(); // need to call this function to fill the other buffer with default values (normal, color, etc)
    return terrain;
}

std::vector<vcl::vec3> generate_positions_on_terrain(int N){
    std::vector<vec3> points = std::vector<vec3>();
    for (int i=0; i<N; i++){
        const float u = rand_interval();
        const float v = rand_interval();
        float const x = 20*(u-0.5f);
        float const y = 20*(v-0.5f);
        int j;
        for (j = 0; j < i && norm(vec2(x, y)- vec2(points[j][0], points[j][1])) > 0.8f;  j++ ){
        }
        if (j==i) {
            points.push_back(evaluate_terrain(u, v));
        }else i--;
    }
    return points;
}



void update_terrain(mesh& terrain, mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters)
{
	// Number of samples in each direction (assuming a square grid)
	int const N = std::sqrt(terrain.position.size());

	// Recompute the new vertices
	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {
			
			// Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku/(N-1.0f);
            const float v = kv/(N-1.0f);

			int const idx = ku*N+kv;

            float noise = 0.0;

            if ((u>0.6 || u<0.3) || (v>0.55 || v<0.35)){
			    // Compute the Perlin noise
			    noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, parameters.frequency_gain);
            }
            else {noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, 0*parameters.frequency_gain);}

			// use the noise as height value
			terrain.position[idx].z = evaluate_terrain(u,v).z + parameters.terrain_height*noise;

            

			// use also the noise as color value
			//terrain.color[idx] = 0.3f*vec3(0,0.5f,0)+0.7f*noise*vec3(1,1,1);
		}
	}

	// Update the normal of the mesh structure
	terrain.compute_normal();
	
	// Update step: Allows to update a mesh_drawable without creating a new one
	terrain_visual.update_position(terrain.position);
	terrain_visual.update_normal(terrain.normal);
	terrain_visual.update_color(terrain.color);
	
}
