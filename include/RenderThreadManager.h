#pragma once
#include <atomic>

const int NUM_THREADS = 8;

class Renderer;
class RenderThreadManager
{
public:
    ~RenderThreadManager() { Destroy(); }
    void Destroy()
    {
        destroy = true;
        for (int i=0; i<NUM_THREADS; ++i)
            if (threads[i].joinable())
                threads[i].join();
    }

    std::thread threads[NUM_THREADS];
    bool running[NUM_THREADS];
    bool destroy;
    std::function<void(int offset, size_t numTriangles)> draw;

    size_t m_numTriangles[NUM_THREADS];
    int m_offset[NUM_THREADS];

    Renderer* renderer;
    std::atomic<int> runningThreads;

    void Initialize()
    {
        destroy = false;
        for (int i=0; i<NUM_THREADS; ++i)
        {
            threads[i] = std::thread([this, i]() { RenderThread(i); });
            running[i] = false;
        }
    }

    void RenderThread(int i)
    {
        while (!destroy)
        {
            if (!running[i])
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            else
            {
                draw(m_offset[i], m_numTriangles[i]);
                running[i] = false;
                runningThreads--;
            }
        }
    }

    
    template<int N>
    void DrawTriangles(void(*fragmentShader)(Point<N>&), uint16_t* indexBuffer, size_t numTriangles, bool backfaceVisible,
                        vec4* vs, Point<N>* points, int offset=0);

    template<int N>
    void DrawTrianglesThreaded(void(*fragmentShader)(Point<N>&), uint16_t* indexBuffer, size_t numTriangles, bool backfaceVisible,
                        vec4* vs, Point<N>* points)
    {
        runningThreads = NUM_THREADS;
        draw = [this, fragmentShader, indexBuffer, backfaceVisible, vs, points]
               (int offset, size_t numTriangles) {
                  DrawTriangles(fragmentShader, indexBuffer, numTriangles, backfaceVisible, vs, points, offset);
              };

        for (int i=0; i<NUM_THREADS; ++i)
        {
            m_numTriangles[i] = numTriangles/NUM_THREADS;
            m_offset[i] = i * (int)m_numTriangles[i];
            if (i==NUM_THREADS-1)
                m_numTriangles[i] += numTriangles%NUM_THREADS;

            running[i] = true;
        }

        while (runningThreads>0)
             ;//std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    
};

