
# 0 - IVF, 1 - IVF++, 2 - IVF+
randomize=0

cd ..
g++ ./src/search_ivf.cpp -O3 -o ./src/search_ivf -I ./src/

path=./data/
result_path=./results
data='gist'
C=4096
K=100

for randomize in {0..2}
do
if [ $randomize == "1" ]
then 
    echo "IVF++"
elif [ $randomize == "2" ]
then 
    echo "IVF+"
    
else
    echo "IVF"
fi

res="${result_path}/${data}_IVF${C}_${randomize}.log"
index="${path}/${data}/${data}_ivf_${C}_${randomize}.index"

query="${path}/${data}/${data}_query.fvecs"
gnd="${path}/${data}/${data}_groundtruth.ivecs"
trans="${path}/${data}/O.fvecs"


./src/search_ivf -d ${randomize} -n ${data} -i ${index} -q ${query} -g ${gnd} -r ${res} -t ${trans} -k ${K}

done
