#include "carriere.hpp"
#include "terrain.hpp"

using namespace vcl;

mesh create_carriere(float u1,float u2,float v1,float v2, perlin_noise_parameters const& parameters){
    // Number of samples in each direction (assuming a square grid)
	mesh carriere;

    // Number of samples of the terrain is N x N
    const unsigned int N = 100;

    carriere.position.resize(N*N);
    carriere.uv.resize(N*N);


	// Recompute the new vertices
	for (int ku = 0; ku < N; ++ku) {
		for (int kv = 0; kv < N; ++kv) {
			
			// Compute local parametric coordinates (u,v) \in [0,1]
            const float u = u1 + (u2-u1)*ku/(N-1.0f);
            const float v = v1 + (v2-v1)*kv/(N-1.0f);

			int const idx = ku*N+kv;

            float noise = 0.0;

            noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, 100*parameters.frequency_gain);

			// use the noise as height value
			carriere.position[idx].z = evaluate_terrain(u,v).z + noise;

            

			// use also the noise as color value
			//terrain.color[idx] = 0.3f*vec3(0,0.5f,0)+0.7f*noise*vec3(1,1,1);
		}
    }

    for(size_t ku=0; ku<N-1; ++ku)
    {
        for(size_t kv=0; kv<N-1; ++kv)
        {
            const unsigned int idx = kv + N*ku; // current vertex offset

            const uint3 triangle_1 = {idx, idx+1+N, idx+1};
            const uint3 triangle_2 = {idx, idx+N, idx+1+N};

            carriere.connectivity.push_back(triangle_1);
            carriere.connectivity.push_back(triangle_2);
        }
    }
}