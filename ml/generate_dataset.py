#!/usr/bin/env python3
"""
Synthetic Dataset Generator for Canteen Food Preparation Time Prediction.

DISCLAIMER:
This dataset is synthetic and modeled for academic evaluation and system integration.
It simulates typical college canteen preparation times across categories, item quantities,
kitchen workloads, time-of-day rush hours, and item preparation complexities.
It does not represent real-world observed empirical sensor data from an operating canteen.
"""

import numpy as np
import pandas as pd
import os

SEED = 42
NUM_SAMPLES = 2500

CATEGORY_MAP = {
    0: "Fast Food",
    1: "South Indian",
    2: "Meals & Chinese",
    3: "Beverages",
    4: "Bakery & Snacks"
}

BASE_PREP_BY_CATEGORY = {
    0: 6.5,   # Fast food (burgers, wraps)
    1: 5.5,   # South Indian (dosas, idlis)
    2: 9.5,   # Meals (fried rice, thali, biryani)
    3: 2.5,   # Beverages (chai, cold coffee)
    4: 2.0    # Bakery/Snacks (pastries, samosas)
}

def is_rush_hour(hour: float) -> bool:
    """Returns True if hour falls during canteen peak periods."""
    return (12.0 <= hour <= 14.0) or (17.0 <= hour <= 18.5)

def generate_dataset(num_samples: int = NUM_SAMPLES, seed: int = SEED) -> pd.DataFrame:
    np.random.seed(seed)

    # 1. Food category (uniform distribution across 5 categories)
    categories = np.random.choice([0, 1, 2, 3, 4], size=num_samples, p=[0.25, 0.20, 0.20, 0.20, 0.15])

    # 2. Order quantity (1 to 5 items, skewed towards 1-2 items)
    quantities = np.random.choice([1, 2, 3, 4, 5], size=num_samples, p=[0.50, 0.25, 0.15, 0.07, 0.03])

    # 3. Preparation complexity (1 to 5, correlated with category)
    complexities = []
    for cat in categories:
        if cat in (3, 4):      # Beverages, Bakery
            c = np.random.choice([1, 2], p=[0.7, 0.3])
        elif cat in (0, 1):    # Fast Food, South Indian
            c = np.random.choice([2, 3, 4], p=[0.2, 0.5, 0.3])
        else:                  # Meals & Chinese
            c = np.random.choice([3, 4, 5], p=[0.2, 0.5, 0.3])
        complexities.append(c)
    complexities = np.array(complexities)

    # 4. Time of day hour (between 8:00 AM and 8:00 PM, with peaks at 12-14 and 17-18.5)
    hours = np.random.uniform(8.0, 20.0, size=num_samples)

    # 5. Existing kitchen workload (minutes of work pending at counter, 0 to 30 mins)
    # Workload tends to be higher during rush hours
    workloads = []
    for h in hours:
        if is_rush_hour(h):
            wl = np.random.gamma(shape=3.0, scale=4.0) # Mean ~12 mins, up to 30
        else:
            wl = np.random.gamma(shape=1.5, scale=2.5) # Mean ~3.75 mins
        workloads.append(min(30.0, max(0.0, wl)))
    workloads = np.array(workloads)

    # 6. Target calculation: Observed preparation duration (in minutes)
    prep_times = []
    for i in range(num_samples):
        cat = categories[i]
        qty = quantities[i]
        comp = complexities[i]
        h = hours[i]
        wl = workloads[i]

        base = BASE_PREP_BY_CATEGORY[cat]
        # Complexity adjustment (+0.8 mins per complexity level above 1)
        comp_mod = 0.8 * (comp - 1)
        # Quantity scaling (economies of scale: additional items take ~35% of base)
        qty_mod = 0.35 * base * (qty - 1)
        # Workload delay (congestion effect)
        wl_delay = 0.12 * wl
        # Rush hour penalty (staff contention, multitasking)
        rush_penalty = 1.8 if is_rush_hour(h) else 0.0
        # Realistic observational noise
        noise = np.random.normal(loc=0.0, scale=0.6)

        total_time = base + comp_mod + qty_mod + wl_delay + rush_penalty + noise
        prep_times.append(max(1.0, round(float(total_time), 2)))

    df = pd.DataFrame({
        "food_category": categories,
        "order_quantity": quantities,
        "preparation_complexity": complexities,
        "time_of_day_hour": np.round(hours, 2),
        "current_kitchen_workload": np.round(workloads, 2),
        "observed_prep_time_minutes": prep_times
    })

    return df

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, "canteen_prep_data.csv")

    df = generate_dataset()
    df.to_csv(output_path, index=False)

    print(f"[DATASET GENERATED] Saved {len(df)} synthetic samples to {output_path}")
    print(df.head(10))
    print("\nSummary Statistics:")
    print(df.describe().T[["mean", "std", "min", "50%", "max"]])

if __name__ == "__main__":
    main()
