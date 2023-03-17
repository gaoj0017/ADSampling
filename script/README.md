# Configuration
* Search parameters can be modified in `../src/search_hnsw.cpp` and `../src/search_ivf.cpp`.
    * find `vector<int> nprobes` and `vector<size_t> efs`
* $\epsilon_0$ and $\Delta_d$ can be modified in `./src/search_ivf.sh` and `./src/search_hnsw.sh`, e.g.,
    * adding `-e 2.5` to modify the $\epsilon_0$ to 2.5.
    * adding `-p 1` to modify the $\Delta_d$ to 1.
