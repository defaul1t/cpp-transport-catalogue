#include "domain.h"

domain::Stop::Stop(std::string&& name, geo_calc::Coordinates coord)
    : name_(std::move(name)), coord_(std::move(coord)) {}