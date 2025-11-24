#pragma once
#include "tanim_sequence.hpp"

namespace tanim
{

class Tanim
{
public:
    Tanim() = default;

    static void Draw();
    static void Init();

private:
    static inline TanimSequence mySequence{};

    static inline int selectedEntry = -1;
    static inline int firstFrame = 0;
    static inline int endFrame = 64;
    static inline int currentFrame = 0;
    static inline bool expanded = true;
};

}  // namespace tanim
