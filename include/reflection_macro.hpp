#pragma once

namespace tanim
{
/// VisitStructContext
struct VSContext
{
};
}  // namespace tanim

#define TANIM_REFLECT(STRUCT_NAME, ...)                                                                                \
    VISITABLE_STRUCT_IN_CONTEXT(tanim::VSContext, STRUCT_NAME, __VA_ARGS__);                                           \
    namespace                                                                                                          \
    {                                                                                                                  \
    inline auto CONCAT(register_, __COUNTER__) = (tanim::internal::GetRegistry().RegisterComponent<STRUCT_NAME>(), 0); \
    }

#define TANIM_REFLECT_NO_REGISTER(STRUCT_NAME, ...) VISITABLE_STRUCT_IN_CONTEXT(tanim::VSContext, STRUCT_NAME, __VA_ARGS__);

#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define CONCAT_IMPL(a, b) a##b
