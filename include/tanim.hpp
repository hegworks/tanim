#pragma once

class Tanim
{
public:
    Tanim() = default;

    static void Draw();
    static void Init();

private:
    static inline int selectedEntry = -1;
    static inline int firstFrame = 0;
    static inline int endFrame = 64;
    static inline int currentFrame = 0;
    static inline bool expanded = true;
};
