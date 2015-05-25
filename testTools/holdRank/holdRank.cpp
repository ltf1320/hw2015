#include "dila.hpp"

struct Hand
{
	int id;
	int cards[7];
	cardPattern pattern;
	void common(int* commonCards)
	{
		for(int i=0;i<5;i++)
			cards[i]=commonCards[i];
	}
	void hold(int card1,int card2)
	{
		cards[5]=card1;
		cards[6]=card2;
	}
	void getHold(Dila *dila)
	{
		dila->deliverHandCard(cards+5);
	}
	void getPattern(){
		pattern=Dila::judgePattern(cards);
	}
	static bool cmp(Hand a,Hand b)
	{
		if(a.pattern!=b.pattern)return a.pattern>b.pattern;
		else return Dila::pk(a.cards,b.cards)>0;
	}
};

int win[52][52];

bool sim(int hold1,int hold2,int playerNum)
{
	Dila dila;
	dila.claimCard(hold1,hold2);
	int common[5];
	dila.deliverCard(5,common);
	Hand* players=new Hand[playerNum];
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
	if(players[0].id==0)
		return true;
	return false;
}


void MentoCarlo(int playerNum,int num)
{
	win[0][1]=0;
	for(int i=0;i<num;i++)
	{
		if(sim(0,1,playerNum))
			win[0][1]++;
	}
	printf("%d\n",win[0][1]);
	printf("%f\n",1.0*win[0][1]/num);
}

int main()
{
	srand(time(0));
	printf("%d\n",RAND_MAX);
	MentoCarlo(8,10000);

}
