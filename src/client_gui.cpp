#include <cstring>
#include <iostream>
#include <ncurses.h>
#include "client_gui.hpp"


Gui::Gui(Panel& panel, PendingDeque& send_stor, PendingDeque& recv_stor)
    : panel_(panel), buff_stor_(), send_stor_(send_stor), recv_stor_(recv_stor)
{
}

auto Gui::fetch_message() -> void
{
    constexpr std::size_t TAPE_SIZE = 50;

    auto msg = recv_stor_.maybe_pop();

    if (msg.has_value()) {
        buff_stor_.emplace_back(std::move(*msg));

        while (buff_stor_.size() > TAPE_SIZE) {
            buff_stor_.pop_front();
        }
    }
}

auto Gui::draw_window(int rows, const char* buf) -> void
{
    // clean up current display
    erase();

    if (rows == 0) { /* do nothing */ }

    if (rows >= 2) {
        mvprintw(1, 0, "%s", panel_.load().c_str());
    }

    if (rows >= 3) {
        auto row = 2;
        auto idx = static_cast<int>(buff_stor_.size()) - 1;

        // print latest first
        for (; row < rows - 2 && idx >= 0; ++row, --idx) {
            mvprintw(row, 0, "%s", buff_stor_[idx].c_str());
        }

        mvprintw(rows - 1, 0, "\r");
    }

    if (rows >= 1) {
        mvprintw(0, 0, "> %s", buf);
    }

    refresh();
}

auto Gui::loop(std::atomic_bool& done) -> void
{
    WINDOW *w;

    // init new terminal window
    if ((w = initscr()) == NULL) {
        std::cout
            << "Not possible to init new terminal window."
            << std::endl;
        done.store(true);
        return;
    }

    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    timeout(GUI_TIMEOUT);

    char buf[MAX_MSG_LENGTH];
    std::memset(buf, '\0', MAX_MSG_LENGTH);

    char *ptr = buf;

    while (!done.load()) {

        fetch_message();
        draw_window(getmaxy(w), buf);

        int ch;
        if ((ch = getch()) != ERR) {

            // end of message
            if (ch == '\n') {
                *ptr = '\0';
                if (ptr != buf) {
                    send_stor_.push_back(std::string(buf));
                }
                ptr = buf;
                *ptr = '\0';
            }

            // remove last letter
            else if (ch == KEY_BACKSPACE) {
                if (ptr > buf) {
                    --ptr;
                    *ptr = '\0';
                }
            }

            // add only printable char
            else if (ptr - buf < static_cast<int>(sizeof(buf)) - 1 && std::isprint(ch)) {
                *ptr = static_cast<char>(ch);
                ++ptr;
                *ptr = '\0';
            }
        }
    }

    delwin(w);
    endwin();
}
