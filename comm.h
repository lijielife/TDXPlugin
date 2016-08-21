#pragma once

typedef struct _List {
	int capacity;  
	int itemSize;  // 每个数据项大小
	int size;  // 当前有多少个数据 
	void *items;
} List;

List *ListNew(int capacity, int itemSize);

void ListClear(List *v);

void ListDestroy(List *v);

int ListAdd(List *v, void *item);

int ListRemove(List *v, int idx);

int ListIndexOf(List *v, void *item);

void *ListGet(List *v, int idx);

void Log(char *buf);

void InitHolidays();

// @return YYYYMMDD
int GetCurDay();

// @return HHmmSS
int GetCurTime();

// 是否是交易日 0:不是  1:是 
int IsTradeDay(int y, int m, int d);

// @return YYYYMMDD 获得最后的交易日
int GetLastTradeDay();

// 在 [开始日 至 结束日] 之间 ，有几个交易日；计算结果包含 beginDay和 endDay
// @param beginDay endDay 格式：YYYYMMDD  
// EG: 20160919~20160919之间有一个交易日； 20160918~20160919有2个交易日 
int GetTradeDayBetween(int beginDay, int endDay);







