#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "dila.hpp"
#include <thread>
#include <mutex>

enum SimType
{
	SimType_FLOP=3,
	SimType_TURN=4,
	SimType_River=5
}

class Simulator
{
	thread simThread;
	mutex mtx;
	std::condition_variable cv;
	bool cvFlag;
	int sum,win;
	int targetSum;
	bool stop;
	void startSim(int* common,int hold1,int hold2,int plyaerNum,SimType type)
	{
		sum=win=0;
		stop=false;
		targetSum=-1;
		simThread=move(thread(work,common,(int)type,hold1,hold2,plyaerNum));
	}

	float stopAndGetRes()
	{
		std::lock_guard<std::mutex> lck (mtx);
		stop=true;
		return 1.0*win/sum;
	}
	
	float stopUntilCount(int cnt)
	{
		std::lock_guard<std::mutex> lck (mtx);
		if(cnt<=sum)
			return stopAndGetRes();
		target=cnt;
		cvFlag=false;
		while (!cvFlag)
        	cv.wait(cvFlag);
		return 1.0*wub/sum;
	}
	void work(int *common,int commonNum,int hold1,int hold2,int plyaerNum)
	{
		detach();
		while(true)
		{
			std::lock_guard<std::mutex> lck (mtx);
			if(stop) break;
			if(sum==target)
			{
				cvFlag=true;
				cv.notify_all();
				break;
			}
			if(sim(common,commonNum,hold1,hold2,playerNum))
				win++;
			sum++;
		}
	}
	bool sim(int*common,int commonNum,int hold1,int hold2,int playerNum)
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
}




#endif
