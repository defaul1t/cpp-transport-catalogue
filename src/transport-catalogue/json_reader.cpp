
#include "json_reader.h"

void JsonReader::JsonIn(
    transport_catalogue::processing::TransportCatalogue& transport_catalogue,
    std::istream& in) {
  this->data_request_ = std::move(json::Load(in));

  const json::Array* base_requests =
      &(data_request_.GetRoot().AsMap().at("base_requests").AsArray());

  //первичная обработка остановок
  for (auto& dict : *base_requests) {
    domain::Stop new_stop;
    new_stop = domain::Stop();

    if (dict.AsMap().at("type").AsString() == "Bus") {
      continue;
    }

    new_stop.name_ = dict.AsMap().at("name").AsString();
    new_stop.coord_.lat = dict.AsMap().at("latitude").AsDouble();
    new_stop.coord_.lng = dict.AsMap().at("longitude").AsDouble();

    transport_catalogue.AddStop(std::move(new_stop.name_), new_stop.coord_);
  }
  //вызов вторичной обработки
  JsonIn(transport_catalogue, true);
}

void JsonReader::JsonIn(
    transport_catalogue::processing::TransportCatalogue& transport_catalogue,
    bool second_proc = true) {
  if (second_proc) {
    const json::Array* base_requests =
        &(data_request_.GetRoot().AsMap().at("base_requests").AsArray());

    //обработка остановок
    for (auto& dict : *base_requests) {
      domain::Stop new_stop;
      new_stop = domain::Stop();

      if (dict.AsMap().at("type").AsString() == "Bus") {
        continue;
      }

      for (auto& [name, meters] : dict.AsMap().at("road_distances").AsMap()) {
        transport_catalogue.AddDistance(
            std::move(std::string(dict.AsMap().at("name").AsString())),
            std::move(std::string(name)), meters.AsInt());
      }
    }

    //обработка автобусов
    for (auto& dict : *base_requests) {
      domain::Bus new_bus;
      std::vector<std::string> names_stop;

      if (dict.AsMap().at("type").AsString() == "Stop") {
        continue;
      }

      new_bus.name_ = dict.AsMap().at("name").AsString();

      new_bus.is_roundtrip_ = dict.AsMap().at("is_roundtrip").AsBool();

      for (auto& stop : dict.AsMap().at("stops").AsArray()) {
        names_stop.push_back(stop.AsString());
      }

      transport_catalogue.AddBus(new_bus.name_, names_stop,
                                 new_bus.is_roundtrip_);

      names_stop.clear();
    }

    const json::Dict* router_setings =
        &(data_request_.GetRoot().AsMap().at("routing_settings").AsMap());

    if (!router_setings->empty()) {
      tr_.SetupRouterSettings(
          RouterSettings(router_setings->at("bus_velocity").AsDouble(),
                         router_setings->at("bus_wait_time").AsInt()));
    }

    tr_.BuildRouter(transport_catalogue);
  }
}

void JsonReader::JsonOut(
    transport_catalogue::processing::TransportCatalogue& transport_catalogue,
    std::ostream& out) {
  const json::Array* stat_requests =
      &(data_request_.GetRoot().AsMap().at("stat_requests").AsArray());

  json::Builder builder_;

  builder_.StartArray();

  for (auto& dict : *stat_requests) {
    builder_.StartDict();
    builder_.Key("request_id").Value(dict.AsMap().at("id").AsInt());

    if (dict.AsMap().at("type").AsString() == "Stop") {
      if (transport_catalogue.FindStop(dict.AsMap().at("name").AsString()) ==
          nullptr) {
        builder_.Key("error_message").Value(std::string("not found"));
      } else {
        std::pair<std::string, json::Array> buses_in_stop;
        buses_in_stop.first = "buses";
        for (const domain::Stop* stop = transport_catalogue.FindStop(
                 dict.AsMap().at("name").AsString());
             auto& bus : stop->buses_) {
          buses_in_stop.second.push_back(std::string(bus));
        }

        builder_.Key(buses_in_stop.first).Value(buses_in_stop.second);
      }
    } else if (dict.AsMap().at("type").AsString() == "Bus") {
      if (transport_catalogue.FindBus(dict.AsMap().at("name").AsString()) ==
          nullptr) {
        builder_.Key("error_message").Value(std::string("not found"));
      } else {
        double geo_length = 0;
        double route_length = 0;
        const domain::Bus* t_bus =
            transport_catalogue.FindBus(dict.AsMap().at("name").AsString());

        for (int i = 0; i < static_cast<int>(t_bus->stops_.size() - 1); ++i) {
          geo_length += geo_calc::ComputeDistance(
              t_bus->stops_.at(i)->coord_, t_bus->stops_.at(i + 1)->coord_);
          route_length += transport_catalogue.FindDist(t_bus->stops_.at(i),
                                                       t_bus->stops_.at(i + 1));
        }

        std::unordered_set<const domain::Stop*> unique(t_bus->stops_.begin(),
                                                       t_bus->stops_.end());

        builder_.Key("curvature").Value(route_length / geo_length);
        builder_.Key("route_length").Value(static_cast<int>(route_length));
        builder_.Key("stop_count")
            .Value(static_cast<int>(t_bus->stops_.size()));
        builder_.Key("unique_stop_count")
            .Value(static_cast<int>(unique.size()));
      }
    } else if (dict.AsMap().at("type").AsString() == "Map") {
      std::ostringstream bf;
      MapRenderer map_renderer = std::move(GetMapData());
      map_renderer.SetTransportCatalogue(transport_catalogue);
      map_renderer.CreateSvg(bf);
      builder_.Key("map").Value(std::move(bf.str()));

    } else if (dict.AsMap().at("type").AsString() == "Route") {
      std::string from = dict.AsMap().at("from").AsString();
      std::string to = dict.AsMap().at("to").AsString();

      std::optional<graph::Router<double>::RouteInfo> res =
          tr_.FindRoute(from, to);

      if (res) {
        builder_.Key("total_time").Value(res.value().weight);
        builder_.Key("items").StartArray();

        for (auto& edge_ : res.value().edges) {
          auto edge_in = tr_.GetEdge(edge_);
          builder_.StartDict();
          if (edge_in.state == graph::StatusEdge::WAIT) {
            builder_.Key("type").Value("Wait");
            builder_.Key("stop_name").Value(edge_in.data);
            builder_.Key("time").Value(edge_in.weight);
          } else {
            builder_.Key("type").Value("Bus");
            builder_.Key("bus").Value(edge_in.data);
            builder_.Key("span_count")
                .Value(static_cast<int>(edge_in.span_count));
            builder_.Key("time").Value(edge_in.weight);
          }
          builder_.EndDict();
        }
        builder_.EndArray();
      } else {
        builder_.Key("error_message").Value(std::string("not found"));
      }
    }

    builder_.EndDict();
  }

  builder_.EndArray();

  json::Print(json::Document(builder_.Build()), out);
}

