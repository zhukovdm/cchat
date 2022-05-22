#ifndef UTILITY_HPP_
#define UTILITY_HPP_


/**
 * @file
 * 
 * This header file declares smaller functions and structures, occasionally
 * used within @b cchat application.
**/
#include <atomic>
#include <unordered_set>
#include <vector>


/**
 * @brief Splits string on @b char separators.
**/
std::vector<std::string> split_string(const std::string& input);


/**
 * @brief Checks if user name is a non-trivial string with alphanumeric chars.
**/
bool is_user_name_valid(const std::string& name);


/**
 * @brief Convert string to the port number.
 *     Invalid input is reported via exception.
**/
uint16_t parse_port(const std::string& word);


/**
 * @brief Creates new POSIX socket.
 *     Throws exception if new socket cannot be created.
**/
int create_new_socket();


/**
 * @brief Allows socket to be reused after restart.
 *     Throws exception if socket cannot be reused.
**/
void allow_socket_reuse(int sock);


/**
 * @brief Sets non-blocking mode on the socket.
 *     Throws exception if socket cannot be configured non-blocking.
**/
void set_socket_non_blocking(int sock);


#endif
