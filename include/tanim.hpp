#pragma once
#include "tanim/include/sequence.hpp"

namespace tanim
{

class Tanim
{
public:
    Tanim() = default;

    static void Init();
    static void Draw();
    static void Update(float dt);

private:
    static inline Timeline m_timeline{};

    static inline int m_selected_sequence = -1;

    static inline int m_player_samples = 60;  // SamplesPerSecond
    static inline int m_player_frame = 0;
    static inline float m_player_time = 0;
    static inline bool m_player_playing = false;

    static void Play();
    static void Pause();

    static float FrameToSeconds(int frame);
    static int SecondsToFrame(float time);
    static float SecondsToSampleTime(float time);
};

}  // namespace tanim
