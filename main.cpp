
#include<iostream>
#include<ctime>
#include<unistd.h>
#include<semaphore>
#include<cstdlib>
#include<chrono>
#include<thread>
using namespace std;

class CircleQ{
	private:
		int size;
		int* list;
		int front;
		int count;
	public:
		CircleQ(int);
		~CircleQ();
		int getCount();
		int get(int index);
		bool put(int value);
		int pop();
		void display();
};

counting_semaphore<>* space = new counting_semaphore<>(1);
counting_semaphore<>* item = new counting_semaphore<>(0);
counting_semaphore _mutex(1);
//counting_semaphore _iomutex(1);
CircleQ* q;

/*********************** Circular Queue Class ***************************/


CircleQ::CircleQ(int _size): size(_size), front(0), count(0){
	list = new int[size];
}

CircleQ::~CircleQ(){
	delete[] list;
}


bool CircleQ::put(int value){
	if(count == size){
		cerr << " THIS SHOULD HAVE NEVER HAPPENED. ";
		cerr << " PUT CALLED WHEN QUEUE IS FULL. " << endl;
	}

	count++;
	list[(front+count-1)%size] = value;
	return true;
}

int CircleQ::pop(){
	if(count == 0){
		cerr << " THIS SHOULD HAVE NEVER HAPPENED. ";
		cerr << " POP CALLED WHEN QUEUE IS EMPTY." << endl;
		return 1;
	}
	int ret = list[front];
	front = (front+1)%size;
	count--;
	return ret;
}

void CircleQ::display(){
	char end = '/';
	for(int ind=0; ind < size; ind++){
		if(ind == size-1)
			end = '|';
		if(ind < count){
			cout << list[(front+ind)%size] << end;
		}else{
			cout << "." << end;
		};
	}
	cout << endl;
}


void produce(char id, int num_jobs){
	srand(static_cast<unsigned int>(time(nullptr)));
	string giveUpMessage = "Producer a has given up.\n";
	giveUpMessage[9] = id;

	while(num_jobs > 0){
		int val = rand()%10 + 1;
		if(space->try_acquire_for(chrono::seconds(10))){
			_mutex.acquire();
			q->put(val);
			_mutex.release();
			item->release();
			num_jobs--;
		}else{
			fprintf(stderr, giveUpMessage.c_str());
			break;
		}
	}
	string endMessage = "Producer a has finished. Jobs abandoned: 00\n";
	endMessage[9] = id;
	endMessage[41] += (num_jobs%100)/10;
	endMessage[42] += num_jobs%10;
	fprintf(stderr, endMessage.c_str());
}

void consume(char id){
	int num_jobs = 0;
	int givenUp = false;

	while(true){
		if(item->try_acquire_for(chrono::seconds(10))){
			_mutex.acquire();
			int ans = q->pop();
			_mutex.release();
			space->release();
			num_jobs++;
			sleep(ans);
		}else{
			givenUp = true;
			break;
		}
	}
	if(givenUp){
		string giveUpMessage = "Consumer a has given up.\n";
		giveUpMessage[9] = id;
		fprintf(stderr, giveUpMessage.c_str());
	}
	string endMessage = "Consumer a has finished. Jobs consumed: 00\n";
	endMessage[9] = id;
	endMessage[40] += (num_jobs%100)/10;
	endMessage[41] += num_jobs%10;
	fprintf(stderr, endMessage.c_str());
}


int main(int argc, char* argv[]) {
	if(argc != 5){
		cerr << "Expecting 4 integer arguments after calling. " << endl;
		return -1;
	}
	const int qsize = atoi(argv[1]);
	int numjobs, numprod, numcons;
	numjobs = atoi(argv[2]);
	numprod = atoi(argv[3]);
	numcons = atoi(argv[4]);
	q = new CircleQ(qsize);

	delete space;
	space = new counting_semaphore<>(qsize);

	char prodname = 'A';
	thread producers[numprod];
	for(int i = 0; i < numprod; i++){
		producers[i] = thread(produce, prodname, numjobs);
		prodname++;
	}

	char consname = 'a';
	thread consumers[numcons];
	for(int i = 0; i < numcons; i++){
		consumers[i] = thread(consume, consname);
		consname++;
	}


	for(int i = 0; i < numprod; i++){
		producers[i].join();
	}
	for(int i = 0; i < numcons; i++){
		consumers[i].join();
	}

	q->display();
	delete space;
	delete item;
	delete q;
	return 0;
}

