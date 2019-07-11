#pragma once

#include <functional>
#include <string_view>

namespace search {
using Iter = std::string_view::const_iterator;
using Match = std::pair<Iter, Iter>;
}
