#include "canteen/ml/RandomForestPredictor.hpp"
#include "canteen/models/EmbeddedModelData.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <algorithm>

namespace canteen {
namespace ml {

double DecisionTree::predict(const std::vector<double>& features) const {
    if (nodes.empty()) {
        return 0.0;
    }

    int curr = 0;
    while (curr >= 0 && curr < static_cast<int>(nodes.size())) {
        const auto& node = nodes[curr];
        // Leaf node indicated by feature == -2
        if (node.feature == -2) {
            return node.value;
        }

        int featIdx = node.feature;
        if (featIdx < 0 || featIdx >= static_cast<int>(features.size())) {
            return node.value; // Fallback if out-of-range feature
        }

        if (features[featIdx] <= node.threshold) {
            curr = node.leftChild;
        } else {
            curr = node.rightChild;
        }
    }

    return (curr >= 0 && curr < static_cast<int>(nodes.size())) ? nodes[curr].value : 0.0;
}

RandomForestPredictor::RandomForestPredictor() {
    // Attempt standard search paths for model.json
    const std::vector<std::string> candidatePaths = {
        "ml/model.json",
        "../ml/model.json",
        "../../ml/model.json",
        "C:/SY/FODS CP FINAL/ml/model.json"
    };

    bool found = false;
    for (const auto& path : candidatePaths) {
        if (loadFromJsonFile(path)) {
            found = true;
            break;
        }
    }

    if (!found) {
        loadEmbeddedFallback();
    }
}

RandomForestPredictor::RandomForestPredictor(const std::string& jsonFilePath) {
    if (!loadFromJsonFile(jsonFilePath)) {
        loadEmbeddedFallback();
    }
}

// Helper: parse comma-separated integer array enclosed in '[' and ']'
static std::vector<int> parseIntArray(const std::string& json, size_t openBracket) {
    std::vector<int> result;
    size_t closeBracket = json.find(']', openBracket);
    if (closeBracket == std::string::npos) return result;

    std::string content = json.substr(openBracket + 1, closeBracket - openBracket - 1);
    std::stringstream ss(content);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        size_t first = token.find_first_not_of(" \t\r\n");
        size_t last = token.find_last_not_of(" \t\r\n");
        if (first != std::string::npos && last != std::string::npos) {
            result.push_back(std::stoi(token.substr(first, last - first + 1)));
        }
    }
    return result;
}

// Helper: parse comma-separated double array enclosed in '[' and ']'
static std::vector<double> parseDoubleArray(const std::string& json, size_t openBracket) {
    std::vector<double> result;
    size_t closeBracket = json.find(']', openBracket);
    if (closeBracket == std::string::npos) return result;

    std::string content = json.substr(openBracket + 1, closeBracket - openBracket - 1);
    std::stringstream ss(content);
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t first = token.find_first_not_of(" \t\r\n");
        size_t last = token.find_last_not_of(" \t\r\n");
        if (first != std::string::npos && last != std::string::npos) {
            result.push_back(std::stod(token.substr(first, last - first + 1)));
        }
    }
    return result;
}

