/*
 * compute-accuracy-AB.c
 *
 *  Created on: Jan 4, 2015
 *      Author: xusong
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>

#define MAX_STRING 100  // train_file的文件名最大长度


const int vocab_hash_size = 30000000;  // Maximum 30 * 0.7 = 21M words in the vocabulary
//const int vocab_hash_size =  120000000;  // Maximum 100 * 0.7 = 70M words in the vocabulary
const long long max_size = 2000;         // max length of strings
const long long N = 1;                   // number of closest words
const long long max_w = 50;              // max length of vocabulary entries
char *vocab;
int *vocab_hash;



// Returns hash value of a word
int GetWordHash(char *word) {
  unsigned long long a, hash = 0;
  for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a]; //采取257进制
  hash = hash % vocab_hash_size;
  return hash;
}

// Returns position of a word in the vocabulary; if the word is not found, returns -1
int SearchVocab(char *word) {
  unsigned int hash = GetWordHash(word);
  while (1) {
    if (vocab_hash[hash] == -1) return -1;
    if (!strcmp(word, &vocab[vocab_hash[hash]* max_w])) return vocab_hash[hash];
    hash = (hash + 1) % vocab_hash_size;
  }
  return -1;
}


int main(int argc, char **argv)
{
  FILE *f, *f_goodcase, *f_badcase;
  char st1[max_size], st2[max_size], st3[max_size], st4[max_size], bestw[N][max_size], file_name[max_size], ch;
  float dist=0, len, bestd[N], vec[max_size];
  long long words, size, a, b, c, d, b1, b2, b3, threshold = 0, goodcase_count = 0, badcase_count = 0;
  unsigned int hash;
  float *M;
  int TCN, CCN = 0, TACN = 0, CACN = 0, SECN = 0, SYCN = 0, SEAC = 0, SYAC = 0, QID = 0, TQ = 0, TQS = 0;
  if (argc < 2) {
    printf("Usage: ./compute-accuracy <FILE> <threshold>\nwhere FILE contains word projections, and threshold is used to reduce vocabulary of the model for fast approximate evaluation (0 = off, otherwise typical value is 30000)\n");
    return 0;
  }
  int i;
  for(i=0; i<argc; i++) printf("%s\n",argv[i]);

  // 1. 读取 word到vocab(同时做hash)，读取vector到M
  strcpy(file_name, argv[1]);
  if (argc > 2) threshold = atoi(argv[2]);
  f = fopen(file_name, "rb");
  f_goodcase = fopen("goodcase.txt", "w");
  f_badcase = fopen("badcase.txt", "w");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  fscanf(f, "%lld", &words);
  printf("vocab-size:%lld\n", words);
  if (threshold) if (words > threshold) words = threshold;
  fscanf(f, "%lld", &size);
  vocab = (char *)malloc(words * max_w * sizeof(char));
  M = (float *)malloc(words * size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB\n", words * size * sizeof(float) / 1048576);
    return -1;
  }
  vocab_hash = (int *)calloc(vocab_hash_size, sizeof(int));
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  for (b = 0; b < words; b++) {
	// 1.1 读取第b个word
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < max_w; a++) vocab[b * max_w + a] = toupper(vocab[b * max_w + a]);

    // 1.2 做hash
	hash = GetWordHash(&vocab[b* max_w]);
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size; //如果hash值冲突了,使用开放地址法解决冲突,
    vocab_hash[hash] = b; //由词的hash值找到她所在词汇表的排序位置


    // 1.3 读取第b个vector
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);


  // 2. 读取测试文件
  TCN = 0;
  while (1) {
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    scanf("%s", st1);  // 参数中< 表示从文件中获取输入流，很好
    for (a = 0; a < strlen(st1); a++) st1[a] = toupper(st1[a]);
    // 读取类别
    if (!strcmp(st1, ":")) {
    	scanf("%s", st1);
    	printf("%s:\n", st1);
    	continue;
    }
    if (feof(stdin)) {
    	printf("mean cosine distance: %.4f; total count: %d \n", dist/TQ, TQ);
    	printf("goodcase: %3.1f\%; badcase:%3.1f\% \n", goodcase_count*100.0/TQ, badcase_count*100.0/TQ);
        break;
    }

    // 读取两个word
    scanf("%s", st2);
    //printf("%s\n",st1);
    //printf("%s\n",st2);
    for (a = 0; a < strlen(st2); a++) st2[a] = toupper(st2[a]);

    // 在词典中查找两个word, 顺序查找太慢
    //for (b = 0; b < words; b++) if (!strcmp(&vocab[b* max_w], st1)) break;    b1 = b;
    //for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st2)) break;   b2 = b;
    b1 = SearchVocab(st1);
    b2 = SearchVocab(st2);

    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;

    // 判断 词典中找不到该单词
    if (b1 == -1) {
    	//printf("%s: Out of dictionary word;\t", st1);
    	//printf("synterm: %s\n", st2);
    	continue;
    }
    if (b2 == -1) {
    	//printf("%s: Out of dictionary word;\t", st2);
    	//printf("synterm: %s\n", st1);
    	continue;
    }
    TQ++;

    // 计算cos距离
    len = 0;
    float dist_tmp = 0;
    for (a = 0; a < size; a++) {
    	//len += M[a + b1 * size] * M[a + b1 * size];  // 验证是否为1
    	dist_tmp +=  M[a + b1 * size] * M[a + b2 * size];
    }
    if(dist_tmp > 0.8) {
    	fprintf(f_goodcase, "%s,%s,%f\n", st1, st2, dist_tmp);
    	goodcase_count++;
    } else if(dist_tmp < 0.2) {
    	fprintf(f_badcase, "%s,%s,%f\n", st1, st2, dist_tmp);
    	badcase_count++;
    }
    //printf("%s,%s,%f\n", st1, st2, dist_tmp);
    dist += dist_tmp;


  }
  fclose(f_goodcase);
  fclose(f_badcase);

  return 0;
}
