from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

BASE_DIR = Path(__file__).resolve().parent.parent
save_dir = BASE_DIR / "data" / "lab3"
save_dir.mkdir(parents=True, exist_ok=True)

# Отдельный график на каждый интегратор (файл = lyapunov_a_<label>.csv).
METHODS = ["rk4", "midPoint", "CD"]

for label in METHODS:
    fname = f"lyapunov_a_{label}.csv"
    df = pd.read_csv(BASE_DIR / "csv" / fname, na_values=["nan", "NaN", ""])
    div = df["diverged"] == 1

    print(f"{label}: {len(df)} строк, lambda {df['lambda'].min():+.4f}..{df['lambda'].max():+.4f}, "
          f"div={int(div.sum())}")

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.axhline(0.0, color="black", lw=0.8, ls="--", zorder=1)
    ax.plot(df.loc[~div, "p1"], df.loc[~div, "lambda"],
            color="tab:blue", lw=1.2, zorder=2)
    if div.any():
        ax.plot(df.loc[div, "p1"], np.full(int(div.sum()), df["lambda"].min()),
                "x", color="crimson", ms=6, label="расхождение", zorder=3)
        ax.legend(frameon=False)

    ax.set_xlabel("a (coefs[0])")
    ax.set_ylabel(r"$\lambda_{max}$")
    ax.set_title(f"Старший показатель Ляпунова, {label}")
    ax.grid(alpha=0.3)

    fig.tight_layout()
    fig.savefig(save_dir / f"lyapunov_a_{label}.png", dpi=150)
    plt.close(fig)
    print(f"  -> {save_dir / f'lyapunov_a_{label}.png'}")
