#ifndef DATAFRAME_CPP
#define DATAFRAME_CPP

#include "DataFrame.h"

DataFrame::DataFrame(const std::vector<std::string> &column_name, const std::vector<std::string> &data_types) {
    row_count = 0;
    date_count = 0;
    int_count = 0;
    string_count = 0;
    float_count = 0;
    for (int i = 0; i < column_name.size(); i++) {
        if (data_types[i] == "date") {
            dates[column_name[i]] = std::vector<time_t>();
            dtype_info[column_name[i]] = "date";
            date_count++;
        } else if (data_types[i] == "int") {
            integers[column_name[i]] = std::vector<int>();
            dtype_info[column_name[i]] = "int";
            int_count++;
        } else if (data_types[i] == "string") {
            alphanumerics[column_name[i]] = std::vector<std::string>();
            dtype_info[column_name[i]] = "string";
            string_count++;
        } else if (data_types[i] == "float") {
            reals[column_name[i]] = std::vector<float>();
            dtype_info[column_name[i]] = "float";
            float_count++;
        } else { std::cout << data_types[i] << "is an invalid data type " << std::endl; }
    }
    //std::cout << std::endl << "DataFrame Created ( #row, #date, #int, #string, #float ) -> ( ";
    //std::cout << row_count << ", " << date_count << ", " << int_count << ", " << string_count << ", " << float_count << " )" << std::endl;
}

std::map<std::string, std::string> DataFrame::parse_json_string(std::string str, int data_size, bool ignore_first) {
    std::map<std::string, std::string> dict;
    int data_counter = 0, temp_counter = 0;
    std::string field_temp = "", value_temp = "";

    if (ignore_first)
        str = str.substr(str.find("}") + 2);

    while (data_counter < data_size) {
        data_counter++;
        str = str.substr(str.find("\"") + 1);
        field_temp = str.substr(0, str.find("\""));
        str = str.substr(str.find(":") + 2);

        if (data_counter < data_size) { value_temp = str.substr(0, str.find(",")); }
        else { value_temp = str.substr(0, str.find("}") - 1); }

        temp_counter = value_temp.find("\"");
        if (temp_counter != std::string::npos) { value_temp = value_temp.substr(1, value_temp.length() - 2); }

        str = str.substr(str.find(",") + 1);
        dict[field_temp] = value_temp;
    }
    return dict;
}

void DataFrame::insert(const std::string &str, int data_size, bool ignore_first) {
    std::map<std::string, std::string> dict = parse_json_string(str, data_size, ignore_first);
    struct std::tm TM;

    if (dict.size() == date_count + string_count + int_count + float_count) {
        for (std::map<std::string, std::string>::iterator i = dict.begin(); i != dict.end(); i++) {
            if (dates.find(i->first) != dates.end()) {
                std::istringstream iss(i->second + " 00:00:00");
                iss >> std::get_time(&TM, "%Y-%m-%d %H:%M:%S");
                dates[i->first].push_back(mktime(&TM));
            } else if (integers.find(i->first) != integers.end()) {
                integers[i->first].push_back(stoi(i->second));
            } else if (alphanumerics.find(i->first) != alphanumerics.end()) {
                alphanumerics[i->first].push_back(i->second);
            } else if (reals.find(i->first) != reals.end()) {
                reals[i->first].push_back(stof(i->second));
            } else {
                std::cout << std::endl << "FATAL ERROR!! DATAFRAME COMPROMISED AS FIELD NOT FOUND IN DATAFRAME";
            }
        }
        row_count++;
    } else
        std::cout << std::endl << "Not Inserting, do not insert " << dict.size() << " fields where only "
                  << date_count + string_count + int_count + float_count << " exists";
}

