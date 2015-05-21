#include <cstdio>
#include <map>

using namespace std;

char line[200];

class Player
{
public:
	int pid;
	int winNum;
	Player()
	{
		pid=-1;
		winNum=0;
	}
};

map<int,Player*> players;

int main()
{
	freopen("data.csv","r",stdin);
	gets(line);
	while(gets(line))
	{
		int pid,rank,tmp;
		char tt[100];
		sscanf(line,"%d, %d, %d, %d, %d, %s %d",&pid,&tmp,&tmp,&tmp,&tmp,tt,&rank);
		puts(line);
		printf("%d %d\n",pid,rank);
		if(players.count(pid)==0)
		{
			Player* pl=new Player();
			pl->pid=pid;
			players[pid]=pl;
		}
		Player* p=players[pid];
		if(rank==1)
			p->winNum++;
	}
	for(auto iter=players.begin();iter!=players.end();iter++)
		printf("pid:%d winNum:%d\n",iter->second->pid,iter->second->winNum);
}


