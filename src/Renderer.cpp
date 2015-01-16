#include <common.h>
#include <Renderer.h>

Renderer::Renderer() : m_timer(/*60.0*/300.0), m_depthBuffer(NULL) 
{}

Renderer::~Renderer()
{
    if (m_depthBuffer)
        delete[] m_depthBuffer;
}


void Renderer::Initialize(const char* title, int x, int y, int width, int height)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    m_window = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN_DESKTOP*/);
    m_screen = SDL_GetWindowSurface(m_window);

    m_framebuffer = (uint32_t*)m_screen->pixels;
    m_width = m_screen->w;
    m_height = m_screen->h;

    m_depthBuffer = new float[m_width*m_height];
}

void Renderer::MainLoop()
{
    SDL_Event e;
    bool quit = false;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                quit = true;
        }

        SDL_LockSurface(m_screen);
        Clear();

        std::string title = "FPS: " + std::to_string(m_timer.GetFPS());
        SDL_SetWindowTitle(m_window, title.c_str());
        m_timer.Update([this](double dt){ 
            if (m_update)
                m_update(dt); 
        });
        if (m_width > 0 && m_height > 0 && m_render) 
            m_render();
        SDL_UnlockSurface(m_screen);
        SDL_UpdateWindowSurface(m_window);
    }
}

void Renderer::CleanUp()
{
    SDL_FreeSurface(m_screen);
    SDL_DestroyWindow(m_window);
    SDL_Quit();

    if (m_depthBuffer)
    {
        delete[] m_depthBuffer;
        m_depthBuffer = NULL;
    }
}

 
