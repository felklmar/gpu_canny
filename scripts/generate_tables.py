raw_data = """
RTX 3060
Grid Config      | Blur    | Grad    | NMS     | D-Thr   | Hyst    | Iters  | Total  
------------------------------------------------------------------------------------
(32, 32) 32      | 3.706   | 7.918   | 9.547   | 2.324   | 56.830  | 39.7   | 80.325 
(32, 32) 256     | 3.653   | 7.783   | 9.395   | 1.065   | 59.472  | 42.9   | 81.368 
(32, 32) 1024    | 3.808   | 8.171   | 9.829   | 1.300   | 59.607  | 41.3   | 82.714 
(32, 8) 32       | 3.041   | 6.408   | 7.567   | 2.497   | 20.585  | 22.9   | 40.097 
(32, 8) 256      | 3.074   | 6.475   | 7.640   | 1.081   | 19.224  | 23.0   | 37.493 
(32, 8) 1024     | 3.073   | 6.474   | 7.638   | 1.345   | 19.237  | 22.8   | 37.768 
(8, 32) 32       | 3.476   | 6.075   | 7.356   | 2.516   | 23.089  | 26.0   | 42.511 
(8, 32) 256      | 3.478   | 6.076   | 7.359   | 1.085   | 21.461  | 26.0   | 39.459 
(8, 32) 1024     | 3.481   | 6.076   | 7.359   | 1.351   | 21.831  | 26.0   | 40.100 

RTX 4070
Grid Config      | Blur    | Grad    | NMS     | D-Thr   | Hyst    | Iters  | Total 
-------------------------------------------------------------------------------------
(32, 32) 32      | 3.612   | 3.325   | 4.327   | 1.671   | 23.719  | 43.8   | 36.654 
(32, 32) 256     | 3.606   | 3.326   | 4.342   | 1.274   | 22.958  | 43.2   | 35.506 
(32, 32) 1024    | 3.578   | 3.330   | 4.360   | 1.310   | 23.278  | 43.4   | 35.855 
(32, 8) 32       | 3.112   | 2.406   | 3.215   | 1.711   | 10.986  | 23.9   | 21.430 
(32, 8) 256      | 3.026   | 2.390   | 3.190   | 1.265   | 10.343  | 23.9   | 20.215 
(32, 8) 1024     | 3.005   | 2.384   | 3.189   | 1.302   | 10.372  | 23.8   | 20.252 
(8, 32) 32       | 3.104   | 2.258   | 3.138   | 1.715   | 11.707  | 26.0   | 21.922 
(8, 32) 256      | 3.185   | 2.259   | 3.118   | 1.304   | 11.343  | 26.0   | 21.209 
(8, 32) 1024     | 3.207   | 2.244   | 3.110   | 1.311   | 11.314  | 26.0   | 21.186 

RTX 4070Ti
Grid Config      | Blur    | Grad    | NMS     | D-Thr   | Hyst    | Iters  | Total  
-------------------------------------------------------------------------------------
(32, 32) 32      | 3.909   | 2.701   | 3.527   | 1.264   | 19.309  | 40.2   | 30.710 
(32, 32) 256     | 4.360   | 2.698   | 3.526   | 1.043   | 18.726  | 39.5   | 30.352 
(32, 32) 1024    | 3.897   | 2.697   | 3.523   | 1.044   | 18.234  | 38.9   | 29.395 
(32, 8) 32       | 3.559   | 2.026   | 2.728   | 1.259   | 10.845  | 24.0   | 20.417 
(32, 8) 256      | 3.659   | 2.025   | 2.726   | 1.027   | 10.384  | 23.9   | 19.820 
(32, 8) 1024     | 3.639   | 2.034   | 2.737   | 1.048   | 10.809  | 24.2   | 20.267 
(8, 32) 32       | 3.821   | 1.953   | 2.681   | 1.268   | 11.383  | 25.0   | 21.105 
(8, 32) 256      | 3.734   | 1.951   | 2.681   | 1.031   | 11.071  | 25.1   | 20.468 
(8, 32) 1024     | 3.699   | 1.947   | 2.676   | 1.037   | 10.632  | 25.0   | 19.991 

A100
Grid Config      | Blur    | Grad    | NMS     | D-Thr   | Hyst    | Iters  | Total  
-------------------------------------------------------------------------------------
(32, 32) 32      | 5.102   | 1.342   | 1.891   | 1.629   | 11.404  | 12.5   | 21.368 
(32, 32) 256     | 5.253   | 1.423   | 2.001   | 0.622   | 11.099  | 13.4   | 20.398 
(32, 32) 1024    | 5.041   | 1.398   | 1.966   | 0.601   | 10.786  | 13.2   | 19.791 
(32, 8) 32       | 4.910   | 1.311   | 1.831   | 1.693   | 14.117  | 24.9   | 23.862 
(32, 8) 256      | 4.959   | 1.322   | 1.844   | 0.593   | 12.921  | 24.9   | 21.638 
(32, 8) 1024     | 4.934   | 1.333   | 1.863   | 0.632   | 12.784  | 24.5   | 21.546 
(8, 32) 32       | 5.133   | 1.238   | 1.789   | 1.658   | 13.927  | 25.0   | 23.745 
(8, 32) 256      | 5.137   | 1.261   | 1.820   | 0.591   | 12.710  | 25.0   | 21.519 
(8, 32) 1024     | 5.297   | 1.277   | 1.842   | 0.603   | 13.305  | 25.0   | 22.324 
"""

