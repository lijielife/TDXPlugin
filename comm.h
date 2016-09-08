#pragma once

#define GET_MAX(a, b) ((a) > (b) ? (a) : (b))
#define GET_MIN(a, b) ((a) < (b) ? (a) : (b))

#define GET_MAX3(a, b, c) GET_MAX(GET_MAX(a, b), c)
#define GET_MAX4(a, b, c, d) GET_MAX(GET_MAX(a, b), GET_MAX(c, d))
#define GET_MAX6(a, b, c, d, e, f) GET_MAX3(a, b, GET_MAX4(c, d, e, f))
#define GET_MIN4(a, b, c, d) GET_MIN(GET_MIN(a, b), GET_MIN(c, d))

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

extern char *GetDllPath();

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







