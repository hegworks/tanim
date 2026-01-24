#pragma once

namespace tanim
{
/// VisitStructContext
struct VSContext
{
};
}  // namespace tanim

/// You must call this in the global namespace, for each ECS component struct that you want to be able to animate using Tanim.
/// You must include all the fields in the component that you want to be able to animate using Tanim.
/// The type of the fields must be supported by Tanim, otherwise you will get compile errors.
/// The arguments follow the VISITABLE_STRUCT format, which is the STRUCT_NAME first, then name of all the fields you want to
/// reflect, separated by commas. You must include the namespaces as part of the STRUCT_NAME. So for example:
/// TANIM_REFLECT(foo::bar::baz, a, b, c);
#define TANIM_REFLECT(STRUCT_NAME, ...)                                                                                \
    VISITABLE_STRUCT_IN_CONTEXT(tanim::VSContext, STRUCT_NAME, __VA_ARGS__);                                           \
    namespace                                                                                                          \
    {                                                                                                                  \
    inline auto CONCAT(register_, __COUNTER__) = (tanim::internal::GetRegistry().RegisterComponent<STRUCT_NAME>(), 0); \
    }

/// If for any reason you can not use TANIM_REFLECT on a component, you must use this macro instead. But then at some point
/// during your application's initialization phase, you must call tanim::RegisterComponent once for that component. The format
/// of arguments is exactly the same as TANIM_REFLECT (check its docs). One example where you might not be able to use
/// TANIM_REFLECT, is if in the constructor of the component that you are reflecting, you are accessing a variable that is not
/// yet initialized by your application. Since the Registration call in the TANIM_REFLECT happens at the very early stages of
/// the application, this is something that might happen.
#define TANIM_REFLECT_NO_REGISTER(STRUCT_NAME, ...) VISITABLE_STRUCT_IN_CONTEXT(tanim::VSContext, STRUCT_NAME, __VA_ARGS__);

#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define CONCAT_IMPL(a, b) a##b

// REF: https://github.com/cbeck88/visit_struct/issues/27#issuecomment-3054127371
/// allows visit_struct to access private variables
#ifndef BEFRIEND_VISITABLE
#define BEFRIEND_VISITABLE()      \
    template <typename, typename> \
    friend struct ::visit_struct::traits::visitable
#endif
