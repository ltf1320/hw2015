#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <map> 
#include <signal.h>

#include "ThreadSafeQueue.h"
#include "../dila.hpp"
using namespace std;

const char eol='\n';

FILE* logFile;

#define LOG(msg,arg...) fprintf(logFile,msg,##arg);fprintf(logFile,"\n");
//#define LOG(msg,arg...) printf("[%d]",my_id);printf(msg,##arg);puts("");

#define LOOP_MSG_UNTIL(endMsg) for(char* msg=getNextMsg();strcmp(msg,endMsg)||((delete [] msg),0);delete [] msg,msg=getNextMsg())

int HandCount=0;


//deal with kill,save the log
void killhandler(int sig)  
{  
	 switch (sig)
     {
     case SIGINT:
     case SIGTERM:
		LOG("ABORTED");
       	fclose(logFile);
		exit(0);
     	break;
     }
}

int my_id;

namespace Action
{
	const char* all_in="all_in";
	const char* check="check";
	const char* blind="blind";
	const char* call="call";
	const char* raise="raise";
	const char* fold="fold";
}

//the color of cards
namespace Color
{
	const char* SPADES="SPADES";
	const char* HEARTS="HEARTS";
	const char* CLUBS="CLUBS";
	const char* DIAMONDS="DIAMONDS";
};

namespace NutHand
{
	const char* HIGH_CARD="HIGH_CARD";
	const char* ONE_PAIR="ONE_PAIR";
	const char* TWO_PAIR="TWO_PAIR";
	const char* THREE_OF_A_KIND="THREE_OF_A_KIND";
	const char* STRAIGHT="STRAIGHT";
	const char* FLUSH="FLUSH";
	const char* FULL_HOUSE="FULL_HOUSE";
	const char* FOUR_OF_A_KIND="FOUR_OF_A_KIND";
	const char* STRAIGHT_FLUSH="STRAIGHT_FLUSH";
	map<string,cardPattern> mp;
	cardPattern getPattern(const char* desc)
	{
		string tmp(desc);
		if(mp.count(tmp)>0)
		{
			return mp[tmp];
		}
		LOG("get pattern faild:%s",desc);
		return UNKOWN_PATTERN;
	}
	void initMp()
	{
		NutHand::mp[string(NutHand::HIGH_CARD)]=::HIGH_CARD;
		NutHand::mp[string(NutHand::ONE_PAIR)]=::ONE_PAIR;
		NutHand::mp[string(NutHand::TWO_PAIR)]=::TWO_PAIR;
		NutHand::mp[string(NutHand::THREE_OF_A_KIND)]=::THREE_OF_A_KIND;
		NutHand::mp[string(NutHand::STRAIGHT)]=::STRAIGHT;
		NutHand::mp[string(NutHand::FLUSH)]=::FLUSH;
		NutHand::mp[string(NutHand::FULL_HOUSE)]=::FULL_HOUSE;
		NutHand::mp[string(NutHand::FOUR_OF_A_KIND)]=::FOUR_OF_A_KIND;
		NutHand::mp[string(NutHand::STRAIGHT_FLUSH)]=::STRAIGHT_FLUSH;
	}
}

struct Card
{
	char color;
	int point;
	void getCard(char* cardDesc)
	{
		char tmp[10];
		char pnt[10];
		sscanf(cardDesc,"%s %s",tmp,pnt);
		getCard(tmp,pnt);
	}
	void getCard(const char* co,const char* po)
	{
		color=co[0];
		switch(po[0])
		{
			case 'A':point=14;break;
			case 'K':point=13;break;
			case 'Q':point=12;break;
			case 'J':point=11;break;
			default:sscanf(po,"%d",&point);break;
		}
	}
	void logCard()
	{
		LOG("%c %d",color,point);
	}
	int getId()
	{
		int res;
		switch(color)
		{
			case 'S':res=0*13;break;
			case 'H':res=1*13;break;
			case 'C':res=2*13;break;
			case 'D':res=3*13;break;
			default:LOG("Error:Unkown Color:%c",color);break;
		}
		res+=point-2;
		return res;
	}
	void getCard(int id)
	{
		point=id%13+2;
		switch(id/13)
		{
			case 0:color='S';break;
			case 1:color='H';break;
			case 2:color='C';break;
			case 3:color='D';break;
			default:LOG("Error:Unkown Color(in id):%d",id/13);break;
		}
	}
};

enum PlayerState
{
	PlayerState_ALL_IN,
	PlayerState_JOIN,
	PlayerState_FOLDED,
	PlayerState_GAME_OVER
};