void DataFrame::insert(const std::map<std::string, time_t> &date, const std::map<std::string, int> &integer,
                       const std::map<std::string, std::string> &alphanumeric,
                       const std::map<std::string, float> &real) {
    if (date.size() == date_count && integer.size() == int_count && alphanumeric.size() == string_count &&
        real.size() == float_count) {
        for (auto i = date.begin(); i != date.end(); i++) {
            if (dates.find(i->first) != dates.end()) { dates[i->first].push_back(i->second); }
        }
        for (auto i = integer.begin(); i != integer.end(); i++) {
            if (integers.find(i->first) != integers.end()) { integers[i->first].push_back(i->second); }
        }
        for (auto i = alphanumeric.begin(); i != alphanumeric.end(); i++) {
            if (alphanumerics.find(i->first) != alphanumerics.end()) { alphanumerics[i->first].push_back(i->second); }
        }
        for (auto i = real.begin(); i != real.end(); i++) {
            if (reals.find(i->first) != reals.end()) { reals[i->first].push_back(i->second); }
        }
        row_count++;
    } else { std::cout << std::endl << "Inappropriate size for insertion"; }
}

void DataFrame::insert(const std::map<std::string, std::string> &date, const std::map<std::string, int> &integer,
                       const std::map<std::string, std::string> &alphanumeric,
                       const std::map<std::string, float> &real) {
    if (date.size() == date_count && integer.size() == int_count && alphanumeric.size() == string_count &&
        real.size() == float_count) {
        struct std::tm TM;
        for (auto i = date.begin(); i != date.end(); i++) {
            if (dates.find(i->first) != dates.end()) {
                std::istringstream iss(i->second + " 00:00:00");
                iss >> std::get_time(&TM, "%Y-%m-%d %H:%M:%S");
                dates[i->first].push_back(mktime(&TM));
            }
        }
        for (auto i = integer.begin(); i != integer.end(); i++) {
            if (integers.find(i->first) != integers.end()) { integers[i->first].push_back(i->second); }
        }
        for (auto i = alphanumeric.begin(); i != alphanumeric.end(); i++) {
            if (alphanumerics.find(i->first) != alphanumerics.end()) { alphanumerics[i->first].push_back(i->second); }
        }
        for (auto i = real.begin(); i != real.end(); i++) {
            if (reals.find(i->first) != reals.end()) { reals[i->first].push_back(i->second); }
        }
        row_count++;
    } else { std::cout << std::endl << "Inappropriate size for insertion"; }
}

void
DataFrame::insert(const std::map<std::string, std::string> &date, const std::map<std::string, std::string> &integer,
                  const std::map<std::string, std::string> &alphanumeric,
                  const std::map<std::string, std::string> &real) {
    if (date.size() == date_count && integer.size() == int_count && alphanumeric.size() == string_count &&
        real.size() == float_count) {
        struct std::tm TM;
        for (auto i = date.begin(); i != date.end(); i++) {
            if (dates.find(i->first) != dates.end()) {
                std::istringstream iss(i->second + " 00:00:00");
                iss >> std::get_time(&TM, "%Y-%m-%d %H:%M:%S");
                dates[i->first].push_back(mktime(&TM));
            }
        }
        for (auto i = integer.begin(); i != integer.end(); i++) {
            if (integers.find(i->first) != integers.end()) { integers[i->first].push_back(std::stoi(i->second)); }
        }
        for (auto i = alphanumeric.begin(); i != alphanumeric.end(); i++) {
            if (alphanumerics.find(i->first) != alphanumerics.end()) { alphanumerics[i->first].push_back(i->second); }
        }
        for (auto i = real.begin(); i != real.end(); i++) {
            if (reals.find(i->first) != reals.end()) { reals[i->first].push_back(std::stof(i->second)); }
        }
        row_count++;
    } else { std::cout << std::endl << "Inappropriate size for insertion"; }
}

void DataFrame::print(int row) {
    std::map<std::string, std::vector<time_t>>::iterator date = dates.begin();
    std::map<std::string, std::vector<int>>::iterator integer = integers.begin();
    std::map<std::string, std::vector<std::string>>::iterator alphanumeric = alphanumerics.begin();
    std::map<std::string, std::vector<float>>::iterator real = reals.begin();

    char temp[100] = "Default";
    struct tm TM;
    std::cout << std::endl << "Printing Date Columns" << std::endl;
    while (date != dates.end()) {
        localtime_r(&(date->second[row]), &TM);
        strftime(temp, 20, "%F", &TM);
        std::cout << temp << std::endl;
        date++;
    }

    std::cout << "Printing Integer Columns" << std::endl;
    while (integer != integers.end()) {
        std::cout << integer->first << " " << integer->second[row] << std::endl;
        integer++;
    }

    std::cout << "Printing String Columns" << std::endl;
    while (alphanumeric != alphanumerics.end()) {
        std::cout << alphanumeric->first << " " << alphanumeric->second[row] << std::endl;
        alphanumeric++;
    }

    std::cout << "Printing Float Columns" << std::endl;
    while (real != reals.end()) {
        std::cout << real->first << " " << real->second[row] << std::endl;
        real++;
    }
}

