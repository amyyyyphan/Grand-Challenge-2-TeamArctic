#include <string>
#include <time.h>
#include <fstream>

// Source: https://stackoverflow.com/a/46866854
class Logger {
    public:
        std::string getCurrentDateTime(std::string s);
        void logger(std::string logMsg);
};

inline std::string getCurrentDateTime(std::string s) {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    if(s == "now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s == "date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
}

inline void logger(std::string logMsg) {
    std::string filePath = "../logs/log_" + getCurrentDateTime("date") + ".txt";
    std::string now = getCurrentDateTime("now");
    std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}