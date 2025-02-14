#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

const int MINUTES_IN_HOUR = 60;
const double METERS_IN_KM = 1000.0;

struct RouterSettings {
  double wait_time_ = 0;
  double bus_velocity_ = 0;
};

class TransportRouter {
 public:
  TransportRouter() = default;

  const graph::Edge<double>& GetEdge(graph::EdgeId id) const {
    return graph_.GetEdge(id);
  }

  void SetupRouterSettings(const RouterSettings& set) {
    set_ = set;
    set_.bus_velocity_ /= MINUTES_IN_HOUR;
  }
  void BuildRouter(transport_catalogue::processing::TransportCatalogue& ts);

  std::optional<graph::Router<double>::RouteInfo> FindRoute(std::string from,
                                                            std::string to) {
    return router_->BuildRoute(stopname_id_data_[from], stopname_id_data_[to]);
  }

 private:
  void CreateWaitEdges(graph::DirectedWeightedGraph<double>& graph);

  void CreateBusEdges(graph::DirectedWeightedGraph<double>& graph,
                      transport_catalogue::processing::TransportCatalogue& ts);

  void ConstructGraph(transport_catalogue::processing::TransportCatalogue& ts);

  void CreateRouterIdentity() {
    router_.reset(
        new graph::Router<double>(std::move(graph::Router<double>(graph_))));
  }

  std::unordered_map<std::string, size_t> stopname_id_data_;
  graph::DirectedWeightedGraph<double> graph_;
  std::unique_ptr<graph::Router<double>> router_;

  RouterSettings set_;
};
