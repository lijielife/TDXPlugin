#include "F.h"
#include "comm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <windows.h>

static char buf[300];
#define BUF (buf+strlen(buf))

typedef struct _KMInfo {
	int dateLen;
	int code;
} KMInfo;

KMInfo di = {0};

void Reset_REF(int len,float* out,float* rLen, float* code, float* tmp) {
	memset(buf, 0, sizeof buf);
	di.dateLen = (int)rLen[0];
	di.code = (int)code[0];
}

// MACD 底背离 
void CrossInfo_REF(int len,float* out, float* k, float* m, float* c) {
	static int cp[100];   // 交叉点位置
	static int cpLow[100];
	static float price[100]; //最低价 
	
	int num = 0;
	for (int i = len - di.dateLen; i < len; ++i) {
		while (k[i] >= m[i] && i < len) ++i;
		if (i >= len) break;
		while (k[i] < m[i] && i < len) ++i;
		if (i >= len) break;
		cp[num++] = i; // len - 1 - i;
	}
	
	for (int i = 0; i < num; ++i) {
		float min = 10000;
		int j = cp[i];
		for (; j >= 0; --j) {
			float d = k[j] - m[j];
			if (min >= d) min = d;
			else break;
		}
		price[i] = c[j];
		cpLow[i] = j;
		
		for (int vx = j + 1; vx >= j-3; --vx) {
			if (price[i] > c[vx]) price[i] = c[vx];
		}
	}
	
	int val = 0;
	for (int i = num - 1; i > 0; --i) {
		int last = i;
		int last2 = i - 1;
		int vv = (k[cpLow[last]] >= k[cpLow[last2]]) && (price[last] < price[last2]);
		val = val || vv;
	}
	
	for (int i = 0; i < len; ++i)
		out[i] = (float)val;
	
	/*
	sprintf(BUF, "[%06d] CP:%d", di.code, num);
	for (int i = 0; i < num; ++i) {
		sprintf(BUF, " %.2f %.2f |", k[cpLow[i]], price[i]);
	}
	sprintf(BUF, " *%d", val);
	WriteLn();
	*/
}

// 计算最大跌幅  close = [..., ref(cose,1), ref(cose,0)]
void CalcMaxDF_REF(int len,float* pfOUT,float* days, float* close,float* code)
{
	int dayLen = days[0];
	float maxDay = 0, maxVal = 0, minDay = 0, minVal = 1000000;
	
	if (dayLen > len) dayLen = len;
	for (int i = 0, begin = len - dayLen; i < dayLen; ++i) {
		if (close[i + begin] > maxVal) {
			maxVal = close[i + begin];
			maxDay = i + begin;
		}
	}
	
	minVal = maxVal;
	for (int i = maxDay; i < len; ++i) {
		if (close[i] < minVal) {
			minVal = close[i];
			minDay = i;
		}
	}
	
	pfOUT[len - 1] = (maxVal - minVal) * 100 / maxVal;
}

// 计算最大涨幅  close = [..., ref(cose,1), ref(cose,0)]
void CalcMaxZF_REF(int len,float* pfOUT,float* days, float* close,float* code)
{
	int dayLen = days[0];
	float maxDay = 0, maxVal = 0, minDay = 0, minVal = 1000000;
	
	if (dayLen > len) dayLen = len;
	for (int i = 0, begin = len - dayLen; i < dayLen; ++i) {
		if (close[i + begin] > maxVal) {
			maxVal = close[i + begin];
			maxDay = i + begin;
		}
	}
	
	minVal = maxVal;
	for (int i = len - dayLen; i < maxDay; ++i) {
		if (close[i] < minVal) {
			minVal = close[i];
			minDay = i;
		}
	}
	
	pfOUT[len - 1] = (maxVal - minVal) * 100 / minVal;
}

// -----------------------------------------------------------------------------
typedef struct _XtInfo {
	int beginPos; // 形态开始位置
	int endPos;
	int dir;  // 方向 0:未知  1:上升  2:下降  3:横盘 
	float beginVal;
	float endVal;
} XtInfo;

#define GET_DIR(b, e) (b == e ? 3 : (b < e ? 1 : 2))
#define GET_MAX(a, b) (a > b ? a : b)
#define GET_MIN(a, b) (a < b ? a : b)

// 获取形态的震幅 %
static float _GetXtZF(float from, float to) {
	float a = GET_MIN(from, to);
	float b = GET_MAX(from, to);
	return (b - a) * 100 / a;
}

