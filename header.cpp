#include "stdio.h"
#include "iniparser.h"
#include "string"
#include "stdint.h"
#include "list"
#include "string"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <sys/stat.h>
#include <openssl/md5.h>
#include "math.h"
#include "openssl/sha.h" /* please read this */
#include <sstream>
#include "vector"
#include "map"
#include "queue"
#define BACKLOG 10     // how many pending connections queue will hold
#ifndef min
#define min(A,B) (((A)>(B)) ? (B) : (A))
#endif /* ~min */
using namespace std; 

/* ----------------------------------------------------------------*/
/*---------------------INI FILE PARAMETERS-------------------------*/
/* ----------------------------------------------------------------*/
char LogFilename[512]="servant.log";
char Port[5];
char hostname[]="nunki.usc.edu";
char HomeDir[512];
uint32_t Location;
int AutoShutdown=900;// In seconds
int TTL=30;
int MsgLifetime=30;
int InitNeighbors=3;
int JoinTimeout=15;
int KeepAliveTimeout=60;
int MinNeighbours=2;
int NoCheck;
double CacheProb=0.1;
double StoreProb=0.1;
double NeighborStoreProb=0.2;
int CacheSize=500; // In KiloBytes
int Retry=15;// In Seconds
int notfound;
int reset = 0;
int file_index_counter = 1;
char initfilename[50];
/* ----------------------------------------------------------------*/

/* ----------------------------------------------------------------*/
/*---------------------Structures----------------------------------*/
/* ----------------------------------------------------------------*/
class Nam
{
FILE *fps;
struct neighbours
{
int host;
int neighbour;
};
public:
vector<neighbours> links[256];
vector<int > node[256];
string fileName[256];
int curr_file;
public:
int setFileName(string filename);
void populate(char *msg,int type,int file_to_write,int mode);
bool nodeIsPresent(int,int);
void writetofile(int file_to_write,char *msg);
void initConnections(int);
} name;

class LRU
{
	struct entry
{
	int index;
	int size;
} ;
vector<entry> q;
int maxsize;
int currentsize;
public:
void init(int maxsizes);
void deletion(int index);

bool removeFile(char type[100],int index);
void hit(int index);
bool removeIndexAndFile(int index,int type);

bool add(int index,int size);
void display()
	{
for(int i=0;i<(int)q.size();i++)
	cout<<"index"<<q[i].index<<"file size"<<q[i].size<<endl;

	}
} lru;
struct activeparam
{
pthread_t read;
pthread_t write;
pthread_t self;
int port;
};

struct searchresponse
{
string fileid;
string mdata;
};

struct soc
{
int port;
int sockfd;
int global_variable;
};

struct timeparam
{
int type;
int value;
string index;
};

struct write_cond_buf
{
int port;
pthread_cond_t *write;
};

struct event
{
pthread_cond_t *disp;
};

struct LocDistStruct
{
   uint16_t port;
   char* hostname;
   uint32_t distance;
};
/* ----------------------------------------------------------------*/



/*-------------------------------Regular Variables------------------*/
char nodeid[18];
char *nodeinstanceid;
FILE *fp;
//number of neighbors that have sent hello back
int neighbour_joined;
int non_beacon_join_timeout = 1;
int auto_co_index=1;
int retry_co_index=1;
int event_shutdown=1;
int shutdowns=1; //Global Variable for handling auto shutdown
/*------------------------------------------------------------------*/




/* ----------------------------------------------------------------*/
/*---------------------Redundant-----------------------------------*/
/* ----------------------------------------------------------------*/
list<string>::iterator iter;
map<int,pthread_cond_t *>::iterator it;
map<int,int>::iterator its;
map<int,int>::iterator it2; // For Retry
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Data Structures-----------------------------*/
/* ----------------------------------------------------------------*/
vector<string> beacons;
vector<activeparam> activeconnections;
vector<timeparam> time_buffer;
std::queue<event> event_dispatcher;
map<int,int> retry_co_map;
map<int,int> auto_co_map;
map<string,string> msg_buffer;
map<string,int> msg_UOID_port_map;
map<string,int> msg_UOID_status_msg_type_map;
map<string,int> msg_UOID_status_filename_map;
map<int,list<string> > write_buffer;
vector<activeparam> tempconnections;
vector<struct LocDistStruct *> neighbors; //vector for neighbors received from join msg req
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Global Condition Variables------------------*/
/* ----------------------------------------------------------------*/
pthread_cond_t event_dispatcher_cond=PTHREAD_COND_INITIALIZER;
pthread_cond_t auto_co[512],retry_co[512];
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Global Locks--------------------------------*/
/* ----------------------------------------------------------------*/
pthread_mutex_t act = PTHREAD_MUTEX_INITIALIZER; // Lock for active connection list
pthread_mutex_t time_buffer_lock=PTHREAD_MUTEX_INITIALIZER; // Lock for the Time Buffer
pthread_mutex_t retry_lock=PTHREAD_MUTEX_INITIALIZER; // Lock for Retry
pthread_mutex_t event_dispatcher_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t auto_co_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_buffer_mutex=PTHREAD_MUTEX_INITIALIZER;//Lock for write buffer 
pthread_mutex_t msg_UOID_port_map_mutex=PTHREAD_MUTEX_INITIALIZER;//Lock for UOID-PORT map 
pthread_mutex_t msg_UOID_status_filename_map_mutex=PTHREAD_MUTEX_INITIALIZER;;
pthread_mutex_t msg_UOID_status_msg_type_map_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t msg_buffer_mutex=PTHREAD_MUTEX_INITIALIZER;//Lock for msg buffer 
/* ----------------------------------------------------------------*/



