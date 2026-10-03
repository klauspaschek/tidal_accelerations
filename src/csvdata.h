#ifndef CSVDATA_H
#define CSVDATA_H

#include <vector>
#include <string>
#include <map>
#include <exception>
#include <stdexcept>
#include <fstream>

#include "exceptions.h"


// manage data in .csv file
// assuming first line gives parameters of table
// maybe followed by some headlines
// followed by actual data corresponding to parameters in first line
class CSVData
{
  private:
    // parameters in first line of .csv file mapped to corresponding index of position
    // to simplify searching for parameters
    std::map<std::string, unsigned int> m_param_indices;
    // 'matrices' for headlines and data
    std::vector<std::vector<std::string> > m_headlines;
    std::vector<std::vector<std::string> > m_data;

  public:
    CSVData(const std::string& filename, const unsigned int& n_headlines)
    {
        // open file
        std::ifstream input(filename);

        if (!input.is_open())
            throw Simple_Err(std::string("Opening file '") + filename + std::string("' failed"));

        std::string line;

        // map parameters to corresponding indices
        read_line(input, line);
        std::vector<std::string> parameters(split(line, ','));

        unsigned int i(0);
        for (std::string parameter:parameters)
        {
            m_param_indices[trim(parameter)] = i;
            ++i;
        }

        // read headlines if existing
        if (n_headlines > 0)
        {
            for (unsigned int j(0); j < n_headlines; ++j)
            {
                read_line(input, line);
                m_headlines.push_back(split(line, ','));
            }
        }

        // read data
        while (read_line(input, line))
            if (!line.empty())
                m_data.push_back(split(line, ','));

        input.close();
    }

    // gives access to data via int for row index and to parameter/column via std::string
    std::string operator()(const unsigned int& row_index, const std::string& col_parameter) const
    {
        unsigned int col_index(index(col_parameter));

        try
        {
            return m_data.at(row_index).at(col_index);
        }
        catch (const std::out_of_range& error)
        {
            throw Index_Err(row_index, m_data.size());
        }
    }

    // looks if entry in row 'row_index' and column 'col_parameter' exists
    // if yes, it returns this entry
    // if not, it returns entry in same row and column 'col_guessed_parameter'
    std::string operator()(const unsigned int& row_index, const std::string& col_parameter, const std::string& col_guessed_parameter) const
    {
        unsigned int col_index_parameter(index(col_parameter)), col_index_guessed_parameter(index(col_guessed_parameter));
        try
        {
            std::string entry(m_data.at(row_index).at(col_index_parameter));

            if (entry != "")
                return entry;
            else
                return m_data.at(row_index).at(col_index_guessed_parameter);
        }
        catch (const std::out_of_range& error)
        {
            throw Index_Err(row_index, m_data.size());
        }
    }

    // returns entry in row 'row_index' and column 'col_parameter'
    // but only if control entry in same row and column 'col_control_parameter' is empty
    // when it is not empty, the control entry is returned
    std::string fancy_access(const unsigned int& row_index, const std::string& col_parameter, const std::string& col_control_parameter) const
    {
        unsigned int col_index_parameter(index(col_parameter)),
                     col_index_control_parameter(index(col_control_parameter));
        try
        {
            std::string entry(m_data.at(row_index).at(col_index_parameter)),
                        control_entry(m_data.at(row_index).at(col_index_control_parameter));

            if (control_entry == "")
                return entry;
            else
                return control_entry;
        }
        catch (const std::out_of_range& error)
        {
            throw Index_Err(row_index, m_data.size());
        }
    }

    // gives access to data via std::string for row and column
    // 'col_row_parameter' defines parameter/column in which 'row_parameters' is looked up
    std::string operator()(const std::string& col_row_parameter, const std::string& row_parameter, const std::string& col_parameter) const
    {
        unsigned int row_index(find_row_index(col_row_parameter, row_parameter)), col_index(index(col_parameter));

        return m_data.at(row_index).at(col_index);
    }

    // getters for member variables
    std::vector<std::string> get_parameters() const
    {
        std::vector<std::string> parameters;
        for (unsigned int i(0); i < m_param_indices.size(); ++i)
        {
            for (const std::pair<const std::string, unsigned int>& parameter:m_param_indices)
            {
                if (parameter.second == i)
                    parameters.push_back(parameter.first);
            }
        }

        return parameters;
    }

    std::vector<std::vector<std::string> > get_headlines() const
    {
        return m_headlines;
    }

    // getters for sizes of member variables to allow iterating over them
    unsigned int get_size_parameters() const
    {
        return m_param_indices.size();
    }

    unsigned int get_size_headlines() const
    {
        return m_headlines.size();
    }

    unsigned int get_size_data() const
    {
        return m_data.size();
    }

  private:
    // use char 'separator' to split std::string 'line' and return components as std::vector<std::string>
    // last character of 'line' is discarded
    // split line at separator, empty components are kept
    std::vector<std::string> split(const std::string& line, const char& separator)
    {
        std::vector<std::string> result;
        std::string::size_type start(0), position(line.find(separator));
        while (position != std::string::npos)
        {
            result.push_back(line.substr(start, position - start));
            start = position + 1;
            position = line.find(separator, start);
        }
        result.push_back(line.substr(start));

        return result;
    }

    // read next line, works for files with Linux (LF) and Windows (CRLF) line endings
    static bool read_line(std::ifstream& input, std::string& line)
    {
        if (!std::getline(input, line))
            return false;
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        return true;
    }

    // remove spaces and tabs at the beginning and end of string
    static std::string trim(const std::string& input_str)
    {
        std::string::size_type first(input_str.find_first_not_of(" \t")), last(input_str.find_last_not_of(" \t"));
        if (first == std::string::npos)
            return std::string("");

        return input_str.substr(first, last - first + 1);
    }

    // gives int index of corresponding 'parameter'
    unsigned int index(const std::string& parameter) const
    {
        try
        {
            return m_param_indices.at(parameter);
        }
        catch (const std::exception& error)
        {
            throw Parameter_Err(parameter, m_param_indices);
        }
    }

    // searches 'row_parameter' in specified column 'row_param_type' and return corresponding index as int
    unsigned int find_row_index(const std::string& row_param_type, const std::string& row_parameter) const
    {
        unsigned int col_index(index(row_param_type)), i(0);
        for (const std::vector<std::string>& row:m_data)
        {
            if (row.at(col_index) == row_parameter)
                return i;

            ++i;
        }

        // prepare exception
        std::vector<std::string> possible;
        for (unsigned int j(0); j < m_data.size(); ++j)
            possible.push_back(m_data.at(j).at(col_index));
        throw Find_Parameter_Err(row_param_type, row_parameter, possible);
    }
};

#endif // CSVDATA_H
