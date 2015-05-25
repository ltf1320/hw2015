#include "dila.hpp"


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
		if(sim(11,12,playerNum))
			win[0][1]++;
	}
	printf("%d\n",win[0][1]);
	printf("%f\n",1.0*win[0][1]/num);
}

int main()
{
	srand(time(0));
	//printf("%d\n",RAND_MAX);
	MentoCarlo(8,10000);
//	printf("randNum=%d\n",randNum);
//	printf("randFail=%d\n",randFail);
//	int hand1[7]={1,15,16,4,10,13,12};
//	int hand2[7]={1,1,15,15,3,3,17};
//	Hand h1(hand1),h2(hand2);
//	cout<<STRAIGHT_FLUSH<<endl;
//	cout<<Hand::cmp(hand1,hand2)<<endl;
//	h1.getPattern();
//	cout<<h1.pattern<<endl;
}
