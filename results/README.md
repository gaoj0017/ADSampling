# Results
The results of the experiments are stored in the files with their names formatted in `{dataset}_{algorithm}_{0/1/2}.log`.
*   0 - AKNN
*   **1 - AKNN++**
*   2 - AKNN+

In the files, each line contains 4 numbers. They represent 
1. search parameter ($N_{ef}$ for HNSW and $N_{probe}$ for IVF)
2. recall (%)
3. average running time per query (us)
4. total evaluated dimensionality
