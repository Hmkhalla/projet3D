#include "vcl/vcl.hpp"
#include <iostream>

#include "terrain.hpp"
#include "tree.hpp"

using namespace vcl;

struct gui_parameters {
	bool display_frame = true;
	bool add_sphere = true;
};

struct user_interaction_parameters {
	vec2 mouse_prev;
	timer_fps fps_record;
	mesh_drawable global_frame;
	gui_parameters gui;
	bool cursor_on_gui;
};

user_interaction_parameters user;

struct scene_environment
{
	camera_around_center camera;
	mat4 projection;
	vec3 light;
};
scene_environment scene;
perlin_noise_parameters parameters;


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_scene();
void display_interface();

mesh terrain;
mesh_drawable terrain_visual;
mesh_drawable tree;
mesh_drawable mushroom;
mesh_drawable billboard_grass;

std::vector<vcl::vec3> tree_position = generate_positions_on_terrain(50);
std::vector<vcl::vec3> mush_position = generate_positions_on_terrain(50);
std::vector<vcl::vec3> grass_position = generate_positions_on_terrain(60);

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = 1280, height = 1024;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();


	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		scene.light = scene.camera.position();
		user.fps_record.update();
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();

		if(user.gui.display_frame) draw(user.global_frame, scene);

		display_interface();
		display_scene();

		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}



void initialize_data()
{
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;
	
	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;
	scene.camera.distance_to_center = 2.5f;
	scene.camera.look_at({4,3,2}, {0,0,0}, {0,0,1});

    // Create visual terrain surface
	terrain = create_terrain();
    terrain_visual = mesh_drawable(terrain);
	update_terrain(terrain, terrain_visual, parameters);
    //terrain.shading.color = {0.6f,0.85f,0.5f};
	image_raw im = image_load_png("../02a_textures/assets/texture_grass.png");

	GLuint texture_image_id = opengl_texture_to_gpu(im, 
		GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_S*/, 
		GL_MIRRORED_REPEAT /**GL_TEXTURE_WRAP_T*/);
	terrain_visual.texture = texture_image_id;
    terrain_visual.shading.phong.specular = 0.0f; // non-specular terrain material

	tree = mesh_drawable(create_tree());

	mushroom = mesh_drawable(create_mushroom());

	billboard_grass = mesh_drawable(mesh_primitive_quadrangle());
	billboard_grass.transform.scale = 0.4f;
	billboard_grass.transform.translate = {0.5f, 0.5f, 0.0f};
	billboard_grass.texture = opengl_texture_to_gpu(image_load_png("../02c_billboards/assets/grass.png"));

}


void display_scene()
{
	draw(terrain_visual, scene);
	//draw_wireframe(terrain, scene);

	for(auto const & point : tree_position){
		const float u = point.x /20.f+0.5f;
		const float v = point.y /20.f+0.5f;
		float const noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, parameters.frequency_gain);
		float const z = point.z+ parameters.terrain_height*noise;
		tree.transform.translate = vcl::vec3({point.x, point.y, z});

		draw(tree, scene);
	}

	for(auto const & point : mush_position){
		const float u = point.x /20.f+0.5f;
		const float v = point.y /20.f+0.5f;
		float const noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, parameters.frequency_gain);
		float const z = point.z+ parameters.terrain_height*noise;
		mushroom.transform.translate = vcl::vec3({point.x, point.y, z});

		draw(mushroom, scene);
	}

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Disable depth buffer writing
    //  - Transparent elements cannot use depth buffer
    //  - They are supposed to be display from furest to nearest elements
    glDepthMask(false);

	billboard_grass.transform.rotate = scene.camera.orientation();

	for(auto const & point : grass_position){
		const float u = point.x /20.f+0.5f;
		const float v = point.y /20.f+0.5f;
		float const noise = noise_perlin({u, v}, parameters.octave, parameters.persistency, parameters.frequency_gain);
		float const z = point.z+ parameters.terrain_height*noise;
		billboard_grass.transform.translate = vcl::vec3({point.x, point.y, z});

		draw(billboard_grass, scene);
	}

	glDepthMask(true);
}


void display_interface()
{
	ImGui::Checkbox("Frame", &user.gui.display_frame);

	bool update = false;
	update |= ImGui::SliderFloat("Persistance", &parameters.persistency, 0.1f, 0.6f);
	update |= ImGui::SliderFloat("Frequency gain", &parameters.frequency_gain, 1.5f, 2.5f);
	update |= ImGui::SliderInt("Octave", &parameters.octave, 1, 8);
	update |= ImGui::SliderFloat("Height", &parameters.terrain_height, 0.1f, 1.5f);

	if(update)// if any slider has been changed - then update the terrain
		update_terrain(terrain, terrain_visual, parameters);
}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.1f, 100.0f);
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	if(!user.cursor_on_gui){
		if(state.mouse_click_left && !state.key_ctrl)
			scene.camera.manipulator_rotate_trackball(p0, p1);
		if(state.mouse_click_left && state.key_ctrl)
			camera.manipulator_translate_in_plane(p1-p0);
		if(state.mouse_click_right)
			camera.manipulator_scale_distance_to_center( (p1-p0).y );
	}

	user.mouse_prev = p1;
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	opengl_uniform(shader, "view", scene.camera.matrix_view());
	opengl_uniform(shader, "light", scene.light, false);
}