// 是否是很小的形态 
static int _IsSmallXt(XtInfo *p) {
	float zf = _GetXtZF(p->beginVal, p->endVal);
	int days = p->endPos - p->beginPos + 1;
	//区间段震幅小于1.5% ；区间段天数小于4天的 
	if (zf < 1.5 /*&& days < 4*/)
		return 1;
	
	return 0;
}

// 获取下一个大形态；若没有，则返回-1 
static int _GetNextLargeXtIndex(List *xtList, int from) {
	for (int i = from; i < xtList->size; ++i) {
		XtInfo *p = (XtInfo*)ListGet(xtList, i);
		if (!_IsSmallXt(p)) return i;
	}
	return -1;
}

static int _TryMergeXt(List *mrgList, List *xtList, int beginIdx) {
	XtInfo* mlx = ListGet(mrgList, mrgList->size - 1);
	XtInfo xt = {0};
	XtInfo* p = (XtInfo*)ListGet(xtList, beginIdx);
	memcpy(&xt, p, sizeof(XtInfo));
	int largeIdx = _GetNextLargeXtIndex(xtList, beginIdx);
	if (largeIdx == -1) largeIdx = xtList->size;
	
	int i = beginIdx + 1;
	for (; i < largeIdx; ++i) {
		p = (XtInfo*)ListGet(xtList, i);
		if (_GetXtZF(xt.beginVal, p->endVal) > 2) {
			break;
		}
	}
	--i;
	p = (XtInfo*)ListGet(xtList, i);
	xt.endPos = p->endPos;
	xt.endVal = p->endVal;
	if (i != beginIdx)
		xt.dir = 3;
	ListAdd(mrgList, &xt);
	return i;
}

// 合并形态 
static void _MeargeXtList(List *xtList, List *xtMergedList) {
	for (int i = 0; i < xtList->size; ++i) {
		XtInfo *p = (XtInfo*)ListGet(xtList, i);
		if (i == 0) {
			ListAdd(xtMergedList, p);
		} else {
			i = _TryMergeXt(xtMergedList, xtList, i);
		}
	}
}

static void _CalcXt(List *xtList, float* mid, int len, int day) {
	XtInfo xi = {0};
	if (day > len) day = len;
	for (int i = 0, begin = len - day; i < day; ++i) {
		if (xi.beginPos == 0) {
			xi.beginPos = begin + i;
			xi.beginVal = mid[begin + i];
			continue;
		}
		if (xi.dir == 0) {
			xi.endPos = begin + i;
			xi.endVal = mid[begin + i];
			xi.dir = GET_DIR(xi.beginVal, xi.endVal);
			continue;
		}
		int dir = GET_DIR(xi.endVal, mid[begin + i]);
		if (xi.dir == dir) {
			xi.endPos = begin + i;
			xi.endVal = mid[begin + i];
		} else {
			ListAdd(xtList, &xi);
			memset(&xi, 0, sizeof(XtInfo));
			i -= 2;
		}
	}
	if (xi.dir != 0) ListAdd(xtList, &xi);
}

static List *xtList = NULL;
static List *xtMergedList = NULL;
void BOLLXT_REF(int len, float* out, float* mid, float* days, float* dwn) {
	if (xtList == NULL) xtList = ListNew(500, sizeof(XtInfo));
	if (xtMergedList == NULL) xtMergedList = ListNew(500, sizeof(XtInfo));
	int day = (int)days[0];
	
	ListClear(xtList);
	ListClear(xtMergedList);
	for (int i = 0; i < len; ++i) {
		mid[i] = ceil(mid[i] * 100) / 100.0;
	}
	_CalcXt(xtList, mid, len, day);
	
	
	/* sprintf(buf, "\n---------Org------------");
	Log(buf);
	for (int i = 0; i < xtList->size; ++i) {
		XtInfo *s = (XtInfo*)ListGet(xtList, i);
		sprintf(buf, "Pos:[%d -> %d] [%.2f -> %.2f] | Dir:%d  ZF:%.2f",  s->beginPos-len+day, s->endPos-len+day, 
			s->beginVal, s->endVal, s->dir, (s->endVal - s->beginVal) * 100 / s->beginVal);
		Log(buf);
	} */
	
	_MeargeXtList(xtList, xtMergedList);
	
	XtInfo *s = (XtInfo*)ListGet(xtList, 0);
	for (int i = 0; i < len; ++i) {
		out[i] = s->beginVal * 0.9;
	}
	for (int i = 0; i < xtList->size; ++i) {
		XtInfo *s = (XtInfo*)ListGet(xtList, i);
		out[s->beginPos] = s->beginVal;
	}
}

