#ifndef CANTEEN_I_PREP_TIME_PREDICTOR_HPP
#define CANTEEN_I_PREP_TIME_PREDICTOR_HPP

#include <vector>
#include <string>

namespace canteen {
namespace ml {

/**
 * @brief Normalized feature vector passed to the machine learning inference engine.
 */
struct OrderPredictionFeatures {
    int foodCategory{0};              ///< 0: Fast Food, 1: South Indian, 2: Meals/Chinese, 3: Beverages, 4: Bakery/Snacks
    int orderQuantity{1};             ///< Total item count in order (>= 1)
    int preparationComplexity{1};     ///< Complexity score (1: simple/premade to 5: elaborate/cooked-to-order)
    double timeOfDayHour{12.0};       ///< Time of day in decimal hours (e.g. 13.5 = 1:30 PM)
    double currentKitchenWorkload{0.0}; ///< Pending work queued at evaluated counter (in minutes)

    std::vector<double> toVector() const {
        return {
            static_cast<double>(foodCategory),
            static_cast<double>(orderQuantity),
            static_cast<double>(preparationComplexity),
            timeOfDayHour,
            currentKitchenWorkload
        };
    }
};

/**
 * @brief Abstract interface for food preparation time prediction engines.
 * Enables clean decoupling of model inference from the DSA queue scheduler.
 */
class IPrepTimePredictor {
public:
    virtual ~IPrepTimePredictor() = default;

    /**
     * @brief Predicts food preparation duration in minutes.
     * @param features Order and counter context features
     * @return Positive preparation time in minutes
     */
    virtual double predict(const OrderPredictionFeatures& features) const = 0;

    /**
     * @brief Raw feature vector inference for direct testing against Python outputs.
     */
    virtual double predictRaw(const std::vector<double>& rawFeatures) const = 0;

    /**
     * @brief Checks if model weights are loaded and ready.
     */
    virtual bool isLoaded() const = 0;

    /**
     * @brief Name and description of the underlying model.
     */
    virtual std::string getModelName() const = 0;

    /**
     * @brief Number of trees/estimators in the ensemble.
     */
    virtual size_t getEstimatorCount() const = 0;
};

} // namespace ml
} // namespace canteen

#endif // CANTEEN_I_PREP_TIME_PREDICTOR_HPP
