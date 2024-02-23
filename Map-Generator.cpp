// Map-Generator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <raylib.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <raygui.h>
#include <filesystem>
#include <algorithm>
#include "Map-Generator.h"

#define RAYGUI_IMPLEMENTATION

#define FONT_LOC {"D:/Coding/Raylib C++/Map-Generator/fonts/Anta/Anta-Regular.ttf"}

const Color PROGRESS_COLOR = { 0, 168, 117, 255 };

std::vector<int> vector_steps{ 2, 4, 5, 8, 10 };
std::vector<float> vector_thick{ 1.25F, 1.5F, 2.0F, 3.0F, 5.0F };

size_t first_on_index = 1;
static int step_size = vector_steps[first_on_index];
float line_thick = 1.5F;

struct Coor
{
    int x;
    int y;
};

std::vector<Coor> walker(int number_steps) {
    std::vector<Coor> the_random_walk(number_steps, Coor{ 0,0 });

    static Coor coor = { 0,0 };
    for (int i = 1; i < number_steps; i++) {
        coor = the_random_walk[i - 1];
        int value = GetRandomValue(1, 4);
        switch (value)
        {
        case 1:
            coor.x += 1;
            break;
        case 2:
            coor.x -= 1;
            break;
        case 3:
            coor.y += 1;
            break;
        case 4:
            coor.y -= 1;
            break;
        default:
            break;
        }
        the_random_walk[i] = coor;
    }
    return the_random_walk;
}

int RoundToNearestMultipleOfSteps(int value)
{
    return (value + 2) / step_size * step_size;
}

Coor GetRandomPosition()
{
    int random_x = GetRandomValue(GetScreenWidth() * 0.4F, GetScreenWidth() * 0.6F);
    int random_y = GetRandomValue(GetScreenHeight() * 0.4F, GetScreenHeight() * 0.6F);

    random_x = RoundToNearestMultipleOfSteps(random_x);
    random_y = RoundToNearestMultipleOfSteps(random_y);

    return { random_x, random_y };
}

void ResizeStartStepsSize(std::vector<Coor>& start_position) {
    for (auto& coor : start_position) {
        coor.x = RoundToNearestMultipleOfSteps(coor.x);
        coor.y = RoundToNearestMultipleOfSteps(coor.y);
    }
}


const int number_steps = 25000;
const int total_walker = 7;

const int step_per_delay = 50;
int the_step_random_walk = number_steps / step_per_delay;

std::vector<Coor> random_start_position() {
    std::vector<Coor> random_start{};
    for (int i = 0; i < total_walker; i++) {
        random_start.push_back(GetRandomPosition());
    }

    return random_start;
}


std::string GetPaddedFilePath(int number) {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << number;

    return oss.str();
}

void isFolderMapExists(const std::filesystem::path& mapFolder) {
    if (!std::filesystem::exists(mapFolder)) {
        std::filesystem::create_directories(mapFolder);
        std::cout << "INFO: Create folder: " << mapFolder << std::endl;
    }
    else {
        std::cout << "INFO: " << mapFolder << " folder already exists." << std::endl;
    }
}

void MoveToMapFolder(const std::string& filename) {
    std::filesystem::path mapFolder = "MAP";

    std::filesystem::path search_path = filename;
    std::filesystem::path dest_path = mapFolder / filename;
    std::filesystem::rename(search_path, dest_path);

    std::cout << "INFO: File: " << filename << " moved to: " << dest_path << std::endl;
}

int GetNextIterationNumber() {
    std::filesystem::path mapFolder = "MAP";
    int max_number = 0;

    isFolderMapExists(mapFolder);

    for (const auto& entry : std::filesystem::directory_iterator(mapFolder)) {

        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();

            if (filename.substr(0, 3) == "map") {
                int number = std::stoi(filename.substr(3, 2));
                max_number = std::max(max_number, number);
            }
        }
    }

    return max_number + 1;
}

