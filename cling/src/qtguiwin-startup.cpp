#include <QApplication>

// Cling's incremental parser can lose the bool_constant base when the MSVC
// STL's is_convertible specialization is instantiated from Qt's connect
// overload selection. Recreate only the const-char-pointer case Qt uses to
// distinguish its legacy string-based connect API.
namespace std {
template <typename From>
struct is_convertible<From, const char*>
    : bool_constant<__is_convertible_to(From, const char*)> {};
}

extern "C" __declspec(dllimport) void qtcling_initialize();
extern "C" __declspec(dllimport) unsigned long long qtcling_callback_count();

qtcling_initialize();
