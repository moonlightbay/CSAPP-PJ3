//21307130028 沈钰

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
typedef struct{        //定义高速缓存行
	int valid_bits;    //有效位
	unsigned  tag;     //标记位
	int stamp;         //时间戳，用于LRU
}cache_line;
char* filepath = NULL;  //trace file path
int s,E,b,S;           // s,E,b,S,分别表示组索引位数，关联性，块位数，组数
int hit=0,miss=0,eviction=0;    //hit,miss,eviction分别表示命中，未命中，驱逐次数
cache_line** cache = NULL;      //cache[S][E]，二级指针

void init_cacheline(){
	cache = (cache_line**)malloc(sizeof(cache_line*)*S);             //为cache组分配空间
	for(int i=0;i<S;i++)
		*(cache+i) = (cache_line*)malloc(sizeof(cache_line)*E);       //每一组为cache行分配空间
	for(int i=0;i<S;i++){
		for(int j=0;j<E;j++){
			cache[i][j].valid_bits = 0;      //有效位初始化为0
			cache[i][j].tag = 0xffffffff;    //tag初始化为无效位
			cache[i][j].stamp = 0;            //时间戳为0		
		}	
	}
}

void update_cache(unsigned address){      //更新高速缓存cache,针对组相联映射
	unsigned s_address =(address>>b) & ((0xffffffff)>>(32-s));           //组索引
	unsigned t_address = address>>(s+b);                                 //标记位
	//判断tag为是否相等，是否命中
    for(int i=0;i<E;i++){
		if((*(cache+s_address)+i)->tag ==t_address){
			cache[s_address][i].stamp = 0;       //刷新时间戳
			hit++;                               //命中数增加
			return;
		}
	}
    //更新高速缓存cache
	for(int i=0;i<E;i++){
		if(cache[s_address][i].valid_bits == 0){      //如果有效位为0，未命中（cold miss）
			cache[s_address][i].tag = t_address;    //更新标记位
			cache[s_address][i].valid_bits = 1;    //有效位置1
			cache[s_address][i].stamp = 0;       //刷新时间戳
			miss++;
			return;
		}
	}
    //暴力实现LRU策略，驱逐该组中时间戳最大的行，并更新（conflict miss）
	int max_stamp=0;                    //最大时间戳
	int max_i;
	for(int i=0;i<E;i++){               //找到时间戳最大的行
		if(cache[s_address][i].stamp > max_stamp){
			max_stamp = cache[s_address][i].stamp;
			max_i = i;
		}
	}
	eviction++;          
	miss++;
	cache[s_address][max_i].tag = t_address;
	cache[s_address][max_i].stamp = 0;	
}
void update_time(){                        //全体cache行时间戳更新
	for(int i=0;i<S;i++){
		for(int j=0;j<E;j++){
			if(cache[i][j].valid_bits == 1)
				cache[i][j].stamp++;		
		}	
	}
}
int main(int argc,char *argv[])                  //解析输入的指令
{
	int opt;         
	while((opt = getopt(argc,argv,"s:E:b:t:")) !=-1){           //parse command line arguments
		switch(opt){
		case 's':
			s=atoi(optarg);                         //s,E,b分别表示组索引位数，行数，块位数
			break;
		case 'E':
			E=atoi(optarg);
			break;
		case 'b':
			b=atoi(optarg);
			break;
		case 't':                                  //要重播的valgrind跟踪的名称
			filepath = optarg;                    
			break;
		}
	}
	S = 1<<s;                                  //组数S=2^s
	init_cacheline();
	FILE* file=fopen(filepath,"r");            //打开trace文件
	if(file == NULL){                          //打开文件失败
		printf("Open file wrong");		
		exit(-1);
	}
	char operation;
	unsigned address;
	int size;	
	while(fscanf(file," %c %x,%d",&operation,&address,&size)>0){   //读取文件中的操作，无需考虑I
		switch(operation){
			case 'L':
				update_cache(address);
				break;
			case 'M':
				update_cache(address);
			case 'S':
				update_cache(address);
				break;
		}
		update_time();                   //时间戳更新
	}
	for(int i=0;i<S;i++)                  //释放cache空间
		free(*(cache+i));
	free(cache);
	fclose(file);	                     //关闭文件	
    printSummary(hit,miss,eviction); 
    return 0;
}

