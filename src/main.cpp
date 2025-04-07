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

struct parker_json_t
{
    float motorTargetAngle;
    float position;
    struct anglePID_t
    {
        float setpoint;
        float input;
        float output;
    } anglePID;
    struct ypr_t
    {
        float yaw;
        float pitch;
        float roll;
    } ypr;
    struct euler_t
    {
        float psi;
        float theta;
        float phi;
    } euler;
    struct gravity_t
    {
        float x;
        float y;
        float z;
    } gravity;
    struct q_t
    {
        float x;
        float y;
        float z;
        float w;
    } q;
    struct aa_t
    {
        float x;
        float y;
        float z;
    } aa;
    struct gy_t
    {
        float x;
        float y;
        float z;
    } gy;
    struct aaReal_t
    {
        float x;
        float y;
        float z;
    } aaReal;
    struct aaWorld_t
    {
        float x;
        float y;
        float z;
    } aaWorld;
};

void check_for_request()
{
    const auto cur_time = blt::system::getCurrentTimeMilliseconds();
    if (cur_time - last_time_ran > 250)
    {
        last_time_ran = cur_time;
        const std::string_view fuck_you{buffer.data()};
        std::string parker_hates_this{fuck_you};
        auto cstr = parker_hates_this.c_str();
        parker_json_t data{};
        EM_ASM(
            {
                const v = await fetch('http://$0/get_stuff', {
                    'credentials': 'omit',
                    'headers': {
                        'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0',
                        'Accept': '/',
                        'Accept-Language': 'en-US,en;q=0.5',
                        'Priority': 'u=4'
                    },
                    'method': 'GET',
                    'mode': 'cors'
                });
                if (!v.ok) {
                    return v.status;
                }

                const j = await v.json();
                const floatArray = j.values;
                Module.HEAPF32.set(floatArray, $1 >> 2);
            },
            cstr,
            reinterpret_cast<float*>(&data)
        );
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

    check_for_request();

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