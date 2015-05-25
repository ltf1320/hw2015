#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <map>
#include <stdio.h>
#include <algorithm>

using namespace std;

//int randNum=0;
//int randFail=0;

enum cardPattern{
	UNKOWN_PATTERN=0,
    HIGH_CARD,
    ONE_PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
};

struct cardCount{
    int point,cnt;
    static bool cmp (const cardCount &a,const cardCount &b){
		if(a.cnt!=b.cnt)return a.cnt>b.cnt;
		else return a.point>b.point;
	}
};

bool cmp(int a,int b){
    return a%13<b%13;
}

class Dila{
public:
    Dila(){
        card_out=new bool[52];
		for(int i=0;i<52;i++)card_out[i]=false;
    }
    Dila(int a,int b){
        card_out=new bool[52];
		for(int i=0;i<52;i++)card_out[i]=false;
        card_out[a]=card_out[b]=true;
    }
    ~Dila(){
        delete [] card_out;
    }
    void deliverHandCard(int cards[]){deliverCard(2,cards);}
    void deliverFlop(int cards[]){deliverCard(3,cards);}
    void deliverTurn(int cards[]){deliverCard(1,cards);}
    void deliverRiver(int cards[]){deliverCard(1,cards);}
    static cardPattern judgePattern(int cards[]){
        cardPattern p=HIGH_CARD;
        int x=5,y=6;///init
        for(int i=0;i<7;i++){
            for(int j=i+1;j<7;j++){
                swap(cards[j],cards[5]);
                swap(cards[i],cards[6]);
                cardPattern temp=judgePattern_5(cards);
                //printf("[%d %d %d]\n",temp,i,j);
                //for(int k=0;k<5;k++)cout<<cards[k]<<" ";
                //cout<<endl;
                if(temp>p)
					p=temp,x=j,y=i;
                swap(cards[i],cards[5]);
                swap(cards[j],cards[6]);
            }
        }
        swap(cards[x],cards[5]);
        swap(cards[y],cards[6]);
        return p;
    }
    static cardPattern judgePattern_5(int cards_[]){
		int cards[5];
        for(int i=0;i<5;i++)cards[i]=cards_[i];
        sort(cards,cards+5,cmp);
	//	for(int i=0;i<5;i++)cout<<tr(cards[i])<<" ";
   //     cout<<endl;
        bool isFlush=true,isStraight=true;
        cardPattern p;
        vector<int> temp;
        for(int i=0;i<5;i++){
            int point=tr(cards[i]);
            if(i==0){temp.push_back(point);continue;}
            if(cards[i]/13!=cards[i-1]/13)isFlush=false;
            if(point!=tr(cards[i-1])+1&&(!(i==4&&tr(cards[i-1])==3&&point==12)))isStraight=false;///bug
            if(point!=temp[temp.size()-1])temp.push_back(point);
        }
        if(isFlush)p=FLUSH;
        if(isStraight)p=STRAIGHT;
        if(isFlush&isStraight)p=STRAIGHT_FLUSH;
        if(isFlush|isStraight)return p;
        if(tr(cards[0])==tr(cards[3])||tr(cards[1])==tr(cards[4])) return p=FOUR_OF_A_KIND;
        if(temp.size()==2)p=FULL_HOUSE;
        if(temp.size()==3){
            if(tr(cards[1])==tr(cards[3])||
               tr(cards[2])==tr(cards[0])||
               tr(cards[2])==tr(cards[5]))p=THREE_OF_A_KIND;//tr(cards[2]==tr(cards[5])))p=THREE_OF_A_KIND;
            else p=TWO_PAIR;
        }
        if(temp.size()==4)p=ONE_PAIR;
        if(temp.size()==5)p=HIGH_CARD;
        return p;
    }
    static int pk(int cards_a[],int cards_b[]){
        //int a_pattern=judgePattern(cards_a),b_pattren=judgePattern(cards_b);
        //if(a_pattern>b_pattren)return 1;
        //else if(a_pattern<b_pattren)return -1;
        //else{
        //}
        //sort(cards_a,cards_a+5,cmp);
        //sort(cards_b,cards_b+5,cmp);
        struct cardCount cards_a_cnt[5],cards_b_cnt[5];
        int lena=0,lenb=0;
        for(int i=0;i<5;i++){
        	if(!i){
            	cards_a_cnt[lena].point=tr(cards_a[i]),cards_a_cnt[lena++].cnt=1;
                cards_b_cnt[lenb].point=tr(cards_b[i]),cards_b_cnt[lenb++].cnt=1;
            }else{
                if(cards_a[i]==cards_a[i-1])cards_a_cnt[lena-1].cnt++;
                else cards_a_cnt[lena].point=tr(cards_a[i]),cards_a_cnt[lena++].cnt=1;
                if(cards_b[i]==cards_b[i-1])cards_b_cnt[lenb-1].cnt++;
                else cards_b_cnt[lenb].point=tr(cards_b[i]),cards_b_cnt[lenb++].cnt=1;
            }
        }
        sort(cards_a_cnt,cards_a_cnt+lena,cardCount::cmp);
        sort(cards_b_cnt,cards_b_cnt+lenb,cardCount::cmp);
        if(lena==4&&cards_a_cnt[1].point==5&&cards_b_cnt[0].point==12)
        	if(cards_b_cnt[1].point==cards_a_cnt[1].point&&cards_b_cnt[0].point==cards_a_cnt[0].point)return 0;
        	else return -1; 
        for(int i=0;i<lena;i++)
            if(cards_a_cnt[i].point!=cards_b_cnt[i].point)
            	return cards_a_cnt[i].point-cards_b_cnt[i].point;
        return 0;
    }
    void deliverCard(int num,int cards[]){
        //srand((int)time(0)); ///the same time
        for(int i=0;i<num;i++){
            int card;
            while(card_out[card=rand()%52]) 
			{
	//			printf("fail:%d\n",card);
	//			randFail++;
			}
	//		printf("get:%d\n",card);
	//		randNum++;
            cards[i]=card;
            card_out[card]=true;
        }
    }
    void claimCard(int num,int cards[])
    {
        for(int i=0;i<num;i++)
            card_out[cards[i]]=true;
    }
	void claimCard(int card1,int card2)
    {
        card_out[card1]=true;
		card_out[card2]=true;
    }
private:
    bool *card_out;
    static int tr(int value){
        return value%13;
    }
};

struct Hand
{
	int id;
	int cards[7];
	cardPattern pattern;
	Hand(){
		pattern=UNKOWN_PATTERN;	
	}
	Hand(int cds[])
	{
		new (this)Hand();
		for(int i=0;i<7;i++)
			cards[i]=cds[i];
	}
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
	//	cout<<"pattern:"<<pattern<<endl;
	}
	static bool cmp(Hand a,Hand b)
	{
		if(a.pattern==UNKOWN_PATTERN)
			a.getPattern();
		if(b.pattern==UNKOWN_PATTERN)
			b.getPattern();
		if(a.pattern!=b.pattern)return a.pattern>b.pattern;
		else return Dila::pk(a.cards,b.cards)>0;
	}
};


