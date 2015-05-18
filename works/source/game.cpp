#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "ThreadSafeQueue.h"
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <vector>

using namespace std;

const char eol='\n';

int my_id;

FILE* logFile;

#define LOG(msg,arg...) fprintf(logFile,msg,##arg);fprintf(logFile,"\n");
//#define LOG(msg,arg...) printf("[%d]",my_id);printf(msg,##arg);puts("");

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
}

struct Card
{
	char* color;
	int point;
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
};

class Game
{
	public:
	vector<Player> players;
	Player me;
	TurnState turnState;
	Card hold[2];
	Card flop[3];
	Card turn;
	Card river;
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
					return r.second;
				}
			}
		}
		static void handleMessageThread(MessageHandle *p)
		{
			while(true){
				char *msg=p->getNextMsg();
				LOG("handle:%s",msg);
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
				if(!strcmp(msg,"game-over"))
				{
					LOG("GAMEOVER RETURN");
					break;
				}
				delete [] msg;
			}
		}
		void handleSeat(){
			LOG("handle seat");
			game.players.clear();
			game.turnState=TurnState_START;
			char* msg=getNextMsg();
			while(strcmp(msg,"/seat")){
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
				sscanf(msg,"%d %d %d",&pid,&jetton,&monney);
				LOG("pid:%d,jetton:%d,monney:%d",pid,jetton,monney);
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
					game.players.push_back(player);
				}
			}
		}
		void handleBlind(){}
		void handleHold(){}
		void handleFlop(){}
		void handleTurn(){}
		void handleRiver(){}
		void handleShowdown(){}
		void handlePotWin(){}
		void handleInquire()
		{
			while(true){
				char *msg=getNextMsg();
				if(!strcmp(msg,"/inquire"))
				{
				//	if(game.state==JOIN)
					{
						sendAction(Action::all_in);
						game.me.state=PlayerState_ALL_IN;
					}
					delete [] msg;
					return;
				}
				delete [] msg;
			}
		}
		ThreadSafeQueue<char*> msgQue;
		thread th;
};



int main(int argc,char**argv){
	if (argc != 6)
	{
		printf("Usage: ./%s server_ip server_port my_ip my_port my_id\n", argv[0]);
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
	LOG("bind failed!\n"); 
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
	snprintf(reg_msg, sizeof(reg_msg) - 1, "reg: %d %s \n", my_id, "tt"); 
	send(m_socket_id, reg_msg, strlen(reg_msg) + 1, 0);
	LOG("connected to server");
	MessageHandle msgh(m_socket_id);
	LOG("start enter game");
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
