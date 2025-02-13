#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <algorithm>

#include "geo.h"
#include "transport_catalogue.h"

namespace inreader {
    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };

    class InputReader {
    public:
        InputReader() = default;
        InputReader(std::istream* str) : str_(str)
        {}
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        void ParseLine(std::string_view line);

        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(transport_catalogue::processing::TransportCatalogue& catalogue, bool first_process, std::vector<std::string>&& save);

        void StartParcing(transport_catalogue::processing::TransportCatalogue& catalogue);
    private:
        
        std::istream* str_ = nullptr;
        std::vector<CommandDescription> commands_;
    };
}