void BOLLXT_mrg_test(int len, float* out, float* mid, float* days, float* c) {
	int day = (int)days[0];
	
	XtInfo *s = (XtInfo*)ListGet(xtMergedList, 0);
	float minVal = 1000;
	for (int i = s->beginPos; i < len; ++i) {
		if (minVal > mid[i]) minVal = mid[i];
	}
	minVal *= 0.9;
	for (int i = 0; i < len; ++i) {
		out[i] = minVal;
	}
	
	//sprintf(buf, "\n---------Mrg------------");
	//Log(buf);
	for (int i = 0; i < xtMergedList->size; ++i) {
		XtInfo *s = (XtInfo*)ListGet(xtMergedList, i);
		out[s->beginPos] = s->beginVal;
		//sprintf(buf, "Pos:[%d -> %d] [%.2f -> %.2f] | Dir:%d  ZF:%.2f",  s->beginPos-len+day, s->endPos-len+day, 
		//	s->beginVal, s->endVal, s->dir, (s->endVal - s->beginVal) * 100 / s->beginVal);
		//Log(buf);
	}
}

List *xtList3;
void BOLLXT_3_test(int len, float* out, float* mid, float* days, float* dwn) {
	if (xtList3 == NULL) xtList3 = ListNew(500, sizeof(XtInfo));
	int day = (int)days[0];
	
	ListClear(xtList3);
	for (int i = 0; i < len; ++i) {
		mid[i] = ceil(mid[i] * 100) / 100.0;
	}
	_CalcXt(xtList3, mid, len, day);
	for (int i = xtList3->size - 1, j = 0; i >= 0; --i, ++j) {
		XtInfo *s = (XtInfo*)ListGet(xtList3, i);
		out[len - 1 - j] = s->dir;
	}
}

//------------------------------------------------------------------------------

// 在指定的日期上， 是第几个交易日（以最近交易日为零开始算） 
// Eg: 如果当前日期是2016.8.19， 那么8.19就返回0； 8.18返回1
// out = [日数] ; 假设date=[..., 2016.8.19] 而days=2016.8.20则 返回0 
void CalcTradeDayInfo_REF(int len, float* out, float* days, float* date, float* code) {
	int d = (int)days[0] + 20000000;
	int rb = 0;
	int last = (int)date[len - 1] + 19000000;
	if (d > last) {
		out[len - 1] = 0;
		return;
	}
	
	for (int i = len - 1, j = 0; i >= 0; --i, ++j) {
		int cd = (int)date[i] + 19000000;
		if (d == cd) {
			rb = j;
			break;
		}
		if (cd < d) {
			rb = j - 1;
			break;
		}
	}
	out[len - 1] = rb;
}

//------------------------------------------------------------------------------
// 指定的日期是否是交易日 
void IsTradDay_REF(int len, float* out, float* days, float* b, float* c) {
	int d = (int)days[0];
	int v = IsTradeDay(d / 10000, d / 100 % 100, d % 100);
	out[len - 1] = v;
}

//------------------------------------------------------------------------------
// 是否是停牌股 out = [1:是  0：不是] 
void IsTP_REF(int len, float* out, float* date, float* vol, float* c) {
	int last = GetLastTradeDay();
	int day0 = (int)date[len-1] + 19000000;
	int day1 = (int)date[len-2] + 19000000;
	
	if (last == day0) {
		out[len-1] = (vol[len-1] == 0);
	} else if (last == day1) {
		out[len-1] = (vol[len-2] == 0);
	} else {
		out[len-1] = 1;
	}
}

//------------------------------------------------------------------------------
// .401 排序信息 
typedef struct _SortInfo {
	int code;  //股票代码
	float val;
	// 排序相关 
	int idx;
	int prev;
	int next;
} SortInfo;

#define SI_LIST_NUM 2
struct _SI {
	List *list[SI_LIST_NUM]; // List<SortInfo>  = [区间数据list, ...] 
	int firstIdx[SI_LIST_NUM]; // [区间数据开始idx , ...]
	DWORD lastTime;
	CRITICAL_SECTION mutex;
} si;

