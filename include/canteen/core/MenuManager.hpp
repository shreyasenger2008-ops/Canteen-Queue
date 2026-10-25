#ifndef CANTEEN_MENU_MANAGER_HPP
#define CANTEEN_MENU_MANAGER_HPP

#include "canteen/models/FoodItem.hpp"
#include "canteen/models/AdminAction.hpp"
#include "canteen/ds/Stack.hpp"
#include <vector>
#include <string>

namespace canteen {

/**
 * @brief Manages canteen menu catalog, searching, sorting, and administrative
 * action history with undo support.
 */
class MenuManager {
private:
    std::vector<FoodItem> items;
    Stack<AdminAction> undoStack;
    int nextActionId{1};

public:
    MenuManager() = default;

    // Item management
    void addItem(const FoodItem& item);
    size_t getItemCount() const { return items.size(); }
    const std::vector<FoodItem>& getItems() const { return items; }
    FoodItem* getItemById(int id);
    const FoodItem* getItemById(int id) const;

    // Searching operations (DSA syllabus)
    int linearSearchById(int id) const;
    int linearSearchByName(const std::string& name, bool exact = false) const;
    int binarySearchById(int id) const;

    // Sorting operations (DSA syllabus)
    void sortItemsById();
    void sortItemsByPrepTime();
    void sortItemsByPrice();

    // Administrative operations with Undo stack
    bool updateItemAvailability(int itemId, bool available);
    bool updateItemPrice(int itemId, double newPrice);
    bool canUndo() const { return !undoStack.isEmpty(); }
    bool undoLastAction(std::string& statusMessageOut);

    // Stack inspection
    size_t getUndoStackSize() const { return undoStack.size(); }

    // Display
    void displayMenu() const;
};

} // namespace canteen

#endif // CANTEEN_MENU_MANAGER_HPP
