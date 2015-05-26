#include "dila.hpp"
#include "simulator.hpp"
#include <thread>
#include "ThreadSafeQueue.h"
#include <cstring>
struct Hold
{
	int hold1,hold2;
	int playerNum;
	Hold(int h1,int h2,int pn)
	{
		hold1=h1,hold2=h2;
		playerNum=pn;
	}
};

ThreadSafeQueue<Hold*> que;

float res[7][52][52];

bool sim(int hold1,int hold2,int playerNum,Hand* players,int* common)
	{
		Dila dila;
		dila.claimCard(hold1,hold2);
		dila.deliverCard(5,common);
		players[0].common(common);
		players[0].hold(hold1,hold2);
		players[0].getPattern();
		//cout<<players[0].pattern<<endl;
		players[0].id=0;
		for(int i=1;i<playerNum;i++)
		{
			players[i].common(common);
			players[i].getHold(&dila);
			players[i].getPattern();
			players[i].id=i;
		}
		sort(players,players+playerNum,Hand::cmp);
		bool res=false;
		if(players[0].id==0)
			res=true;
		return res;
	}

void MentoCarlo(int num)
{
	Hand players[10];
	int common[5];
	while(true)
	{
		auto res=que.popFront();
		if(!res.first) return;
		int win=0;
		for(int i=0;i<num;i++)
		{
			if(sim(res.second->hold1,res.second->hold2,res.second->playerNum,players,common))
				win++;
		}
		res[res.second->playerNum-2][res.second->hold1][res.second->hold2]=1.0*win/num;
		delete res.second;
	}
}


int main()
{
	srand(time(0));
	int simNum=10000;
	for(int p=2;p<=2;p++)
	for(int i=0;i<52;i++)
		for(int j=i+1;j<52;j++)
		{
			que.push(new Hold(i,j,p));
		}
	vector<thread> threads;
	for(int i=0;i<4;i++)
	{
		threads.push_back(move(thread(MentoCarlo,simNum)));
	}
	for(auto iter=threads.begin();iter!=threads.end();iter++)
		iter->join();
	FILE* resFile=fopen("holdRank.data","wb");
	fwrite(res,sizeof(res),1,resFile);
	fclose(resFile);
}
