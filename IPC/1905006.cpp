#include<bits/stdc++.h>
#include<pthread.h>
#include<random>
#include<semaphore.h>
#include <unistd.h>

using namespace std;

typedef long long int ll;
#define IDLE 0
#define INQUEUE 1
#define PRINTING 2
#define TOTAL_PRINTING_STATION 4
int reading_count = 0;
int submission_count = 0;
pthread_mutex_t mutex_lock;
sem_t semaphore_binding;
sem_t semaphore_read_write;
vector<sem_t> semaphore_printing(50);
ll starting_time = 0;
int Total_Students , Group_Size;


class Student{
    int id;
    int grp_no;

    public:
    pthread_t thread;
	int printing_time;
    int binding_time;
    int arrival_time;
    int isLeader;
    int writing_time;
    int printing_station_num;
    int state;

    Student(int a , int b){
        id = a;
        grp_no = b;
        isLeader = 0;
        state = IDLE;
        printing_station_num = (id % TOTAL_PRINTING_STATION) + 1;
         
    }

    int getid(){
        return id;
    }

    int getgrp_no(){
        return grp_no;
    }
};

class Staff{
    
    public:
    int id;
    pthread_t thread;
    int waiting_time;
    int reading_time;
    Staff(int a){
        id = a;
    }
    
    int getid(){
        return id;
    }

};

vector<Student> Students;
vector<Staff> Staffs;

void print_Student(){
    for(int i = 0 ; i < Students.size() ; i++){
        cout<< "Student no - "<<Students[i].getid()<<"  -- Grp - "<<Students[i].getgrp_no()<<" - Leader"<<Students[i].isLeader<<endl;
        cout<<"Arri - "<<Students[i].arrival_time<<endl;
        cout<<"Pri - "<<Students[i].printing_time<<endl;
        cout<<"Bind - "<<Students[i].binding_time<<endl;
        cout<<"Writing - "<<Students[i].writing_time<<endl;
        cout<<"Station num - "<<Students[i].printing_station_num<<endl;
    }
}

void print_Staff(){
    for(int i = 0 ; i < 2 ; i++){
        cout<<"ID - "<<Staffs[i].getid();
        cout<<"Wait - "<<Staffs[i].waiting_time<<endl;
        cout<<"Read - "<<Staffs[i].reading_time<<endl;
    }
}

void *StudentFunc(void *arg);

void Semaphore_init(int tot_std){

    pthread_mutex_init(&mutex_lock , 0);
    for(int i = 0 ; i < tot_std ; i++){
        sem_init(&semaphore_printing[i] , 0 , 0);
    }
    sem_init(&semaphore_binding , 0 , 2);
    sem_init(&semaphore_read_write , 0 , 1);

    starting_time = time(NULL);
}

void test_Printing(int id)
{
	
    bool is_raedy_for_printing = 1;
    int temp_printing_station_id = Students[id-1].printing_station_num;
    for(int i = 1 ; i <= Total_Students ; i++){
        if(Students[i-1].state == PRINTING && Students[i-1].printing_station_num == temp_printing_station_id){
            is_raedy_for_printing = 0;
            break;
        }
    }

    if(is_raedy_for_printing && Students[id-1].state == INQUEUE){
        Students[id-1].state = PRINTING;
        sem_post(&semaphore_printing[id-1]);
    }

}


void Enter_Printing(int id){
    pthread_mutex_lock(&mutex_lock);
    Students[id-1].state = INQUEUE;
    test_Printing(id);
    pthread_mutex_unlock(&mutex_lock);
    sem_wait(&semaphore_printing[id-1]);

}

void Exit_Printing(int id){

    int first_member_of_this_group = (id - 1) / Group_Size;
    int last_member_of_this_group = first_member_of_this_group + Group_Size - 1;

    vector<int> groupmates;
    vector<int> other_members;
    int temp = (id - 1 + TOTAL_PRINTING_STATION) % Total_Students;

    for(;temp != id - 1;){
        if(temp >= first_member_of_this_group && temp <= last_member_of_this_group){
            groupmates.push_back(temp);
        }
        else{
            other_members.push_back(temp);
        }
        temp = (temp + TOTAL_PRINTING_STATION) % Total_Students;
    }

    pthread_mutex_lock(&mutex_lock);
    Students[id-1].state = IDLE;
    for(int i = 0 ; i < groupmates.size() ; i++){
        test_Printing(groupmates[i]);
    }
    for(int i = 0; i < other_members.size() ; i++){
        test_Printing(other_members[i]);
    }

    pthread_mutex_unlock(&mutex_lock);

}

void Reader(int id){
    pthread_mutex_lock(&mutex_lock);
    reading_count++;
    if(reading_count == 1){
        sem_wait(&semaphore_read_write);
    }
    pthread_mutex_unlock(&mutex_lock);
    sleep(Staffs[id-1].waiting_time);

    pthread_mutex_lock(&mutex_lock);
    reading_count--;
    if(reading_count == 0){
        sem_post(&semaphore_read_write);
    }
    pthread_mutex_unlock(&mutex_lock);
}

