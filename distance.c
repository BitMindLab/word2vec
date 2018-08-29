//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries 单词的最大长度

int main(int argc, char **argv) {
  FILE *f;
  char st1[max_size];  // 函数栈中开辟了多大空间，是一个char空间作为指针而已吗？
  char *bestw[N];  // best N closest words  都存储在函数栈中吗，还是只有都搞忘了，
  char file_name[max_size], st[100][max_size];  // 100是什么东东？               // 为什么debug会回到main？因为采用了-O3
  float dist, len, bestd[N], vec[max_size];
  long long words, size, a, b, c, d, cn, bi[100];
  char ch;
  float *M;
  char *vocab;
  if (argc < 2) {
    printf("Usage: ./distance <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }

  // 1. 读word到vocab， 读vector到M
  strcpy(file_name, argv[1]);
  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  fscanf(f, "%lld", &words); // words = 71291,是vocab size
  fscanf(f, "%lld", &size);  // size = 200, 是vector的维度
  vocab = (char *)malloc((long long)words * max_w * sizeof(char)); //为什么不用二维指针存储？二维指针貌似占空间更多一些
  for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);  // 读取第b个单词的第a个字符
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0; // 0即 \0
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);  // 归一化，求cosine距离的时候直接相乘即可
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);

  // 2. 输入 word 并 查询
  while (1) {
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    printf("Enter word or sentence (EXIT to break): ");
    a = 0;

    // 2.1 输入 word或者sentence
    while (1) {
      st1[a] = fgetc(stdin);
      if ((st1[a] == '\n') || (a >= max_size - 1)) {
        st1[a] = 0;
        break;
      }
      a++;
    }
    if (!strcmp(st1, "EXIT")) break;
    cn = 0;
    b = 0;
    c = 0;

    // 2.2 查询
    while (1) {
      st[cn][b] = st1[c]; //
      b++;
      c++;
      st[cn][b] = 0;
      if (st1[c] == 0) break;  //
      if (st1[c] == ' ') {  //
        cn++;
        b = 0;
        c++;
      }
    }
    cn++;
    // 顺序查找，找到position  这里并未利用hash，比较慢！ 但是输入一个单词的话，只需要遍历查询一次，所以关系不大
    // 如果频繁查找的话，则采用hash
    for (a = 0; a < cn; a++) {
      for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
      if (b == words) b = -1;
      bi[a] = b;
      printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
      if (b == -1) {
        printf("Out of dictionary word!\n");
        break;
      }
    }
    // 输入word的vector存入vec
    if (b == -1) continue;
    printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn; b++) {  // cn一般是1，是指每行输入单词的个数
      if (bi[b] == -1) continue;
      for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
    }

    // 再次归一化。没必要，已验证，注释掉dist不变
//    len = 0;
//    for (a = 0; a < size; a++) len += vec[a] * vec[a]; //前面已经归一化了，这里归一化没有什么意义了，因为len≈1
//    len = sqrt(len);
//    for (a = 0; a < size; a++) vec[a] /= len;

    // 遍历词典中每个word
    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    for (c = 0; c < words; c++) {
      a = 0;
      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
      if (a == 1) continue;
      dist = 0;
      for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
      for (a = 0; a < N; a++) {
        if (dist > bestd[a]) { // 如果单词c应该排在第top-a的位置，则进行插入操作
          for (d = N - 1; d > a; d--) { // 先下移
            bestd[d] = bestd[d - 1];
            strcpy(bestw[d], bestw[d - 1]);
          }
          bestd[a] = dist;    //再插入
          strcpy(bestw[a], &vocab[c * max_w]);
          break;
        }
      }
    }
    for (a = 0; a < N; a++) printf("%50s\t\t%f\n", bestw[a], bestd[a]);
  }
  return 0;
}
