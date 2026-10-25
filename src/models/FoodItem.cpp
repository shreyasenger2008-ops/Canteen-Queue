#include "canteen/models/FoodItem.hpp"
#include <iomanip>
#include <sstream>

namespace canteen {

FoodItem::FoodItem(int id, std::string name, std::string category, double price, int prepTimeMinutes, bool available)
    : id(id), name(std::move(name)), category(std::move(category)), price(price),
      preparationTimeMinutes(prepTimeMinutes), isAvailable(available) {}

void FoodItem::display() const {
    std::cout << toString() << "\n";
}

std::string FoodItem::toString() const {
    std::ostringstream oss;
    oss << "[Item #" << id << "] " << std::left << std::setw(20) << name
        << " | Cat: " << std::setw(12) << category
        << " | Price: Rs." << std::fixed << std::setprecision(2) << std::setw(6) << price
        << " | Prep: " << std::setw(2) << preparationTimeMinutes << " mins"
        << " | Status: " << (isAvailable ? "AVAILABLE" : "OUT OF STOCK");
    return oss.str();
}

} // namespace canteen
