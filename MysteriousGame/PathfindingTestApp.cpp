//#include <iostream>
//#include <array>
//#include <chrono>
//#include <vector>
//#include <set>
//#include <tuple>
//#include <algorithm>
//#include <memory>
//
//#include "CDT.h"
//#include "RStarTree.h"
//#include "MapShape.h"
//#include "PolyAnya.h"
//#include "Map.h"
//
//#define SDL_MAIN_USE_CALLBACKS 1
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>
//
//#define LOG std::cout << "[MapStateApp] "
//
//static SDL_Window* window = NULL;
//static SDL_Renderer* renderer = NULL;
//
//static std::vector<std::pair<sxi::AABB, int8_t>> aabbs;
//static std::vector<glm::vec2> points;
//static std::vector<glm::vec2> path;
//static std::vector<glm::vec2> polygon;
//static std::vector<int> edges;
//static std::vector<bool> constrained;
//constexpr float X_1 = 0;
//constexpr float Y_1 = 0;
//constexpr float X_2 = 720;
//constexpr float Y_2 = 1280;
//constexpr float X_BUFFER = (X_1 < 0) * -X_1 - (X_1 > 0) * X_1 + 15;
//constexpr float Y_BUFFER = (Y_1 < 0) * -Y_1 - (Y_1 > 0) * Y_1 + 15;
//constexpr float X_RES = X_2 - X_1;
//constexpr float Y_RES = Y_2 - Y_1;
//
//static bool underConstruction = false;
//static std::vector<glm::vec2> shapeUnderConstruction{};
//static int depth = 0;
//
//enum Views
//{
//    CONNECTIONS = 0,
//    DEBUG_CONNECTIONS = 1,
//    AABBS = 2,
//    COUNT = 3
//};
//unsigned int view = 0;
//
//static std::chrono::duration<float> dt = std::chrono::duration<float>(1.f/60);
//static std::chrono::time_point<std::chrono::steady_clock> lastFrame;
//
//static std::unordered_map<unsigned int, bool> moves
//{
//    { SDLK_W, false },
//    { SDLK_A, false },
//    { SDLK_S, false },
//    { SDLK_D, false },
//    { SDLK_UP, false },
//    { SDLK_LEFT, false },
//    { SDLK_DOWN, false },
//    { SDLK_RIGHT, false },
//};
//
//enum Dir
//{
//    UP = 0,
//    LEFT = 1,
//    DOWN = 2,
//    RIGHT = 3
//};
//
//const int speed = 200;
//
//struct PathPoint
//{
//    glm::vec2 pos;
//    std::array<unsigned int, 4> moveKeys;
//
//    PathPoint(float x, float y, const std::array<unsigned int, 4>& mk) : pos(x, y), moveKeys(mk) {}
//    PathPoint(glm::vec2&& p, const std::array<unsigned int, 4>& mk) : pos(p), moveKeys(mk) {}
//
//    void update()
//    {
//        if (moves[moveKeys[UP]])
//        {
//            pos.x -= speed * dt.count();
//        }
//        if (moves[moveKeys[DOWN]])
//        {
//            pos.x += speed * dt.count();
//        }
//        if (moves[moveKeys[LEFT]])
//        {
//            pos.y -= speed * dt.count();
//        }
//        if (moves[moveKeys[RIGHT]])
//        {
//            pos.y += speed * dt.count();
//        }
//        pos = glm::clamp(pos, glm::vec2(X_1, Y_1) + 5.f, glm::vec2(X_2, Y_2) - 5.f);
//    }
//
//} start(80, 80, { SDLK_W, SDLK_A, SDLK_S, SDLK_D }), goal(650, 1000, {SDLK_UP, SDLK_LEFT, SDLK_DOWN, SDLK_RIGHT});
//
//static void collectRST()
//{
//    aabbs.clear();
//    aabbs = sxi::Map::instance().collectRST();
//    std::sort(aabbs.begin(), aabbs.end(), [](std::pair<sxi::AABB, int8_t> left, std::pair<sxi::AABB, int8_t> right) { return left.second > right.second; });
//}
//
//static void collectCDT(bool onlyOn)
//{
//    points.clear();
//    edges.clear();
//    constrained.clear();
//    sxi::Map::instance().collectCDT(points, constrained, edges, onlyOn);
//}
//
//static void getMousePos(float& x, float& y)
//{
//    SDL_GetMouseState(&y, &x);
//    y = fminf(fmaxf(Y_1, y - Y_BUFFER), Y_2);
//    x = fminf(fmaxf(X_1, x - X_BUFFER), X_2);
//}
//
//void updateMap()
//{
//    switch (view)
//    {
//    case Views::CONNECTIONS:
//        collectCDT(true);
//        break;
//    case Views::DEBUG_CONNECTIONS:
//        collectCDT(false);
//    case Views::AABBS:
//        collectRST();
//        break;
//    }
//}
//
//SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
//{
//    sxi::Map::instance().initialize(X_1, Y_1, X_2, Y_2);
//    updateMap();
//
//    SDL_SetAppMetadata("Map State", "1.0", "com.vicddc.mapstate");
//
//    if (!SDL_Init(SDL_INIT_VIDEO))
//    {
//        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//
//    if (!SDL_CreateWindowAndRenderer("MapState", Y_RES + 30, X_RES + 30, 0, &window, &renderer))
//    {
//        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
//        return SDL_APP_FAILURE;
//    }
//
//    lastFrame = std::chrono::steady_clock::now();
//    return SDL_APP_CONTINUE;
//}
//
//SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
//{
//    if (event->type == SDL_EVENT_QUIT)
//    {
//        return SDL_APP_SUCCESS;
//    }
//
//    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
//    {
//        underConstruction = true;
//        float x, y;
//        getMousePos(x, y);
//        shapeUnderConstruction.emplace_back(x, y);
//    }
//
//    if (event->type == SDL_EVENT_KEY_DOWN)
//    {
//        if (event->key.key == SDLK_HOME)
//        {
//            depth = std::min(sxi::RST::depth + 1, depth + 1);
//        }
//        if (event->key.key == SDLK_END)
//        {
//            depth = std::max(0, depth - 1);
//        }
//
//        if (event->key.key == SDLK_TAB)
//        {
//            underConstruction = false;
//            sxi::Map::instance().insert(sxi::ShapeType::Default, shapeUnderConstruction);
//            shapeUnderConstruction.clear();
//            updateMap();
//        }
//
//        if (event->key.key == SDLK_BACKSPACE)
//        {
//            float x, y;
//            getMousePos(x, y);
//            glm::vec2 mouse(x, y);
//            polygon.clear();
//            polygon = sxi::Map::instance().remove(mouse);
//            updateMap();
//        }
//
//        if (event->key.key == SDLK_ESCAPE)
//        {
//            if (underConstruction)
//            {
//                underConstruction = false;
//                shapeUnderConstruction.clear();
//            }
//            else
//            {
//                return SDL_APP_SUCCESS;
//            }
//        }
//        if (event->key.key == SDLK_SPACE)
//        {
//            view = (view + 1) % Views::COUNT;
//            updateMap();
//        }
//        
//        unsigned int key = event->key.key;
//        if (moves.find(key) != moves.end())
//        {
//            moves[key] = true;
//        }
//    }
//    if (event->type == SDL_EVENT_KEY_UP)
//    {
//        unsigned int key = event->key.key;
//        if (moves.find(key) != moves.end())
//        {
//            moves[key] = false;
//        }
//    }
//    return SDL_APP_CONTINUE;
//}
//
//static void ShowCDT()
//{
//    SDL_SetRenderDrawColor(renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
//    for (int i = 0; i < edges.size(); i += 2)
//    {
//        int start = edges[i];
//        int end = edges[i + 1];
//        if (constrained[i * 0.5])
//            SDL_SetRenderDrawColor(renderer, 80, 12, 21, SDL_ALPHA_OPAQUE);
//        else
//            SDL_SetRenderDrawColor(renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
//        float sx1 = X_BUFFER + points[start].x;
//        float sy1 = Y_BUFFER + points[start].y;
//        float sx2 = X_BUFFER + points[end].x;
//        float sy2 = Y_BUFFER + points[end].y;
//        SDL_RenderLine(renderer, sy1, sx1, sy2, sx2);
//    }
//
//    SDL_SetRenderDrawColor(renderer, 12, 21, 150, SDL_ALPHA_OPAQUE);
//    for (int i = 0; i < polygon.size(); i++)
//    {
//        float sx1 = X_BUFFER + polygon[i].x;
//        float sy1 = Y_BUFFER + polygon[i].y;
//        float sx2 = X_BUFFER + polygon[(i + 1) % polygon.size()].x;
//        float sy2 = Y_BUFFER + polygon[(i + 1) % polygon.size()].y;
//        SDL_RenderLine(renderer, sy1, sx1, sy2, sx2);
//    }
//}
//
//static std::vector<uint8_t> colors = {
//    255, 0, 0,
//    255, 255, 0,
//    0, 255, 0,
//    0, 255, 255,
//    0, 0, 255,
//};
//
//static void ShowRST()
//{
//    for (const std::pair<sxi::AABB, int8_t>& aabb : aabbs)
//    {
//        if (aabb.second < depth)
//            continue;
//
//        SDL_SetRenderDrawColor(renderer, colors[3 * aabb.second + 0], colors[3 * aabb.second + 1], colors[3 * aabb.second + 2], SDL_ALPHA_OPAQUE);
//        SDL_RenderLine(renderer, Y_BUFFER + aabb.first.topLeft.y, X_BUFFER + aabb.first.topLeft.x, Y_BUFFER + aabb.first.botRight.y, X_BUFFER +aabb.first.topLeft.x);
//        SDL_RenderLine(renderer, Y_BUFFER + aabb.first.botRight.y, X_BUFFER + aabb.first.topLeft.x, Y_BUFFER + aabb.first.botRight.y, X_BUFFER +aabb.first.botRight.x);
//        SDL_RenderLine(renderer, Y_BUFFER + aabb.first.botRight.y, X_BUFFER + aabb.first.botRight.x, Y_BUFFER + aabb.first.topLeft.y, X_BUFFER +aabb.first.botRight.x);
//        SDL_RenderLine(renderer, Y_BUFFER + aabb.first.topLeft.y, X_BUFFER + aabb.first.botRight.x, Y_BUFFER + aabb.first.topLeft.y, X_BUFFER +aabb.first.topLeft.x);
//    }
//}
//
//static void ShowPath()
//{
//    path.clear();
//    path = sxi::Map::instance().findPath(start.pos, goal.pos);
//
//    SDL_SetRenderDrawColor(renderer, 100, 100, 12, SDL_ALPHA_OPAQUE);
//    for (int i = 0; i < (int)path.size() - 1; i++)
//    {
//        float sx1 = X_BUFFER + path[i].x;
//        float sy1 = Y_BUFFER + path[i].y;
//        float sx2 = X_BUFFER + path[i + 1].x;
//        float sy2 = Y_BUFFER + path[i + 1].y;
//        SDL_RenderLine(renderer, sy1, sx1, sy2, sx2);
//    }
//    SDL_FRect rectStart = { Y_BUFFER + start.pos.y - 5, X_BUFFER + start.pos.x - 5, 10, 10 };
//    SDL_FRect rectGoal = { Y_BUFFER + goal.pos.y - 5, X_BUFFER + goal.pos.x - 5, 10, 10 };
//    SDL_SetRenderDrawColor(renderer, 120, 12, 120, SDL_ALPHA_OPAQUE);
//    SDL_RenderFillRect(renderer, &rectStart);
//    SDL_SetRenderDrawColor(renderer, 12, 120, 12, SDL_ALPHA_OPAQUE);
//    SDL_RenderFillRect(renderer, &rectGoal);
//}
//
//static void ShowShapeUnderConstruction()
//{
//    SDL_SetRenderDrawColor(renderer, 80, 80, 80, SDL_ALPHA_OPAQUE);
//    int lastPointIndex = shapeUnderConstruction.size() - 1;
//    for (int i = 0; i < lastPointIndex; i++)
//    {
//        float sx1 = X_BUFFER + shapeUnderConstruction[i].x;
//        float sy1 = Y_BUFFER + shapeUnderConstruction[i].y;
//        float sx2 = X_BUFFER + shapeUnderConstruction[i + 1].x;
//        float sy2 = Y_BUFFER + shapeUnderConstruction[i + 1].y;
//        SDL_RenderLine(renderer, sy1, sx1, sy2, sx2);
//    }
//
//    float x, y;
//    getMousePos(x, y);
//    float sx1 = X_BUFFER + shapeUnderConstruction[lastPointIndex].x;
//    float sy1 = Y_BUFFER + shapeUnderConstruction[lastPointIndex].y;
//    float sx2 = X_BUFFER + x;
//    float sy2 = Y_BUFFER + y;
//    SDL_RenderLine(renderer, sy1, sx1, sy2, sx2);
//}
//
//SDL_AppResult SDL_AppIterate(void* appstate)
//{   
//    auto now = std::chrono::steady_clock::now();
//    dt = std::chrono::duration<float>(now - lastFrame);
//    lastFrame = now;
//
//    start.update();
//    goal.update();
//
//    SDL_SetRenderDrawColor(renderer, 25, 25, 25, SDL_ALPHA_OPAQUE);
//    SDL_RenderClear(renderer);
//    if (view == Views::CONNECTIONS || view == Views::DEBUG_CONNECTIONS)
//        ShowCDT();
//    else if (view == Views::AABBS)
//        ShowRST();
//    if (underConstruction)
//        ShowShapeUnderConstruction();
//    ShowPath();
//    SDL_RenderPresent(renderer);
//    return SDL_APP_CONTINUE;
//}
//
//void SDL_AppQuit(void *appstate, SDL_AppResult result) {}
