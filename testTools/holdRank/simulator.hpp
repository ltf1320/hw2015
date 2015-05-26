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
	SimType_River=5
};

class Simulator
{
	thread simThread;
	mutex mtx;
	condition_variable cv;
	bool cvFlag;
	int sum,win;
	int targetSum;
	bool stop;
	void startSim(int* common,int hold1,int hold2,int playerNum,SimType type)
	{
		sum=win=0;
		stop=false;
		targetSum=-1;
		simThread=move(thread(work,this,common,(int)type,hold1,hold2,playerNum));
		simThread.detach();
	}

	float stopAndGetRes()
	{
		std::lock_guard<std::mutex> lck (mtx);
		stop=true;
		return 1.0*win/sum;
	}
	
	float stopUntilCount(int cnt)
	{	
		std::unique_lock <std::mutex> lck(mtx);
		if(cnt<=sum)
			return stopAndGetRes();
		targetSum=cnt;
		while(!cvFlag)
			cv.wait(lck);
		return 1.0*win/sum;
	}
	static void work(Simulator* th,int *common,int commonNum,int hold1,int hold2,int playerNum)
	{
		while(true)
		{
			std::lock_guard<std::mutex> lck (th->mtx);
			if(th->stop) break;

			if(th->sum==th->targetSum)
			{
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
		bool res=false;
		if(players[0].id==0)
			return true;
		delete [] players;
		return res;
	}
};




#endif
