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

float resf[52][52];

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
		resf[res.second->hold1][res.second->hold2]=1.0*win/num;
		delete res.second;
	}
}


int main(int argc,char **argv)
{
	if(argc!=3)
	{
		printf("arg needed\n");
		return -1;
	}
	memset(resf,0,sizeof(resf));
	srand(time(0));
	int simNum=10000;
	int p;
	sscanf(argv[1],"%d",&p);
	printf("p=%d,file=%s\n",p,argv[2]);
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
	for(int i=0;i<52;i++)
		for(int j=i+1;j<52;j++)
		{
			resf[j][i]=resf[i][j];
		}
	FILE* resFile=fopen(argv[2],"wb");
	fwrite(resf,sizeof(resf),1,resFile);
	fclose(resFile);
}