int main()
{
    const Color BASE_COLOR = Color{ 30, 30, 30, 55 };
    const Color MAP_COLOR = Color{ 30, 40, 50, 255 };

    const int screenWidth = 1600;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    //SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(screenWidth, screenHeight, "Map Generator");
    SetTargetFPS(60);

    Font font = LoadFontEx(FONT_LOC, 70, 0, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);


    std::vector<Coor> start_position{};
    start_position = random_start_position();


    std::vector<std::vector<Coor>> all_walks{};
    for (int i = 0; i < total_walker; i++) {
        all_walks.push_back(walker(number_steps));
    }

    // std::vector<Color> colors{
    //     {210, 210, 210, 255}, 
    //     {200, 200, 200, 255}, 
    //     {160, 160, 160, 255}, 
    //     {190, 190, 190, 255}, 
    //     {180, 180, 180, 255}, 
    //     {170, 170, 170, 255}, 
    //     {220, 220, 220, 255}
    // };

    std::vector<Color> colors{
        {35, 164, 255, 255},    // Dodger Blue for Lakes and Rivers
        {30, 139, 34, 255},    // Dark Green for Forests
        {85, 107, 47, 255},     // Dark Olive Green for Marshes
        {210, 180, 140, 255},   // Tan for Deserts
        {160, 82, 45, 255},     // Sienna for Mountains or Highlands
        {255, 255, 0, 255},     // Yellow for Plains or Grasslands
        {192, 192, 192, 255}    // Silver for Rocky Areas
    };




    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BASE_COLOR);

        float screen_w = static_cast<float>(GetScreenWidth());
        float screen_h = static_cast<float>(GetScreenHeight());

        static float map_width = 0;
        static float map_height = 0;
        static Rectangle map = {};
        static Rectangle title_rect = {};

        static float horizontal_ratio_coef = 0.7F;
        static float vertical_ratio_coef = 0.8F;

        static bool isFullScreenShot = false;
        static bool isCleanScreenShot = false;

        {
            // TITLE MAPS
            if (!isCleanScreenShot) {
                title_rect = {
                    (map.x),
                    (map.y - (map.height * 0.135F)),
                    (map.width),
                    (map.height * 0.10F)
                };
            }
            else {
                title_rect = {
                    (map.x),
                    (map.y - (map.height * 0.135F)) + (screen_h * 0.01F),
                    (map.width),
                    (map.height * 0.10F)
                };
            }
            // DrawRectangleRec(title_rect, BLACK);

            float font_size = title_rect.height * 0.95F;
            float font_space = 0.0F;
            const char* title_text = "MAPS";
            Vector2 title_text_measure = MeasureTextEx(font, title_text, font_size, font_space);
            Vector2 title_text_coor = {
                title_rect.x + (title_rect.width - title_text_measure.x) / 2,
                title_rect.y + (title_rect.height - title_text_measure.y) / 2
            };

            //if (!isScreenShot) {
                DrawTextEx(font, title_text, title_text_coor, font_size, font_space, RAYWHITE);
            //}

        }


        {
            float horizontal_ratio = 14.0F / 9.0F;
            float square_ratio = 9.0F / 9.0F;
            float vertical_ratio = 9.0F / 14.0F;

            float aspect_ratio = 16.0F / 9.0F;
            float window_ratio = screen_w / screen_h;

            
            if (window_ratio >= horizontal_ratio) {
                map_height = screen_h * horizontal_ratio_coef;
                map_width = map_height * aspect_ratio;
            }
            else if (window_ratio <= horizontal_ratio && window_ratio >= square_ratio) {
                map_width = screen_w * vertical_ratio_coef;
                map_height = map_width / aspect_ratio;
            }
            else if (window_ratio <= square_ratio && window_ratio >= vertical_ratio) {
                map_width = screen_w * vertical_ratio_coef;
                map_height = map_width / aspect_ratio;
            }
            else if (window_ratio <= vertical_ratio) {
                map_width = screen_w * vertical_ratio_coef;
                map_height = map_width / aspect_ratio;
            }


            if (!isCleanScreenShot) {
                map = {
                    ((screen_w - map_width) / 2),
                    ((screen_h - map_height) / 2),
                    map_width,
                    map_height
                };
            }
            else {
                map = {
                    ((screen_w - map_width) / 2),
                    ((screen_h - map_height) / 2) + (screen_h * 0.04F),
                    map_width,
                    map_height
                };
            }

            // DRAW BASE MAP
            DrawRectangleRounded(map, 0.05F, 10, MAP_COLOR);
            
            // OUTERLINE OF MAP
            DrawRectangleRoundedLines(map, 0.05F, 10, 2.F, LIGHTGRAY);
            

            float padding_scissor = 5.F;
            BeginScissorMode(
                map.x + (padding_scissor * 1),
                map.y + (padding_scissor * 1),
                map.width - (padding_scissor * 2),
                map.height - (padding_scissor * 2)
            );

            for (size_t i = 0; i < all_walks.size(); i++) {
                for (size_t j = 0; j < all_walks[i].size() - 1; j++) {

                    Coor titik_awal = all_walks[i][j];
                    Coor titik_akhir = all_walks[i][j + 1];

                    DrawLineEx(
                        { (float)(start_position[i].x + step_size * titik_awal.x), (float)(start_position[i].y + step_size * titik_awal.y) },
                        { (float)(start_position[i].x + step_size * titik_akhir.x), (float)(start_position[i].y + step_size * titik_akhir.y) },
                        line_thick,
                        colors[i]
                    );

                    // DrawCircle(
                    //    start_position[k].x + step_size * titik_awal.x,
                    //    start_position[k].y + step_size * titik_awal.y,
                    //    1.F,
                    //    RED
                    // );
                }
            }

            EndScissorMode();

            

            {
                // STEP SIZE BUTTONS BASE
                float base_height = map_height * 0.6F;
                float base_width = base_height * 0.2F;
                Rectangle base = {
                    map.x + map.width + (base_width * 0.2F),
                    map.y + (map.height - base_height) / 2,
                    base_width,
                    base_height
                };
                // DrawRectangleRec(base, BLACK);

                float inner_y = base.height * 0.1F;
                Rectangle inner_base = {
                    base.x,
                    base.y + inner_y,
                    base.width,
                    base.height - inner_y
                };
                // DrawRectangleRec(inner_base, GRAY);

                float font_size = inner_y * 0.65F;
                float font_space = 0.0F;
                const char* size_text = "STEP";
                Vector2 size_text_measure = MeasureTextEx(font, size_text, font_size, font_space);
                Vector2 size_text_coor = {
                    base.x + (base.width - size_text_measure.x) / 2,
                    base.y + (inner_y - size_text_measure.y) / 2
                };
                if (!isCleanScreenShot) {
                    DrawTextEx(font, size_text, size_text_coor, font_size, font_space, LIGHTGRAY);
                }

                // BUTTONS SIZE STEP
                size_t vector_size = vector_steps.size();
                for (size_t i = 0; i < vector_size; i++) {
                    float base_button_height = inner_base.height / vector_size;
                    static size_t index_button = first_on_index;

                    Rectangle base_button = {
                        inner_base.x,
                        inner_base.y + (i * base_button_height),
                        inner_base.width,
                        base_button_height
                    };

                    Color color = GRAY;
                    float padding = base_button_height * 0.075F;
                    Rectangle button = {
                        base_button.x + (padding * 1),
                        base_button.y + (padding * 1),
                        base_button.width - (padding * 2),
                        base_button.height - (padding * 2)
                    };

                    // HOVER BUTTON
                    if (CheckCollisionPointRec(GetMousePosition(), button)) {
                        color = LIGHTGRAY;

                        // CLICK BUTTON
                        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                            step_size = vector_steps[i];
                            ResizeStartStepsSize(start_position);
                            index_button = i;
                        }
                    }
                    else {
                        color = GRAY;
                    }

                    if (index_button == i) {
                        color = SKYBLUE;
                    }

                    if (!isCleanScreenShot) {
                        DrawRectangleRounded(button, 0.2F, 10, color);
                    }

                    float font_size = button.height * 0.6F;
                    float font_space = 0.0F;
                    std::string step_size_string = std::to_string((i + 1));
                    const char* step_size_text = step_size_string.c_str();
                    Vector2 step_text_measure = MeasureTextEx(font, step_size_text, font_size, font_space);
                    Vector2 step_text_coor = {
                        button.x + (button.width - step_text_measure.x) / 2,
                        button.y + (button.height - step_text_measure.y) / 2
                    };

                    if (!isCleanScreenShot) {
                        DrawTextEx(font, step_size_text, step_text_coor, font_size, font_space, BLACK);
                    }
                }


            }

            {
                if (!isCleanScreenShot) {
                    // LINE THICK BUTTONS BASE
                    float base_height = map_height * 0.6F;
                    float base_width = base_height * 0.2F;
                    Rectangle base = {
                        map.x - base_width - (base_width * 0.2F),
                        map.y + (map.height - base_height) / 2,
                        base_width,
                        base_height
                    };
                    // DrawRectangleRec(base, BLACK);

                    float inner_y = base.height * 0.1F;
                    Rectangle inner_base = {
                        base.x,
                        base.y + inner_y,
                        base.width,
                        base.height - inner_y
                    };
                    // DrawRectangleRec(inner_base, GRAY);

                    float font_size = inner_y * 0.65F;
                    float font_space = 0.0F;
                    const char* size_text = "THICK";
                    Vector2 size_text_measure = MeasureTextEx(font, size_text, font_size, font_space);
                    Vector2 size_text_coor = {
                        base.x + (base.width - size_text_measure.x) / 2,
                        base.y + (inner_y - size_text_measure.y) / 2
                    };
                    DrawTextEx(font, size_text, size_text_coor, font_size, font_space, LIGHTGRAY);

                    // BUTTONS LINE THICK
                    size_t vector_size = vector_thick.size();
                    for (size_t i = 0; i < vector_size; i++) {
                        float base_button_height = inner_base.height / vector_size;
                        static size_t index_button = first_on_index;

                        Rectangle base_button = {
                            inner_base.x,
                            inner_base.y + (i * base_button_height),
                            inner_base.width,
                            base_button_height
                        };

                        Color color = GRAY;
                        float padding = base_button_height * 0.075F;
                        Rectangle button = {
                            base_button.x + (padding * 1),
                            base_button.y + (padding * 1),
                            base_button.width - (padding * 2),
                            base_button.height - (padding * 2)
                        };

                        // HOVER BUTTON
                        if (CheckCollisionPointRec(GetMousePosition(), button)) {
                            color = LIGHTGRAY;

                            // CLICK BUTTON
                            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                                line_thick = vector_thick[i];
                                index_button = i;
                            }
                        }
                        else {
                            color = GRAY;
                        }

                        if (index_button == i) {
                            color = SKYBLUE;
                        }

                        DrawRectangleRounded(button, 0.2F, 10, color);

                        float font_size = button.height * 0.6F;
                        float font_space = 0.0F;
                        std::string step_size_string = std::to_string((i + 1));
                        const char* step_size_text = step_size_string.c_str();
                        Vector2 step_text_measure = MeasureTextEx(font, step_size_text, font_size, font_space);
                        Vector2 step_text_coor = {
                            button.x + (button.width - step_text_measure.x) / 2,
                            button.y + (button.height - step_text_measure.y) / 2
                        };
                        DrawTextEx(font, step_size_text, step_text_coor, font_size, font_space, BLACK);

                    }
                }

            }

            if (isFullScreenShot) {
                ScreenShot();
                isFullScreenShot = false;
                horizontal_ratio_coef = 0.7F;
            }

            if (isCleanScreenShot) {
                ScreenShot();
                isCleanScreenShot = false;
                horizontal_ratio_coef = 0.7F;
            }

        }

        {
            // BUTTON RETRY RANDOMs

        }

        {
            // DRAW INFO MAPS RATIO
            if (!isCleanScreenShot) {
                Rectangle ratio_rect = {
                    (map.x),
                    // (map.y + map.height + (map.height * 0.0225F)),
                    (map.y - (map.height * 0.025F * 3)),
                    (map.width / 3),
                    (map.height * 0.065F)
                };
                // DrawRectangleRounded(ratio_rect, 0.2F, 10, BLACK);

                std::string map_size_char = "MAP SIZE: ";
                std::string width = std::to_string((int)map.width);
                std::string height = std::to_string((int)map.height);
                std::string ratio_char = "RATIO: ";

                float the_ratio = (float)(map.width / map.height);
                std::stringstream stream{};
                stream << std::fixed << std::setprecision(2) << the_ratio;
                std::string ratio = stream.str();
                std::string text = map_size_char + width + "/" + height + "  |  " + ratio_char + ratio;

                float font_size = ratio_rect.height * 0.60F;
                float font_space = 0.0F;
                const char* ratio_text = text.c_str();
                Vector2 ratio_text_measure = MeasureTextEx(font, ratio_text, font_size, font_space);
                Vector2 ratio_text_coor = {
                    ratio_rect.x + 10,
                    ratio_rect.y + (ratio_rect.height - ratio_text_measure.y) / 2,
                };
                DrawTextEx(font, ratio_text, ratio_text_coor, font_size, font_space, RAYWHITE);
            }
        }

        {
            // DRAW MAP PROGRESS
            if (!isCleanScreenShot) {
                Rectangle progress_rect = {
                    (map.x + (map.width / 3) * 2),
                    (map.y + map.height + (map.height * 0.02F)),
                    (map.width / 3),
                    (map.height * 0.065F)
                };
                // DrawRectangleRec(progress_rect, BLACK);

                std::string map_progress_char = "PROGRESS ";
                float progress = (static_cast<float>(the_step_random_walk * step_per_delay) / (number_steps)) * 100.0F;
                int progress_percent = (int)progress;
                std::string progress_char = std::to_string(progress_percent);
                std::string text = map_progress_char;

                float font_size = progress_rect.height * 0.55F;
                float font_space = 0.0F;
                float roundness = 0.3F;
                const char* progress_text = text.c_str();
                Vector2 progress_text_measure = MeasureTextEx(font, progress_text, font_size, font_space);
                Vector2 progress_text_coor = {
                    progress_rect.x + 10,
                    progress_rect.y + (progress_rect.height - progress_text_measure.y) / 2
                };
                DrawTextEx(font, progress_text, progress_text_coor, font_size, font_space, RAYWHITE);

                float progress_height = progress_text_measure.y + 6;
                Rectangle the_progress_rect = {
                    progress_text_coor.x + progress_text_measure.x + 5,
                    progress_rect.y + (progress_rect.height - progress_height) / 2,
                    progress_rect.width - (progress_text_measure.x) - (10 * 2),
                    progress_height
                };
                // DrawRectangleRec(the_progress_rect, RAYWHITE);
                // DrawRectangleRounded(the_progress_rect, roundness, 10, BLACK);

                float padding = the_progress_rect.height * 0.1F;
                Rectangle inner_progress_rect = {
                    the_progress_rect.x + (padding * 1),
                    the_progress_rect.y + (padding * 1),
                    the_progress_rect.width - (padding * 2),
                    the_progress_rect.height - (padding * 2)
                };

                float progress_bar_height = inner_progress_rect.height;
                float pregress_bar_width = inner_progress_rect.width * 0.725F;
                Rectangle progress_bar_area = {
                    inner_progress_rect.x + 0,
                    inner_progress_rect.y + (inner_progress_rect.height - progress_bar_height) / 2,
                    pregress_bar_width,
                    progress_bar_height
                };
                DrawRectangleRounded(progress_bar_area, roundness, 10, RAYWHITE);

                float bar_padding = progress_bar_area.height * 0.1F;
                float progress_bar_width = (progress_percent / 100.F) * progress_bar_area.width;
                Rectangle progress_bar = {
                    progress_bar_area.x + (bar_padding * 1),
                    progress_bar_area.y + (bar_padding * 1),
                    progress_bar_width - (bar_padding * 2),
                    progress_bar_area.height - (bar_padding * 2),
                };
                DrawRectangleRounded(progress_bar, roundness, 10, LIME);

                Rectangle percentage_area = {
                    inner_progress_rect.x + (pregress_bar_width)+4,
                    inner_progress_rect.y + (inner_progress_rect.height - progress_bar_height) / 2,
                    inner_progress_rect.width * 0.275F - 4,
                    progress_bar_height
                };

                font_size *= 0.9F;
                std::string percent_char = std::to_string(progress_percent);
                text = percent_char + "%";
                const char* percent_text = text.c_str();
                Vector2 percent_text_measure = MeasureTextEx(font, percent_text, font_size, font_space);
                Vector2 percent_text_coor = {
                    percentage_area.x + (percentage_area.width - percent_text_measure.x) / 2,
                    percentage_area.y + (percentage_area.height - percent_text_measure.y) / 2
                };
                DrawTextEx(font, percent_text, percent_text_coor, font_size, font_space, RAYWHITE);
            }

        }


        

        if (IsKeyPressed(KEY_ONE)) {
            step_size = 4;
            ResizeStartStepsSize(start_position);
        }
        else if (IsKeyPressed(KEY_TWO)) {
            step_size = 6;
            ResizeStartStepsSize(start_position);
        }
        else if (IsKeyPressed(KEY_THREE)) {
            step_size = 8;
            ResizeStartStepsSize(start_position);
        }
        else if (IsKeyPressed(KEY_FOUR)) {
            step_size = 10;
            ResizeStartStepsSize(start_position);
        }
        else if (IsKeyPressed(KEY_FOUR)) {
            step_size = 10;
            ResizeStartStepsSize(start_position);
        }

        // TODO: MAKE SCREENSHOT MECHANISM
        // MOVE THE screenshot to MAP folder, after screenshot, search map.png.
        // 
        // TODO: HAVE 2 MODE OF SCREENSHOT:
        // 1 THE SCREEN
        // 2 ONLY MAPS, AKA: ZOOM TO ONLY MAPS AND SCREENSHOT
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S) || IsKeyDown(KEY_RIGHT_CONTROL) && IsKeyPressed(KEY_S)) {
            isFullScreenShot = true;
            horizontal_ratio_coef = 1.1F;
            
        }
         else if (IsKeyPressed(KEY_S)) {
            isCleanScreenShot = true;
            horizontal_ratio_coef = 0.75F;
         }

        // INSTANT RETRY REMAP
        static bool remap = false;
        if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_SPACE)) || (IsKeyDown(KEY_RIGHT_CONTROL) && IsKeyPressed(KEY_SPACE))) {
            start_position = random_start_position();
            all_walks.clear();

            for (int i = 0; i < total_walker; i++) {
                all_walks.push_back(walker(number_steps));
            }
        }
        // CINEMATIC RETRY REMAP
        else if (IsKeyPressed(KEY_SPACE) && remap == false) {
            start_position = random_start_position();
            all_walks.clear();
            all_walks.resize(total_walker);
            remap = true;
            the_step_random_walk = 0;
        }

        if (remap == true) {
            // static int the_walker = 0;

            static bool not_delay = true;
            static std::chrono::steady_clock::time_point delay_timer_start{};

            if (not_delay == false) {
                auto current_time = std::chrono::steady_clock::now();
                auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - delay_timer_start).count();

                if (elapsed_time > 1) {
                    not_delay = true;
                }
            }


            if (the_step_random_walk <= (number_steps / step_per_delay)) {

                if (not_delay == true) {
                    for (int i = 0; i < total_walker; i++) {
                        Coor coor = { 0,0 };
                        if (!all_walks[i].empty()) {
                            coor = all_walks[i].back();
                        }

                        for (int j = 0; j < step_per_delay; j++) {

                            int value = GetRandomValue(1, 4);
                            switch (value)
                            {
                            case 1:
                                coor.x += 1;
                                break;
                            case 2:
                                coor.x -= 1;
                                break;
                            case 3:
                                coor.y += 1;
                                break;
                            case 4:
                                coor.y -= 1;
                                break;
                            default:
                                break;
                            }

                            all_walks[i].push_back(coor);
                        }
                    }

                    the_step_random_walk++;
                    not_delay = false;
                    delay_timer_start = std::chrono::steady_clock::now();
                }

            }
            else {
                remap = false;

            }
        }

        
        {
            float height_offset = 0.8F;

            if (!isCleanScreenShot) {
                // INSTANT REMAP
                Rectangle remap_section_rect = {
                    (map.x),
                    (map.y + map.height + (map.height * 0.02F)),
                    (map.width / 3),
                    (map.height * 0.065F)
                };
                 //DrawRectangleRec(remap_section_rect, BLACK);

                float font_size = remap_section_rect.height * 0.55F;
                float font_space = 0.0F;

                const char* instant_text = "- INSTANT REMAP          ";
                Vector2 instant_text_measure = MeasureTextEx(font, instant_text, font_size, font_space);
                Vector2 instant_text_coor = {
                    remap_section_rect.x + 10,
                    remap_section_rect.y + (remap_section_rect.height - instant_text_measure.y) / 2
                };
                DrawTextEx(font, instant_text, instant_text_coor, font_size, font_space, RAYWHITE);

                // KEY BINDINGS
                const char* key_bindings_text = ": CTRL + SPACE";
                Vector2 key_bindings_coor = {
                    remap_section_rect.x + instant_text_measure.x,
                    instant_text_coor.y
                };
                DrawTextEx(font, key_bindings_text, key_bindings_coor, font_size, font_space, RAYWHITE);

                // CINEMATIC REMAP
                const char* cinematic_text = "- CINEMATIC REMAP";
                // Vector2 cinematic_text_measure = MeasureTextEx(font, cinematic_text, font_size, font_space);
                Vector2 cinematic_text_coor = {
                    instant_text_coor.x,
                    instant_text_coor.y + (remap_section_rect.height * height_offset)
                };
                DrawTextEx(font, cinematic_text, cinematic_text_coor, font_size, font_space, RAYWHITE);

                // KEY BINDINGS
                key_bindings_text = ": SPACE";
                key_bindings_coor = {
                    remap_section_rect.x + instant_text_measure.x,
                    cinematic_text_coor.y
                };
                DrawTextEx(font, key_bindings_text, key_bindings_coor, font_size, font_space, RAYWHITE);

                // FULL SCREENSHOT
                Rectangle screenshot_section_rect = remap_section_rect;
                screenshot_section_rect.x += (map.width / 3);
                //DrawRectangleRec(screenshot_section_rect, GRAY);

                const char* full_screenshot_text = "- FULL SCREENSHOT           ";
                Vector2 full_screenshot_text_measure = MeasureTextEx(font, full_screenshot_text, font_size, font_space);
                Vector2 full_screenshot_text_coor = {
                    screenshot_section_rect.x + 25,
                    screenshot_section_rect.y + (screenshot_section_rect.height - full_screenshot_text_measure.y) / 2
                };
                DrawTextEx(font, full_screenshot_text, full_screenshot_text_coor, font_size, font_space, RAYWHITE);
                // KEY BINDINGS
                key_bindings_text = ": CTRL + S";
                key_bindings_coor = {
                    screenshot_section_rect.x + full_screenshot_text_measure.x,
                    full_screenshot_text_coor.y
                };
                DrawTextEx(font, key_bindings_text, key_bindings_coor, font_size, font_space, RAYWHITE);

                // CLEAN SCREENSHOT
                const char* clean_screenshot_text = "- CLEAN SCREENSHOT";
                Vector2 clean_screenshot_text_coor = {
                    full_screenshot_text_coor.x,
                    full_screenshot_text_coor.y + (screenshot_section_rect.height * height_offset)
                };
                DrawTextEx(font, clean_screenshot_text, clean_screenshot_text_coor, font_size, font_space, RAYWHITE);
                // KEY BINDINGS
                key_bindings_text = ": S";
                key_bindings_coor = {
                    screenshot_section_rect.x + full_screenshot_text_measure.x,
                    clean_screenshot_text_coor.y
                };
                DrawTextEx(font, key_bindings_text, key_bindings_coor, font_size, font_space, RAYWHITE);

            }

        }

        

        EndDrawing();

    }

    CloseWindow();
    return 0;
}

void ScreenShot()
{
    int iter = GetNextIterationNumber();

    std::string filename = "map" + GetPaddedFilePath(iter) + ".png";

    TakeScreenshot(filename.c_str());

    MoveToMapFolder(filename);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
