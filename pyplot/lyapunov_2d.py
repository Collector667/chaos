from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
from matplotlib.patches import Patch

BASE_DIR = Path(__file__).resolve().parent.parent
save_dir = BASE_DIR / "data" / "lab3"
save_dir.mkdir(parents=True, exist_ok=True)

DIV_COLOR = "crimson"

methods = [("rk4", "rk4"), ("midPoint", "midPoint"), ("CD", "CD")]


def plot_grid(fname, label, out_name, xlabel, ylabel, title):
    df = pd.read_csv(BASE_DIR / "csv" / fname)
    x_vals = np.sort(df["p1"].unique())
    y_vals = np.sort(df["p2"].unique())
    extent = [x_vals[0], x_vals[-1], y_vals[0], y_vals[-1]]

    def grid_of(col):
        piv = df.pivot(index="p2", columns="p1", values=col)
        return piv.reindex(index=y_vals, columns=x_vals).to_numpy()

    lam = grid_of("lambda")
    div_mask = grid_of("diverged") == 1

    vmax = float(np.nanmax(lam[~div_mask])) if (~div_mask).any() else 1.0
    vmax = max(vmax, 1e-3)

    fig, ax = plt.subplots(figsize=(10, 6))

    base = np.ma.masked_where(div_mask, lam)
    cmap = plt.get_cmap("viridis").copy()
    cmap.set_bad((0, 0, 0, 0))
    im = ax.imshow(
        base,
        origin="lower",
        aspect="auto",
        extent=extent,
        cmap=cmap,
        vmin=0.0,
        vmax=vmax,
        interpolation="nearest",
    )

    if div_mask.any():
        ax.imshow(
            np.ma.masked_where(~div_mask, np.ones_like(lam, dtype=float)),
            origin="lower", aspect="auto", extent=extent,
            cmap=ListedColormap([DIV_COLOR]), vmin=0, vmax=1, interpolation="nearest",
        )

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(f"{title}, {label}")

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label(r"$\lambda_{max}$")

    if div_mask.any():
        ax.legend(handles=[Patch(facecolor=DIV_COLOR, edgecolor="none", label="расхождение")],
                  loc="upper center", bbox_to_anchor=(0.5, -0.13), ncol=1, frameon=False)

    fig.tight_layout()
    fig.savefig(save_dir / f"{out_name}.png", dpi=150)
    plt.close(fig)

    print(f"{label}: {len(df)} строк, div={int(div_mask.sum())}, "
          f"lambda {np.nanmin(lam):+.4f}..{np.nanmax(lam):+.4f} -> {out_name}.png")


for label, suffix in methods:
    plot_grid(
        f"lyapunov_2D_ab_{suffix}.csv", label,
        f"lyapunov_2D_ab_{suffix}",
        "a (coefs[0])", "b (coefs[1])",
        r"Старший показатель Ляпунова $\lambda_{max}$ (a, b)",
    )

for label, suffix in methods:
    plot_grid(
        f"lyapunov_dt_{suffix}.csv", label,
        f"lyapunov_dt_{suffix}",
        "a (coefs[0])", "dt (шаг интегрирования)",
        r"Старший показатель Ляпунова $\lambda_{max}$ (a, dt)",
    )
