#pragma once

#include <string>

namespace tw {

/* pencode: percent encode a string */
std::string pencode(const std::string &s);

/* noncegen: generate a random alphanumeric string */
std::string noncegen();

}
