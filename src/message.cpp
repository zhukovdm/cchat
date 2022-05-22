#include <algorithm>
#include <map>
#include "utility.hpp"
#include "message.hpp"


bool is_hist_count_valid(const std::string& cnt)
{
    const unsigned long MAX_COUNT = 10;

    auto d = std::all_of(cnt.begin(), cnt.end(), [](char c){
        return std::isdigit(c);
    });

    auto l = std::strtoul(cnt.c_str(), nullptr, 10) <= MAX_COUNT;

    return d && l;
}


bool is_chat_command_format(const std::vector<std::string>& words)
{
    return words.size() == 2
        && words[0] == "chat"
        && is_user_name_valid(words[1]);
}


bool is_hist_command_format(const std::vector<std::string>& words)
{
    return words.size() == 3
        && words[0] == "hist"
        && is_hist_count_valid(words[1])
        && is_user_name_valid(words[2]);
}


auto parse_command(const std::string& input) -> Command
{
    auto words = split_string(input);

    // help, pending/waiting userX messages, quit
    if (words.size() == 1) {
        std::map<std::string, Command> m {
            { "help", Command::HELP },
            { "pend", Command::PEND },
            { "quit", Command::QUIT }
        };

        auto it = m.find(words[0]);
        if (it != m.end()) { return it->second; }
    }

    if (is_chat_command_format(words)) { return Command::CHAT; }

    if (is_hist_command_format(words)) { return Command::HIST; }

    return Command::BAD;
}


auto parse_chat_command(const std::string& command) -> std::string
{
    return split_string(command)[1];
}


auto parse_hist_command(const std::string& command) -> std::pair<unsigned long, std::string>
{
    auto tokens = split_string(command);
    return { std::strtoul(tokens[1].c_str(), nullptr, 10), std::move(tokens[2]) };
}
