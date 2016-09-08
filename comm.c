#include "comm.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <stdio.h>

List *ListNew(int capacity, int itemSize) {
	List* v = (List*)malloc(sizeof(List));
	memset(v, 0, sizeof(List));
	v->capacity = capacity;
	v->itemSize = itemSize;
	v->items = malloc(capacity * itemSize);
	return v;
}

void ListClear(List *v) {
	if (v == NULL) return;
	v->size = 0;
}

void ListDestroy(List *v) {
	if (v == NULL) return;
	free(v->items);
	free(v);
}

int ListAdd(List *v, void *item) {
	if (v == NULL || item == NULL ||  v->size >= v->capacity) return 0;
	char *p = (char *)(v->items);
	p += v->size * v->itemSize;
	memcpy(p, item, v->itemSize);
	++v->size;
	return 1;
}

int ListRemove(List *v, int idx) {
	if (v == NULL ||  idx >= v->size) return 0;
	char *p = (char *)(v->items);
	p += v->itemSize * idx;
	for (int i = idx; i < v->size - 1; ++i) {
		memcpy(p, p + v->itemSize, v->itemSize);
		p += v->itemSize;
	}
	--v->size;
	return 1;
}

int ListIndexOf(List *v, void *item) {
	if (v == NULL || item == NULL) return -1;
	char *p = (char *)(v->items);
	for (int i = 0; i < v->size; ++i) {
		if (memcmp(item, p, v->itemSize) == 0) 
			return i;
		p += v->itemSize;
	}
	return -1;
}


void *ListGet(List *v, int idx) {
	if (v == NULL || idx >= v->size) return NULL;
	char *p = (char *)(v->items);
	p += idx * v->itemSize;
	return p;
}

// -------------------------------------------------------------------------------

static int openIO = 0;
static HANDLE io = NULL;
void Log(char *buf) {
	if (!openIO) {
		openIO = 1;
		AllocConsole();
		io = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	strcat(buf, "\n");
	WriteConsole(io, buf, strlen(buf), NULL, NULL);
	buf[0] = '\0';
}

//--------------------------------------------------------------------------------
typedef struct _YearHoliday {
	int year;
	int days[50];
} YearHoliday;

typedef struct _Holidays {
	int num;
	YearHoliday hol[30];
} Holidays;

typedef struct _Head {
	int dataLen[50]; //每条数据的长度
} Head;

static Holidays hilos = {0};

void InitHolidays() {
	char path[250] = {0};
	strcat(path, GetDllPath());
	strcat(path, "holiday.db");
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		return;
	memset(&hilos, 0, sizeof hilos);
	Head hd = {0};
	fread(&hd, sizeof(hd), 1, f);
	for (int i = 0; hd.dataLen[i] != 0; ++i) {
		fread(&hilos.hol[i], hd.dataLen[i], 1, f);
		hilos.num = i + 1;
	}
	fclose(f);
}

// @return YYYYMMDD
int GetCurDay()
{
	time_t timer;
    struct tm *tblock;
    timer = time(NULL);  
    tblock = localtime(&timer); 
    int y = tblock->tm_year + 1900;
    int m = tblock->tm_mon + 1;
    int d = tblock->tm_mday;
    return y * 10000 + m * 100 + d;
}

// @return HHmmSS
int GetCurTime()
{
	time_t timer;
    struct tm *tblock;
    timer = time(NULL);  
    tblock = localtime(&timer); 
    
    int h = tblock->tm_hour;
    int m = tblock->tm_min;
    int s = tblock->tm_sec;
    return h * 10000 + m * 100 + s;
}

// 是否是交易日 0:不是  1:是 
int IsTradeDay(int y, int m, int d) {
	int md = m * 100 + d;
	int *p = NULL;
	if (y <= 0 || m <= 0 || d <= 0)
		return 0;
		
	struct tm cd = {0};
	cd.tm_year = y - 1900;
	cd.tm_mon = m - 1;
	cd.tm_mday = d;
	cd.tm_hour = 1;
	time_t t = mktime(&cd);
	struct tm *tblock = localtime(&t);
	if (tblock->tm_wday == 0 || tblock->tm_wday == 6)
		return 0;
	
	for (int i = 0; i < hilos.num;  ++i) {
		if (hilos.hol[i].year != y)
			continue;
		p = hilos.hol[i].days;
		while (*p != 0 && *p != md) ++p;
		return *p != md;
	}
	return 1;
}

int GetLastTradeDay() {
	time_t timer;
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);
    if ((tblock->tm_hour * 100 + tblock->tm_min) < 930) {
    	timer -= 12*3600;
    }
    while (1) {
    	tblock = localtime(&timer);
    	int v = IsTradeDay(tblock->tm_year + 1900, tblock->tm_mon + 1, tblock->tm_mday);
    	if (v) {
			return (tblock->tm_year+1900)*10000 + (tblock->tm_mon+1)*100 + tblock->tm_mday;
		} else {
			timer -= 24 * 3600;
		}
    }
    return 0;
}

int GetTradeDayBetween(int beginDay, int endDay) {
	struct tm b = {0};
	int days = 0;
	int curDay = 0;
	int cd = GetCurDay();
	
	if (beginDay > cd || endDay > cd)
		return 0;
	if (beginDay > endDay) {
		int tmp = endDay;
		endDay = beginDay;
		beginDay = tmp;
	}
	
	b.tm_year = beginDay/10000 - 1900;
	b.tm_mon = beginDay % 10000 / 100 - 1;
	b.tm_mday = beginDay % 100;
	
	while (1) {
		time_t t = mktime(&b);
		struct tm *k = localtime(&t);
		curDay = (k->tm_year + 1900)*10000 + (k->tm_mon + 1) * 100 + k->tm_mday;
		if (IsTradeDay(k->tm_year + 1900, k->tm_mon + 1, k->tm_mday)) {
			++days;
		}
		if (curDay == endDay) break;
		++b.tm_mday;
	}
	
	return days;
}


