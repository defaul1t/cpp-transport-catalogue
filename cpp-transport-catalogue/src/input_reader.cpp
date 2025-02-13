#include "input_reader.h"

/*
 Парсит строку в структуру CommandDescription и сохраняет результат в commands_
 */ 
std::vector<std::string> ParserByParameters(std::string_view string_line, char type_delimeter)
{
    std::vector <std::string> res;
  
        res.reserve(std::count(string_line.begin(), string_line.end(), type_delimeter));
        std::string parameter;
        for (auto symbol : string_line)
        {
            if (symbol == type_delimeter)
            {
                parameter.erase(parameter.begin(), parameter.begin() + parameter.find_first_not_of(" "));
                parameter.erase(parameter.begin() + parameter.find_last_not_of(" ") + 1, parameter.end());

                res.push_back(std::move(parameter));
                parameter.clear();

            }
            else
            {
                parameter += symbol;
            }
        }

        if (!parameter.empty()) 
        {
            parameter.erase(parameter.begin(), parameter.begin() + parameter.find_first_not_of(" "));
            parameter.erase(parameter.begin() + parameter.find_last_not_of(" ")+1, parameter.end());
            res.push_back(parameter);
        }

    return res;
}

void inreader::InputReader::ParseLine(std::string_view line) 
{
    CommandDescription res;
    if (line.find(":") != std::string::npos)
    {
        if (line[0] == 'B')
        {
            if (line.find(">")!=std::string::npos) {
                res.command = "ACBus";
            }
            else
            {
                res.command = "ALBus";
            }
           
            res.id = std::move(std::string(line.begin() + line.find_first_of(" ") + 1, 
                line.begin() + line.find_first_of(":")));
            res.description = std::string(line.begin() + line.find_first_of(":") + 1, line.end());

        }
        else 
        {
           
            res.command = "AStop";
            res.id = std::move(std::string(line.begin() + line.find_first_of(" ") + 1,
                line.begin() + line.find_first_of(":")));
            res.description = std::string(line.begin() + line.find_first_of(":")+1, line.end());
        }
   
    }
    else if(line[0]=='B') 
    {
        res.command = "GBus";
        res.id = std::move(std::string(line.begin() + line.find_first_of(" ") + 1,
            line.begin() + line.find_first_of(":")));
        
    }
    else 
    {
        return;
    }
    commands_.push_back(res);

}
/**
 * Наполняет данными транспортный справочник, используя команды из commands_
 */
void inreader::InputReader::ApplyCommands(transport_catalogue::processing::TransportCatalogue& catalogue, bool first_proccess, std::vector<std::string>&& save_)
{
    if (first_proccess) {
        std::sort(commands_.begin(), commands_.end(), [](const auto& lhs, const auto& rhs)
            {
                return lhs.command == "AStop" && rhs.command != "AStop";
            });
    }

    std::vector<std::string> save_id = save_;
    int current_id = 0;

    for (auto& command : commands_) 
    {
        if (command.command == "AStop")
        {
            if (first_proccess) {
                save_id.push_back(command.id); // id это имя остановки или автобуса
                std::vector<std::string> params = std::move(ParserByParameters(command.description, ','));
                geo_calc::Coordinates coord(std::stod(params[0]), std::stod(params[1]));
                catalogue.AddStop(std::move(command.id), coord);
                
            }
            else 
            {
                std::vector<std::string> params = std::move(ParserByParameters(command.description, ','));
                if (params.size() > 2) // первые два это координата, остальные строки с дистанцией
                {
                    int dist = 0;
                    std::string first_station, second_station;
                    for (int i = 2; i < static_cast<int>(params.size()); ++i) 
                    {// Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino, 4100m to Tret
                        dist = stoi(std::string(params[i].begin(), params[i].begin() + params[i].find("m")));
                        first_station = save_id[current_id]; // commain.id? why save_id
                        second_station = std::string(params[i].begin() + params[i].find(" to ") + 4, params[i].end()); // мб запятая
                        catalogue.AddDistance(std::move(first_station), std::move(second_station), dist);
                    }
                }
                current_id++;
            }
        }
        else if (command.command == "ACBus" && first_proccess) 
        {
            std::vector<std::string> params = std::move(ParserByParameters(command.description, '>'));
            catalogue.AddBus(command.id, params, true);
        }
        else if (command.command == "ALBus" && first_proccess)
        {
            std::vector<std::string> params = std::move(ParserByParameters(command.description, '-'));
            std::vector <std::string> second_part(params.begin(), params.end()-1);
            for (int i = second_part.size() - 1; i >= 0; --i) 
            {
                params.push_back(std::move(second_part[i]));
            }
            catalogue.AddBus(command.id, params, false);
        }
    }

    if (first_proccess) 
    {
        first_proccess = false;
        ApplyCommands(catalogue, first_proccess, std::move(save_id));
    }

}

void inreader::InputReader::StartParcing(transport_catalogue::processing::TransportCatalogue& catalogue)
{
    int base_request_count;
    *str_ >> base_request_count >> std::ws;

    {
        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            getline(*str_, line);
            this->ParseLine(line);
        }
        this->ApplyCommands(catalogue, true, std::vector<std::string>{});
    }
}