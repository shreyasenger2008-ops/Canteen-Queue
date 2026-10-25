# Machine Learning Model Specification & Integration Report

This document specifies the machine learning pipeline, dataset assumptions, training methodology, evaluation metrics, and C++ inference integration for predicting food preparation duration in the **AI-Based College Canteen Queue Optimization System**.

---

## 1. Objective and Model Selection

### Problem Statement
In high-volume college canteens, static menu preparation times fail to reflect dynamic kitchen conditions:
- Orders for multiple items exhibit non-linear preparation scaling.
- Cooking durations vary significantly across culinary categories (e.g. freshly cooked Dosas vs premade Pastries).
- Kitchen congestion and burner contention increase preparation times when queues are full.
- Meal rush hours (lunch and evening tea) introduce multitasking overhead.

### Model Selected
A **`RandomForestRegressor`** from `scikit-learn` was selected and trained:
- **Number of Estimators**: 20 Decision Trees
- **Maximum Depth**: 6 levels per tree
- **Minimum Samples Split**: 4
- **Minimum Samples Leaf**: 2
- **Random State**: 42 (fully reproducible)

### Rationale
1. **Handling Non-Linear Interactions**: Random Forests naturally capture non-linear interactions between categories, order batching economies of scale, kitchen workloads, and rush-hour delays without requiring manual polynomial feature engineering.
2. **Robustness & Generalization**: Averaging predictions across decorrelated trees mitigates variance and resists overfitting on noisy observations.
3. **Deterministic & Verifiable C++ Portability**: A tree ensemble consists entirely of piecewise-constant axis-aligned splits (`if (features[i] <= threshold)`). This enables an exact, zero-dependency C++ implementation that produces mathematically identical predictions to Python to within floating-point machine epsilon ($< 10^{-6}$), avoiding the setup complexity and platform instability of external ONNX Runtime binaries.

---

## 2. Dataset Generation and Features

### Synthetic Data Notice
> [!IMPORTANT]
> The dataset `ml/canteen_prep_data.csv` was programmatically generated using a physics-modeled synthetic distribution (`ml/generate_dataset.py`, $N = 2,500$ samples, random seed 42). It models realistic cafeteria operational dynamics for academic queue optimization research. It does **not** represent empirical sensor data collected from an operating physical kitchen.

### Input Features & Target Variable

| Feature Name | Type | Domain / Values | Description |
|---|---|---|---|
| `food_category` | Categorical (Integer) | 0 to 4 | 0: Fast Food (Burgers, Rolls)<br>1: South Indian (Dosas, Idlis)<br>2: Meals & Chinese (Fried Rice, Thali)<br>3: Beverages (Cold Coffee, Chai)<br>4: Bakery & Snacks (Pastries, Samosas) |
| `order_quantity` | Discrete | 1 to 5 | Total count of items in the order. |
| `preparation_complexity` | Ordinal | 1 to 5 | Inherent cooking complexity (1: simple/premade, 5: elaborate/made-to-order). |
| `time_of_day_hour` | Continuous | 8.00 to 19.99 | Decimal hour of order placement (e.g., 12.50 = 12:30 PM). |
| `current_kitchen_workload` | Continuous | 0.00 to 30.00 | Total pending queue preparation minutes at the candidate counter. |
| **`observed_prep_time_minutes`** | **Target (Continuous)** | **1.00 to 28.03** | **Actual observed food preparation duration in minutes.** |

### Data Generation Modeling Rules
- **Base Duration**: Fast Food ($\approx 6.5$m), South Indian ($\approx 5.5$m), Meals ($\approx 9.5$m), Beverages ($\approx 2.5$m), Bakery/Snacks ($\approx 2.0$m).
- **Quantity Scaling**: Sub-linear economies of scale ($\text{base} \times (1 + 0.35 \times (\text{qty} - 1))$).
- **Kitchen Workload Congestion**: $+0.12 \times \text{workload}$ minutes.
- **Peak Rush Hour**: Orders placed between 12:00–14:00 (lunch) or 17:00–18:30 (snacks) incur a $+1.8$ minute staff multitasking delay.
- **Observational Noise**: Gaussian perturbation $\epsilon \sim \mathcal{N}(0, 0.6)$.

---

## 3. Training Procedure & Evaluation Results

### Experimental Setup
- **Dataset Size**: 2,500 samples
- **Data Partition**: 80% Training Set (2,000 samples), 20% Held-Out Test Set (500 samples)
- **Train/Test Random Seed**: 42

### Baseline Comparison
A standard academic baseline was implemented: **Training-Set Mean Predictor**, which predicts the mean preparation time of the training set ($\bar{y}_{\text{train}} = 9.7348$ minutes) for all test samples.

### Measured Held-Out Evaluation Metrics

