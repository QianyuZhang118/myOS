#include "tlib.h"



// 初始化为0
void 	tBitmapInit(tBitmap * bitmap)   
{
    bitmap->bitmap = 0;
}

// 把pos位置1
void 	tBitmapSet(tBitmap * bitmap,uint32_t pos)
{
	  bitmap->bitmap |= (1 << pos);    
}

// 把pos位置0
void tBitmapClear(tBitmap * bitmap,uint32_t pos)
{
		bitmap->bitmap &= ~(1 << pos);   
}


//找去位图中第一个被置位的数
uint32_t tBitmapGetFirstSet(tBitmap * bitmap)
{
   	static const uint8_t quickFindTable[] =     
	 {
	    /* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	 };

	 if (bitmap->bitmap & 0xff)        														 		//最低字节有不为0
    {
        return quickFindTable[bitmap->bitmap & 0xff];         	
    }
    else if (bitmap->bitmap & 0xff00)   														//最低字节
    {
        return quickFindTable[(bitmap->bitmap >> 8) & 0xff] + 8;        
    }
    else if (bitmap->bitmap & 0xff0000)															//次最高字节
    {
        return quickFindTable[(bitmap->bitmap >> 16) & 0xff] + 16;        
    }
    else if (bitmap->bitmap & 0xFF000000)														//最高字节
    {
        return quickFindTable[(bitmap->bitmap >> 24) & 0xFF] + 24;
    }
    else
    {
        return tBitmapPosCount();
    }
}

uint32_t tBitmapPosCount(void)
{
    return 32;
}


