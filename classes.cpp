#include "header.h"


void LRU::init(int maxsizes)
	{
	maxsize=maxsizes;
	currentsize=0;
	}

bool LRU::removeIndexAndFile(int index,int type)
	{
	
	delIndex(index,type);
	return true;
	}
void LRU::deletion(int index)
{

entry e;
for(int i=0;i<(int)q.size();i++)
		{
		if(q[i].index==index)
			{
			e.index=q[i].index;
			e.size=q[i].size;
			q.erase(q.begin()+i);
			currentsize-=e.size;
			removeIndexAndFile(index,0);
			removeIndexAndFile(index,1);
			removeIndexAndFile(index,2);
			}
		}


}

void LRU::hit(int index)
	{
entry e;
for(int i=0;i<(int)q.size();i++)
		{
		if(q[i].index==index)
			{
			e.index=q[i].index;
			e.size=q[i].size;
			q.erase(q.begin()+i);
			q.push_back(e);
			}
		}

	}
bool LRU::add(int index, int size)
{
entry e;
currentsize+=size;
//cout<<currentsize;

	if(currentsize<=maxsize)
	{	e.index=index;
		e.size=size;
		q.push_back(e);
		display();
		return true;
	}
    else
	{
	
	while(currentsize>maxsize)
		{
		int in;
		if(q.size()!=0)
			{
		currentsize=currentsize-q[0].size;
		in=q[0].index;
		q.erase(q.begin());
		if(!removeIndexAndFile(in,0))
			{

			}
		if(!removeIndexAndFile(in,1))
			{

			}
		
		if(!removeIndexAndFile(in,2))
			{


			}
			}

			else
			{
				break;
			}
		}
		e.index=index;
		e.size=size;
		q.push_back(e);
				display();

	}
return true;
}

