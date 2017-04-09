/** 
 * The code is released under GPL v2 
 */

// Cross Platform Timer
// Adapted from Boost Library

#ifndef CPTIMER_H
#define CPTIMER_H



#ifdef _MSC_VER

#include <ctime>

#else

#include <stdlib.h>

//for timing under Linux
#include <sys/param.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>

#endif

class CPTimer
{
#ifdef _MSC_VER	
	// Windows visual studio version
public:
	CPTimer()
	{ 
		_start_time = std::clock(); 
		_elapsed = 0.0;
		paused = false;

	}
	// postcondition: elapsed()==0
	
	void   start()
	{ 
		_start_time = std::clock(); 
		_elapsed = 0.0;
		paused = false;

	} // post: elapsed()==0
	
	void   restart()
	{
		 _start_time = std::clock();
		 _elapsed = 0.0;
		 paused = false;

	} // post: elapsed()==0
	
	void resume()
	{
		if(paused)
		{
			_start_time = std::clock(); 
			paused = false;
		}
			
	}
		
	void pause()
	{
		if(!paused)
		{
			_elapsed += lastLap();
			paused = true;
		}
	}
	double elapsed() const
	{
		if(paused)
		{
			return _elapsed;
		}
		else
		{
			return _elapsed + lastLap();
		}
	}

	double lastLap() const                  // return elapsed time in seconds
	{
		return  double(std::clock() - _start_time) / CLOCKS_PER_SEC; 
	}

	
private:
	std::clock_t _start_time;
	double _elapsed;
	bool	paused;
	
#else
	// Linux version
	public:
		CPTimer()
		{ 
			times(&_start_time);
			_elapsed = 0.0;
			paused = false;
		}
	// postcondition: elapsed()==0
	
		void   start()
		{ 
			times(&_start_time);
			_elapsed = 0.0;
			paused = false;
		} // post: elapsed()==0
	
		void   restart()
		{
			times(&_start_time);
			_elapsed = 0.0;
			paused = false;
		} // post: elapsed()==0
		
		void resume()
		{
			if(paused)
			{
				times(&_start_time);
				paused = false;
			}
			
		}
		
		void pause()
		{
			if(!paused)
			{
				_elapsed += lastLap();
				paused = true;
			}
		}
		double elapsed() const
		{
			if(paused)
			{
				return _elapsed;
			}
			else
			{
				return _elapsed + lastLap();
			}
		}
		double lastLap() const                  // return elapsed time in seconds
		{
			struct tms now;
			times(&now);
			long int clk_tck = sysconf(_SC_CLK_TCK);
			double utime = ((double)(now.tms_utime-_start_time.tms_utime))/clk_tck;
			double stime = ((double)(now.tms_stime-_start_time.tms_stime))/clk_tck;
			return  utime+stime;
		}
	
	private:
		struct tms _start_time;
		double _elapsed;
		bool	paused;
#endif
}; // timer


#endif  // CPTIMER_H

