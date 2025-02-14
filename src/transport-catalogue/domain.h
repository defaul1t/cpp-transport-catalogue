#pragma once
#include <set>
#include <string>
#include <vector>

#include "geo.h"

namespace domain {
struct Bus;

struct Stop {
  Stop() = default;
  Stop(std::string&& name, geo_calc::Coordinates coord);
  std::string name_;
  geo_calc::Coordinates coord_ = {0, 0};
  std::set<std::string_view> buses_;
};

struct Bus {
  Bus() = default;
  std::string name_;
  std::vector<domain::Stop*> stops_;
  bool is_roundtrip_ = false;
};

}  // namespace domain