| Metric | Baseline (Mean Predictor) | Trained RandomForestRegressor | Absolute Improvement | Relative Error Reduction |
|---|---|---|---|---|
| **Mean Absolute Error (MAE)** | **4.5003 mins** | **0.8139 mins** | **-3.6864 mins** | **81.91% reduction** |
| **Root Mean Squared Error (RMSE)** | **5.5477 mins** | **1.0411 mins** | **-4.5066 mins** | **81.23% reduction** |
| **Coefficient of Determination ($R^2$)** | 0.0000 | **0.9648** | +0.9648 | Explains 96.48% of variance |

### Feature Importance Distribution
Calculated via Gini impurity / variance reduction across all 20 estimators:
1. `food_category`: **73.95%**
2. `order_quantity`: **18.68%**
3. `current_kitchen_workload`: **6.84%**
4. `preparation_complexity`: **0.34%**
5. `time_of_day_hour`: **0.19%**

---

## 4. C++ Model Inference Architecture

### Pipeline Workflow
```
[ Python Training Pipeline ]
    generate_dataset.py -> canteen_prep_data.csv
    train_model.py      -> model.joblib
    evaluate_model.py   -> metrics.json
    export_model.py     -> model.json + EmbeddedModelData.hpp + test_predictions.json
                               │
                               ▼
[ C++ Inference Engine (canteen::ml) ]
    RandomForestPredictor::loadFromJsonFile("ml/model.json")
    Parses 20 Decision Trees -> std::vector<DecisionTree>
                               │
                               ▼
[ Prediction Interface (IPrepTimePredictor) ]
    predict(OrderPredictionFeatures) -> double (minutes)
                               │
                               ▼
[ DSA Queue Scheduler (CounterManager) ]
    For each candidate counter C:
      ProjectedCompletion(C) = Workload(C) + PredictedPrepTime(Order, C)
    Select counter C* minimizing ProjectedCompletion
    Enqueue order into C*'s custom FIFO queue
```

### Tree Traversal Implementation
Each decision tree stores parallel flattened vectors for feature indices, thresholds, left child indices, right child indices, and leaf values. Traversal is an iterative loop running in $O(\text{depth})$:
```cpp
double DecisionTree::predict(const std::vector<double>& features) const {
    if (nodes.empty()) return 0.0;
    int curr = 0;
    while (curr >= 0 && curr < static_cast<int>(nodes.size())) {
        const auto& node = nodes[curr];
        if (node.feature == -2) return node.value; // Leaf node
        int featIdx = node.feature;
        if (featIdx < 0 || featIdx >= static_cast<int>(features.size())) return node.value;
        curr = (features[featIdx] <= node.threshold) ? node.leftChild : node.rightChild;
    }
    return (curr >= 0 && curr < static_cast<int>(nodes.size())) ? nodes[curr].value : 0.0;
}
```
Ensemble prediction computes the arithmetic mean:
$$\hat{y}(\mathbf{x}) = \frac{1}{M} \sum_{m=1}^M \text{Tree}_m(\mathbf{x})$$

### Cross-Language Precision Verification
To ensure the C++ port is exact, `ml/export_model.py` sampled held-out test points and recorded Python's predictions to 6 decimal places in `ml/test_predictions.json`.

C++ unit test `ML_CompareWithPythonHeldOutPredictions` asserted agreement:
- **Sample 0**: Python: `3.832597` | C++: `3.832597` | Difference: $5.0 \times 10^{-8}$
- **Sample 1**: Python: `10.019401` | C++: `10.019401` | Difference: $2.5 \times 10^{-7}$
- **Sample 2**: Python: `12.011613` | C++: `12.011613` | Difference: $2.5 \times 10^{-7}$
- **Sample 3**: Python: `25.135599` | C++: `25.135599` | Difference: $3.5 \times 10^{-15}$
- **Sample 4**: Python: `7.531225` | C++: `7.531225` | Difference: $3.0 \times 10^{-7}$

---

## 5. Fault Tolerance & Fallback Strategy

The application is engineered to guarantee operational continuity under all failure modes:

1. **Missing Model File (`model.json` not found)**:
   - If the JSON model file is missing (e.g. executable launched from another working directory), `RandomForestPredictor` catches the missing file and automatically initializes `EmbeddedModelData.hpp` (a compiled in-source decision tree fallback) or uses the training set mean ($9.73$m).
   - The application does not crash.
2. **Invalid or Malformed Inputs**:
   - `predictRaw()` verifies that the feature vector has at least 5 elements.
   - Inspects all features for `NaN` and `Infinity` using `std::isnan` and `std::isinf`, throwing `std::invalid_argument`.
   - The DSA scheduler catches this and uses the order's static menu prep time sum as a fallback.
3. **Physical Bound Clamping**:
   - Every prediction is clamped to a minimum of $1.0$ minute, ensuring no counter can be assigned a zero or negative preparation duration.

---

## 6. Known Limitations

- **Synthetic Training Domain**: Accuracy gains are measured against the synthetic generation rules. Deploying to a physical canteen would require logging real order timestamps and retraining.
- **Counter Multi-Item Specialization**: The current scheduler models general kitchen queues. Future iterations could assign specific items in a multi-item order to separate specialized preparation counters.
