#pragma once

#include <iostream>
#include <map>
#include "Directive.hpp"

class   Location
{
    public:
        std::map<std::string, Directive> directives;


};