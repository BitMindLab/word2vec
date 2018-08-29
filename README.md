## word2vec代码分分析

### 1. word2vec.c

* 训练得到的vector:

        格式：<vocab-size> <vector-size> \n <word> <vector>
        71291 200
        </s> 0.002001 0.002210 -0.001915 ...
        the -0.682389 0.301308 0.630009 ...


* 这里的词典大小是7万，论文提到googleNews库字典大小100万个高频词，晓伟word2vec词典大小922万，貌似会造成查找比较慢（用hash）。



### 2. distance.c

* 输入一个单词w，找到top-N近邻
* 输入一个词组(多个单词w)，对每个单词向量加和，找到top-N近邻
* 查找w，遍历其他所有单词
* w的查找采用顺序查找（因为只需要查找一次，做hash还不够麻烦呢）

### 3. word-analogy.c


### 4. compute-accuracy.c

* 对每组ABCD，查找position及对应的vector
* ABCD均采用的顺序查找
* 查找次数较多，特别是词典很大的情况下查询特别慢。因此改用hash

### 5. compute-accuracy-AB.c


-------

# word2vec评价指标
[demo](http://radimrehurek.com/2014/02/word2vec-tutorial/#app)
### 1. A B C D ###
* 实现：compute-accuracy.c
* 基本思想：

        vector('Paris') - vector('France') + vector('Italy') ≈ vector('Rome')
        vector('king') - vector('man') + vector('woman') ≈ vector('queen')

* 输入四元word： ` A is to B as C is to D `
* 四个评价指标：
    * ACCURACY TOP1
    * Total accuracy
    * Semantic accuracy
    * Syntactic accuracy
* 什么才要采用四元的评价指标？为什么使用加减法？

* badcase
    * 比如。。。。


*why this kind of addition/subtraction trick actually works：
* seeking a word which is similar to king and woman but is different from man.
* Linguistic Regularities in Sparse and Explicit Word Representations. Omer Levy and Yoav Goldberg. CoNLL 2014.


### 2. A B  ###
* 主要思想：most similar, 输入二元同义词库 A is similar to B
* 实现：compute-accuracy-AB.c
* 脚本：10.12.12.42:/search/xusong/word2vec/demo-word-accuracy.sh
* 同义词库来源：10.134.10.24:/search/hgc/Online_Files/synterm.txt
* 同义词库预处理：1. 选取级别较高的同义词（级别0或9）； 2. 将所有半角字符转化为全角字符，并统一GBK编码


| 数据  | vocab-size | mean cosine distance |  goodcase | badcase |  备注 |
| ------------------- | -------------------- | ------------------- | ------------------------- | ------------------ | --------------- | 
| vectors.skip.100.0826.qt |  4002019 |  0.5772 | 21.4%	| 9.9% | 8.26号的词向量，基于querytitle生成的词典 |
| vectors.skip.1107.100 | 4193048 | 0.6021 | 24.5% | 9.1% |  11.07号的词向量，利用正文生成的词典 |
| vectors.skip.1107.10m | 10814214 | 0.6002 | 24.5% | 9.3% | 11.07号的词向量，利用正文生成的词典 |
| vectors.skip.1124.10m.10w | 9227144 | 0.6123 | 29.1% | 11.4% |  11.24生成，10w代表滑动窗口为10,10m表示大约1千万词典大小. 基于querytitle生成的词典|
| vectors.skip.1124.4m | 4009075 | 0.5973 | 24.6%	 | 11.3% | 11.24生成, 滑动窗口为4. 基于querytitle生成的词典|
| vectors.skip.1124.4m.10w | 4009075 | 0.6155 | 29.7% | 11.2% | 11.24的数据，滑动窗口为10. 基于querytitle生成的词典|

cosine距离越大，相似性越高，word2vec效果越好（加粗数值表示最好结果）.综合评价，最后一个版本效果最好。

* badcase见表格链接，主要有
    * 中英文翻译（占很大比例）：比如 ｂｕｔｔｏｎ,按键,0.179536； ｐｅｐｓｉ,百事,0.199440；印度,ｉｎｄｉａ,0.153918等
    * 数字（占很大比例）： 二百三十九,２３９,0.178804等
    * 简称： 北京电影学院,北影,0.012966；  北大西洋公约组织,北约,0.003944等
    * 同义词词库错别字：大开眼界,大开眼戒,0.160034 
* 英文库中的bad case
    * 不同词性之间 
    * 
   
**疑问：**
* word2vec为什么处理不好这些bad case？同义词库又是如何成功获取这些case的？
word embedding原理： words that appear in similar contexts will be close to each other in the projected space.
中英文的词如果不具有相似的context，就无法捕捉到同义性。



-------

# 名词解释


index分词：专门针对
query分词：专门针对query的分词


-------

# 优化



![word2vec.PNG](https://bitbucket.org/repo/4XAeeB/images/481171660-word2vec.PNG)

* 紧密度优化，例如紧密度词典竟然没有“天气预报”，但关系应该不大
* 空格合并
* 编码



* 架构：skip-gram（慢、对罕见字有利）vs CBOW（快）
* 训练算法：分层softmax（对罕见字有利）vs 负采样（对常见词和低纬向量有利）
* 欠采样频繁词：可以提高结果的准确性和速度（适用范围1e-3到1e-5）
* 维度词向量：通常情况下表现都很好
* 文本（window）大小：skip-gram通常在10附近，CBOW通常在5附近



-------

# 六、疑问汇总
* 对于query index分词不一致的情况
* 线上线下打分的一致性：（见邮件合并代码之后线上线下不一致统计）
* Site查询是什么东东？Site查询是特殊查询语法，应当忽略site部分。线下训练时最好也直接丢弃site查询，不过比例应当非常小。
* 为什么要打词？干什么用的？打就是查，打词就是根据query，对返回的文档计算特征(94+2维)，然后计算相应的score
* mingrong: 注意：大家连线上打词一定要注意安全!至少要保证cache和线上版本一致，否则可能会出问题，导致严重后果
* word2vec_new_qt文件有很多重复的行
* word2vec_new_doc中也有一些重复的行

* 标点符号是怎么处理的？
-------




# 扩展阅读


*USING WORD2VEC WITH NLTK

http://streamhacker.com/2014/12/29/word2vec-nltk/ （应该是Using word2vec by gensim with NLTK Corpus ）



* 400 lines c++11 version: https://github.com/jdeng/word2vec
 * python version: http://radimrehurek.com/gensim/models/word2vec.html
* java version: 
    * https://github.com/NLPchina/Word2VEC_java
    * https://github.com/ansjsun/word2vec_java
    * Parallel java version: https://github.com/siegfang/word2vec
* cuda version: https://github.com/whatupbiatch/cuda-word2vec
 经测试，OK。  也可以使用该java代码加载上述c++11版本的model，但需要自行添加load的代码。



# 待做 

* 词频统计，并按照词频排序，观察，去除高频低频词
* 如何评价titleRank效果好坏?每次都找人标注太
* 为什么翻译效果不好？因为是根据context来
* 窗口大小的影响，以及效果好坏的评估
* compute-accuracy-AB  加入 all 选项，观测平均水平。
* 降维到2维，看看到底是怎么个情况?
* cosine距离和欧氏距离，到底该用哪个？或者两个都可以用，只是不同距离表示不同的视角？cosine的理想场景是在标准圆上，不考虑vector-size，只考虑方向，但是word2vec中有vector-size,单位圆上的点对向量加减法貌似有冲突吗吧？multi-view distance
* supervised multi-metric learning word2vec. 
* A is to B is C is to D这个思路感觉非常好啊，比A is similar to B要好。因为similarity因视角不同而不同，也可以说是因距离度量不同而不同。因此可以采用multi-metric优化pair或者直接优化ABCD
* metric表示共性，比如名词，形容词，学校，食品，数字，褒义词贬义词。类似单个word多标签问题，word-pair的relation问题。supervised embedding
* multi-metric问题来了，两个vector的距离到底是什么？不同视角均值，测不准原理，
