#pragma once

class Timer
{
public:
    Timer(double targetFPS) { Reset(targetFPS); }
    void Reset(double targetFPS)
    {
        m_lastTime = SDL_GetTicks();
        m_leftOver = 0.0;
        m_secondCounter = m_fps = m_frameCounter = 0;
        m_target = 1.0 / targetFPS;
    }

    uint32_t GetFPS() { return m_fps; }

    void Update(std::function<void(double)> update)
    {
        double currentTime = SDL_GetTicks();
        double deltaTime = (currentTime - m_lastTime)/1000.0f;
        m_lastTime = currentTime;
        // Second counter to keep track of whether we have crossed a second
        m_secondCounter += static_cast<uint32_t>(deltaTime * 1000.0f);

        if (deltaTime > 1.0f) 
            deltaTime = 1.0f;

        // donot accumulate small errors
        if (fabs(deltaTime - m_target) < 1.0 / 4000.0f)
            deltaTime = m_target;
        
        m_leftOver += deltaTime;
        // For current second, one more frame has passed
        if (m_leftOver >= m_target)
            ++m_frameCounter;                   // Don't do this inside loop below: larger time gap means less FPS

        // update a frame by the target time: if more time has passed than target time, update more than once
        // since target time is constant, using target instead of delta time ensures constant output on multiple devices
        while (m_leftOver >= m_target)
        {
            m_leftOver -= m_target;
            update(m_target);
        }

        // Calculate FPS using frameCounter and secondCounter
        if (m_secondCounter > 1000)
        {
            m_fps = m_frameCounter;
            m_frameCounter = 0;
            m_secondCounter %= 1000;
        }
    }

private:
    double m_lastTime, m_target, m_leftOver;
    uint32_t m_fps, m_frameCounter, m_secondCounter;
};
