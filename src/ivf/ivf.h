/*
We highlight the function search which are closely related to our proposed algorithms.
We have included detailed comments in these functions. 

We explain the important variables for the enhanced IVF as follows.
1. d - It represents the number of initial dimensions.
    * For IVF  , d = D. 
    * For IVF+ , d = 0. 
    * For IVF++, d = delta_d. 
2. L1_data - The array to store the first d dimensions of a vector.
3. res_data - The array to store the remaining dimensions of a vector.

*/
#include <limits>
#include <queue>
#include <vector>
#include <algorithm>
#include <map>

#include "adsampling.h"
#include "matrix.h"
#include "utils.h"


class IVF{
public:
    size_t N;
    size_t D;
    size_t C;
    size_t d; // the dimensionality of first a few dimensions

    float* L1_data;
    float* res_data;
    float* centroids;

    size_t* start;
    size_t* len;
    size_t* id;

    IVF();
    IVF(const Matrix<float> &X, const Matrix<float> &_centroids, int adaptive=0);
    ~IVF();

    ResultHeap search(float* query, size_t k, size_t nprobe, float distK = std::numeric_limits<float>::max()) const;
    void save(char* filename);
    void load(char* filename);

};

IVF::IVF(){
    N = D = C = d = 0;
    start = len = id = NULL;
    L1_data = res_data = centroids = NULL;
}

IVF::IVF(const Matrix<float> &X, const Matrix<float> &_centroids, int adaptive){
    
    N = X.n;
    D = X.d;
    C = _centroids.n;
    
    assert(D > 32);
    start = new size_t [C];
    len   = new size_t [C];
    id    = new size_t [N];

    std::vector<size_t> * temp = new std::vector<size_t> [C];
    
    for(int i=0;i<X.n;i++){
        int belong = 0;
        float dist_min = X.dist(i, _centroids, 0);
        for(int j=1;j<C;j++){
            float dist = X.dist(i, _centroids, j);
            if(dist < dist_min){
                dist_min = dist;
                belong = j;
            }
        }
        if(i % 50000 == 0){
            std::cerr << "Processing - " << i << " / " << X.n  << std::endl;
        }
        temp[belong].push_back(i);
    }
    std::cerr << "Cluster Generated!" << std::endl;

    size_t sum = 0;
    for(int i=0;i<C;i++){
        len[i] = temp[i].size();
        start[i] = sum;
        sum += len[i];
        for(int j=0;j<len[i];j++){
            id[start[i] + j] = temp[i][j];
        }
    }

    if(adaptive == 1)d = 32;        // IVF++ - optimize cache (d = 32 by default)
    else if(adaptive == 0) d = D;   // IVF   - plain scan
    else d = 0;                     // IVF+  - plain ADSampling        

    L1_data   = new float [N * d + 1];
    res_data  = new float [N * (D - d) + 1];
    centroids = new float [C * D];
    
    for(int i=0;i<N;i++){
        int x = id[i];
        for(int j=0;j<D;j++){
            if(j < d) L1_data[i * d + j] = X.data[x * D + j];
            else res_data[i * (D-d) + j - d] = X.data[x * D + j];
        }
    }

    std::memcpy(centroids, _centroids.data, C * D * sizeof(float));
    delete [] temp;

}

IVF::~IVF(){
    if(id != NULL)delete [] id;
    if(len != NULL)delete [] len;
    if(start != NULL)delete [] start;
    if(L1_data != NULL)delete [] L1_data;
    if(res_data != NULL)delete [] res_data;
    if(centroids != NULL)delete [] centroids;
}

