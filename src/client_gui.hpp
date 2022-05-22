#ifndef CLIENT_GUI_HPP_
#define CLIENT_GUI_HPP_


/**
 * @file
 *
 * This header file declares Gui used by Client instances.
**/
#include <atomic>
#include <deque>
#include <string>
#include "logger.hpp"
#include "storage.hpp"


using Panel = ValueStorage<Message>;
using MsgBuffer = std::deque<Message>;


/**
 * @brief Terminal-based Gui running on Client instances.
**/
class Gui final
{
private:
    static constexpr int GUI_TIMEOUT = 100;
    static constexpr std::size_t MAX_MSG_LENGTH = 32;

    Panel& panel_;
    MsgBuffer buff_stor_;
    PendingDeque& send_stor_;
    PendingDeque& recv_stor_;

    /**
     * @brief Try fetch message from receive storage and put it to buffer.
    **/
    void fetch_message();

    /**
     * @brief Draws terminal window according to the schema.
    **/
    void draw_window(int rows, const char* buf);

public:

    /**
     * @brief Constructs terminal-based Gui.
     * 
     * @note Gui sends messages outside via @b send_stor .
     *     Gui receive messages from outside via @b recv_stor .
    **/
    Gui(Panel& panel, PendingDeque& send_stor, PendingDeque& recv_stor);

    /**
     * @brief Draws the interface in an infinite loop and gets
     *     user input.
     *
     * @note The routine (cycle) is blocking and explicit @b wait_for
     *     is not necessary.
    **/
    void loop(std::atomic_bool& done);

    Gui(Gui&& gui) = delete;
    Gui(const Gui& gui) = delete;
    Gui& operator=(Gui&& gui) = delete;
    Gui& operator=(const Gui& gui) = delete;
};


#endif
