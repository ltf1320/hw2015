#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "dila.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>

enum SimType
{
	SimType_HOLD=0,
	SimType_FLOP=3,
	SimType_TURN=4,
	SimType_RIVER=5
};

class SimRes
{
public:
	int sum,win;
	float rate;
};

class Simulator
{
private:
	bool running;
	bool cvFlag;
	bool stop;
	thread simThread;
	mutex mtx;
	condition_variable cv;
public:
	Simulator()
	{
		running=false;
	}
	int sum,win;
	int targetSum;
	
	void startSim(int* common,int hold1,int hold2,int playerNum,SimType type)
	{
		stopAndGetRes();
		std::lock_guard<std::mutex> lck (mtx);
		sum=win=0;
		stop=false;
		targetSum=-1;
		running=true;
		simThread=move(thread(work,this,common,(int)type,hold1,hold2,playerNum));
		simThread.detach();
	}
	bool isRunning()
	{
		std::lock_guard<std::mutex> lck (mtx);
		return running;
	}
	SimRes stopAndGetRes()
	{
		std::lock_guard<std::mutex> lck (mtx);
		if(!running)
		{
			SimRes res;
			res.win=0;
			res.sum=0;
			res.rate=0;
			return res;
		}
		stop=true;
		SimRes res;
		res.win=win;
		res.sum=sum;
		res.rate=1.0*win/sum;
		return res;
	}
	
	SimRes stopUntilCount(int cnt)
	{	
		std::unique_lock <std::mutex> lck(mtx);
		if(cnt<=sum)
			return stopAndGetRes();
		targetSum=cnt;
		while(!cvFlag)
			cv.wait(lck);
		SimRes res;
		res.win=win;
		res.sum=sum;
		res.rate=1.0*win/sum;
		return res;
	}
protected:
	static void work(Simulator* th,int *common,int commonNum,int hold1,int hold2,int playerNum)
	{
		while(true)
		{
			std::lock_guard<std::mutex> lck (th->mtx);
			if(th->stop)
			{
				th->running=false;break;
			}
			if(th->sum==th->targetSum)
			{
				th->running=false;
				th->cvFlag=true;
				th->cv.notify_all();
				break;
			}

			if(sim(common,commonNum,hold1,hold2,playerNum))
				th->win++;
			th->sum++;
		}
		
	}
	static bool sim(int*common,int commonNum,int hold1,int hold2,int playerNum)
	{
		Dila dila;
		dila.claimCard(commonNum,common);
		dila.claimCard(hold1,hold2);
		dila.deliverCard(5-commonNum,common+commonNum);
		Hand players[10];
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
};




#endif
