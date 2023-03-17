
# Prerequisites
*   Python == 3.8, numpy == 1.20.3, faiss

# Datasets

The tested datasets are available at https://www.cse.cuhk.edu.hk/systems/hash/gqr/datasets.html. 

1. Download the dataset GIST from ftp://ftp.irisa.fr/local/texmex/corpus/gist.tar.gz. The dataset is large. This step may take several minutes. The data format can be found in http://corpus-texmex.irisa.fr/.
    ```shell
    wget -O ./gist.tar.gz ftp://ftp.irisa.fr/local/texmex/corpus/gist.tar.gz --no-check-certificate
    ```

2. Unzip the dataset. 

    ```shell
    tar -zxvf ./gist.tar.gz -C ./
    ```

3. Preprocess the dataset with random orthogonal transformation. 

    ```shell
    python randomize.py
    ```

4. Generate the clustering of the dataset for IVF. 

    ```shell
    python ivf.py
    ```