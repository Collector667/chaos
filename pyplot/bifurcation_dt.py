from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = Path(__file__).resolve().parent.parent
save_dir = BASE_DIR / "data" / "lab2"
save_dir.mkdir(parents=True, exist_ok=True)

methods = [
    ("bifurcation_dt_rk4.csv",      "rk4"),
    ("bifurcation_dt_midPoint.csv", "midPoint"),
    ("bifurcation_dt_CD.csv",       "CD"),
]

variables = [("nmax_x", "Xmax", r"$x_{max}$"),
             ("nmax_y", "Ymax", r"$y_{max}$"),
             ("nmax_z", "Zmax", r"$z_{max}$")]

for fname, label in methods:
    df = pd.read_csv(BASE_DIR / "csv" / fname)
    print(f"{label}: {len(df)} строк")

    a_vals = np.sort(df["a"].unique())
    dt_vals = np.sort(df["dt"].unique())

    for col, name, varlabel in variables:
        pivot = df.pivot(index="dt", columns="a", values=col)
        nmax = pivot.reindex(index=dt_vals, columns=a_vals).to_numpy()

        plt.figure(figsize=(10, 6))
        im = plt.imshow(
            nmax,
            origin="lower",
            aspect="auto",
            extent=[a_vals[0], a_vals[-1], dt_vals[0], dt_vals[-1]],
            cmap="viridis",
            interpolation="nearest",
        )
        plt.xlabel("a (coefs[0])")
        plt.ylabel("dt (шаг интегрирования)")
        plt.title(f"Число локальных максимумов {varlabel} ({label})")
        cbar = plt.colorbar(im)
        cbar.set_label("Число локальных максимумов")
        plt.tight_layout()
        plt.savefig(save_dir / f"bifurcation_dt_{name}_{label}", dpi=150)
        plt.close()
        print(f"  -> bifurcation_dt_{name}_{label}.png (nmax {nmax.min()}..{nmax.max()})")

print(f"Графики сохранены в {save_dir}")
