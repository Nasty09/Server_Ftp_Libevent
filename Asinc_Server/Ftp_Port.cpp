#include "Server.hpp"

void Ftp_Port::Parse(const std::string &type, const std::string &msg) {
    std::vector<std::string> val;
    std::string tmp;
    for (int i = 5; i < msg.size(); ++i)
    {
        if (msg[i] == ',' || msg[i] == '\r')
        {
            val.emplace_back(tmp);
            tmp.clear();
            continue;
        }
        tmp += msg[i];
    }
    if (val.size() != 6) {
        ResponseCMD("501 Syntax error in parameters or arguments");
        return;
    }
    char *errbuf;
    ip = val[0] + "." + val[1] + "." + val[2] + "." + val[3];
    port = strtol(val[4].c_str(), &errbuf, 10) * 256 + strtol(val[5].c_str(), &errbuf, 10);
    std::cout << "IP is " << ip << "\n";
    std::cout << "Port is " << port << "\n";
    ResponseCMD("200 PORT command successful.\r\n");
}
