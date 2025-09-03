#ifndef DATAFRAME_H
#define DATAFRAME_H

#include<vector>
#include<map>
#include<ctime>
#include<iostream>
#include<sstream>
#include<iomanip>
#include <fstream>

#include "Utilities.h"

class DataFrame {    //Date, Open, High, Low, Close, Batch_ID
private:
    std::map<std::string, std::vector<time_t>> dates;
    std::map<std::string, std::vector<int>> integers;
    std::map<std::string, std::vector<std::string>> alphanumerics;
    std::map<std::string, std::vector<float>> reals;
    std::map<std::string, std::string> dtype_info;
    const Config config;
public:
    int date_count, int_count, string_count, float_count;
    long int row_count;

    //accepts two arguments both list of string type
    DataFrame(const std::vector<std::string> &, const std::vector<std::string> &);

    DataFrame(const DataFrame &other) = default;

    DataFrame &operator=(const DataFrame &other) = default;

    std::map<std::string, std::string> parse_json_string(std::string, int, bool);

    void insert(const std::string &, int, bool);

    void insert(const std::map<std::string, time_t> &, const std::map<std::string, int> &,
                const std::map<std::string, std::string> &, const std::map<std::string, float> &);

    void insert(const std::map<std::string, std::string> &, const std::map<std::string, int> &,
                const std::map<std::string, std::string> &, const std::map<std::string, float> &);

    void insert(const std::map<std::string, std::string> &, const std::map<std::string, std::string> &,
                const std::map<std::string, std::string> &, const std::map<std::string, std::string> &);

    void print(int);

    void get(int, DOHLCI &);

    void get(IVOL_Query &);

    void get(Expiry_Query &);

    void get(std::vector<Simulation_Results> &);

    void get(std::vector<Yesterday_Params> &);

    void To_CSV(const std::string &, const std::string &) const;

};

#endif // !DATAFRAME_H