static void SetSortInfo(int len, int code, float val, int* firstIdx, List *list) {
	SortInfo info = {0};
	info.code = code;
	info.val = val;
	info.idx = list->size;
	info.prev = info.next = -1;
	
	// begin sort
	if (*firstIdx == -1) {
		*firstIdx = 0;
		ListAdd(list, &info);
	} else {
		int findIdx = -1;
		int before = -1; //向前插入? 
		for (int i = *firstIdx; i >= 0 && i < list->size;) {
			SortInfo* t = (SortInfo*)ListGet(list, i);
			if (info.val > t->val) {
				findIdx = i;
				before = 1;
				break;
			}
			if (t->next == -1) {
				findIdx = i;
				before = 0;
				break;
			}
			i = t->next;
		}
		if (before == 1) {
			SortInfo* t = (SortInfo*)ListGet(list, findIdx);
			info.prev = t->prev;
			info.next = t->idx;
			if (t->prev != -1) {
				SortInfo* p = (SortInfo*)ListGet(list, t->prev);
				p->next = info.idx;
			} else {
				*firstIdx = info.idx;
			}
			t->prev = info.idx;
			ListAdd(list, &info);
		} else if (before == 0) {
			SortInfo* t = (SortInfo*)ListGet(list, findIdx);
			info.prev = t->idx;
			info.next = - 1;
			t->next = info.idx;
			ListAdd(list, &info);
		} else {
			sprintf(buf, "SetSortInfo Error: before = -1  code = %d val=%f", info.code, info.val);
			Log(buf);
		}
	}
}

// id = [0:区间数据, 1] 
void SetSortInfo_REF(int len, float* out, float* code, float* val, float* id) {
	EnterCriticalSection(&si.mutex);
	if (si.list[0] == NULL) {
		for (int i = 0; i < SI_LIST_NUM; ++i) {
			si.list[i] = ListNew(5000, sizeof(SortInfo));
		}
	}
	DWORD diff = GetTickCount() - si.lastTime;
	if (diff > 1000) { //大于1000毫秒，表示重新开始的排序 
		for (int i = 0; i < SI_LIST_NUM; ++i) {
			ListClear(si.list[i]);
			si.firstIdx[i] = -1;
		}
	}
	
	int _id = (int)id[0];
	SetSortInfo(len, (int)code[0], val[len-1], &(si.firstIdx[_id]), si.list[_id]);
	
	si.lastTime = GetTickCount();
	LeaveCriticalSection(&si.mutex);
}

void GetSortInfo_REF(int len, float* out, float* code, float* id, float* c) {
	int cc = (int)code[0];
	out[len-1] = 0;
	int _id = (int)id[0];
	
	for (int i = si.firstIdx[_id], j = 0; i >= 0 && si.list[_id] != NULL && i < si.list[_id]->size; ++j) {
		SortInfo* t = (SortInfo*)ListGet(si.list[_id], i);
		if (t->code == cc) {
			out[len-1] = j + 1;
			break;
		}
		i = t->next;
	}
}

//------------------------------------------------------------------------------
struct _BOLLSK {
	float um[30];  // up - mid
	float ml[30];  // mid - low
	int num;
	CRITICAL_SECTION mutex;
} bollSK;
// BOLL线收口 out=[收口大小]
void BOLLSK_REF(int len, float* out, float* up, float* mid, float* low) {
	EnterCriticalSection(&bollSK.mutex);
	bollSK.num = GET_MIN(len, 30);
	for (int i = 0, j = len - bollSK.num; i < bollSK.num; ++i, ++j) {
		bollSK.um[i] = up[j] - mid[j];
		bollSK.ml[i] = mid[j] - low[j];
	}
	
	
	LeaveCriticalSection(&bollSK.mutex);
}

//------------------------------------------------------------------------------
PluginTCalcFuncInfo g_CalcFuncSets[] = 
{
	{10,(pPluginFUNC)&Reset_REF},
	{11,(pPluginFUNC)&CrossInfo_REF},
	
	{20,(pPluginFUNC)&CalcMaxDF_REF},
	{21,(pPluginFUNC)&CalcMaxZF_REF},
	
	{30,(pPluginFUNC)&BOLLXT_REF},
	{33,(pPluginFUNC)&BOLLXT_3_test},   // test
	{34,(pPluginFUNC)&BOLLXT_mrg_test}, // test
	
	{40,(pPluginFUNC)&SetSortInfo_REF},
	{41,(pPluginFUNC)&GetSortInfo_REF},
	
	{50,(pPluginFUNC)&BOLLSK_REF},
	
	{100,(pPluginFUNC)&CalcTradeDayInfo_REF},
	{101,(pPluginFUNC)&IsTradDay_REF},
	{102,(pPluginFUNC)&IsTP_REF},
	{0,NULL},
};

//导出给TCalc的注册函数
BOOL RegisterTdxFunc(PluginTCalcFuncInfo** pFun)
{
	if(*pFun==NULL) {
		(*pFun)=g_CalcFuncSets;
		InitHolidays();
		InitializeCriticalSection(&si.mutex);
		InitializeCriticalSection(&bollSK.mutex);
		return TRUE;
	}
	
	return FALSE;
}