gpus = ["RTX 3060", "RTX 4070", "RTX 4070 Ti", "A100"]
threads_list = [32, 256, 1024]

row_labels = {
    "Blur": "Gaussovské rozostření",
    "Grad": "Nalezení gradientů",
    "NMS": "Ztenčení hran",
    "D-Thr": "Dvojité prahování",
    "Hyst": "Hystereze",
    "Total": "\\textbf{Celkový čas}"
}

# Initialize data dictionary
data = {t: {g: {} for g in gpus} for t in threads_list}

# Parse the raw data
current_gpu = ""
for line in raw_data.strip().split('\n'):
    line = line.strip()
    if not line: continue
    
    if "RTX 3060" in line: current_gpu = "RTX 3060"
    elif "RTX 4070Ti" in line: current_gpu = "RTX 4070 Ti"
    elif "RTX 4070" in line: current_gpu = "RTX 4070"
    elif "A100" in line: current_gpu = "A100"
    elif line.startswith("(8, 32)"):
        parts = [p.strip() for p in line.split('|')]
        threads = int(parts[0].split()[-1])
        
        data[threads][current_gpu]["Blur"] = parts[1]
        data[threads][current_gpu]["Grad"] = parts[2]
        data[threads][current_gpu]["NMS"] = parts[3]
        data[threads][current_gpu]["D-Thr"] = parts[4]
        data[threads][current_gpu]["Hyst"] = parts[5]
        data[threads][current_gpu]["Total"] = parts[7]

# Function to generate tables for specific thread counts
def generate_thread_table(threads):
    latex = []
    latex.append("\\begin{table}[!htbp]")
    latex.append("\\centering")
    latex.append("\\renewcommand{\\arraystretch}{1.1}")
    latex.append("\\resizebox{\\textwidth}{!}{\\begin{tabular}{|l||r|r|r|r|}")
    latex.append("\\hline")
    latex.append("    & \\textbf{RTX 3060} & \\textbf{RTX 4070} & \\textbf{RTX 4070 Ti} & \\textbf{A100} \\\\ \\hline \\hline")
    
    for metric in ["Blur", "Grad", "NMS", "D-Thr", "Hyst"]:
        row = f"    {row_labels[metric]}"
        for gpu in gpus:
            row += f" & {data[threads][gpu][metric]}"
        if metric == "Hyst":
            row += " \\\\ \\hline \\hline"
        else:
            row += " \\\\ \\hline"
        latex.append(row)
        
    row = f"    {row_labels['Total']}"
    for gpu in gpus:
        row += f" & \\textbf{{{data[threads][gpu]['Total']}}}"
    row += " \\\\ \\hline"
    latex.append(row)
    
    latex.append("\\end{tabular}}")
    latex.append(f"\\caption{{Časy pro jednorozměrný blok s {threads} vlákny.}}")
    latex.append("\\end{table}\n")
    return "\n".join(latex)

# Function to generate the final summary table
def generate_summary_table():
    latex = []
    latex.append("\\begin{table}[!htbp]")
    latex.append("\\centering")
    latex.append("\\renewcommand{\\arraystretch}{1.3}")
    latex.append("\\resizebox{\\textwidth}{!}{\\begin{tabular}{|l||r|r|r|r|}")
    latex.append("\\hline")
    latex.append("    \\textbf{Počet vláken v bloku} & \\textbf{RTX 3060} & \\textbf{RTX 4070} & \\textbf{RTX 4070 Ti} & \\textbf{A100} \\\\ \\hline \\hline")
    
    for threads in threads_list:
        totals = {gpu: float(data[threads][gpu]['Total']) for gpu in gpus}
        fastest_gpu = min(totals, key=totals.get)
        
        row = f"    {threads:<4}"
        for gpu in gpus:
            val = data[threads][gpu]['Total']
            if gpu == fastest_gpu:
                row += f" & \\textit{{\\textbf{{{val}}}}}"
            else:
                row += f" & {val}"
        row += " \\\\ \\hline"
        latex.append(row)

    latex.append("\\end{tabular}}")
    latex.append("\\caption{Celkové časy pro různé konfigurace jednorozměrného bloku vláken.}")
    latex.append("\\end{table}")
    return "\n".join(latex)

# Output all tables
print("% --- JEDNOTLIVÉ TABULKY ---\n")
for t in threads_list:
    print(generate_thread_table(t))

print("% --- SOUHRNNÁ TABULKA ---\n")
print(generate_summary_table())