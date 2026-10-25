#!/usr/bin/env python3
"""
Train RandomForestRegressor for Canteen Food Preparation Time Prediction.
Evaluates against a baseline mean predictor on a held-out test set.
"""

import os
import joblib
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
import numpy as np

SEED = 42

FEATURE_COLS = [
    "food_category",
    "order_quantity",
    "preparation_complexity",
    "time_of_day_hour",
    "current_kitchen_workload"
]
TARGET_COL = "observed_prep_time_minutes"

def train():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_path = os.path.join(script_dir, "canteen_prep_data.csv")
    model_path = os.path.join(script_dir, "model.joblib")

    if not os.path.exists(data_path):
        raise FileNotFoundError(f"Dataset not found at {data_path}. Run generate_dataset.py first.")

    df = pd.read_csv(data_path)
    X = df[FEATURE_COLS]
    y = df[TARGET_COL]

    # Split into 80% train and 20% held-out test
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.20, random_state=SEED
    )

    # 1. Baseline Model: Predict training-set mean
    train_mean = float(y_train.mean())
    baseline_preds = np.full(shape=len(y_test), fill_value=train_mean)
    baseline_mae = mean_absolute_error(y_test, baseline_preds)
    baseline_rmse = np.sqrt(mean_squared_error(y_test, baseline_preds))

    # 2. Random Forest Regressor
    rf = RandomForestRegressor(
        n_estimators=20,
        max_depth=6,
        min_samples_split=4,
        min_samples_leaf=2,
        random_state=SEED,
        n_jobs=-1
    )
    rf.fit(X_train, y_train)

    rf_preds = rf.predict(X_test)
    rf_mae = mean_absolute_error(y_test, rf_preds)
    rf_rmse = np.sqrt(mean_squared_error(y_test, rf_preds))
    rf_r2 = r2_score(y_test, rf_preds)

    # Save trained model and test split for evaluation script
    joblib.dump(rf, model_path)

    # Save test set for reproducible evaluation
    test_df = X_test.copy()
    test_df[TARGET_COL] = y_test
    test_df.to_csv(os.path.join(script_dir, "test_split.csv"), index=False)

    print("=================================================================")
    print("                 MODEL TRAINING & EVALUATION                     ")
    print("=================================================================")
    print(f"Training samples: {len(X_train)} | Held-out test samples: {len(X_test)}")
    print(f"\n[Baseline Model (Training Set Mean = {train_mean:.2f} mins)]")
    print(f"  MAE : {baseline_mae:.4f} mins")
    print(f"  RMSE: {baseline_rmse:.4f} mins")

    print(f"\n[Trained RandomForestRegressor (20 trees, max_depth=6)]")
    print(f"  MAE : {rf_mae:.4f} mins")
    print(f"  RMSE: {rf_rmse:.4f} mins")
    print(f"  R^2 : {rf_r2:.4f}")

    mae_improvement = ((baseline_mae - rf_mae) / baseline_mae) * 100.0
    rmse_improvement = ((baseline_rmse - rf_rmse) / baseline_rmse) * 100.0
    print(f"\nImprovement over baseline:")
    print(f"  MAE reduction : {mae_improvement:.2f}%")
    print(f"  RMSE reduction: {rmse_improvement:.2f}%")
    print(f"Model successfully saved to {model_path}")
    print("=================================================================")

if __name__ == "__main__":
    train()