void DataFrame::get(int row, DOHLCI &row_data) {
    row_data.date = dates["date"][row];
    row_data.open = reals["open"][row];
    row_data.high = reals["high"][row];
    row_data.low = reals["low"][row];
    row_data.close = reals["close"][row];
    row_data.batch_id = integers["batch_id"][row];
}

void DataFrame::get(IVOL_Query &temp) {
    for (int i = 0; i < row_count; i++) {
        temp.ivols[alphanumerics["Date"][i]] = std::make_tuple(reals["IV_Current"][i], reals["IV_Next"][i]);
    }
}

void DataFrame::get(Expiry_Query &temp) {
    for (int i = 0; i < row_count; i++) {
        temp.days_to_expiry[alphanumerics["date"][i]] = std::make_tuple(integers["current_week"][i],
                                                                        integers["next_week"][i]);
    }
}

void DataFrame::To_CSV(const std::string &filename, const std::string &suffix) const {
    std::vector<std::string> columns;
    std::ofstream myfile;

    std::string file = config.file_location + filename + suffix + ".csv";

    myfile.open(file.c_str());
    for (auto &i: dtype_info) {
        myfile << i.first << ",";
        columns.push_back(i.first);
    }
    myfile << "\n";

    for (int i = 0; i < row_count; i++) {
        for (auto &col: columns) {
            if (reals.find(col) != reals.end()) { myfile << reals.at(col)[i] << ","; }
            else if (integers.find(col) != integers.end()) { myfile << integers.at(col)[i] << ","; }
            else if (alphanumerics.find(col) != alphanumerics.end()) { myfile << alphanumerics.at(col)[i] << ","; }
            else { myfile << Time_T_To_String(dates.at(col)[i]) << ","; }
        }
        myfile << "\n";
    }
    myfile.close();
}

void DataFrame::get(std::vector<Simulation_Results> &temp_sim) {
    Strategy_Parameters temp_strat;
    Simulation_Results Sim;

    for (int i = 0; i < row_count; i++) {
        temp_strat.Initialize((integers["current_week"][i] == 1 ? true : false), integers["strategy_type"][i],
                              integers["strategy_version"][i], reals["pos_ratio"][i],
                              alphanumerics["p_multiplier_string"][i], alphanumerics["m_multiplier_string"][i],
                              alphanumerics["pr_multiplier_string"][i],
                              std::make_tuple(reals["sim_vol"][i], reals["sim_vol"][i]),
                              10000.0);
        temp_strat.Fill_Params(reals["pos_param"][i], reals["mov_param"][i], reals["prof_param"][i]);
        Sim.Package(temp_strat, alphanumerics["underlying"][i], alphanumerics["date"][i], 9);
        Sim.capital = reals["capital"][i];
        Sim.pnl = reals["pnl"][i];
        Sim.first_drawdown_pnl = reals["first_drawdown_pnl"][i];
        Sim.second_drawdown_pnl = reals["second_drawdown_pnl"][i];
        Sim.drawdown_count = integers["drawdown_count"][i];
        Sim.trade_count = integers["trade_count"][i];
        temp_sim.push_back(Sim);
    }
}

void DataFrame::get(std::vector<Yesterday_Params> &y_param_vec) {
    Yesterday_Params temp;
    for (int i = 0; i < row_count; i++) {
        temp.fill(integers["strategy_variant"][i], reals["pos_param"][i], reals["mov_param"][i],
                  reals["prof_param"][i]);
        y_param_vec.push_back(temp);
    }
    std::sort(y_param_vec.begin(), y_param_vec.end());
}

#endif // !DATAFRAME_CPP

