#include "canteen/core/MenuManager.hpp"
#include "canteen/algorithms/Searching.hpp"
#include "canteen/algorithms/Sorting.hpp"
#include <iostream>
#include <iomanip>

namespace canteen {

void MenuManager::addItem(const FoodItem& item) {
    items.push_back(item);
}

FoodItem* MenuManager::getItemById(int id) {
    int idx = linearSearchById(id);
    if (idx != -1) {
        return &items[idx];
    }
    return nullptr;
}

const FoodItem* MenuManager::getItemById(int id) const {
    int idx = linearSearchById(id);
    if (idx != -1) {
        return &items[idx];
    }
    return nullptr;
}

int MenuManager::linearSearchById(int id) const {
    return searching::linearSearchFoodItemById(items.data(), items.size(), id);
}

int MenuManager::linearSearchByName(const std::string& name, bool exact) const {
    return searching::linearSearchFoodItemByName(items.data(), items.size(), name, exact);
}

int MenuManager::binarySearchById(int id) const {
    // If not sorted, user will see the exception or must sort first
    return searching::binarySearchFoodItemById(items.data(), items.size(), id);
}

void MenuManager::sortItemsById() {
    sorting::mergeSortFoodItemsById(items.data(), items.size());
}

void MenuManager::sortItemsByPrepTime() {
    sorting::mergeSortFoodItemsByPrepTime(items.data(), items.size());
}

void MenuManager::sortItemsByPrice() {
    sorting::mergeSortFoodItemsByPrice(items.data(), items.size());
}

bool MenuManager::updateItemAvailability(int itemId, bool available) {
    FoodItem* item = getItemById(itemId);
    if (item == nullptr) {
        return false;
    }

    bool previousState = item->getIsAvailable();
    if (previousState == available) {
        return true; // No change needed
    }

    item->setIsAvailable(available);

    // Record reversible action on custom stack
    std::string desc = "Changed availability from " +
                       std::string(previousState ? "AVAILABLE" : "OUT OF STOCK") +
                       " to " + (available ? "AVAILABLE" : "OUT OF STOCK");

    AdminAction action(nextActionId++, ActionType::UPDATE_ITEM_AVAILABILITY, itemId,
                        item->getName(), previousState, available, 0.0, 0.0, desc, 0);
    undoStack.push(std::move(action));
    return true;
}

bool MenuManager::updateItemPrice(int itemId, double newPrice) {
    if (newPrice < 0.0) {
        return false;
    }
    FoodItem* item = getItemById(itemId);
    if (item == nullptr) {
        return false;
    }

    double previousPrice = item->getPrice();
    item->setPrice(newPrice);

    std::string desc = "Updated price from Rs." + std::to_string(previousPrice) +
                       " to Rs." + std::to_string(newPrice);

    AdminAction action(nextActionId++, ActionType::UPDATE_ITEM_PRICE, itemId,
                        item->getName(), false, false, previousPrice, newPrice, desc, 0);
    undoStack.push(std::move(action));
    return true;
}

bool MenuManager::undoLastAction(std::string& statusMessageOut) {
    if (undoStack.isEmpty()) {
        statusMessageOut = "Undo failed: Action history stack is empty.";
        return false;
    }

    AdminAction action = undoStack.pop();

    if (action.type == ActionType::UPDATE_ITEM_AVAILABILITY) {
        FoodItem* item = getItemById(action.targetId);
        if (item != nullptr) {
            item->setIsAvailable(action.prevBoolValue);
            statusMessageOut = "Successfully undone: Restored " + item->getName() +
                               " availability to " + (action.prevBoolValue ? "AVAILABLE" : "OUT OF STOCK");
            return true;
        }
    } else if (action.type == ActionType::UPDATE_ITEM_PRICE) {
        FoodItem* item = getItemById(action.targetId);
        if (item != nullptr) {
            item->setPrice(action.prevDoubleValue);
            statusMessageOut = "Successfully undone: Restored " + item->getName() +
                               " price to Rs." + std::to_string(action.prevDoubleValue);
            return true;
        }
    }

    statusMessageOut = "Undo failed: Target entity no longer exists.";
    return false;
}

void MenuManager::displayMenu() const {
    std::cout << "\n============================== CANTEEN MENU ==============================\n";
    std::cout << std::left << std::setw(8) << "ID"
              << std::setw(22) << "Item Name"
              << std::setw(16) << "Category"
              << std::setw(12) << "Price"
              << std::setw(12) << "Prep Time"
              << "Status\n";
    std::cout << "--------------------------------------------------------------------------\n";
    for (const auto& item : items) {
        std::cout << "#" << std::left << std::setw(7) << item.getId()
                  << std::setw(22) << item.getName()
                  << std::setw(16) << item.getCategory()
                  << "Rs." << std::fixed << std::setprecision(2) << std::setw(8) << item.getPrice()
                  << std::setw(2) << item.getPreparationTimeMinutes() << " mins    "
                  << (item.getIsAvailable() ? "[AVAILABLE]" : "[OUT OF STOCK]") << "\n";
    }
    std::cout << "==========================================================================\n";
}

} // namespace canteen
