#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PI 3.141592653589793238462643
// sound map
#define OVERTONE_MAX 24
#define EX_OVERTONE_NUM 8
#define SOUND_NUM 12
#define SAMPLING_RATE 44100

typedef unsigned int Hertz;
typedef float *SoundMapEntity;
typedef struct tagSOUNDMAP
{
  Hertz          base_frequency;
  SoundMapEntity entity[SOUND_NUM];
} SOUNDMAP, *SoundMap;
typedef double OverTone[OVERTONE_MAX+EX_OVERTONE_NUM];
typedef double ExOverTone[EX_OVERTONE_NUM];
typedef double Tuning[SOUND_NUM];

SoundMap soundMapCreate(Hertz base_frequency, OverTone overtone, ExOverTone ex_overtone_frequency, Tuning tuning)
{
  SoundMap smap = malloc(sizeof(SOUNDMAP));
  if (smap == NULL) return NULL;
  
  for (int i = 0; i < SOUND_NUM; i++)
  {
    Hertz frequency = (Hertz)(base_frequency * tuning[i]);
    for (int j = 0; j < frequency; j++)
    {
      double radian_sample_ratio = 2 * PI * frequency / SAMPLING_RATE;
      double a = 0.;
      for (int k = 0; k < OVERTONE_MAX; k++)
        a += sin(j*radian_sample_ratio*k) * overtone[k];
      for (int k = 0; k < EX_OVERTONE_NUM; k++)
        a += sin(j*radian_sample_ratio*ex_overtone_frequency[k]) * overtone[OVERTONE_MAX+k];
      smap->entity[i][j] = (float)a;
    }
  }
  return smap;
}


//vector
#define SOUNDSAMPLEUNIT 32
typedef struct tagVECTORNODE
{
  struct tagVECTORNODE* prev;
  struct tagVECTORNODE* next;
  void*                 buf;
} VECTORNODE, *VectorNode;
typedef struct tagVECTOR
{
  size_t element_size;
  size_t chunk_size;
  size_t chunk_num;
  size_t wrote_size; //SoundSampleUnit
  size_t chunk_per_SSUnit; //chunk per SoundSampleUnit
  VectorNode start;
  VectorNode end;
} VECTOR, *Vector;
typedef size_t SoundSampleUnit;

Vector vectorCreate(size_t element_size, size_t chunk_per_SSUnit)
{
  Vector vec = malloc(sizeof(VECTOR));
  if (vec == NULL) return NULL;
  VectorNode node = malloc(sizeof(VECTORNODE));
  if (node == NULL) {free(vec); return NULL;}
  
  node->prev = NULL;
  node->next = NULL;
  node->buf  = malloc(chunk_per_SSUnit * SOUNDSAMPLEUNIT);
  if (node->buf == NULL) {free(node); free(vec); return NULL;}
  
  vec->element_size     = element_size;
  vec->chunk_size       = chunk_per_SSUnit * SOUNDSAMPLEUNIT;
  vec->chunk_num        = 1;
  vec->wrote_size       = 0;
  vec->chunk_per_SSUnit = chunk_per_SSUnit;
  vec->start            = node;
  vec->end              = node;
  
  return vec;
}

int vectorNewNode(Vector vec)
{
  VectorNode node = malloc(sizeof(VECTORNODE));
  if (node == NULL) return -1;
  
  node->prev      = vec->end;
  node->next      = NULL;
  node->buf       = malloc(vec->chunk_size);
  if (node->buf == NULL) {free(node); return -1;}
  vec->end        = node;
  vec->chunk_num += 1;
  
  return 0;
}

int vectorWrite(Vector vec, void* buf, SoundSampleUnit size)
{
  if (   vec == NULL
      || buf == NULL) return -1;
  
  int modulo = vec->wrote_size % vec->chunk_per_SSUnit;
  if (modulo > 0)
  {
    int free_size = 4 - modulo;
    memcpy((void*)((char*)(vec->end->buf)+modulo*SOUNDSAMPLEUNIT), buf, free_size*SOUNDSAMPLEUNIT);
    size -= free_size;
  }
  while(1)
  {
    if (vectorNewNode(vec)) return -1;
    int temp;
    if (size >= 4) temp=4;
    else           temp=size;
    memcpy(vec->end->buf, buf, temp*SOUNDSAMPLEUNIT);
    size -= temp;
    if (size == 0) break;
  }
  return 0;
}

//score
#define BINARYSCORE_PAGE_NUM  1024
#define BINARYSCORE_PAGE_SIZE 2048
typedef struct tagBINARYSCORE
{
  uint8_t opecode;
  uint8_t arg[3];
} BINARYSCORE, BinaryScore;
static BinaryScore *score[BINARYSCORE_PAGE_NUM];
static int score_page_num;
static int score_wrote_num;
int bsNewPage(void)
{
  if (score_page_num >= BINARYSCORE_PAGE_NUM) return -1;
  BinaryScore *p = malloc(sizeof(BinaryScore) * BINARYSCORE_PAGE_SIZE);
  if (p == NULL) return -1;
  score[score_page_num] = p;
  score_page_num++;
  return 0;
}
int bsFini(void)
{
  for (int i = 0; i < BINARYSCORE_PAGE_NUM; i++)
  {
    free(score[i]);
    score[i] = NULL;
  }
  score_page_num = 0;
  score_wrote_size = 0;
  return 0;
}
int bsInit(void)
{
  bsFini();
  return bsNewPage();
}
int bsWrite(BinaryScore bs)
{
  if ((score_wrote_size % BINARYSCORE_PAGE_SIZE) == 0) 
    if(bsNewPage()) 
      return -1;
  score[score_wrote_size / BINARYSCORE_PAGE_SIZE][score_wrote_size % BINARYSCORE_PAGE_SIZE] = bs;
  score_wrote_size++;
  return 0;
}

//analyzer
int isSystemChar(char c)
{
  return (c == ';') || (c == '@')
}
int analyzerRun(const char* filename)
{
  if (filename == NULL) return -1;
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) return -1;
  
  BinaryScore bs;
  int beat, meter;
  if (fscanf(fp, "%d,%d;", &beat, &meter) == EOF) return -1;
  ;
  while(feof(fp))
  {
    char sound, octave, length
    char c;
    c = fgetc(fp);
    if (c == '[') isChord = 1;
    if (c == ']') isChord = 0;
    if (fscanf(fp, "%c%c%c", &sound, &octave, &length) == EOF) return -1;
  }
}
