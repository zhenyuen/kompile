#include <iostream>
#include <string>

namespace logging {
void error(std::string message) { fprintf(stderr, "Error: %s\n", message.c_str()); }

void info(std::string message) { fprintf(stdout, "Error: %s\n", message.c_str()); }
}  // namespace logging


class Logging
{
private:
    /* data */
public:
    Logging() = default;
    ~Logging() = default;

    void error(std::string message);
    void info(std::string message);
};

void Logging::error(std::string message) { fprintf(stderr, "Error: %s\n", message.c_str()); }
void Logging::info(std::string message) { fprintf(stdout, "Error: %s\n", message.c_str()); }

