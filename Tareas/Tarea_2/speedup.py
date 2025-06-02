import matplotlib.pyplot as plt
import pandas as pd

threads_cuda = {
    "CUDA": 256,
    "CUDA Ifs": 256,
    "CUDA 2D": 256,
}
threads_opencl = {"OpenCL": 128, "OpenCL Ifs": 128, "OpenCL 2D": 128}

serial_df = pd.read_csv("serial_benchmark.csv")
cuda_df = pd.read_csv("cuda_benchmark.csv")
opencl_df = pd.read_csv("opencl_benchmark.csv")

serial_df["Time[μs]"] = serial_df["Time[s]"] * 1_000_000
lengths = serial_df["Length"].unique()


def get_speedup_df(df, mode_name, threads_dict):
    speedup_data = []
    for variant, threads in threads_dict.items():
        mode_data = df[(df["Mode"] == variant) & (df["Threads"] == threads)]

        for length in lengths:
            serial_time = serial_df[serial_df["Length"] == length]["Time[μs]"].values
            other_time = mode_data[mode_data["Length"] == length]["Time[μs]"].values

            speedup = serial_time[0] / other_time[0]
            speedup_data.append(
                {
                    "Length": length,
                    "Speedup": speedup,
                    "Mode": variant,
                    "Backend": mode_name,
                }
            )
    return pd.DataFrame(speedup_data)


cuda_speedup_df = get_speedup_df(cuda_df, "CUDA", threads_cuda)
opencl_speedup_df = get_speedup_df(opencl_df, "OpenCL", threads_opencl)

combined_df = pd.concat([cuda_speedup_df, opencl_speedup_df], ignore_index=True)

plt.figure(figsize=(12, 7))
styles = {
    "CUDA": {"color": "blue", "linestyle": "-"},
    "CUDA Ifs": {"color": "blue", "linestyle": "-."},
    "CUDA 2D": {"color": "blue", "linestyle": ":"},
    "OpenCL": {"color": "green", "linestyle": "-"},
    "OpenCL Ifs": {"color": "green", "linestyle": "-."},
    "OpenCL 2D": {"color": "green", "linestyle": ":"},
}

for backend in ["CUDA", "OpenCL"]:
    backend_df = combined_df[combined_df["Backend"] == backend]
    for mode in backend_df["Mode"].unique():
        data = backend_df[backend_df["Mode"] == mode]
        label = f"{mode}"
        plt.plot(
            data["Length"],
            data["Speedup"],
            label=label,
            color=styles[backend]["color"],
            linestyle=styles[mode]["linestyle"],
            marker="o",
        )

plt.title("Speedup sobre ejecución serial")
plt.xlabel("Tamaño de mundo")
plt.ylabel("Speedup")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
plt.savefig("img/speedup.png")