enum TurnState
{
	TurnState_START,
	TurnState_HOLD,
	TurnState_FLOP,
	TurnState_TURN,
	TurnState_RIVER,
	TurnState_SHOWDOWN
};

class Player
{
public:
	PlayerState state;
	int jetton;
	int monney;
	int bet;
	int pid;
	void dobet(int num)
	{
		if(num>jetton)
			num=jetton;
		jetton-=num;
		bet+=num;
	}
};

class Game
{
	public:
	map<int,Player> players;
	Player me;
	TurnState turnState;
	Card hold[2];
	Card flop[3];
    Card turn;
	Card river;
	int common[5];
	int pot;
	Player* getPlayer(int pid)
	{
		if(pid==me.pid)
			return &me;
		else return &players[pid];
	}
	int* getCommon()
	{
		for(int i=0;i<3;i++)
			common[i]=flop[i].getId();
		common[3]=turn.getId();
		common[4]=river.getId();
		return common;
	}
};

class PlayerResult
{
public:
	Card hold[2];
	char NutHand[20];
	Player player;
	int win;
	int rank;
};

class GameResult
{
public:
	Game game;
	map<int,PlayerResult> result;
	void checkResult()
	{
		game.getCommon();
		for(auto iter=result.begin();iter!=result.end();++iter)
		{
			Hand h;
			h.common(game.common);
			h.hold(iter->second.hold[0].getId(),iter->second.hold[1].getId());
			h.getPattern();
			if(h.pattern!=NutHand::getPattern(iter->second.NutHand))
			{
				LOG("error pattern:");
				LOG("Hand:%d",HandCount);
				LOG("pid:%d",iter->second.player.pid);
				LOG("cards:");
/*				
				for(int i=0;i<7;i++)
				{
					LOG("%d ",h.cards[i]);
					Card tmp;
					tmp.getCard(h.cards[i]);
					tmp.logCard();
				}

				LOG("------------");*/
				for(int i=0;i<3;i++)
				{
					game.flop[i].logCard();
					LOG("%d",game.flop[i].getId());
				}
				game.turn.logCard();
				game.river.logCard();
				iter->second.hold[0].logCard();
				iter->second.hold[1].logCard();
				
				LOG("server pattern:%d",NutHand::mp[string(iter->second.NutHand)]);
				LOG("Dila pattern:%d",h.pattern);
			}
		}
	}
};




class MessageHandle
{
	public:
		MessageHandle(int sock)
		{
			th=thread(handleMessageThread,this);
			this->sock=sock;
		}
		int recvMessage(const char *buffer,int len)
		{
			char* tmp=new char[len+1];
			strcpy(tmp,buffer);
			msgQue.push(tmp);
			if(!strcmp(tmp,"game-over"))
				return -1;
			return 0;
		}
		void waitForEnd()
		{
			th.join();
		}
	protected:
		int sock;
		Game game;

