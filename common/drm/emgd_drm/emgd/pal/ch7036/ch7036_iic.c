/*-----------------------------------------------------------------------------
* Copyright (c) Chrontel Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*-----------------------------------------------------------------------------
* @file  ch7036_iic.c
* @version 1.2.4
*-----------------------------------------------------------------------------
*/





#include "ch7036_iic.h"
#include "ch7036_def_regmap.h"


static uint8 g_nCurPage = 0;
static uint8 g_nCurRegmap[REG_PAGE_NUM][REG_NUM_PER_PAGE];

int g_verbose = 0; 

void DeviceReset(DEV_CONTEXT* pDevContext)
{
	
	uint8 reg=0x00;

   I2CWrite(pDevContext,0x03, 0x04);
   
   I2CWrite(pDevContext,0x52, 0x2B); 
   I2CWrite(pDevContext,0x5a, 0x06);
   I2CWrite(pDevContext,0x5a, 0x04);
   I2CWrite(pDevContext,0x5a, 0x06);

   I2CWrite(pDevContext,0x52, 0x29);
   I2CWrite(pDevContext,0x52, 0x2b);

   I2CWrite(pDevContext,0x5a, 0x04);

  
   I2CWrite(pDevContext,0x03, 0x01); 
   reg = I2CRead(pDevContext,0x16);

   reg = reg & 0xF7;
   I2CWrite(pDevContext,0x16, reg);
   


}


void DeviceSetup(DEV_CONTEXT* pDevContext)
{
	if (g_verbose) {
			uint8 page, index;
			uint16 k =0;       
			uint16 rc[5];
			for(page=0; page<REG_PAGE_NUM; ++page)
			{
				rc[page]=0;
				if(page == 0x02) continue; 
				I2CWrite(pDevContext,0x03, page); 
				for(index=0x07; index<REG_NUM_PER_PAGE; ++index)
				{
					if(g_nCurRegmap[page][index] != g_DefRegMap[page][index])
					{   
			
						k++;  
						rc[page]++;
						I2CWrite(pDevContext,index, g_nCurRegmap[page][index]);
						
					}
				}
			}
	
			for(page=0; page<REG_PAGE_NUM; ++page) 
				
				;

	}
	else {
			uint8 page, index;
			for(page=0; page<REG_PAGE_NUM; ++page)
			{
				if(page == 0x02) continue; 
				I2CWrite(pDevContext,0x03, page);
				for(index=0x07; index<REG_NUM_PER_PAGE; ++index)
				{
					if(g_nCurRegmap[page][index] != g_DefRegMap[page][index])
					{
						I2CWrite(pDevContext,index, g_nCurRegmap[page][index]);
					}
				}
			}
	}
}


uint8 iic_read(uint8 index)
{
	return g_nCurRegmap[g_nCurPage][index];
}

void iic_write(uint8 index, uint8 value)
{
	if(index == 0x03)
	{
		g_nCurPage = value;
	}
	else
	{
		g_nCurRegmap[g_nCurPage][index] = value;
	}
}

void iic_reset()
{
	uint8 page, index;
	for(page=0; page<REG_PAGE_NUM; ++page)
	{
		for(index=0; index<REG_NUM_PER_PAGE; ++index)
		{
			g_nCurRegmap[page][index] = g_DefRegMap[page][index];
		}
	}
	g_nCurPage = 0x00; 
}

uint32 iic_read_ex(MULTI_REG_ID reg_id)
{
	uint8 oldpage;
	uint32 n0, n1, n2, n3, mask, i, value;

	
	n0 = n1 = n2 = n3 = value = mask = 0;

	
	i = (uint32)reg_id;

	
	oldpage = iic_read(0x03);
	iic_write(0x03, g_MultiRegTable[i].PageIndex);

	if(NOOP_INDEX != g_MultiRegTable[i].TopRegIndex)
		n0 = iic_read(g_MultiRegTable[i].TopRegIndex);

	if(NOOP_INDEX != g_MultiRegTable[i].HighRegIndex) 
		n1 = iic_read(g_MultiRegTable[i].HighRegIndex);

	if(NOOP_INDEX != g_MultiRegTable[i].MiddleRegIndex)
		n2 = iic_read(g_MultiRegTable[i].MiddleRegIndex);

	n3 = iic_read(g_MultiRegTable[i].LowRegIndex);

	iic_write(0x03, oldpage);

	if(0 != n0)
	{
		mask = 1;
		mask <<= g_MultiRegTable[i].TopEndBit - g_MultiRegTable[i].TopStartBit + 1;
		mask -= 1;
		mask <<= g_MultiRegTable[i].TopStartBit;
		n0 &= mask;
		n0 >>= g_MultiRegTable[i].TopStartBit;
		n0 <<= (g_MultiRegTable[i].HighEndBit - g_MultiRegTable[i].HighStartBit + 1)+(g_MultiRegTable[i].MiddleEndBit - g_MultiRegTable[i].MiddleStartBit + 1) + (g_MultiRegTable[i].LowEndBit - g_MultiRegTable[i].LowStartBit + 1);
	}
	if(0 != n1)
	{
		mask = 1;
		mask <<= g_MultiRegTable[i].HighEndBit - g_MultiRegTable[i].HighStartBit + 1;
		mask -= 1;
		mask <<= g_MultiRegTable[i].HighStartBit;
		n1 &= mask;
		n1 >>= g_MultiRegTable[i].HighStartBit;
		n1 <<= (g_MultiRegTable[i].MiddleEndBit - g_MultiRegTable[i].MiddleStartBit + 1) + (g_MultiRegTable[i].LowEndBit - g_MultiRegTable[i].LowStartBit + 1);
	}

	if(0 != n2)
	{
		mask = 1;
		mask <<= g_MultiRegTable[i].MiddleEndBit - g_MultiRegTable[i].MiddleStartBit + 1;
		mask -= 1;
		mask <<= g_MultiRegTable[i].MiddleStartBit;
		n2 &= mask;
		n2 >>= g_MultiRegTable[i].MiddleStartBit;	
		n2 <<= g_MultiRegTable[i].LowEndBit - g_MultiRegTable[i].LowStartBit + 1;
	}

	if(0 != n3)
	{
		mask = 1;
		mask <<= g_MultiRegTable[i].LowEndBit - g_MultiRegTable[i].LowStartBit + 1;
		mask -= 1;
		mask <<= g_MultiRegTable[i].LowStartBit;
		n3 &= mask;
		n3 >>= g_MultiRegTable[i].LowStartBit;
	}

	value = n0 | n1 | n2 | n3;
	return value;
}

