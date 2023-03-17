# [SIGMOD 2023] High-Dimensional Approximate Nearest Neighbor Search: with Reliable and Efficient Distance Comparison Operations

> Jianyang Gao, Cheng Long

We note that we have included detailed comments of our core algorithms in 
* `./src/adsampling.h`
* `./src/hnswlib/hnswalg.h`
* `./src/ivf/ivf.h`

## Prerequisites

* Eigen == 3.4.0
    1. Download the Eigen library from https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz.
    2. Unzip it and move the `Eigen` folder to `./src/`.
    

---
## GIST Reproduction

The tested datasets are available at https://www.cse.cuhk.edu.hk/systems/hash/gqr/datasets.html. 

1. Download and preprocess the datasets. Detailed instructions can be found in `./data/README.md`.

2. Index the datasets. It could take several hours. 
    ```sh
    # Index IVF/IVF+/IVF++
    ./script/index_ivf.sh

    # Index HNSW/HNSW+/HNSW++
    ./script/index_hnsw.sh
    ```
3. Test the queries of the datasets. The results are generated in `./results/`. Detailed configurations can be found in `./script/README.md`.
    ```sh
    # Index IVF/IVF+/IVF++
    ./script/search_ivf.sh

    # Index HNSW/HNSW+/HNSW++
    ./script/search_hnsw.sh
    ```
