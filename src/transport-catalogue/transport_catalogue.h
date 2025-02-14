#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace transport_catalogue {

namespace processing {
class TransportCatalogue {
 public:
  struct TransportHasher {
    size_t operator()(
        std::pair<domain::Stop*, domain::Stop*> pair_pointer) const {
      return hasher(pair_pointer.first) + hasher(pair_pointer.second) * 37;
    }
    std::hash<const void*> hasher;
  };

  void AddStop(std::string&& name, geo_calc::Coordinates coord);

  const domain::Stop* FindStop(const std::string& name_stop) const;

  void AddBus(const std::string& name, std::vector<std::string>& stopnames,
              bool roundtrip);

  const domain::Bus* FindBus(const std::string& name_bus) const;

  void AddDistance(std::string&& first, std::string&& second, int dist);

  int FindDist(domain::Stop* stop1, domain::Stop* stop2);

  const std::deque<domain::Bus>& GetBusData() const;

  std::vector<std::string> GetBusNames();

  std::vector<std::string> GetStopNames();

 private:
  std::deque<domain::Stop> stops_;
  std::unordered_map<std::string_view, domain::Stop*> stopname_to_stops_;
  std::deque<domain::Bus> buses_;
  std::unordered_map<std::string_view, const domain::Bus*> busname_to_bus_;
  std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int,
                     TransportHasher>
      dist_data_;
};
}  // namespace processing

}  // namespace transport_catalogue