		void sendAction(const char* action)
		{
			LOG("Action:%s",action);
			send(sock,action,strlen(action),0);
			send(sock," \n",2,0);
		}
		char* getNextMsg()
		{
			while(true){
				pair<bool,char*> r=msgQue.popFront();
				this_thread::sleep_for(std::chrono::milliseconds(10));
				if(r.first)
				{
//					LOG("handle:%s",r.second);
					return r.second;
				}
			}
		}
		static void handleMessageThread(MessageHandle *p)
		{
			while(true){
				char *msg=p->getNextMsg();
				//LOG("handle:%s",msg);
				if(!strcmp(msg,"inquire/"))
					p->handleInquire();
				if(!strcmp(msg,"seat/"))
					p->handleSeat();
				if(!strcmp(msg,"blind/"))
					p->handleBlind();
				if(!strcmp(msg,"hold/"))
					p->handleHold();
				if(!strcmp(msg,"flop/"))
					p->handleFlop();
				if(!strcmp(msg,"turn/"))
					p->handleTurn();
				if(!strcmp(msg,"river/"))
					p->handleRiver();
				if(!strcmp(msg,"showdown/"))
					p->handleShowdown();
				if(!strcmp(msg,"pot-win/"))
					p->handlePotWin();
				if(!strcmp(msg,"notify/"))
					p->handleNotify();
				if(!strcmp(msg,"game-over"))
				{
					LOG("GAMEOVER RETURN");
					break;
				}
				delete [] msg;
			}
		}
		void handleSeat(){
		//	LOG("handle seat");
			game.players.clear();
			game.turnState=TurnState_START;
		
			LOOP_MSG_UNTIL("/seat"){
				char *p=msg;
				int pid,jetton,monney;
				while(*p!=':')
				{
					p++;
					if(*p==0)
					{
						p=msg-2;
						break;
					}
				}
				p+=2;
				sscanf(p,"%d %d %d",&pid,&jetton,&monney);			
				//LOG("pid:%d,jetton:%d,monney:%d",pid,jetton,monney);
				if(pid==my_id)
				{
					game.me.jetton=jetton;
					game.me.monney=monney;
					game.me.state=PlayerState_JOIN;
				}
				else
				{
					Player player;
					player.pid=pid;
					player.jetton=jetton;
					player.monney=monney;
					player.state=PlayerState_JOIN;
					game.players[pid]=player;
				}
			}
		//	LOG("handle seat end");
		}
		void handleBlind(){
		//	LOG("handle blind");
			LOOP_MSG_UNTIL("/blind")
			{
				int pid,bet;
				sscanf(msg,"%d: %d",&pid,&bet);
				game.getPlayer(pid)->dobet(bet);
			}
		//	LOG("handle blind end");
		}
		void handleHold(){
			char* msg=getNextMsg();
			game.hold[0].getCard(msg);
			delete [] msg;
			msg=getNextMsg();
			game.hold[1].getCard(msg);
			delete [] msg;
			msg=getNextMsg();
			delete [] msg;
		//	LOG("hold");
		}
		void handleFlop(){
			char* msg;
			for(int i=0;i<3;i++)
			{
				msg=getNextMsg();
				game.flop[i].getCard(msg);
				delete [] msg;				
			}
			msg=getNextMsg();
			delete [] msg;
		}
		void handleTurn(){
			char *msg=getNextMsg();
			game.turn.getCard(msg);
			delete [] msg;
			msg=getNextMsg();
			delete [] msg;
		}
		void handleRiver(){
			char *msg=getNextMsg();
			game.river.getCard(msg);
			delete [] msg;
			msg=getNextMsg();
			delete [] msg;
		}
		void handleShowdown(){
			LOOP_MSG_UNTIL("/common")
			{
	//			LOG(msg);
			}
	
		//	LOG("common");
		//	for(int i=0;i<3;i++) game.flop[i].logCard();
		//	game.turn.logCard();
		//	game.river.logCard();
		//	LOG("/common");

			GameResult result;
			result.game=game;
			LOOP_MSG_UNTIL("/showdown")
			{
				int rank,pid;
				PlayerResult pres;
				char ct1[10],ct2[10],pt1[10],pt2[10];
				sscanf(msg,"%d:%d %s %s %s %s %s",&rank,&pid,ct1,pt1,ct2,pt2,pres.NutHand);
				pres.hold[0].getCard(ct1,pt1);
				pres.hold[1].getCard(ct2,pt2);
				pres.player=*(game.getPlayer(pid));
				result.result[pid]=pres;
			}
			result.checkResult();
			LOG("result Checked");
			HandCount++;
		}
		void handlePotWin(){
			LOOP_MSG_UNTIL("/pot-win")
			{
				int pid,num;
				sscanf(msg,"%d %d",&pid,&num);
			}
		}
		void handleInquire()
		{
			//LOG("handle inquire");
			LOOP_MSG_UNTIL("/inquire")
			{
				if(msg[0]!='t')
				{
					int pid,jetton,monney,bet;
					char act[10];
					sscanf(msg,"%d %d %d %d %s",&pid,&jetton,&monney,&bet,act);
					
					Player* np=game.getPlayer(pid);
					np->jetton=jetton;
					np->monney=monney;
					np->bet=bet;
					if(!strcmp(act,Action::all_in))
						np->state=PlayerState_ALL_IN;
					if(!strcmp(act,Action::fold))
						np->state=PlayerState_FOLDED;

			//		LOG("pid:%d jetton:%d monney:%d bet:%d",pid,game.getPlayer(pid)->jetton,game.getPlayer(pid)->monney,game.getPlayer(pid)->bet)
				}
				else
				{
					sscanf(msg,"total pot:%d",&game.pot);
				}
			}
			sendAction(Action::call);
			/*
			int allinCnt=0;
			for(auto iter=game.players.begin();iter!=game.players.end();iter++)
				allinCnt++;
			if(allinCnt>=4)
				sendAction(Action::fold);
			else{
			if(game.hold[0].point==1||game.hold[1].point==1||game.hold[0].point==game.hold[1].point)
				sendAction(Action::all_in);
			else
				sendAction(Action::fold);
			}
		//	LOG("handle inquire end");
			*/
		}
		void handleNotify()
		{
			LOOP_MSG_UNTIL("/notify")
			{
				if(msg[0]!='t')
				{
					int pid,jetton,monney,bet;
					char act[10];
					sscanf(msg,"%d %d %d %d %s",&pid,&jetton,&monney,&bet,act);
					
					Player* np=game.getPlayer(pid);
					np->jetton=jetton;
					np->monney=monney;
					np->bet=bet;
					if(!strcmp(act,Action::all_in))
						np->state=PlayerState_ALL_IN;
					if(!strcmp(act,Action::fold))
						np->state=PlayerState_FOLDED;
				}
				else
				{
					sscanf(msg,"total pot:%d",&game.pot);
				}
			}
		}

		
		ThreadSafeQueue<char*> msgQue;
		thread th;
};


