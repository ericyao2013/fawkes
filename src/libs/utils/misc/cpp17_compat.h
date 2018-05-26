#pragma once

#if __GNUC__ < 7

#include <experimental/optional>
namespace std {
template<class T>
using optional = std::experimental::optional<T>;
}

#else

#include <optional>

#endif

