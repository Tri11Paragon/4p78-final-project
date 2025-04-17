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
#include <deque>
#include <fcntl.h>
#include <future>
#include <imgui.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <blt/gfx/window.h>
#include <blt/gfx/renderer/font_renderer.h>
#include <blt/std/requests.h>
#include <blt/std/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include "blt/gfx/renderer/resource_manager.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#endif

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager resources;
blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
blt::gfx::font_renderer_t fr2d{};
blt::gfx::first_person_camera_2d camera;

std::array<char, 100> buffer;
size_t last_time_ran = 0;
size_t last_time_ran1 = 0;

int send_socket = -1;

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

struct boy_trust_t
{
	std::deque<blt::vec2f> point_cloud;
	std::vector<blt::gfx::line2d_t> lines;

	void consolidate(const blt::u64 run_time)
	{
		const auto cur_time = blt::system::getCurrentTimeMilliseconds();
		if (cur_time - last_time_ran1 > run_time)
		{
			last_time_ran1 = cur_time;
			// constexpr float min_dist = 5;
			// for (auto [i, a] : enumerate(point_cloud))
			// {
			// 	for (auto [j, b] : enumerate(point_cloud))
			// 	{
			// 		if (i == j)
			// 			continue;
			// 		auto diff = (a - b).abs();
			// 		if (diff.x() < min_dist && diff.y() < min_dist)
			// 		{
			//
			// 		}
			// 	}
			// }
		}
	}
} point_data;

void handle_data(needed_t& data)
{
	if (data.distance > 8000)
		return;
	blt::vec2f current_position;
	current_position[0] = data.position[0] + data.distance * std::cos(data.yaw + static_cast<float>(blt::PI / 2.0f));
	current_position[1] = data.position[1] + data.distance * std::sin(data.yaw + static_cast<float>(blt::PI / 2.0f));
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
	fr2d.create_default(250, 2048);
}

void update(const blt::gfx::window_data& data)
{
	global_matrices.update_perspectives(data.width, data.height, 90, 0.1, 2000);

	camera.update();
	camera.update_view(global_matrices);
	global_matrices.update();

	static float point_size = 25;

	static sockaddr_in server_addr{};
	if (send_socket == -1 && ready)
	{
		send_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (send_socket < 0)
			throw std::runtime_error("Error creating socket");

		std::memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(42069);

		if (inet_pton(AF_INET, buffer.data(), &server_addr.sin_addr) <= 0)
		{
			close(send_socket);
			throw std::runtime_error("Invalid address or address not supported.");
		}

		if (connect(send_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
		{
			close(send_socket);
			throw std::runtime_error("Error connecting socket");
		}

		fcntl(send_socket, F_SETFL, O_NONBLOCK);
	}

	auto cur_time = blt::system::getCurrentTimeMilliseconds();
	if (send_socket != -1 && (cur_time - last_time_ran > 100))
	{
		constexpr blt::u32 pack = 1;
		// if (sendto(send_socket, &pack, sizeof(pack), 0, reinterpret_cast<const sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
		// {
		// BLT_WARN("Failed to send packet!");
		// }
		if (send(send_socket, &pack, sizeof(pack), 0) < 0)
		{
			BLT_WARN("Failed to send packet!");
		}
		last_time_ran = cur_time;
	}

	static needed_t robot_data;
	static char local_buff[1024];
	ssize_t recv_size;
	if ((recv_size = recv(send_socket, local_buff, sizeof(local_buff), 0)) > 0)
	{
		blt::u32 id;
		std::memcpy(&id, local_buff, sizeof(blt::u32));
		switch (id)
		{
			case 1:
			{
				std::memcpy(&robot_data, local_buff + sizeof(blt::u32)*2, sizeof(needed_t));
				robot_data.position *= 25.4;

				handle_data(robot_data);
			}
			break;
			case 3: BLT_ERROR("Not implemented!");
				break;
			case 0:
				point_data.point_cloud.clear();
				break;
			case 2: BLT_WARN("Received set target command, not supported on client!");
				break;
			default: BLT_WARN("Received unknown command '{}', not supported on client! Recv: {}", id, recv_size);
				break;
		}
	}

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

	renderer_2d.drawPoint(blt::gfx::point2d_t{robot_data.position, 35}, blt::make_color(0, 0, 1), 3);
	renderer_2d.drawLine(blt::gfx::line2d_t{robot_data.position, {0, 0}}, blt::make_color(0, 0, 1), 2);
	fr2d.render_text("Yaw " + std::to_string(robot_data.yaw), 32).setPosition(robot_data.position + blt::vec2{10, 0});

	blt::vec2f current_position;
	current_position[0] = robot_data.position[0] + 100 * std::cos(robot_data.yaw);
	current_position[1] = robot_data.position[1] + 100 * std::sin(robot_data.yaw);
	renderer_2d.drawLine(blt::gfx::line2d_t{robot_data.position, current_position}, blt::make_color(1, 0, 0), 2);

	for (const auto& point_cloud : point_data.point_cloud)
		renderer_2d.drawPoint(blt::gfx::point2d_t{point_cloud, point_size}, blt::make_color(0, 1, 0), 1);
	for (const auto& line : point_data.lines)
		renderer_2d.drawLine(line, blt::make_color(1, 0, 0), 0);

	renderer_2d.render(data.width, data.height);
	fr2d.render();
}

void destroy(const blt::gfx::window_data&)
{
	close(send_socket);
	global_matrices.cleanup();
	resources.cleanup();
	renderer_2d.cleanup();
	fr2d.cleanup();
	blt::gfx::cleanup();
}

int main()
{
	blt::gfx::init(blt::gfx::window_data{"Draw Window", init, update, destroy}.setSyncInterval(1));
	std::exit(0);
}
