#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <exception>


class Parameter_Err : public std::exception
{
  private:
    std::string m_message;

  public:
    Parameter_Err(const std::string& index, const std::map<std::string, unsigned int>& parameters)
    {
        m_message = std::string("Parameter '") + index + std::string("' not found - possible parameters: ");
        for (const std::pair<const std::string, unsigned int>& parameter:parameters)
            m_message += std::string("'") + parameter.first + std::string("' ");
    }
    Parameter_Err() = delete;

    virtual const char* what() const noexcept override
    {
        return m_message.c_str();
    }
};

class Index_Err : public std::exception
{
  private:
    std::string m_message;

  public:
    Index_Err(const unsigned int& index, const unsigned int& container_size)
    {
        m_message = std::string("Illegal index '") + std::to_string(index)
        + std::string("' to access element - valid range is [0:") + std::to_string(container_size - 1)
        + std::string("]");
    }
    Index_Err() = delete;

    virtual const char* what() const noexcept override
    {
        return m_message.c_str();
    }
};

class Find_Parameter_Err : public std::exception
{
  private:
    std::string m_message;

  public:
    Find_Parameter_Err(const std::string& index_type, const std::string& index, const std::vector<std::string>& possible_indices)
    {
        m_message = std::string("Index '") + index + std::string("' of type '") + index_type
        + std::string("' not found - possible indices: ");
        for (const std::string& element:possible_indices)
            m_message += std::string("'") + element + std::string("' ");
    }
    Find_Parameter_Err() = delete;

    virtual const char* what() const noexcept override
    {
        return m_message.c_str();
    }
};

class Simple_Err : public std::exception
{
  private:
    std::string m_message;

  public:
    Simple_Err(const std::string& message)
    : m_message(message)
    {}
    Simple_Err() = delete;

    virtual const char* what() const noexcept override
    {
        return m_message.c_str();
    }
};

#endif // EXCEPTIONS_H
