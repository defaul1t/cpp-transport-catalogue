#include "stat_reader.h"

void statreader::StatReader::ParseAndPrintStat(
    transport_catalogue::processing::TransportCatalogue& transport_catalogue,
    std::string_view request) {
  if (request[0] == 'B') {
    std::string busname(request.begin() + request.find_first_of(" "),
                        request.end());
    busname = busname.substr(1);

    auto bus = transport_catalogue.FindBus(busname);
    if (bus == nullptr) {
      *os_ << "Bus " << busname << ": not found\n";
      return;
    }
    unsigned total_stops = 0, unique_stops = 0;
    double geo_length = 0;
    int road_length = 0;
    std::unordered_set<const domain::Stop*> unique(bus->stops_.begin(),
                                                   bus->stops_.end());
    unique_stops = unique.size();
    total_stops = bus->stops_.size();

    for (unsigned i = 0; i < total_stops - 1; ++i) {
      geo_length += ComputeDistance(bus->stops_.at(i)->coord_,
                                    bus->stops_.at(i + 1)->coord_);
      road_length += transport_catalogue.FindDist(bus->stops_.at(i),
                                                  bus->stops_.at(i + 1));
    }

    *os_ << "Bus " << busname << ": " << total_stops << " stops on route, "
         << unique_stops << " unique stops, " << std::setprecision(6)
         << static_cast<double>(road_length) << " route length, "
         << static_cast<double>(road_length) / geo_length << " curvature\n";
  } else {
    std::string stopname(request.begin() + request.find_first_of(" "),
                         request.end());
    stopname = stopname.substr(1);

    auto stop = transport_catalogue.FindStop(stopname);
    if (stop == nullptr) {
      *os_ << "Stop " << stopname << ": not found\n";
      return;
    }

    if (stop->buses_.empty()) {
      *os_ << "Stop " << stopname << ": no buses\n";
      return;
    }

    *os_ << "Stop " << stopname << ": buses";
    for (auto& busname : stop->buses_) {
      *os_ << " " << busname;
    }
    *os_ << std::endl;
  }
}
void statreader::StatReader::StartRequestParcing(
    transport_catalogue::processing::TransportCatalogue& tansport_catalogue) {
  int stat_request_count;
  *is_ >> stat_request_count >> std::ws;
  for (int i = 0; i < stat_request_count; ++i) {
    std::string line;
    getline(*is_, line);
    ParseAndPrintStat(tansport_catalogue, line);
  }
}