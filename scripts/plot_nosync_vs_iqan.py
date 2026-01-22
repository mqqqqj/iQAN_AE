import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter

plt.rcParams["font.family"] = "DejaVu Serif"
plt.rcParams["font.size"] = 18

plt.rcParams["axes.titlesize"] = 32
plt.rcParams["axes.labelsize"] = 30
plt.rcParams["xtick.labelsize"] = 18
plt.rcParams["ytick.labelsize"] = 18
plt.rcParams["legend.fontsize"] = 28

plt.rcParams["axes.grid"] = True
plt.rcParams["grid.alpha"] = 0.4
plt.rcParams["grid.linestyle"] = "--"
plt.rcParams["lines.linewidth"] = 3.0
plt.rcParams["lines.markersize"] = 10
plt.rcParams["axes.linewidth"] = 1.5

# === 2. 配置参数 ===
datasets = ["deep100m", "sift100m", "glove200", "webvid", "laion10m"]
datasets_title = ["DEEP100M", "SIFT100M", "Glove200", "WebVid2.5M", "LAION10M"]

algos_config = {
    'tuned_iqan':   {'label': 'iQAN (Tuned)',    'color': '#EE6677', 'marker': 'o', 'style': '-'},
    'nosync': {'label': 'No-Sync',         'color': '#CCBB44', 'marker': 's', 'style': '-.'}
}

algorithms = list(algos_config.keys())

def get_file_name(dataset, algo_name):
    return f"{dataset}_8t_{algo_name}.csv"

def plot_max_ndc_vs_recall():
    fig, axes = plt.subplots(nrows=1, ncols=5, figsize=(25, 5.5))
    plt.subplots_adjust(left=0.08, right=0.99, top=0.80, bottom=0.18, wspace=0.25)

    valid_handles = []
    valid_labels = []

    for i, ds_name in enumerate(datasets):
        ax = axes[i]
        
        for algo in algorithms:
            file_name = get_file_name(ds_name, algo)
            config = algos_config[algo]
            
            try:
                df = pd.read_csv(file_name)
                df.columns = df.columns.str.strip() 
                
                df_filtered = df[(df['recall'] >= 0.95) & (df['recall'] <= 0.991)].sort_values(by='recall')
                
                if df_filtered.empty:
                    continue
                
                line = ax.plot(df_filtered['recall'], df_filtered['max_dist_comps'],
                        label=config['label'],
                        color=config['color'],
                        marker=config['marker'],
                        linestyle=config['style'],
                        markerfacecolor='white',
                        markeredgewidth=2,
                        zorder=3)
                
                if i == 0:
                    valid_handles.append(line[0])
                    valid_labels.append(config['label'])
                
            except FileNotFoundError:
                print(f"❌ 找不到: {file_name}")
            except KeyError:
                pass

        ax.set_title(datasets_title[i], pad=12)
        
        ax.set_xlim(0.95, 0.991) 
        ax.set_xlabel('Recall@100')
        ax.xaxis.set_major_formatter(FormatStrFormatter('%.2f'))

        ax.grid(True, which='major', alpha=0.4, zorder=0)

        if i == 0:
            ax.set_ylabel('Max-t NDC')
        else:
            pass 

    if valid_handles:
        fig.legend(valid_handles, valid_labels, 
                   loc='upper center', 
                   bbox_to_anchor=(0.5, 1.02), 
                   ncol=len(algorithms),       
                   frameon=False,              
                   columnspacing=2.5,          
                   handletextpad=0.5)

    output_file = 'nosync_vs_iqan.pdf'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')

if __name__ == "__main__":
    plot_max_ndc_vs_recall()