void Writer(int id){
    sem_wait(&semaphore_read_write);
    sleep(Students[id-1].writing_time);
    sem_post(&semaphore_read_write);
}

void* Student_func(void* arg){
   
    int std_id = *(int* ) arg;
    sleep(Students[std_id - 1].arrival_time);
    cout<<"Student "<<std_id<<" has arrived at the print station at time "<<time(NULL) - starting_time<<endl;
    Enter_Printing(std_id);
    sleep(Students[std_id-1].printing_time);
    cout<<"Student No. "<<std_id<<" has finished printing at time "<<time(NULL) - starting_time<<endl;
    Exit_Printing(std_id);
    
    if(Students[std_id-1].isLeader == 1){
        int temp = Group_Size - 1;
        int tmp2 = std_id - 1;
        while(temp != 0){
            pthread_join(Students[tmp2-1].thread , NULL);
            temp--;
        }

        cout<<"Group No. "<<Students[std_id-1].getgrp_no()<<" has finished printing at time "<<time(NULL) - starting_time<<endl;

        sem_wait(&semaphore_binding);
        cout<<"Group No. "<<Students[std_id-1].getgrp_no()<<" has started binding at time "<<time(NULL) - starting_time<<endl;
        sleep(Students[std_id-1].binding_time);
        sem_post(&semaphore_binding);
        cout<<"Group No. "<<Students[std_id-1].getgrp_no()<<"  has finished binding at time "<<time(NULL) - starting_time<<endl;

        Writer(std_id);
        pthread_mutex_lock(&mutex_lock);
        submission_count++;
        pthread_mutex_unlock(&mutex_lock);
        cout<<"Group No. "<<Students[std_id-1].getgrp_no()<<" has submitted the report at time "<<time(NULL) - starting_time<<endl;
    }
    pthread_exit(NULL);
}


void* Staff_func(void* arg){
    while(true){
        int staff_id =*(int *) arg;
        sleep(Staffs[staff_id - 1].waiting_time);
        cout<<"Staff "<< staff_id<<"  has started reading the entry book at time "<<(time(NULL) - starting_time);
        cout<<". No. of submissions = "<<submission_count<<endl<<endl;
        sleep(Staffs[staff_id - 1].reading_time);
        
        pthread_mutex_lock(&mutex_lock);
        if(submission_count == Total_Students / Group_Size){
            pthread_mutex_unlock(&mutex_lock);
            break;
        }
        pthread_mutex_unlock(&mutex_lock);

        Staffs[staff_id - 1].waiting_time = rand() % 3 + 3;

    }
    pthread_exit(NULL);
}



int main(){
    int student_num , group_size;
    int printing_time , binding_time , read_write_time;
    
    freopen("input.txt", "r", stdin);
    
    cin>>student_num>>group_size>>printing_time>>binding_time>>read_write_time;
    Total_Students = student_num;
    Group_Size = group_size;

	freopen("output.txt", "w", stdout);
    
    //cout<<student_num<<group_size<<printing_time<<binding_time<<read_write_time;
    
    for(int i = 1 ; i <= student_num ; i++){
        int id = i;
        int grp_no = (i-1)/5 + 1;
        Student temp(id , grp_no);
        if(i%group_size == 0)
            temp.isLeader = 1;
        Students.push_back(temp);
    }
    int arrival_time = 5;
    random_device rd;
    mt19937 generator(rd()); // Mersenne Twister engine
    poisson_distribution<int> a(arrival_time),p(printing_time),b(binding_time),rw(read_write_time); 
    for(int i = 1 ; i <= student_num ; i++){
         Students[i-1].arrival_time = a(generator);
         Students[i-1].printing_time = p(generator);
         if(i % group_size == 0){
            Students[i-1].binding_time = b(generator);
            Students[i-1].writing_time = rw(generator);
         }
         
    }
    Staff staff1(1);
    Staff staff2(2);
    staff1.waiting_time = 3;
    staff2.waiting_time = 4;
    staff1.reading_time = rw(generator);
    staff2.reading_time = rw(generator);

    Staffs.push_back(staff1);
    Staffs.push_back(staff2);

    //print_Student();
    //print_Staff();

    Semaphore_init(student_num);

    int address[student_num];
    for(int i = 0 ; i < student_num ; i++){
        address[i] = i+1;
    }

	for(int i = 0 ; i < student_num ; i++){
        pthread_create(&(Students[i].thread) , NULL , Student_func , (void* )&(address[i]));
    }

    pthread_t thread_of_Staff[2];
    pthread_create(&thread_of_Staff[0] , NULL , Staff_func , (void* )&Staffs[0].id);
    pthread_create(&thread_of_Staff[1] , NULL , Staff_func , (void* )&Staffs[1].id);


    for(int i = 0 ; i < student_num ; i++){
        if(Students[i].isLeader == 1){
            pthread_join(Students[i].thread , NULL);
        }
    }

    for(int i = 0 ; i < semaphore_printing.size() ; i++){
        sem_destroy(&semaphore_printing[i]);
    }
    sem_destroy(&semaphore_binding);
    sem_destroy(&semaphore_read_write);

    return 0;
}