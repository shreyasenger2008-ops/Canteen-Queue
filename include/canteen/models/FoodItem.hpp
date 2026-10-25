#ifndef CANTEEN_FOOD_ITEM_HPP
#define CANTEEN_FOOD_ITEM_HPP

#include <string>
#include <iostream>

namespace canteen {

/**
 * @brief Represents a food item on the college canteen menu.
 */
class FoodItem {
private:
    int id{0};
    std::string name;
    std::string category;
    double price{0.0};
    int preparationTimeMinutes{0};
    bool isAvailable{true};

public:
    FoodItem() = default;
    FoodItem(int id, std::string name, std::string category, double price, int prepTimeMinutes, bool available = true);

    // Getters
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getCategory() const { return category; }
    double getPrice() const { return price; }
    int getPreparationTimeMinutes() const { return preparationTimeMinutes; }
    bool getIsAvailable() const { return isAvailable; }

    // Setters
    void setId(int newId) { id = newId; }
    void setName(const std::string& newName) { name = newName; }
    void setCategory(const std::string& newCat) { category = newCat; }
    void setPrice(double newPrice) { price = newPrice; }
    void setPreparationTimeMinutes(int newPrepTime) { preparationTimeMinutes = newPrepTime; }
    void setIsAvailable(bool available) { isAvailable = available; }

    // Display helper
    void display() const;
    std::string toString() const;
};

} // namespace canteen

#endif // CANTEEN_FOOD_ITEM_HPP
