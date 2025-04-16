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
blt::gfx::first_person_camera_2d camera;

std::array<char, 100> buffer;
size_t last_time_ran = 0;

bool ready = false;

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

struct needed_t
{
	float yaw = 0;
	float distance = 0;
	blt::vec2f position;
};

blt::vec2 current_position;

struct boy_trust_t
{
	std::vector<blt::vec2f> point_cloud;
	std::vector<blt::gfx::line2d_t> lines;

	void consolidate(blt::u64 run_time)
	{

	}
} point_data;

bool check_for_request(needed_t& data)
{
	if (!ready)
		return false;
	const auto cur_time = blt::system::getCurrentTimeMilliseconds();
	if (cur_time - last_time_ran > 100)
	{
		last_time_ran = cur_time;
		const std::string_view bad_code{buffer.data()};
		if (bad_code.empty())
			return false;
		const std::string this_is_terrible{bad_code};
		const auto result = blt::requests::send_get_request("http://" + this_is_terrible + "/get_stuff_bin");
		if (result.size() != sizeof(needed_t))
		{
			BLT_WARN("Got string {}", result);
			BLT_WARN("Size of string from ESP32 ({}) doesn't match the size of the struct ({})", result.size(), sizeof(needed_t));
			return false;
		}
		std::memcpy(&data, result.data(), sizeof(needed_t));
		data.position *= 25.4;
		// blt::mem::fromBytes<true>(result.data(), data.yaw);
		// blt::mem::fromBytes<true>(result.data() + sizeof(float), data.distance);
		// blt::mem::fromBytes<true>(result.data() + sizeof(float) * 2, data.position);
		// BLT_TRACE("STRING {}", result);
		// BLT_TRACE("GOT {} {} {} {}", data.yaw, data.distance, data.position.x(), data.position.y());
		return true;
	}
	return false;
}

void handle_data(needed_t& data)
{
	if (data.distance > 8000)
		return;
	blt::vec2f current_position;
	current_position[0] = data.position[0] + data.distance * std::cos(data.yaw + static_cast<float>(blt::PI/2.0f));
	current_position[1] = data.position[1] + data.distance * std::sin(data.yaw + static_cast<float>(blt::PI/2.0f));
	point_data.point_cloud.push_back(current_position);
}

void init(const blt::gfx::window_data&)
{
	using namespace blt::gfx;

	std::memset(buffer.data(), 0, buffer.size());
	std::strcpy(buffer.data(), "192.168.5.11");

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

	static float point_size = 25;

	ImGui::SetNextWindowSize(ImVec2(300, static_cast<float>(data.height)));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		ImGui::InputText("IP Address", buffer.data(), buffer.size());
		ImGui::Checkbox("Run", &ready);
		ImGui::InputFloat("Point Size", &point_size);

		if (ImGui::CollapsingHeader("Point Consolidation"))
		{
			static bool run_point = false;
			static int run_time = 100;
			ImGui::Checkbox("Run", &run_point);
			ImGui::InputInt("Run Time", &run_time);
			if (run_point)
				point_data.consolidate(static_cast<blt::u64>(run_time));
		}
	}
	ImGui::End();

	needed_t robot_data;
	if (check_for_request(robot_data))
	{
		handle_data(robot_data);
	}

	for (const auto& point_cloud : point_data.point_cloud)
		renderer_2d.drawPoint(blt::gfx::point2d_t{point_cloud, point_size}, blt::make_color(0, 1, 0), 1);
	for (const auto& line : point_data.lines)
		renderer_2d.drawLine(line, blt::make_color(1, 0, 0), 0);

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
