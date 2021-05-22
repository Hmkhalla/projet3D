#pragma once

#include "vcl/vcl.hpp"


struct perlin_noise_parameters
{
	float persistency = 0.6f;
	float frequency_gain = 2.392f;
	int octave = 8;
	float terrain_height = 0.923f;
};

vcl::vec3 evaluate_terrain(float u, float v);
vcl::mesh create_terrain();
std::vector<vcl::vec3> generate_positions_on_terrain(int N);
void update_terrain(vcl::mesh& terrain, vcl::mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters);
