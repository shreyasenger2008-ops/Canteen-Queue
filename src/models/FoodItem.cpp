#include "canteen/models/FoodItem.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace canteen {

FoodItem::FoodItem(int id, std::string name, std::string category, double price, int prepTimeMinutes, bool available, int comp)
    : id(id), name(std::move(name)), category(std::move(category)), price(price),
      preparationTimeMinutes(prepTimeMinutes), isAvailable(available), complexity(comp) {}

int FoodItem::getCategoryCode() const {
    std::string cat = category;
    std::transform(cat.begin(), cat.end(), cat.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (cat.find("fast") != std::string::npos || cat.find("burger") != std::string::npos || cat.find("roll") != std::string::npos) {
        return 0; // Fast Food
    }
    if (cat.find("south") != std::string::npos || cat.find("dosa") != std::string::npos || cat.find("idli") != std::string::npos) {
        return 1; // South Indian
    }
    if (cat.find("meal") != std::string::npos || cat.find("chinese") != std::string::npos || cat.find("rice") != std::string::npos || cat.find("thali") != std::string::npos || cat.find("biryani") != std::string::npos) {
        return 2; // Meals & Chinese
    }
    if (cat.find("bev") != std::string::npos || cat.find("coffee") != std::string::npos || cat.find("chai") != std::string::npos || cat.find("drink") != std::string::npos || cat.find("tea") != std::string::npos) {
        return 3; // Beverages
    }
    if (cat.find("bake") != std::string::npos || cat.find("snack") != std::string::npos || cat.find("pastry") != std::string::npos || cat.find("samosa") != std::string::npos) {
        return 4; // Bakery & Snacks
    }
    return 0; // Default
}

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
