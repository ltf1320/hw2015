#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <map>
#include <stdio.h>
#include <algorithm>

using namespace std;

enum cardPattern{
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
};

bool cmp(int a,int b){
    return a%13<b%13;
}

bool cmp1(cardCount a,cardCount b){
    if(a.cnt!=b.cnt)return a.cnt>b.cnt;
    else return a.point>b.point;
}

class Dila{
public:
    Dila(){
        card_out=new bool[52];
    }
    //固定两张牌
    Dila(int a,int b){
        card_out=new bool[52];
        card_out[a%13]=card_out[b%13]=true;
    }
    ~Dila(){
        delete [] card_out;
    }
    void deliverHandCard(int cards[]){deliverCard(2,cards);}
    void deliverFlop(int cards[]){deliverCard(3,cards);}
    void deliverTurn(int cards[]){deliverCard(1,cards);}
    void deliverRiver(int cards[]){deliverCard(1,cards);}
    //判断7张牌所能组成最大的牌型
    cardPattern judgePattern(int cards[]){
        cardPattern p=HIGH_CARD;
        int x,y;
        for(int i=0;i<7;i++){
            for(int j=i+1;j<7;j++){
                swap(cards[j],cards[5]);
                swap(cards[i],cards[6]);
                cardPattern temp=judgePattern_5(cards);
                //printf("[%d %d %d]\n",temp,i,j);
                //for(int k=0;k<5;k++)cout<<cards[k]<<" ";
                //cout<<endl;
                if(temp>p)p=temp,x=i,y=j;
                swap(cards[i],cards[5]);
                swap(cards[j],cards[6]);
            }
        }
        swap(cards[x],cards[5]);
        swap(cards[y],cards[6]);
        return p;
    }
    //判断5张牌的牌型
    cardPattern judgePattern_5(int cards_[]){
        int cards[5];
        for(int i=0;i<5;i++)cards[i]=cards_[i];
        //for(int i=0;i<5;i++)cout<<cards[i]<<" ";
        //cout<<endl;
        sort(cards,cards+5,cmp);
        bool isFlush=true,isStraight=true;
        cardPattern p;
        vector<int> temp;
        for(int i=0;i<5;i++){
            int point=tr(cards[i]);
            if(i==0){temp.push_back(point);continue;}
            if(cards[i]/13!=cards[i-1]/13)isFlush=false;
            if(point!=tr(cards[i-1])+1)isStraight=false;
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
               tr(cards[2]==tr(cards[5])))p=THREE_OF_A_KIND;
            else p=TWO_PAIR;
        }
        if(temp.size()==4)p=ONE_PAIR;
        if(temp.size()==5)p=HIGH_CARD;
        return p;
    }
    //比较两幅牌的大小，传入两组七张牌
    int pk(int cards_a[],int cards_b[]){
        int a_pattern=judgePattern(cards_a),b_pattren=judgePattern(cards_b);
        if(a_pattern>b_pattren)return 1;
        else if(a_pattern<b_pattren)return -1;
        else{
            sort(cards_a,cards_a+5,cmp);
            sort(cards_b,cards_b+5,cmp);
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
            sort(cards_a_cnt,cards_a_cnt+lena,cmp1);
            sort(cards_b_cnt,cards_b_cnt+lenb,cmp1);
            for(int i=0;i<lena;i++)
                if(cards_a_cnt[i].point!=cards_b_cnt[i].point)
                    return cards_a_cnt[i].point-cards_b_cnt[i].point;
            return 0;
        }
    }
    //发num张牌，存入cards数组中
    void deliverCard(int num,int cards[]){
        srand((int)time(0));
        for(int i=0;i<num;i++){
            int card;
            while(card_out[card=rand()%52]);
            cards[i]=card;
            card_out[card]=true;
        }
    }
private:
    bool *card_out;
    int tr(int value){
        return value%13;
    }
};

int main(){
    Dila a;
    cout<<"1:deliver card"<<endl;
    cout<<"2:judge pattern"<<endl;
    cout<<"3:pk"<<endl;
    cout<<"0:exit"<<endl;
    while(1){
        int choice;
        cin>>choice;
        switch(choice){
            case 0:return 0;
            case 1:{
                int num,cards[7];
                cin>>num;
                a.deliverCard(num,cards);
                for(int i=0;i<num;i++)cout<<cards[i]<<" ";
                cout<<endl;
                break;
            }
            case 2:{
                int cards[7];
                for(int i=0;i<7;i++)cin>>cards[i];
                cout<<a.judgePattern(cards)<<endl;
                break;
            }
            case 3:{
                int cards_a[7],cards_b[7];
                for(int i=0;i<7;i++)cin>>cards_a[i];
                for(int i=0;i<7;i++)cin>>cards_b[i];
                int t=a.pk(cards_a,cards_b);
                if(t>0)cout<<"A win"<<endl;
                else if(!t)cout<<"Tie"<<endl;
                else cout<<"B win"<<endl;
                break;
            }
        }
    }
    return 0;
}


