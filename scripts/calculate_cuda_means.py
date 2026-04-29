import re
import sys
from collections import defaultdict
import statistics

def calculate_cuda_means(filename):
    try:
        with open(filename, 'r') as file:
            content = file.read()
    except FileNotFoundError:
        print(f"Error: Could not find '{filename}'.")
        return

    # Split the text by the grid configuration pattern: e.g., "(8, 8) 32 |"
    # This separates the file into chunks belonging to each configuration
    parts = re.split(r"(\(\d+,\s*\d+\)\s*\d+)\s*\|", content)

    # Nested dictionary to hold all timings per configuration
    data = defaultdict(lambda: defaultdict(list))

    # Iterate through the split parts (configs and their respective log blocks)
    for i in range(1, len(parts), 2):
        config = parts[i].strip()
        block = parts[i+1]

        # Extract values using regex
        gb = re.search(r"Gaussian Blur took:\s*([\d.]+)", block)
        grad = re.search(r"Gradients took:\s*([\d.]+)", block)
        nms = re.search(r"Non-Maximum Suppression took:\s*([\d.]+)", block)
        dt = re.search(r"Double Thresholding took:\s*([\d.]+)", block)
        eh = re.search(r"Edge Hysteresis took:\s*([\d.]+)", block)
        iters = re.search(r"\((\d+)\s*iterations\)", block)
        total = re.search(r"Total time taken:\s*([\d.]+)", block)

        if gb: data[config]["Blur"].append(float(gb.group(1)))
        if grad: data[config]["Grad"].append(float(grad.group(1)))
        if nms: data[config]["NMS"].append(float(nms.group(1)))
        if dt: data[config]["D-Thr"].append(float(dt.group(1)))
        if eh: data[config]["Hyst"].append(float(eh.group(1)))
        if iters: data[config]["Iters"].append(float(iters.group(1)))
        if total: data[config]["Total"].append(float(total.group(1)))

    # Print the beautifully formatted markdown-style table
    print(f"{'Grid Config':<16} | {'Blur':<7} | {'Grad':<7} | {'NMS':<7} | {'D-Thr':<7} | {'Hyst':<7} | {'Iters':<6} | {'Total':<7} | {'Count'}")
    print("-" * 92)

    for config, metrics in data.items():
        blur = statistics.mean(metrics["Blur"]) if metrics["Blur"] else 0
        grad = statistics.mean(metrics["Grad"]) if metrics["Grad"] else 0
        nms = statistics.mean(metrics["NMS"]) if metrics["NMS"] else 0
        dt = statistics.mean(metrics["D-Thr"]) if metrics["D-Thr"] else 0
        hyst = statistics.mean(metrics["Hyst"]) if metrics["Hyst"] else 0
        iters = statistics.mean(metrics["Iters"]) if metrics["Iters"] else 0
        total = statistics.mean(metrics["Total"]) if metrics["Total"] else 0
        count = len(metrics["Total"]) if "Total" in metrics else 0

        print(f"{config:<16} | {blur:<7.3f} | {grad:<7.3f} | {nms:<7.3f} | {dt:<7.3f} | {hyst:<7.3f} | {iters:<6.1f} | {total:<7.3f} | {count}")

if __name__ == "__main__":
    calculate_cuda_means(sys.argv[1])