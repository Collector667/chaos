from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = Path(__file__).resolve().parent.parent
save_dir = BASE_DIR / "data" / "lab2"
save_dir.mkdir(parents=True, exist_ok=True)

methods = [
    ("bifurcation_a_rk4.csv",      "rk4",      "r"),
    ("bifurcation_a_midPoint.csv", "midPoint", "g"),
    ("bifurcation_a_CD.csv",       "CD",       "b"),
]

variables = [("xmax", "Xmax", r"$x_{max}$"),
             ("ymax", "Ymax", r"$y_{max}$"),
             ("zmax", "Zmax", r"$z_{max}$")]

for fname, label, color in methods:
    df = pd.read_csv(BASE_DIR / "csv" / fname, na_values=[""])
    print(f"{label}: {len(df)} строк, a from {df['a'].min():.3f} to {df['a'].max():.3f}")

    for var, name, ylabel in variables:
        plt.figure(figsize=(10, 6))
        plt.plot(df["a"], df[var], c=color, marker=".", linestyle="none", markersize=0.5)
        plt.xlabel("Ось a")
        plt.ylabel(ylabel)
        plt.title(f"Бифуркационная диаграмма по {var[0]} ({label})")
        plt.tight_layout()
        plt.savefig(save_dir / f"bifurcation_{name}_{label}", dpi=150)
        plt.close()
        print(f"  -> bifurcation_{name}_{label}.png")

print(f"Графики сохранены в {save_dir}")
