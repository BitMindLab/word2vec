make
if [ ! -e text8 ]; then
  wget http://mattmahoney.net/dc/text8.zip -O text8.gz
  gzip -d text8.gz -f
fi
# 1. 四元组 A is to B as C is to D
time ./word2vec -train text8 -output vectors.bin -cbow 1 -size 200 -window 8 -negative 25 -hs 0 -sample 1e-4 -threads 20 -binary 1 -iter 15
./compute-accuracy vectors.bin 30000 < questions-words.txt # (后面的< ...作为输入流文件)
# to compute accuracy with the full vocabulary, use: ./compute-accuracy vectors.bin < questions-words.txt


# 2. 二元组 A is similar to B
./compute-accuracy-AB vectors.bin < questions-words-AB.txt



