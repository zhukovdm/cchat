#include "storage.hpp"


User::User()
    : active_(0), pending_()
{
}

auto User::try_acquire(int id) -> bool
{
    int value = 0;
    return active_.compare_exchange_strong(value, id);
}

auto User::release(int id) -> void
{
    active_.compare_exchange_strong(id, 0);
}

auto User::get_pending() -> PendingMap&
{
    return pending_;
}
