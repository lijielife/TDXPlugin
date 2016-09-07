#pragma once

#define GET_MAX(a, b) ((a) > (b) ? (a) : (b))
#define GET_MIN(a, b) ((a) < (b) ? (a) : (b))

#define GET_MAX3(a, b, c) GET_MAX(GET_MAX(a, b), c)
#define GET_MAX4(a, b, c, d) GET_MAX(GET_MAX(a, b), GET_MAX(c, d))
#define GET_MAX6(a, b, c, d, e, f) GET_MAX3(a, b, GET_MAX4(c, d, e, f))
#define GET_MIN4(a, b, c, d) GET_MIN(GET_MIN(a, b), GET_MIN(c, d))

typedef struct _List {
	int capacity;  
	int itemSize;  // ÿ���������С
	int size;  // ��ǰ�ж��ٸ����� 
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

// �Ƿ��ǽ����� 0:����  1:�� 
int IsTradeDay(int y, int m, int d);

// @return YYYYMMDD ������Ľ�����
int GetLastTradeDay();

// �� [��ʼ�� �� ������] ֮�� ���м��������գ����������� beginDay�� endDay
// @param beginDay endDay ��ʽ��YYYYMMDD  
// EG: 20160919~20160919֮����һ�������գ� 20160918~20160919��2�������� 
int GetTradeDayBetween(int beginDay, int endDay);







