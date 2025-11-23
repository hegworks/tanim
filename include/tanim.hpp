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
    static inline bool expanded = true;
    static inline int currentFrame = 100;
};
