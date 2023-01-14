#pragma once

#include "utils.hpp"

//! copies orig to browser snap dir, because snap won't open files in /tmp
//! \sa https://github.com/elsamuko/fsrc/issues/6
boost::filesystem::path copyToSnapDir(const fs::path& orig);
