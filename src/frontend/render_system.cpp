#include "render_system.hpp"namespace MegaEmu{    namespace Frontend    {        RenderSystem::RenderSystem()            : window(nullptr), renderer(nullptr), initialized(false)        {        }        RenderSystem::~RenderSystem()        {            Shutdown();        }        bool RenderSystem::Initialize(const char *title, int width, int height)        {            if (initialized)            {                return true;            }            window = SDL_CreateWindow(                title,                SDL_WINDOWPOS_CENTERED,                SDL_WINDOWPOS_CENTERED,                width,                height,                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);            if (!window)            {                return false;            }            renderer = SDL_CreateRenderer(                window,                -1,                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);            if (!renderer)            {                SDL_DestroyWindow(window);                window = nullptr;                return false;            }            initialized = true;            return true;        }        void RenderSystem::Shutdown()        {            if (renderer)            {                SDL_DestroyRenderer(renderer);                renderer = nullptr;            }            if (window)            {                SDL_DestroyWindow(window);                window = nullptr;            }            initialized = false;        }        void RenderSystem::Clear()        {            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);            SDL_RenderClear(renderer);        }        void RenderSystem::Present()        {            SDL_RenderPresent(renderer);        }        void RenderSystem::DrawTexture(SDL_Texture *texture, const SDL_Rect *src, const SDL_Rect *dst)        {            if (texture)            {                SDL_RenderCopy(renderer, texture, src, dst);            }        }        void RenderSystem::DrawRect(const SDL_Rect *rect, const SDL_Color &color)        {            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);            SDL_RenderFillRect(renderer, rect);        }        void RenderSystem::DrawText(const char *text, int x, int y, const SDL_Color &color)        {            // TODO: Implementar renderização de texto usando SDL_ttf        }    } // namespace Frontend} // namespace MegaEmu