from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
from matplotlib.patches import Patch

BASE_DIR = Path(__file__).resolve().parent.parent
save_dir = BASE_DIR / "data" / "lab2"
save_dir.mkdir(parents=True, exist_ok=True)

methods = [
    ("bifurcation_2D_ab_rk4.csv",      "rk4"),
    ("bifurcation_2D_ab_midPoint.csv", "midPoint"),
    ("bifurcation_2D_ab_CD.csv",       "CD"),
]

variables = [("nuniq_x", "X", r"$x$"),
             ("nuniq_y", "Y", r"$y$"),
             ("nuniq_z", "Z", r"$z$")]

# Цвета вне viridis: серый — равновесие, красный — расхождение
EQ_COLOR = "#bdbdbd"
DIV_COLOR = "crimson"

for fname, label in methods:
    df = pd.read_csv(BASE_DIR / "csv" / fname)
    print(f"{label}: {len(df)} строк")

    a_vals = np.sort(df["p1"].unique())
    b_vals = np.sort(df["p2"].unique())
    extent = [a_vals[0], a_vals[-1], b_vals[0], b_vals[-1]]

    def grid_of(col):
        piv = df.pivot(index="p2", columns="p1", values=col)
        return piv.reindex(index=b_vals, columns=a_vals).to_numpy()

    div_grid = grid_of("diverged")
    div_mask = div_grid == 1

    for col, name, varlabel in variables:
        n_grid = grid_of(col)
        eq_mask = (~div_mask) & (n_grid == 0)
        pos_mask = (~div_mask) & (n_grid > 0)
        vmax = max(1, int(np.nanmax(n_grid)))   # своя шкала для каждой переменной

        fig, ax = plt.subplots(figsize=(10, 6))

        # Основной слой: периодичность / хаос
        base = np.ma.masked_where(~pos_mask, n_grid)
        cmap = plt.get_cmap("viridis").copy()
        cmap.set_bad((0, 0, 0, 0))
        im = ax.imshow(
            base,
            origin="lower",
            aspect="auto",
            extent=extent,
            cmap=cmap,
            vmin=1,
            vmax=vmax,
            interpolation="nearest",
        )

        # Наложения: равновесие и расхождение
        if eq_mask.any():
            ax.imshow(
                np.ma.masked_where(~eq_mask, np.ones_like(n_grid, dtype=float)),
                origin="lower", aspect="auto", extent=extent,
                cmap=ListedColormap([EQ_COLOR]), vmin=0, vmax=1, interpolation="nearest",
            )
        if div_mask.any():
            ax.imshow(
                np.ma.masked_where(~div_mask, np.ones_like(n_grid, dtype=float)),
                origin="lower", aspect="auto", extent=extent,
                cmap=ListedColormap([DIV_COLOR]), vmin=0, vmax=1, interpolation="nearest",
            )

        ax.set_xlabel("a (coefs[0])")
        ax.set_ylabel("b (coefs[1])")
        ax.set_title(f"Уникальные локальные максимумы {varlabel} (a, b), {label}")

        cbar = fig.colorbar(im, ax=ax)
        cbar.set_label("Число уникальных максимумов")

        handles = []
        if eq_mask.any():
            handles.append(Patch(facecolor=EQ_COLOR, edgecolor="none", label="равновесие (0)"))
        if div_mask.any():
            handles.append(Patch(facecolor=DIV_COLOR, edgecolor="none", label="расхождение"))
        if handles:
            ax.legend(handles=handles, loc="upper center", bbox_to_anchor=(0.5, -0.13),
                      ncol=len(handles), frameon=False)

        fig.tight_layout()
        fig.savefig(save_dir / f"bifurcation_2D_AB_{name}_{label}", dpi=150)
        plt.close(fig)
        print(f"  -> bifurcation_2D_AB_{name}_{label}.png "
              f"(nuniq {int(n_grid.min())}..{int(n_grid.max())}, eq={int(eq_mask.sum())}, div={int(div_mask.sum())})")

print(f"Графики сохранены в {save_dir}")
