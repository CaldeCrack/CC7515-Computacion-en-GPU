import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

csv_path = "cuda_benchmark.csv"
df = pd.read_csv(csv_path)
df["Cells/s (Millions)"] = df["Cells/s"] / 1_000_000_000
df["Length"] = df["Length"].apply(np.log2)
modes = ["CUDA", "CUDA Ifs", "CUDA 2D"]

for mode in modes:
    mode_df = df[df["Mode"] == mode]
    if mode_df.empty:
        continue

    plt.figure(figsize=(10, 6))
    for threads in sorted(mode_df["Threads"].unique()):
        subset = mode_df[mode_df["Threads"] == threads]
        if not subset.empty:
            plt.plot(
                subset["Length"],
                subset["Cells/s (Millions)"],
                label=f"{threads} tpb",
                marker="o",
            )

    plt.title(f"Rendimiento de ejecución para modo: {mode}")
    plt.xlabel("Tamaño de mundo (2^x)")
    plt.ylabel("Celdas/s (mil millones)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()
    plt.savefig(f'img/{mode.lower().replace(" ", "_")}_times.png')
