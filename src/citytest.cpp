#include "game/citygen/atomupdater.hpp"
#include "game/citygen/grid.hpp"
#include <SDL2/SDL.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <vector>

class AtomRenderer
{
public:
    explicit AtomRenderer(vkopter::game::citygen::Grid<256, 256, 1> &g)
        : grid(g)
    {
        using namespace std;
        using namespace std::filesystem;

        SDL_CreateWindowAndRenderer(1600, 900, 0, &win, &ren);

        vector<string> fileNames;
        for (const auto &e : directory_iterator("data/citygen/img")) {
            std::string fp = e.path().string();
            fileNames.push_back(fp);
        }
        sort(fileNames.begin(), fileNames.end());

        for (unsigned long i = 0; i < fileNames.size(); ++i) {
            auto img = SDL_LoadBMP(fileNames[i].c_str());
            images.push_back(SDL_CreateTextureFromSurface(ren, img));
            SDL_FreeSurface(img);
        }
    }

    ~AtomRenderer()
    {
        for (auto *i : images) {
            SDL_DestroyTexture(i);
        }
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
    }

    void render()
    {
        using namespace std::chrono;

        static high_resolution_clock::time_point t1;
        static high_resolution_clock::time_point t2;

        if (duration_cast<milliseconds>(t2 - t1).count() > 16) {
            SDL_RenderClear(ren);

            for (auto x = 0; x < grid.WIDTH; ++x) {
                for (auto y = 0; y < grid.HEIGHT; ++y) {
                    SDL_Rect dst{x * IMAGE_SIZE, y * IMAGE_SIZE, (IMAGE_SIZE), (IMAGE_SIZE)};
                    SDL_RenderCopy(ren, images[grid(x, y, 0).type], nullptr, &dst);
                }
            }

            SDL_RenderPresent(ren);

            t1 = high_resolution_clock::now();
        }

        t2 = high_resolution_clock::now();
    }

private:
    vkopter::game::citygen::Grid<256, 256, 1> &grid;

    SDL_Window *win;
    SDL_Renderer *ren;
    std::vector<SDL_Texture *> images;

    const int IMAGE_SIZE = 16;
};

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    vkopter::game::citygen::Grid<256, 256, 1> grid;
    vkopter::game::citygen::AtomUpdater<256, 256, 1, 4> au(grid);

    AtomRenderer ar(grid);

    SDL_Event e;

    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case SDLK_LEFT:
                    break;
                case SDLK_RIGHT:
                    break;
                case SDLK_UP:
                    break;
                case SDLK_DOWN:
                    break;
                case SDLK_s:
                    break;
                case SDLK_r:
                    //au.reloadScripts();
                    break;
                case SDLK_c:
                    grid.clear();
                    grid(16, 16, 0) = vkopter::game::citygen::RoadNS;
                    break;
                default:
                    break;
                }
            }
        }

        au.updateRnd();
        ar.render();
    }

    return 0;
}