/* ----------------------------------------------------------------*/
/*---------------------Thread Variables----------------------------*/
/* ----------------------------------------------------------------*/
pthread_t serv, cli[40],c_serv[40];
pthread_t event_dispatcher_t;
pthread_t timer_tt;
pthread_t ui;
/* ----------------------------------------------------------------*/

/*------------------Files--------------------------------------------*/
map<int,string> file_index;
map<int,string> sha1_index;
map<int,unsigned char *> vector_index;
map<int,string> cache_file_index;
map<int,string> cache_sha1_index;
map<int,unsigned char *> cache_vector_index;
map<int,string> fileid_index;
pthread_mutex_t fileindexlock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sha1indexlock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vectorindexlock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cfileindexlock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t csha1indexlock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cvectorindexlock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileidindex=PTHREAD_MUTEX_INITIALIZER;


pthread_mutex_t lrulock=PTHREAD_MUTEX_INITIALIZER;
unsigned char vectors[512][128];



/* ----------------------------------------------------------------*/
/*---------------------Thread Functions----------------------------*/
/* ----------------------------------------------------------------*/
void *server(void *ptr);
void *siginthandle(void *ptr);
void *client(void *ptr);
void *c_server(void *ptr);
void *read(void *ptr);
extern void *UIHandler(void *ptr);
void *write(void *ptr);
void *timer(void *ptr);
void *eventdispatcher(void *ptr);
void sigpipehandler(int sig);
void commandline(int argc,char **argv);
void *NonBeacon_client(void *ptr);
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Regular Functions---------------------------*/
/* ----------------------------------------------------------------*/
void getType(char *,unsigned char *);
void logging(unsigned char *msg,int type);
void threadfailed();
void removeConnection(int port);
void setAuto(int port);
int getAuto(int port);
bool duplicateMessage();
void *get_in_addr(struct sockaddr *sa);
int isBeacon();
void siginthandler(int sig);
bool isActive(int port);
int sock_ready(int sock); 
void messageconstruction(unsigned char *msg,int type,int errorcode);
void getnodeid();
bool isPresent(int);
bool tieBreak(int, int, int);
void GetUOID(char *node_inst_id,char *obj_type,unsigned char *uoid_buf,int uoid_buf_sz);
 void status_msg_generate(int ttl,string filename,int type);
void killsigserver(int sig);
bool sort_neighbour_list(struct LocDistStruct *,struct LocDistStruct *);
void catch_alarm_msg_join (int sig);
void non_beacon_client_connect();
int find_Msg_Type(unsigned char *buf);
void setAgainKeepAlive(int index);
void setKeepAlive(string index);
void setAgainKeepAliveRetry(string index);
void store(string filename,int ttl,vector<string> keywords);
string createMetadata();
void InitRandom(long l_seed);
void hex_display(int len, unsigned char *buf);

/* ----------------------------------------------------------------*/
vector<string> keywords;
void breakKey(string str);
void display();

vector<unsigned char *> bitvector;
int mods(unsigned char *);
int mods1(unsigned char buf[16]);
void setBit(int position,unsigned char *);
string store_nonUI(string filename,int ttl,vector<string> keywords);
void search(int search_type,vector<string> keywords);
void parse_search_response(char *msg);

void makePersistent();
 void populate();
void stripIndex(char *buf,unsigned char *vec);
string constructFileName(int index,int type);
 bool checkKeyWordExists(int index,vector<string> keys);
bool bitwiseOr(unsigned char a[128],unsigned char b[128]);
string generateMetaData(string filename);
string generateFileId(int index);
bool isPresentFileId(int index);

vector<searchresponse> searchProc(vector<string> keys,int type);

/* ---------------------------------------------*/
void get_UI(string FileID,string Sha1_str);
void delete_UI(string FileSpec);
pthread_mutex_t file_index_counter_mutex=PTHREAD_MUTEX_INITIALIZER;
int get_file_index_counter();
/*-----------------------------------------------*/

struct getres
{
string fileid;
string sha1;
string nonce;
string filename;
int filesize;
vector<string> keyword;
};
void searchResponse(vector<string> Fileid,vector<string> metadata);

int search_flag=1;
void displaySearchResponse(getres sr,int index);
pthread_cond_t search_cond=PTHREAD_COND_INITIALIZER;
pthread_mutex_t globalbufferlock=PTHREAD_MUTEX_INITIALIZER;

vector<struct getres> localsearchbuffer;
vector<struct getres> globalsearchbuffer;
bool fileExistsInCurr(string filename);
int getRequest(string fileid,string sha1);

struct FileSpec
{
string FileName;
string SHA1;
string Nonce;
string Password;
};
void delProc(string filename, string sha1, string nonce);
int passWordExists(string filename,string sha1,string nonce);
bool checkFileExists(int index);
string generateRandomPassword();
string readPassword(int index);
void delFileRequest(string filename,string sha1, string nonce, string password);
void delFile(int index,int type);
void delIndex(int index,int type);


void delFileResponse(string a);
int indexOfFile(string filename,string sha1,string nonce);


 bool compareHex(string a, string b);
string assign( unsigned char b[20]);
void reassign(char a[40], string b);
 pthread_mutex_t pass_lock[512];
 pthread_mutex_t meta_lock[512];
 pthread_mutex_t data_lock[512];


 void unLockMeta(int index);

 void lockMeta(int index);
void lockPass(int index);
void unLockPass(int index);
int shortindexOfFile(string filename,string sha1,string nonce);

void vstripIndex(char *buf,unsigned char *vec);
string getExtFile;
pthread_mutex_t filecounterlock=PTHREAD_MUTEX_INITIALIZER;
 map<string,vector<int> > sortIn(map<int,string> in);
