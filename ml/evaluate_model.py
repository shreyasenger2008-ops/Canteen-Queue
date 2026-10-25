#!/usr/bin/env python3
"""
Evaluate trained model against baseline on held-out test split.
Outputs metrics.json with complete details.
"""

import os
import json
import joblib
import pandas as pd
import numpy as np
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score

FEATURE_COLS = [
    "food_category",
    "order_quantity",
    "preparation_complexity",
    "time_of_day_hour",
    "current_kitchen_workload"
]
TARGET_COL = "observed_prep_time_minutes"

def evaluate():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    model_path = os.path.join(script_dir, "model.joblib")
    test_path = os.path.join(script_dir, "test_split.csv")
    metrics_path = os.path.join(script_dir, "metrics.json")

    if not os.path.exists(model_path) or not os.path.exists(test_path):
        raise FileNotFoundError("Model or test split not found. Run train_model.py first.")

    model = joblib.load(model_path)
    test_df = pd.read_csv(test_path)

    X_test = test_df[FEATURE_COLS]
    y_test = test_df[TARGET_COL]

    # Baseline evaluation
    full_data = pd.read_csv(os.path.join(script_dir, "canteen_prep_data.csv"))
    train_mean = float(full_data[TARGET_COL].mean())
    baseline_preds = np.full(shape=len(y_test), fill_value=train_mean)
    baseline_mae = float(mean_absolute_error(y_test, baseline_preds))
    baseline_rmse = float(np.sqrt(mean_squared_error(y_test, baseline_preds)))

    # Model evaluation
    rf_preds = model.predict(X_test)
    rf_mae = float(mean_absolute_error(y_test, rf_preds))
    rf_rmse = float(np.sqrt(mean_squared_error(y_test, rf_preds)))
    rf_r2 = float(r2_score(y_test, rf_preds))

    # Feature importances
    feature_importances = {
        name: round(float(imp), 4)
        for name, imp in zip(FEATURE_COLS, model.feature_importances_)
    }

    metrics = {
        "dataset": {
            "source": "synthetic",
            "total_samples": len(full_data),
            "test_samples": len(test_df),
            "features": FEATURE_COLS,
            "target": TARGET_COL,
            "disclaimer": "Synthetic data generated for academic queue scheduling evaluation."
        },
        "baseline_model": {
            "type": "TrainingSetMeanPredictor",
            "mean_prep_time_minutes": round(train_mean, 4),
            "test_mae_minutes": round(baseline_mae, 4),
            "test_rmse_minutes": round(baseline_rmse, 4)
        },
        "random_forest_model": {
            "type": "RandomForestRegressor",
            "n_estimators": model.n_estimators,
            "max_depth": model.max_depth,
            "test_mae_minutes": round(rf_mae, 4),
            "test_rmse_minutes": round(rf_rmse, 4),
            "test_r2_score": round(rf_r2, 4),
            "feature_importances": feature_importances,
            "mae_reduction_percent": round(((baseline_mae - rf_mae) / baseline_mae) * 100.0, 2),
            "rmse_reduction_percent": round(((baseline_rmse - rf_rmse) / baseline_rmse) * 100.0, 2)
        }
    }

    with open(metrics_path, "w") as f:
        json.dump(metrics, f, indent=2)

    print(f"Metrics saved to {metrics_path}:")
    print(json.dumps(metrics, indent=2))

if __name__ == "__main__":
    evaluate()