ResultHeap IVF::search(float* query, size_t k, size_t nprobe, float distK) const{
    // the default value of distK is +inf 
    Result* centroid_dist = new Result [C];

    // Find out the closest N_{probe} centroids to the query vector.
    for(int i=0;i<C;i++){
#ifdef COUNT_DIST_TIME
        StopW stopw = StopW();
#endif
        centroid_dist[i].first = sqr_dist(query, centroids+i*D, D);
#ifdef COUNT_DIST_TIME
        adsampling::distance_time += stopw.getElapsedTimeMicro();
#endif               
        centroid_dist[i].second = i;
    }

    // Find out the closest N_{probe} centroids to the query vector.
    std::partial_sort(centroid_dist, centroid_dist + nprobe, centroid_dist + C);
    
    size_t ncan = 0;
    for(int i=0;i<nprobe;i++)
        ncan += len[centroid_dist[i].second];
    if(d == D)adsampling::tot_dimension += 1ll * ncan * D;
    float * dist = new float [ncan];
    Result * candidates = new Result [ncan];
    int * obj= new int [ncan];

    // Scan a few initial dimensions and store the distances.
    // For IVF (i.e., apply FDScanning), it should be D. 
    // For IVF+ (i.e., apply ADSampling without optimizing data layout), it should be 0.
    // For IVF++ (i.e., apply ADSampling with optimizing data layout), it should be delta_d (i.e., 32). 
    int cur = 0;
    for(int i=0;i<nprobe;i++){
        int cluster_id = centroid_dist[i].second;
        for(int j=0;j<len[cluster_id];j++){
            size_t can = start[cluster_id] + j;
#ifdef COUNT_DIST_TIME
            StopW stopw = StopW();
#endif
            float tmp_dist = sqr_dist(query, L1_data + can * d, d);
#ifdef COUNT_DIST_TIME
            adsampling::distance_time += stopw.getElapsedTimeMicro();
#endif      
            if(d > 0)dist[cur] = tmp_dist;
            else dist[cur] = 0;
            obj[cur] = can;
            cur ++;
        }    
    }
    ResultHeap KNNs;

    // d == D indicates FDScanning. 
    if(d == D){ 
        for(int i=0;i<ncan;i++){
            candidates[i].first = dist[i];
            candidates[i].second = id[obj[i]];
        }
        std::partial_sort(candidates, candidates + k, candidates + ncan);
        
        for(int i=0;i<k;i++){
            KNNs.emplace(candidates[i].first, candidates[i].second);
        }
    }
    // d < D indicates ADSampling with and without cache-level optimization
    if(d < D){
        auto cur_dist = dist;
        for(int i=0;i<nprobe;i++){
            int cluster_id = centroid_dist[i].second;
            for(int j=0;j<len[cluster_id];j++){
                size_t can = start[cluster_id] + j;
#ifdef COUNT_DIST_TIME
                StopW stopw = StopW();
#endif
                float tmp_dist = adsampling::dist_comp(distK, res_data + can * (D-d), query + d, *cur_dist, d);
#ifdef COUNT_DIST_TIME
                adsampling::distance_time += stopw.getElapsedTimeMicro();
#endif                     
                if(tmp_dist > 0){
                    KNNs.emplace(tmp_dist, id[can]);
                    if(KNNs.size() > k) KNNs.pop();
                }
                if(KNNs.size() == k && KNNs.top().first < distK){
                    distK = KNNs.top().first;
                }
                cur_dist++;
            }
        }
    }

    delete [] centroid_dist;
    delete [] dist;
    delete [] candidates;
    delete [] obj;
    return KNNs;
}


void IVF::save(char * filename){
    std::ofstream output(filename, std::ios::binary);

    output.write((char *) &N, sizeof(size_t));
    output.write((char *) &D, sizeof(size_t));
    output.write((char *) &C, sizeof(size_t));
    output.write((char *) &d, sizeof(size_t));

    if(d > 0)output.write((char *) L1_data,  N * d       * sizeof(float));
    if(d < D)output.write((char *) res_data, N * (D - d) * sizeof(float));
    output.write((char *) centroids, C * D * sizeof(float));

    output.write((char *) start, C * sizeof(size_t));
    output.write((char *) len  , C * sizeof(size_t));
    output.write((char *) id   , N * sizeof(size_t));

    output.close();
}

void IVF::load(char * filename){
    std::ifstream input(filename, std::ios::binary);
    cerr << filename << endl;

    if (!input.is_open())
        throw std::runtime_error("Cannot open file");

    input.read((char *) &N, sizeof(size_t));
    input.read((char *) &D, sizeof(size_t));
    input.read((char *) &C, sizeof(size_t));
    input.read((char *) &d, sizeof(size_t));
    cerr << N << " " << D << " " << C << " " << d << endl;

    L1_data   = new float [N * d + 10];
    res_data  = new float [N * (D - d) + 10];
    centroids = new float [C * D];
    
    start = new size_t [C];
    len   = new size_t [C];
    id    = new size_t [N];

    if(d > 0)input.read((char *) L1_data,  N * d       * sizeof(float));
    if(d < D)input.read((char *) res_data, N * (D - d) * sizeof(float));
    input.read((char *) centroids, C * D * sizeof(float));

    input.read((char *) start, C * sizeof(size_t));
    input.read((char *) len  , C * sizeof(size_t));
    input.read((char *) id   , N * sizeof(size_t));
    
    input.close();
}