bool RandomForestPredictor::loadFromJsonString(const std::string& jsonContent) {
    if (jsonContent.empty()) {
        return false;
    }

    std::vector<DecisionTree> parsedTrees;
    size_t pos = 0;

    while (true) {
        size_t treeTag = jsonContent.find("\"tree_index\"", pos);
        if (treeTag == std::string::npos) {
            break;
        }

        // Find feature array
        size_t featKey = jsonContent.find("\"feature\"", treeTag);
        if (featKey == std::string::npos) break;
        size_t featOpen = jsonContent.find('[', featKey);
        auto features = parseIntArray(jsonContent, featOpen);

        // Find threshold array
        size_t threshKey = jsonContent.find("\"threshold\"", featOpen);
        if (threshKey == std::string::npos) break;
        size_t threshOpen = jsonContent.find('[', threshKey);
        auto thresholds = parseDoubleArray(jsonContent, threshOpen);

        // Find children_left array
        size_t leftKey = jsonContent.find("\"children_left\"", threshOpen);
        if (leftKey == std::string::npos) break;
        size_t leftOpen = jsonContent.find('[', leftKey);
        auto childrenLeft = parseIntArray(jsonContent, leftOpen);

        // Find children_right array
        size_t rightKey = jsonContent.find("\"children_right\"", leftOpen);
        if (rightKey == std::string::npos) break;
        size_t rightOpen = jsonContent.find('[', rightKey);
        auto childrenRight = parseIntArray(jsonContent, rightOpen);

        // Find value array
        size_t valKey = jsonContent.find("\"value\"", rightOpen);
        if (valKey == std::string::npos) break;
        size_t valOpen = jsonContent.find('[', valKey);
        auto values = parseDoubleArray(jsonContent, valOpen);

        size_t nNodes = features.size();
        if (nNodes == 0 || thresholds.size() != nNodes ||
            childrenLeft.size() != nNodes || childrenRight.size() != nNodes ||
            values.size() != nNodes) {
            // Malformed tree node array sizes
            break;
        }

        DecisionTree tree;
        tree.treeIndex = static_cast<int>(parsedTrees.size());
        tree.nodeCount = static_cast<int>(nNodes);
        tree.nodes.resize(nNodes);

        for (size_t i = 0; i < nNodes; ++i) {
            tree.nodes[i].feature = features[i];
            tree.nodes[i].threshold = thresholds[i];
            tree.nodes[i].leftChild = childrenLeft[i];
            tree.nodes[i].rightChild = childrenRight[i];
            tree.nodes[i].value = values[i];
        }

        parsedTrees.push_back(std::move(tree));
        pos = valOpen;
    }

    if (parsedTrees.empty()) {
        return false;
    }

    trees = std::move(parsedTrees);
    loaded = true;
    return true;
}

bool RandomForestPredictor::loadFromJsonFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    bool ok = loadFromJsonString(buffer.str());
    if (ok) {
        modelSource = "RandomForestRegressor JSON (" + std::to_string(trees.size()) + " trees from " + filePath + ")";
    }
    return ok;
}

void RandomForestPredictor::loadEmbeddedFallback() {
    trees.clear();

    DecisionTree tree;
    tree.treeIndex = 0;
    tree.nodeCount = FIRST_TREE_NODE_COUNT;
    tree.nodes.resize(FIRST_TREE_NODE_COUNT);

    for (int i = 0; i < FIRST_TREE_NODE_COUNT; ++i) {
        tree.nodes[i].feature = FIRST_TREE_FEATURE[i];
        tree.nodes[i].threshold = FIRST_TREE_THRESHOLD[i];
        tree.nodes[i].leftChild = FIRST_TREE_CHILD_LEFT[i];
        tree.nodes[i].rightChild = FIRST_TREE_CHILD_RIGHT[i];
        tree.nodes[i].value = FIRST_TREE_VALUE[i];
    }

    trees.push_back(std::move(tree));
    loaded = true;
    modelSource = "Embedded Decision Tree Fallback (1 tree, compiled constants)";
}

double RandomForestPredictor::predict(const OrderPredictionFeatures& features) const {
    return predictRaw(features.toVector());
}

double RandomForestPredictor::predictRaw(const std::vector<double>& rawFeatures) const {
    // 1. Safe fallback check
    if (!loaded || trees.empty()) {
        return fallbackMean;
    }

    // 2. Validate input features size
    if (rawFeatures.size() < 5) {
        throw std::invalid_argument("Prediction requires at least 5 features: " +
                                    std::to_string(rawFeatures.size()) + " provided.");
    }

    // 3. Check for NaN or Inf
    for (size_t i = 0; i < 5; ++i) {
        if (std::isnan(rawFeatures[i]) || std::isinf(rawFeatures[i])) {
            throw std::invalid_argument("Feature vector contains invalid NaN or Infinity at index " +
                                        std::to_string(i));
        }
    }

    // 4. Ensemble averaging over all decision trees
    double sum = 0.0;
    for (const auto& tree : trees) {
        sum += tree.predict(rawFeatures);
    }

    double avgPrediction = sum / static_cast<double>(trees.size());

    // 5. Physical bounds: Food preparation cannot be negative or zero (minimum 1.0 min)
    return std::max(1.0, avgPrediction);
}

} // namespace ml
} // namespace canteen
