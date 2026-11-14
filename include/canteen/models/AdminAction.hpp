#ifndef CANTEEN_ADMIN_ACTION_HPP
#define CANTEEN_ADMIN_ACTION_HPP

#include <string>
#include <sstream>

namespace canteen {

enum class ActionType {
    UPDATE_ITEM_AVAILABILITY,
    UPDATE_ITEM_PRICE,
    UPDATE_COUNTER_AVAILABILITY,
    ORDER_ACKNOWLEDGED
};

inline std::string actionTypeToString(ActionType type) {
    switch (type) {
        case ActionType::UPDATE_ITEM_AVAILABILITY:    return "UPDATE_ITEM_AVAILABILITY";
        case ActionType::UPDATE_ITEM_PRICE:           return "UPDATE_ITEM_PRICE";
        case ActionType::UPDATE_COUNTER_AVAILABILITY: return "UPDATE_COUNTER_AVAILABILITY";
        case ActionType::ORDER_ACKNOWLEDGED:          return "ORDER_ACKNOWLEDGED";
        default:                                     return "UNKNOWN_ACTION";
    }
}

inline std::ostream& operator<<(std::ostream& os, ActionType type) {
    return os << actionTypeToString(type);
}

/**
 * @brief Represents an administrative modification in the canteen system.
 * Stored in the custom Stack to enable LIFO undo capabilities for reversible actions.
 */
struct AdminAction {
    int actionId{0};
    ActionType type{ActionType::UPDATE_ITEM_AVAILABILITY};
    std::string actionType;
    int targetId{0};
    std::string targetName;
    bool prevBoolValue{false};
    bool newBoolValue{false};
    double prevDoubleValue{0.0};
    double newDoubleValue{0.0};
    std::string description;
    long long timestamp{0};

    AdminAction() = default;

    AdminAction(int id, ActionType t, int target, std::string name,
                bool prevB, bool newB, double prevD, double newD,
                std::string desc, long long ts = 0)
        : actionId(id), type(t), targetId(target), targetName(std::move(name)),
          prevBoolValue(prevB), newBoolValue(newB),
          prevDoubleValue(prevD), newDoubleValue(newD),
          description(std::move(desc)), timestamp(ts) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << "[Action #" << actionId << "] " << actionTypeToString(type)
            << " on " << targetName << " (ID: " << targetId << "): " << description;
        return oss.str();
    }
};

} // namespace canteen

#endif // CANTEEN_ADMIN_ACTION_HPP
