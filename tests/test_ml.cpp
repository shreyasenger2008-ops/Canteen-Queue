#include "test_framework.hpp"
#include "canteen/ml/RandomForestPredictor.hpp"
#include "canteen/core/CanteenSystem.hpp"
#include "canteen/models/Order.hpp"
#include <vector>
#include <cmath>
#include <limits>

using namespace canteen;
using namespace canteen::ml;

TEST_CASE(ML_ModelLoadSuccess) {
    RandomForestPredictor predictor;
    ASSERT_TRUE(predictor.isLoaded());
    // Confirms all 20 decision trees from scikit-learn were loaded
    ASSERT_EQ(predictor.getEstimatorCount(), 20);
}

TEST_CASE(ML_FinitePositivePredictions) {
    RandomForestPredictor predictor;
    ASSERT_TRUE(predictor.isLoaded());

    // Test each food category (0 to 4) under normal conditions
    for (int cat = 0; cat < 5; ++cat) {
        OrderPredictionFeatures feats;
        feats.foodCategory = cat;
        feats.orderQuantity = 2;
        feats.preparationComplexity = (cat >= 2) ? 4 : 2;
        feats.timeOfDayHour = 12.5; // Lunch rush
        feats.currentKitchenWorkload = 5.0; // 5 mins pending

        double pred = predictor.predict(feats);

        ASSERT_FALSE(std::isnan(pred));
        ASSERT_FALSE(std::isinf(pred));
        ASSERT_TRUE(pred > 0.0);
        // Realistic bound: preparation shouldn't exceed 45 minutes
        ASSERT_TRUE(pred < 45.0);
    }
}

TEST_CASE(ML_CompareWithPythonHeldOutPredictions) {
    RandomForestPredictor predictor;
    ASSERT_TRUE(predictor.isLoaded());

    // Verified held-out samples directly from ml/test_predictions.json
    struct TestCase {
        std::vector<double> features;
        double pythonPrediction;
    };

    const std::vector<TestCase> testSamples = {
        // Sample 0: [Category: 3 (Beverage), Qty: 2, Comp: 1, Hour: 19.17, Workload: 0.63]
        {{3.0, 2.0, 1.0, 19.17, 0.63}, 3.832597},
        // Sample 1: [Category: 0 (Fast Food), Qty: 1, Comp: 3, Hour: 8.74, Workload: 10.73]
        {{0.0, 1.0, 3.0, 8.74, 10.73}, 10.019401},
        // Sample 2: [Category: 0 (Fast Food), Qty: 2, Comp: 3, Hour: 12.94, Workload: 9.50]
        {{0.0, 2.0, 3.0, 12.94, 9.50}, 12.011613},
        // Sample 3: [Category: 2 (Meals), Qty: 5, Comp: 4, Hour: 10.80, Workload: 3.45]
        {{2.0, 5.0, 4.0, 10.80, 3.45}, 25.135599},
        // Sample 4: [Category: 1 (South Indian), Qty: 1, Comp: 3, Hour: 10.00, Workload: 0.98]
        {{1.0, 1.0, 3.0, 10.00, 0.98}, 7.531225}
    };

    for (const auto& sample : testSamples) {
        double cppPrediction = predictor.predictRaw(sample.features);
        double diff = std::abs(cppPrediction - sample.pythonPrediction);
        // Verify C++ and Python agree to within 1e-4 minutes (0.006 seconds)
        ASSERT_TRUE(diff < 1e-4);
    }
}

TEST_CASE(ML_SafeInputValidationAndErrorHandling) {
    RandomForestPredictor predictor;
    ASSERT_TRUE(predictor.isLoaded());

    // Case 1: Incomplete feature vector (< 5 features)
    std::vector<double> incompleteFeatures = {0.0, 2.0, 3.0};
    ASSERT_THROWS(predictor.predictRaw(incompleteFeatures), std::invalid_argument);

    // Case 2: NaN in input feature
    std::vector<double> nanFeatures = {0.0, 1.0, std::numeric_limits<double>::quiet_NaN(), 12.0, 5.0};
    ASSERT_THROWS(predictor.predictRaw(nanFeatures), std::invalid_argument);

    // Case 3: Infinity in input feature
    std::vector<double> infFeatures = {0.0, 1.0, 2.0, std::numeric_limits<double>::infinity(), 5.0};
    ASSERT_THROWS(predictor.predictRaw(infFeatures), std::invalid_argument);
}

TEST_CASE(ML_MissingModelFileFallbackSafety) {
    // Attempt loading a file that definitely does not exist
    RandomForestPredictor fallbackPredictor("nonexistent_directory/nonexistent_model.json");

    // Must not crash; automatically falls back to embedded compiled tree
    ASSERT_TRUE(fallbackPredictor.isLoaded());
    ASSERT_TRUE(fallbackPredictor.getEstimatorCount() >= 1);

    OrderPredictionFeatures feats;
    feats.foodCategory = 0;
    feats.orderQuantity = 1;
    feats.preparationComplexity = 3;
    feats.timeOfDayHour = 12.0;
    feats.currentKitchenWorkload = 0.0;

    double pred = 0.0;
    ASSERT_NO_THROW(pred = fallbackPredictor.predict(feats));
    ASSERT_TRUE(pred > 0.0);
    ASSERT_FALSE(std::isnan(pred));
}

TEST_CASE(ML_PredictionsReachSchedulingEngine) {
    CanteenSystem system;
    ASSERT_TRUE(system.getPredictor().isLoaded());
    ASSERT_TRUE(system.getUseMLPredictor());

    // Place an order for Veg Cheese Burger (Item 101, Fast Food)
    int orderId = system.placeOrder("STU99", "ML Test Student", {{101, 1}});
    ASSERT_TRUE(orderId > 0);

    Order placedOrder;
    ASSERT_TRUE(system.searchOrderById(orderId, placedOrder));

    // Confirm that totalPreparationTimeMinutes is set to a valid predicted duration
    ASSERT_TRUE(placedOrder.getTotalPreparationTimeMinutes() > 0);

    // Confirm assigned counter workload matches the predicted prep time
    int counterId = placedOrder.getAssignedCounterId();
    Counter* counter = system.getCounterManager().getCounterById(counterId);
    ASSERT_TRUE(counter != nullptr);
    ASSERT_EQ(counter->getCurrentWorkloadMinutes(), placedOrder.getTotalPreparationTimeMinutes());
}

TEST_CASE(ML_PreservesFIFOOrderingUnderPredictedAllocations) {
    CanteenSystem system;

    // Place three consecutive orders at Counter 1
    int o1 = system.placeOrder("STU_A", "Alice", {{101, 1}}, 1);
    int o2 = system.placeOrder("STU_B", "Bob",   {{105, 1}}, 1);
    int o3 = system.placeOrder("STU_C", "Carol", {{107, 1}}, 1);

    Counter* c1 = system.getCounterManager().getCounterById(1);
    ASSERT_TRUE(c1 != nullptr);
    ASSERT_EQ(c1->getPendingOrderCount(), 3);

    // Verify strict FIFO queue order: Order 1 -> Order 2 -> Order 3
    Order completed1 = c1->completeFrontOrder();
    ASSERT_EQ(completed1.getId(), o1);

    Order completed2 = c1->completeFrontOrder();
    ASSERT_EQ(completed2.getId(), o2);

    Order completed3 = c1->completeFrontOrder();
    ASSERT_EQ(completed3.getId(), o3);

    ASSERT_EQ(c1->getPendingOrderCount(), 0);
    ASSERT_EQ(c1->getCurrentWorkloadMinutes(), 0);
}
