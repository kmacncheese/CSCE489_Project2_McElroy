#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore 
{
public:

	Semaphore(int count);
	~Semaphore();

	void wait();
	void signal();

private:
	int S;
	pthread_mutex_t s_mutex;
	pthread_cond_t cond;
};

#endif
