# No-Sync vs iQAN Performance Evaluation

This repository is dedicated to reproducing the comparative results of the No-Sync strategy and iQAN, specifically focusing on the max-t NDC metric.

To capture the optimal performance of iQAN, we conducted an extensive grid search over its hyperparameters based on the evaluation scripts provided in the iQAN repository. For each dataset, we evaluated 25 different parameter configurations and selected the best-performing result.

## Getting Started

Please refer to the detailed instructions in [README_iQAN.md](README_iQAN.md) to set up the environment and build the project.

Once the environment is ready, you can generate the experimental results for both strategies by running the provided scripts:

```
#For Best Tuned iQAN
bash scripts/tuned_iqan.sh

#For No-Sync
bash scripts/no_sync.sh
```

After the scripts have finished executing, you can compare the max-t NDC outputs by running the script below to verify the findings:
```
python scripts/plot_nosync_vs_iqan.py
```
 