void init()
{
	signal(SIGTERM, killhandler);  
    signal(SIGINT, killhandler); 
	NutHand::initMp();
}

int main(int argc,char**argv){
	 
	init();
	if (argc != 6)
	{
		LOG("Usage: ./%s server_ip server_port my_ip my_port my_id\n", argv[0]);
		return -1;
	}

	/* 获取输入参数 */
	in_addr_t server_ip = inet_addr(argv[1]);
	in_port_t server_port = htons(atoi(argv[2])); 
	in_addr_t my_ip = inet_addr(argv[3]);
	in_port_t my_port = htons(atoi(argv[4])); 
	my_id = atoi(argv[5]);
	logFile=fopen(argv[5],"w");
	/* 创建socket */
	int m_socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if(m_socket_id < 0)
	{
		LOG("init socket failed!\n");
		return -1;
	}

	/* 设置socket选项，地址重复使用，防止程序非正常退出，下次启动时bind失败 */
	int is_reuse_addr = 1;
	setsockopt(m_socket_id, SOL_SOCKET, SO_REUSEADDR, (const char*)&is_reuse_addr, sizeof(is_reuse_addr));

	/* 绑定指定ip和port，不然会被server拒绝 */
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = my_ip;
	my_addr.sin_port = my_port;
	if(::bind(m_socket_id, (struct sockaddr*)&my_addr, sizeof(my_addr)))
	{
		LOG("bind failed! %s:%d\n",argv[3],my_port); 
		return -1;
	}

	/* 连接server */
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = server_ip;
	server_addr.sin_port = server_port;
	while(connect(m_socket_id, (struct sockaddr*)&server_addr, sizeof	(server_addr)) < 0)
	{
		usleep(100*1000); /* sleep 100ms, 然后重试，保证无论server先起还是后起，都不会	有问题 */
	}
	//LOG("connected to server");
	/* 向server注册 */
	char reg_msg[50] = {'\0'};
	snprintf(reg_msg, sizeof(reg_msg) - 1, "reg: %d %s need_notify \n", my_id, "tt"); 
	send(m_socket_id, reg_msg, strlen(reg_msg) + 1, 0);
	LOG("connected to server");
	MessageHandle msgh(m_socket_id);
	//LOG("start enter game");
	/* 接收server消息，进入游戏 */
	char buffer[1024] = {'\0'};
	int nowBuffer=0;
	while(1)
	{
		int length = recv(m_socket_id, buffer+nowBuffer, sizeof(buffer) - nowBuffer, 0);
		//LOG("recv message:%s\n",buffer+nowBuffer);
		if(length > 0)
		{ 
			char *start=buffer;
			length+=nowBuffer;
			nowBuffer=0;
			while(start<buffer+length){
				int len=0;
				bool flag=false;
				for(;start+len<=buffer+length;len++)
				{
					if(*(start+len)==eol)
					{
						*(start+len-1)=0;
						if(msgh.recvMessage(start,len-1)==-1)
						{
							msgh.waitForEnd();
							fclose(logFile);
							/* 关闭socket */
							close(m_socket_id);
							return 0;
						}
						start+=len+1;
						flag=true;
						break;
					}
				}
				if(flag) continue;
				if(start+len>buffer+length)
				{
					while(start<buffer+length)
					{
						buffer[nowBuffer++]=*start;
						start++;
					}
					break;
				}
			}
		} 
	}
	fclose(logFile);
	/* 关闭socket */
	close(m_socket_id);
	
	return 0;
}
