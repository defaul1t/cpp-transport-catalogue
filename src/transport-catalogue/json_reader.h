#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

class JsonReader {
 public:
  JsonReader() = default;

  void JsonIn(
      transport_catalogue::processing::TransportCatalogue& transport_catalogue,
      bool second_proc);

  void JsonIn(
      transport_catalogue::processing::TransportCatalogue& transport_catalogue,
      std::istream& in);

  void JsonOut(
      transport_catalogue::processing::TransportCatalogue& transport_catalogue,
      std::ostream& out);

  MapRenderer GetMapData();

  json::Document data_request_;

  TransportRouter tr_;
};