void iic_write_ex(MULTI_REG_ID reg_id, uint32 value)
{
	uint8 oldpage;
	uint32 n1, n2, n3, n4, mask, nOrigin, i, value_local;

	
	n1 = n2 = n3 = n4 = mask = nOrigin = i = 0;
	value_local = value;

	
	i = (uint32)reg_id;

	
	oldpage = iic_read(0x03);
	iic_write(0x03, g_MultiRegTable[i].PageIndex);

	mask = 1;
	mask <<= g_MultiRegTable[i].LowEndBit - g_MultiRegTable[i].LowStartBit + 1;
	mask -= 1;
	n1 = value_local & mask;
	n1 <<= g_MultiRegTable[i].LowStartBit;
	nOrigin = iic_read(g_MultiRegTable[i].LowRegIndex);
	mask <<= g_MultiRegTable[i].LowStartBit;
	nOrigin &= ~mask;
	n1 |= nOrigin;
	iic_write(g_MultiRegTable[i].LowRegIndex, (uint8)n1);
	value_local >>= g_MultiRegTable[i].LowEndBit - g_MultiRegTable[i].LowStartBit + 1;

	if(NOOP_INDEX != g_MultiRegTable[i].MiddleRegIndex)
	{
		mask = 1;
		mask <<= g_MultiRegTable[i].MiddleEndBit - g_MultiRegTable[i].MiddleStartBit + 1;
		mask -= 1;
		n2 = value_local & mask;
		n2 <<= g_MultiRegTable[i].MiddleStartBit;
		nOrigin = iic_read(g_MultiRegTable[i].MiddleRegIndex);
		mask <<= g_MultiRegTable[i].MiddleStartBit;
		nOrigin &= ~mask;
		n2 |= nOrigin;
		iic_write(g_MultiRegTable[i].MiddleRegIndex, (uint8)n2);
		value_local >>= g_MultiRegTable[i].MiddleEndBit - g_MultiRegTable[i].MiddleStartBit + 1;

		if(NOOP_INDEX != g_MultiRegTable[i].HighRegIndex)
		{
			mask = 1;
			mask <<= g_MultiRegTable[i].HighEndBit - g_MultiRegTable[i].HighStartBit + 1;
			mask -= 1;
			n3 = value_local & mask;
			n3 <<= g_MultiRegTable[i].HighStartBit; 
			nOrigin = iic_read(g_MultiRegTable[i].HighRegIndex);
			mask <<= g_MultiRegTable[i].HighStartBit;
			nOrigin &= ~mask;
			n3 |= nOrigin;
			iic_write(g_MultiRegTable[i].HighRegIndex, (uint8)n3);
			value_local >>= g_MultiRegTable[i].HighEndBit - g_MultiRegTable[i].HighStartBit + 1;

			if(NOOP_INDEX != g_MultiRegTable[i].TopRegIndex)
			{
				mask = 1;
				mask <<= g_MultiRegTable[i].TopEndBit - g_MultiRegTable[i].TopStartBit +1;
				mask -= 1;
				n4 = value_local & mask;
				n4 <<= g_MultiRegTable[i].TopStartBit;
				nOrigin = iic_read(g_MultiRegTable[i].TopRegIndex);
				mask <<= g_MultiRegTable[i].TopStartBit;
				nOrigin &= ~mask;
				n4 |= nOrigin;
				iic_write(g_MultiRegTable[i].TopRegIndex,(uint8)n4);
			}
		}		
	}

	iic_write(0x03, oldpage);
}


