#include "map_renderer.h"


bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}


void MapRenderer::CreatePolyline() 
{
	c_number = 0;
	bus_counter = 0;
	for (auto& num : sizes_pol)
	{
		if (num != 0) {
			svg::Polyline pol;

			for (int i = 0; i < num; ++i, ++c_number)
			{
				pol.AddPoint(sph->operator()(coord.at(c_number)));
			}

			pol.SetStrokeWidth(line_width);
			pol.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			pol.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			pol.SetFillColor(svg::Color("none"));
			pol.SetStrokeColor(color_palette.at(current_color % modul_color));

			current_color++;
			scheme_.Add(pol);


		}
		bus_counter++;
	}

}

void MapRenderer::CreateBusNames() 
{
	current_color = 0;

	for (auto& name : name_bus)
	{
		if (ts_->FindBus(name)->stops_.empty()) { continue; }

		svg::Text text;
		svg::Text sub_text;
		text.SetPosition(sph->operator()(ts_->FindBus(name)->stops_.at(0)->coord_));
		sub_text.SetPosition(sph->operator()(ts_->FindBus(name)->stops_.at(0)->coord_));
		text.SetOffset({ bus_label_offset.at(0), bus_label_offset.at(1) });
		sub_text.SetOffset({ bus_label_offset.at(0), bus_label_offset.at(1) });
		text.SetFontSize(bus_label_font_size);
		sub_text.SetFontSize(bus_label_font_size);
		text.SetFontFamily("Verdana");
		sub_text.SetFontFamily("Verdana");
		text.SetFontWeight("bold");
		sub_text.SetFontWeight("bold");
		text.SetData(name);
		sub_text.SetData(name);

		sub_text.SetStrokeColor(underlayer_color);

		sub_text.SetFillColor(underlayer_color);
		sub_text.SetStrokeWidth(underlayer_width);
		sub_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		sub_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		text.SetFillColor(color_palette.at(current_color % modul_color));
		scheme_.Add(sub_text);
		scheme_.Add(text);


		if (!ts_->FindBus(name)->is_roundtrip_ && ts_->FindBus(name)->stops_.at(ts_->FindBus(name)->stops_.size() / 2) != ts_->FindBus(name)->stops_.at(0))
		{
			text.SetPosition(sph->operator()(ts_->FindBus(name)->stops_.at(ts_->FindBus(name)->stops_.size() / 2)->coord_));
			sub_text.SetPosition(sph->operator()(ts_->FindBus(name)->stops_.at(ts_->FindBus(name)->stops_.size() / 2)->coord_));
			scheme_.Add(sub_text);
			scheme_.Add(text);
		}


		current_color++;

	}
}

void MapRenderer::CreateStopCircles() 
{
	stopname = ts_->GetStopNames();
	std::sort(stopname.begin(), stopname.end());

	for (auto& name : stopname)
	{
		if (ts_->FindStop(name)->buses_.empty()) { continue; }

		svg::Circle cr;
		cr.SetCenter(sph->operator()(ts_->FindStop(name)->coord_));
		cr.SetRadius(stop_radius);
		cr.SetFillColor(svg::Color("white"));
		scheme_.Add(cr);

	}
}

void MapRenderer::CreateStopNames() 
{

	for (auto& name : stopname)
	{
		if (ts_->FindStop(name)->buses_.empty()) { continue; }

		svg::Text text;
		svg::Text sub_text;

		text.SetPosition(sph->operator()(ts_->FindStop(name)->coord_));
		sub_text.SetPosition(sph->operator()(ts_->FindStop(name)->coord_));
		text.SetOffset(svg::Point(stop_label_offset.at(0), stop_label_offset.at(1)));
		sub_text.SetOffset(svg::Point(stop_label_offset.at(0), stop_label_offset.at(1)));
		text.SetFontSize(stop_label_font_size);
		sub_text.SetFontSize(stop_label_font_size);
		text.SetFontFamily("Verdana");
		sub_text.SetFontFamily("Verdana");
		text.SetData(name);
		sub_text.SetData(name);
		sub_text.SetStrokeColor(underlayer_color);
		sub_text.SetFillColor(underlayer_color);
		sub_text.SetStrokeWidth(underlayer_width);
		sub_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		sub_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		text.SetFillColor(svg::Color("black"));
		scheme_.Add(sub_text);
		scheme_.Add(text);

	}

}


void MapRenderer::CreateSvg(std::ostream& os)
{

	name_bus = ts_->GetBusNames();
	std::sort(name_bus.begin(), name_bus.end());
	current_color = 0;
	modul_color = color_palette.size();

	for (auto& name : name_bus)
	{
		int c_size = 0;
		for (auto& stop : ts_->FindBus(name)->stops_)
		{
			coord.emplace_back(stop->coord_);
			c_size++;
		}
		sizes_pol.emplace_back(c_size);


	}

	if (!coord.empty()) {

		SphereProjector sph_iden{ coord.begin(), coord.end(), widght, height, padding };
		sph = &sph_iden;

		CreatePolyline();

		CreateBusNames();

		CreateStopCircles();

		CreateStopNames();

	}


	scheme_.Render(os);
}


void MapRenderer::SetTransportCatalogue(transport_catalogue::processing::TransportCatalogue& ts)
{
	ts_ = &ts;
}