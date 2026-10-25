#ifndef CANTEEN_ORDER_STATUS_HPP
#define CANTEEN_ORDER_STATUS_HPP

#include <string>

namespace canteen {

/**
 * @brief Represents the lifecycle states of a canteen order.
 * State progression:
 *   QUEUED -> PREPARING -> READY -> COLLECTED (terminal)
 *   QUEUED -> CANCELLED (terminal)
 *   PREPARING -> CANCELLED (terminal)
 */
enum class OrderStatus {
    QUEUED,     ///< Order received and waiting in counter queue
    PREPARING,  ///< Kitchen staff currently preparing the order
    READY,      ///< Food prepared, ready for student pickup
    COLLECTED,  ///< Order collected by student (terminal state)
    CANCELLED   ///< Order cancelled by student or staff (terminal state)
};

inline std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::QUEUED:    return "QUEUED";
        case OrderStatus::PREPARING: return "PREPARING";
        case OrderStatus::READY:     return "READY";
        case OrderStatus::COLLECTED: return "COLLECTED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        default:                     return "UNKNOWN";
    }
}

inline std::ostream& operator<<(std::ostream& os, OrderStatus status) {
    return os << orderStatusToString(status);
}

/**
 * @brief Validates whether a state transition from `current` to `next` is permitted.
 * @param current Current order status
 * @param next Target order status
 * @return true if valid, false otherwise
 */
inline bool canTransition(OrderStatus current, OrderStatus next) {
    if (current == next) {
        return true;
    }
    switch (current) {
        case OrderStatus::QUEUED:
            return (next == OrderStatus::PREPARING || next == OrderStatus::CANCELLED);
        case OrderStatus::PREPARING:
            return (next == OrderStatus::READY || next == OrderStatus::CANCELLED);
        case OrderStatus::READY:
            return (next == OrderStatus::COLLECTED);
        case OrderStatus::COLLECTED:
            return false; // Terminal state: cannot transition out
        case OrderStatus::CANCELLED:
            return false; // Terminal state: cannot transition out
        default:
            return false;
    }
}

} // namespace canteen

#endif // CANTEEN_ORDER_STATUS_HPP
