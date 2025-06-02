import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

csv_path = "cuda_benchmark.csv"
df = pd.read_csv(csv_path)

df["Cells/s (Millions)"] = df["Cells/s"] / 1_000_000_000
df["Length"] = df["Length"].apply(np.log2)
thread_config = {"CUDA": 256, "CUDA Ifs": 256, "CUDA 2D": 256}

plt.figure(figsize=(10, 6))
for mode, threads in thread_config.items():
    subset = df[(df["Mode"] == mode) & (df["Threads"] == threads)]
    if not subset.empty:
        plt.plot(
            subset["Length"],
            subset["Cells/s (Millions)"],
            label=f"{mode} ({threads} tpb)",
            marker="o",
        )
    else:
        print(f"Warning: No data found for mode '{mode}' with threads = {threads}")

plt.title("Comparación de ejecuciones para los distintos modos de CUDA")
plt.xlabel("Tamaño de mundo (2^x)")
plt.ylabel("Celdas/s (mil millones)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
plt.savefig("img/cuda_time_comparison.png")
