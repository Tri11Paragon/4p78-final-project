/*
*  Copyright (C) 2024  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <imgui.h>
#include <blt/gfx/window.h>
#include <blt/std/time.h>
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include "blt/gfx/renderer/resource_manager.h"
#include <string>
#include <blt/std/requests.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#endif

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager resources;
blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera camera;

std::array<char, 100> buffer;
size_t last_time_ran = 0;

struct everything_t
{
	float motorTargetAngle = 0;
	float distance_reading = 0;
	float position = 0;
	blt::vec3f anglePID;
	blt::vec3f posPID;
	blt::vec3f ypr;
	blt::vec3f euler;
	blt::vec3f gravity;
	blt::vec4f q;
	blt::vec3f aa;
	blt::vec3f gy;
	blt::vec3f aaReal;
	blt::vec3f aaWorld;
};

blt::vec2 current_position;

void check_for_request()
{
	const auto cur_time = blt::system::getCurrentTimeMilliseconds();
	if (cur_time - last_time_ran > 250)
	{
		last_time_ran = cur_time;
		const std::string_view bad_code{buffer.data()};
		const std::string this_is_terrible{bad_code};
		const auto result = blt::requests::send_get_request("http://" + this_is_terrible + "/get_stuff_bin");
		if (result.size() != sizeof(everything_t))
		{
			BLT_WARN("Size of string from ESP32 ({}) doesn't match the size of the struct ({})", result.size(), sizeof(everything_t));
			return;
		}
		everything_t data;
		std::memcpy(&data, result.data(), sizeof(everything_t));
	}
}

void init(const blt::gfx::window_data&)
{
	using namespace blt::gfx;

	global_matrices.create_internals();
	resources.load_resources();
	renderer_2d.create();
}

void update(const blt::gfx::window_data& data)
{
	global_matrices.update_perspectives(data.width, data.height, 90, 0.1, 2000);

	camera.update();
	camera.update_view(global_matrices);
	global_matrices.update();

	ImGui::SetNextWindowSize(ImVec2(300, static_cast<float>(data.height)));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		ImGui::InputText("IP Address", buffer.data(), buffer.size());
	}
	ImGui::End();

	renderer_2d.render(data.width, data.height);
}

void destroy(const blt::gfx::window_data&)
{
	global_matrices.cleanup();
	resources.cleanup();
	renderer_2d.cleanup();
	blt::gfx::cleanup();
}

int main()
{
	blt::gfx::init(blt::gfx::window_data{"Draw Window", init, update, destroy}.setSyncInterval(1));
}
