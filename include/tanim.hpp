#pragma once
#include "tanim_sequence.hpp"

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
    static inline TanimSequence mySequence{};

    static inline int selectedEntry = -1;
    static inline int firstFrame = 0;
    static inline int endFrame = 64;
    static inline int currentFrame = 0;
    static inline bool expanded = true;
    static inline int samplesPS = 60;  // SamplesPerSecond

    static inline float playerTime = 0;
    static inline int playerFrame = 0;
    static inline bool playerPlaying = false;

    static void Play();
    static void Pause();

    static float FrameToSeconds(int frame);
    static int SecondsToFrame(float time);
    static float SecondsToSampleTime(float time);
};

}  // namespace tanim
