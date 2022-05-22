#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_


/**
 * @file
 *
 * This header file declares Message structures and operations on messages.
**/
#include <string>


enum class Command
{
    HELP,
    PEND,
    QUIT,
    CHAT,
    HIST,
    BAD
};


/**
 * @brief Recognizes if string represents any valid Command.
**/
Command parse_command(const std::string& input);


/**
 * @brief Parse CHAT command.
 *
 * @return Opponent's name.
**/
std::string parse_chat_command(const std::string& command);


/**
 * @brief Parse HIST command.
 *
 * @return Pair of count and opponent's name.
**/
std::pair<unsigned long, std::string> parse_hist_command(const std::string& command);


#endif