MapRenderer JsonReader::GetMapData() {
  MapRenderer mr;
  json::Node render_sett =
      data_request_.GetRoot().AsMap().at("render_settings");

  mr.widght = render_sett.AsMap().at("width").AsDouble();
  mr.height = render_sett.AsMap().at("height").AsDouble();
  mr.padding = render_sett.AsMap().at("padding").AsDouble();
  mr.line_width = render_sett.AsMap().at("line_width").AsDouble();
  mr.stop_radius = render_sett.AsMap().at("stop_radius").AsDouble();
  mr.bus_label_font_size =
      render_sett.AsMap().at("bus_label_font_size").AsInt();
  mr.bus_label_offset.emplace_back(
      render_sett.AsMap().at("bus_label_offset").AsArray().at(0).AsDouble());
  mr.bus_label_offset.emplace_back(
      render_sett.AsMap().at("bus_label_offset").AsArray().at(1).AsDouble());
  mr.stop_label_font_size =
      render_sett.AsMap().at("stop_label_font_size").AsInt();
  mr.stop_label_offset.emplace_back(
      render_sett.AsMap().at("stop_label_offset").AsArray().at(0).AsDouble());
  mr.stop_label_offset.emplace_back(
      render_sett.AsMap().at("stop_label_offset").AsArray().at(1).AsDouble());

  json::Node u_color = render_sett.AsMap().at("underlayer_color");
  if (u_color.IsArray()) {
    if (u_color.AsArray().size() == 3) {
      mr.underlayer_color =
          svg::Rgb({static_cast<uint8_t>(u_color.AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(u_color.AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(u_color.AsArray().at(2).AsInt())});
    } else {
      mr.underlayer_color =
          svg::Rgba({static_cast<uint8_t>(u_color.AsArray().at(0).AsInt()),
                     static_cast<uint8_t>(u_color.AsArray().at(1).AsInt()),
                     static_cast<uint8_t>(u_color.AsArray().at(2).AsInt()),
                     u_color.AsArray().at(3).AsDouble()});
    }
  } else if (u_color.IsString()) {
    mr.underlayer_color = u_color.AsString();
  } else {
    mr.underlayer_color = svg::Color("none");
  }

  mr.underlayer_width = render_sett.AsMap().at("underlayer_width").AsDouble();

  for (auto& color : render_sett.AsMap().at("color_palette").AsArray()) {
    if (color.IsArray()) {
      if (color.AsArray().size() == 3) {
        mr.color_palette.emplace_back(
            svg::Rgb({static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                      static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                      static_cast<uint8_t>(color.AsArray().at(2).AsInt())}));
      } else {
        mr.color_palette.emplace_back(
            svg::Rgba({static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                       static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                       static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
                       color.AsArray().at(3).AsDouble()}));
      }
    } else if (color.IsString()) {
      mr.color_palette.emplace_back(color.AsString());
    } else {
      mr.color_palette.emplace_back(svg::Color("none"));
    }
  }

  return mr;
}