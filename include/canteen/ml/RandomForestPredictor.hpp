#ifndef CANTEEN_RANDOM_FOREST_PREDICTOR_HPP
#define CANTEEN_RANDOM_FOREST_PREDICTOR_HPP

#include "canteen/ml/IPrepTimePredictor.hpp"
#include <vector>
#include <string>

namespace canteen {
namespace ml {

/**
 * @brief Node representation in a regression decision tree.
 */
struct DecisionTreeNode {
    int feature{-2};        ///< Feature index to split on (-2 for leaf node)
    double threshold{0.0};  ///< Threshold for splitting
    int leftChild{-1};      ///< Index of left child node (-1 for leaf)
    int rightChild{-1};     ///< Index of right child node (-1 for leaf)
    double value{0.0};      ///< Prediction value at leaf node
};

/**
 * @brief Individual Decision Tree Regressor evaluated in the ensemble.
 */
struct DecisionTree {
    int treeIndex{0};
    int nodeCount{0};
    std::vector<DecisionTreeNode> nodes;

    /**
     * @brief Traverses decision tree to predict preparation duration.
     * Complexity: O(depth) where depth <= 6
     */
    double predict(const std::vector<double>& features) const;
};

/**
 * @brief RandomForestRegressor C++ inference engine.
 * Evaluates an ensemble of decision trees trained in Python/scikit-learn.
 * Supports loading from external JSON files and provides an embedded fallback.
 */
class RandomForestPredictor : public IPrepTimePredictor {
private:
    std::vector<DecisionTree> trees;
    std::vector<std::string> featureNames;
    bool loaded{false};
    std::string modelSource{"UNLOADED"};
    double fallbackMean{9.7348}; // Training set mean from canteen_prep_data.csv

public:
    RandomForestPredictor();
    explicit RandomForestPredictor(const std::string& jsonFilePath);

    // Model loading
    bool loadFromJsonFile(const std::string& filePath);
    bool loadFromJsonString(const std::string& jsonContent);
    void loadEmbeddedFallback();

    // Inference interface
    double predict(const OrderPredictionFeatures& features) const override;
    double predictRaw(const std::vector<double>& rawFeatures) const override;

    // Status inspection
    bool isLoaded() const override { return loaded; }
    std::string getModelName() const override { return modelSource; }
    size_t getEstimatorCount() const override { return trees.size(); }
    double getFallbackMean() const { return fallbackMean; }
    const std::vector<DecisionTree>& getTrees() const { return trees; }
};

} // namespace ml
} // namespace canteen

#endif // CANTEEN_RANDOM_FOREST_PREDICTOR_HPP
