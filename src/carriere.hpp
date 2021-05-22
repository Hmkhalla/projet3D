#pragma once

#include "vcl/vcl.hpp"


struct perlin_noise_parameters
{
	float persistency = 0.6f;
	float frequency_gain = 2.392f;
	int octave = 8;
	float carriere_height = 0.923f;
};

vcl::mesh create_carriere();