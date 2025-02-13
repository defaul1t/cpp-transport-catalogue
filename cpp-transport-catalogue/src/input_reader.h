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
        // ����������, ������ �� ������� (���� command ��������)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // �������� �������
        std::string id;           // id �������� ��� ���������
        std::string description;  // ��������� �������
    };

    class InputReader {
    public:
        InputReader() = default;
        InputReader(std::istream* str) : str_(str)
        {}
        /**
         * ������ ������ � ��������� CommandDescription � ��������� ��������� � commands_
         */
        void ParseLine(std::string_view line);

        /**
         * ��������� ������� ������������ ����������, ��������� ������� �� commands_
         */
        void ApplyCommands(transport_catalogue::processing::TransportCatalogue& catalogue, bool first_process, std::vector<std::string>&& save);

        void StartParcing(transport_catalogue::processing::TransportCatalogue& catalogue);
    private:
        
        std::istream* str_ = nullptr;
        std::vector<CommandDescription> commands_;
    };
}