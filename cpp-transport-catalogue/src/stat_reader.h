#pragma once

#include <iosfwd>
#include <string_view>
#include <unordered_set>
#include <iomanip>
#include <ostream>
#include <istream>

#include "transport_catalogue.h"


namespace statreader {

    class StatReader {
    public:
        StatReader(std::ostream* os, std::istream* is) : os_(os), is_(is) {}
        void StartRequestParcing(transport_catalogue::processing::TransportCatalogue& tansport_catalogue);
        void ParseAndPrintStat(transport_catalogue::processing::TransportCatalogue& tansport_catalogue, std::string_view request);
    private:
        std::ostream* os_ = nullptr;
        std::istream* is_ = nullptr;
    };
}