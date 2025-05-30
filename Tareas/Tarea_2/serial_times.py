import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv("serial_benchmark.csv")
df["Cells/s (Millions)"] = df["Cells/s"] / 1_000_000

modes = ["Serial", "Serial Ifs", "Serial 2D"]
colors = ["blue", "green", "red"]
linestyles = ["-", "--", "-."]
plt.figure(figsize=(10, 6))

for mode, color, style in zip(modes, colors, linestyles):
    mode_data = df[df["Mode"] == mode]
    if not mode_data.empty:
        plt.plot(
            mode_data["Length"],
            mode_data["Cells/s (Millions)"],
            label=mode,
            color=color,
            linestyle=style,
            marker="o",
        )

plt.title("Rendimiento ejecución serial")
plt.xlabel("Tamaño de mundo")
plt.ylabel("Celdas/s (millones)")
plt.legend()
plt.grid(True)
plt.tight_layout()

# Show or save the plot
plt.show()
plt.savefig("img/serial_times.png")
