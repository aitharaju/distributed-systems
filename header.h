#include "stdio.h"
#include "iniparser.h"
#include "string"
#include "stdint.h"
#include "list"
#include "string"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
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
extern char LogFilename[512];
extern char Port[5];
extern char hostname[14];
extern char HomeDir[512];
extern uint32_t Location;
extern int AutoShutdown;// In seconds
extern int TTL;
extern int MsgLifetime;
extern int InitNeighbors;
extern int JoinTimeout;
extern int KeepAliveTimeout;
extern int MinNeighbours;
extern int NoCheck;
extern double CacheProb;
extern double StoreProb;
extern double NeighborStoreProb;
extern int CacheSize; // In KiloBytes
extern int Retry;// In Seconds
extern int notfound;
extern int reset;
extern int file_index_counter;
extern char initfilename[50];
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
};

extern Nam name;

struct searchresponse
{
string fileid;
string mdata;
};

struct activeparam
{
pthread_t read;
pthread_t write;
pthread_t self;
int port;
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
extern char nodeid[18];
extern char *nodeinstanceid;
extern FILE *fp;
//number of neighbors that have sent hello back
extern int neighbour_joined;
extern int non_beacon_join_timeout;
extern int auto_co_index;
extern int retry_co_index;
extern int event_shutdown;
extern int shutdowns; //Global Variable for handling auto shutdown
/*------------------------------------------------------------------*/




/* ----------------------------------------------------------------*/
/*---------------------Redundant-----------------------------------*/
/* ----------------------------------------------------------------*/
extern list<string>::iterator iter;
extern map<int,pthread_cond_t *>::iterator it;
extern map<int,int>::iterator its;
extern map<int,int>::iterator it2; // For Retry
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Data Structures-----------------------------*/
/* ----------------------------------------------------------------*/
extern vector<string> beacons;
extern vector<activeparam> activeconnections;
extern vector<timeparam> time_buffer;
extern std::queue<event> event_dispatcher;
extern map<int,int> retry_co_map;
extern map<int,int> auto_co_map;
extern map<string,string> msg_buffer;
extern map<string,int> msg_UOID_port_map;
extern map<string,int> msg_UOID_status_msg_type_map;
extern map<string,int> msg_UOID_status_filename_map;
extern map<int,list<string> > write_buffer;
extern vector<activeparam> tempconnections;
extern vector<struct LocDistStruct *> neighbors; //vector for neighbors received from join msg req
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Global Condition Variables------------------*/
/* ----------------------------------------------------------------*/
extern pthread_cond_t event_dispatcher_cond;
extern pthread_cond_t auto_co[512],retry_co[512];
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Global Locks--------------------------------*/
/* ----------------------------------------------------------------*/
extern pthread_mutex_t act ; // Lock for active connection list
extern pthread_mutex_t time_buffer_lock; // Lock for the Time Buffer
extern pthread_mutex_t retry_lock; // Lock for Retry
extern pthread_mutex_t event_dispatcher_lock;
extern pthread_mutex_t log_lock;
extern pthread_mutex_t auto_co_lock;
extern pthread_mutex_t write_buffer_mutex;//Lock for write buffer 
extern pthread_mutex_t msg_UOID_port_map_mutex;//Lock for UOID-PORT map 
extern pthread_mutex_t msg_UOID_status_msg_type_map_mutex;//Lock for UOID_status_msg_type_map
extern pthread_mutex_t msg_UOID_status_filename_map_mutex;
extern pthread_mutex_t msg_buffer_mutex;//Lock for msg buffer 
extern pthread_mutex_t lrulock;

/* ----------------------------------------------------------------*/



/* ----------------------------------------------------------------*/
/*---------------------Thread Variables----------------------------*/
/* ----------------------------------------------------------------*/
extern pthread_t serv, cli[40],c_serv[40];
extern pthread_t event_dispatcher_t;
extern pthread_t timer_tt;
extern pthread_t ui;
/* ----------------------------------------------------------------*/
/*------------------Files--------------------------------------------*/
extern map<int,string> file_index;
extern map<int,string> sha1_index;
extern map<int,unsigned char *> vector_index;
extern map<int,string> cache_file_index;
extern map<int,string> cache_sha1_index;
extern map<int,unsigned char *> cache_vector_index;
extern map<int,string> fileid_index;
extern unsigned char vectors[512][128];
extern pthread_mutex_t fileindexlock;
extern pthread_mutex_t sha1indexlock;
extern pthread_mutex_t vectorindexlock;
extern pthread_mutex_t cfileindexlock;
extern pthread_mutex_t csha1indexlock;
extern pthread_mutex_t cvectorindexlock;
extern pthread_mutex_t fileidindex;


/* ----------------------------------------------------------------*/
/*---------------------Thread Functions----------------------------*/
/* ----------------------------------------------------------------*/
extern void *server(void *ptr);
extern void *siginthandle(void *ptr);
extern void *client(void *ptr);
extern void *c_server(void *ptr);
extern void *read(void *ptr);
extern void *UIHandler(void *ptr);
extern void *write(void *ptr);
extern void *timer(void *ptr);
extern void *eventdispatcher(void *ptr);
extern void sigpipehandler(int sig);
extern void commandline(int argc,char **argv);
extern void *NonBeacon_client(void *ptr);
/* ----------------------------------------------------------------*/


/* ----------------------------------------------------------------*/
/*---------------------Regular Functions---------------------------*/
/* ----------------------------------------------------------------*/
extern void getType(char *,unsigned char *);
extern void logging(unsigned char *msg,int type);
extern void threadfailed();
extern void removeConnection(int port);
extern void setAuto(int port);
extern int getAuto(int port);
extern bool duplicateMessage();
extern void *get_in_addr(struct sockaddr *sa);
extern int isBeacon();
extern void siginthandler(int sig);
extern bool isActive(int port);
extern int sock_ready(int sock); 
extern void messageconstruction(unsigned char *msg,int type,int errorcode);
extern void getnodeid();
extern bool isPresent(int);
extern bool tieBreak(int, int, int);
extern void GetUOID(char *node_inst_id,char *obj_type,unsigned char *uoid_buf,int uoid_buf_sz);
extern void status_msg_generate(int ttl,string filename,int type);
extern void killsigserver(int sig);
extern bool sort_neighbour_list(struct LocDistStruct *,struct LocDistStruct *);
extern void catch_alarm_msg_join (int sig);
extern void non_beacon_client_connect();
extern int find_Msg_Type(unsigned char *buf);
extern void setAgainKeepAlive(int index);
extern void setKeepAlive(string index);
extern void setAgainKeepAliveRetry(string index);
extern void store(string filename,int ttl,vector<string> keywords);
extern string createMetadata();
extern void InitRandom(long l_seed);
extern void makePersistent();

extern void hex_display(int len, unsigned char *buf);

/* ----------------------------------------------------------------*/
extern vector<string> keywords;
extern void breakKey(string str);
extern void display();
extern vector<unsigned char *> bitvector;
extern int mods(unsigned char *);
extern int mods1(unsigned char buf[16]);
extern void setBit(int position,unsigned char *);
extern string store_nonUI(string filename,int ttl,vector<string> keywords);

extern void makePersistent();
extern void populate();
extern void readMetadata(char  *filepath);
extern void stripIndex(char *buf,unsigned char *vec);
extern string constructFileName(int index,int type);
extern bool checkKeyWordExists(int index,vector<string> keys);
extern bool bitwiseOr(unsigned char a[128],unsigned char b[128]);
extern string generateMetaData(string filename);
extern string generateFileId(int index);
extern bool isPresentFileId(int index);
extern vector<searchresponse> searchProc(vector<string> keys,int type);


extern void search(int search_type,vector<string> keywords);
extern void parse_search_response(char *msg);

/* ---------------------------------------------*/

extern void get_UI(string FileID,string Sha1_str);
extern void delete_UI(string FileSpec);

extern pthread_mutex_t file_index_counter_mutex;
extern int get_file_index_counter();
/*-----------------------------------------------*/
extern void vstripIndex(char *buf,unsigned char *vec);



class MetaDatas
{
FILE *mdt;
dictionary *d;

public:

void init(char metafilename[100])
	{
	d=iniparser_load(metafilename);
		if(d==NULL)
		{	
		cout<<"Unable to open the Meta Data File"<<endl;
		return ;
		}
	}
string getFileName()
	{
	string tem;
	char datafilename[100];
	if(iniparser_find_entry(d,(char *)"metadata:FileName"))
		strcpy(datafilename,(char *)iniparser_getstr(d,(char *)"metadata:FileName"));
	tem.assign(datafilename);
	return tem;
	}
int getFileSize()
	{
	int filesize;
	if(iniparser_find_entry(d,(char *)"metadata:FileSize"))
		filesize=iniparser_getint(d,(char *)"metadata:FileSize",notfound);
	return filesize;
	}
string getSHA()
	{
	string metashastring;
	char metasha[40];
	unsigned char metashatemp[20];
	if(iniparser_find_entry(d,(char *)"metadata:SHA1"))
		strcpy(metasha,(char *)iniparser_getstr(d,(char *)"metadata:SHA1"));
	stripIndex(metasha,metashatemp);
	metashastring.assign((const char*)metashatemp,20);
	return metashastring;
	}
string getNonce()
	{
	string metanoncestring;
	char metanonce[40];
	unsigned char metanoncetemp[20];
	if(iniparser_find_entry(d,(char *)"metadata:Nonce"))
		strcpy(metanonce,(char *)iniparser_getstr(d,(char *)"metadata:Nonce"));
	stripIndex(metanonce,metanoncetemp);
	metanoncestring.clear();
	metanoncestring.assign(( char*)metanoncetemp,20);
	return metanoncestring;
	}
unsigned char * getVector()
	{

	//Doubt
	char metavector[256];
	if(iniparser_find_entry(d,(char *)"metadata:Bit-vector"))
		strcpy(metavector,(char *)iniparser_getstr(d,(char *)"metadata:Bit-vector"));
	vstripIndex(metavector,vectors[file_index_counter]);
    return vectors[file_index_counter];
	}
vector<string> getKeywords()
	{

vector<string> keyword;
char metakey[300];
char *pch;
string s;
if(iniparser_find_entry(d,(char *)"metadata:Keywords"))
		strcpy(metakey,(char *)iniparser_getstr(d,(char *)"metadata:Keywords"));
pch=strtok(metakey," ");
while(1)
		{
		s.clear();
		if(pch==NULL)
			break;
		s.assign(pch);
		keyword.push_back(s);
		pch=strtok(NULL," ");
		}
return keyword;
	}

};
extern map<string,vector<int> > sortIn(map<int,string> in);

class populates
{
	char prefix[100];
	char suffix[100];
	FILE *index_ptr;
	char i_file[100];
	char temp[300];
	char mystring[300];
	int index;
	string index_string;
	public:
	void init(char fname[100])
	{
    memset(i_file,0,100);
	memset(prefix,0,100);
	memset(suffix,0,100);
	strcpy(prefix,HomeDir);
	strcat(prefix,"/");
	strcpy(suffix,fname);
	strcpy(i_file,prefix);
	strcat(i_file,suffix);
	}
	bool populateData(int type,int filetype)
	{
    switch(type)
	  {
	  case 0:
		{
			dictionary *d;
			d=iniparser_load(i_file);
			if(d==NULL)
			{
				FILE *t_ini;
				t_ini=fopen(i_file,"w");
				if(t_ini==NULL)
				{
				cout<<"cannot open file"<<endl;
				return false;
				}
				fclose(t_ini);
				return false;

			}
			if(iniparser_find_entry(d,(char *)"FileIndex:FileIndex"))
				file_index_counter=iniparser_getint(d,(char *)"FileIndex:FileIndex",1);
			break;
		}
      case 1:
		{
		memset(mystring,0,300);
		index_ptr=fopen(i_file,"r");

		if(index_ptr==NULL)
			{
			FILE *t_ini;
				t_ini=fopen(i_file,"w");
				if(t_ini==NULL)
					{

				cout<<"cannot open file"<<endl;
				return false;
				}
			fclose(t_ini);
			return false;

			}
		while(1)
			{
				memset(temp,0,300);
				memset(mystring,0,300);

				if(fgets (mystring,300,index_ptr)==NULL)
					break;
				sscanf(mystring,"%d=%s",&index,temp);
				string index_string;
				unsigned char sha_temp[20];

				switch(filetype)
				{
					case 0:
					index_string.assign(temp);
					file_index.insert(pair<int,string>(index,index_string));
						break;

					case 1:
			
					stripIndex(temp,sha_temp);
					index_string.assign(( char*)sha_temp,20);
					sha1_index.insert(pair<int,string>(index,index_string));
					break;

					case 2:
					index_string.assign(temp,20);
					cache_file_index.insert(pair<int,string>(index,index_string));
					break;

					case 3:
					//ecout<<"Populating the index"<<index<<endl;
					stripIndex(temp,sha_temp);
					index_string.assign((const char*)sha_temp,20);
					
					cache_sha1_index.insert(pair<int,string>(index,index_string));
						break;
					
					case 4:
					
					vstripIndex(temp,vectors[index]);
					//hex_display(128,vectors[index]);
					vector_index.insert(pair<int,unsigned char *>(index,vectors[index]));
					break;
					
					case 5:
					vstripIndex(temp,vectors[index]);
					cache_vector_index.insert(pair<int,unsigned char *>(index,vectors[index]));
					break;

					case 6:
					stripIndex(temp,sha_temp);
					index_string.assign((const char*)sha_temp,20);
					fileid_index.insert(pair<int,string>(index,index_string));
						break;
				}
			}
		 fclose(index_ptr); 
		 break;
		}
	}
	return true;
  }
  bool makePersistent(int type,int filetype)
	{
	index_ptr=fopen(i_file,"w");
	if(index_ptr==NULL)
		{
		cout<<"Error Opening the Index File"<<endl;
		return false;
		}
	switch(type)
		{
		case 0:
			{
			fputs("[FileIndex]\n",index_ptr);
			fprintf(index_ptr,"FileIndex=%d\n",file_index_counter);
			fflush(stdin);
			break;
			}
		case 1:
			{
			map<string,vector<int> >::iterator out_t;
			map<string,vector<int> > out;

			switch (filetype)
				{
				case 0:
					{
					out=sortIn(file_index);

					for(out_t=out.begin();out_t!=out.end();out_t++)
					{
						vector<int> ab;
						ab=out[(*out_t).first];
						for(int i=0;i<(int)ab.size();i++)
						{
						fprintf(index_ptr,"%d=%s\r\n",ab[i],(*out_t).first.c_str());
						fflush(stdin);
						}
					}
						
					

					break;
					}
				case 1:
					{
					out=sortIn(cache_file_index);

					for(out_t=out.begin();out_t!=out.end();out_t++)
					{
						vector<int> ab;
						ab=out[(*out_t).first];
						for(int i=0;i<(int)ab.size();i++)
						{
						fprintf(index_ptr,"%d=%s\r\n",ab[i],(*out_t).first.c_str());
						fflush(stdin);
						}
					}

		
					break;
					}
				}
			break;
			}
		case 2:
			{
						map<string,vector<int> >::iterator out_t;
			map<string,vector<int> > out;


			switch(filetype)
				{

				case 0:
					{
						out=sortIn(sha1_index);

					for(out_t=out.begin();out_t!=out.end();out_t++)
					{
						vector<int> ab;
						ab=out[(*out_t).first];
						for(int i=0;i<(int)ab.size();i++)
						{
						fprintf(index_ptr,"%d=",ab[i]);
						for(int j=0;j<20;j++)
								fprintf(index_ptr,"%02x",(unsigned char)(*out_t).first[j]);
							fputs("\r\n",index_ptr);
						fflush(stdin);
						}
					}

						/*for(file_iter=sha1_index.begin();file_iter!=sha1_index.end();file_iter++)
						{
							
							fprintf(index_ptr,"%d=",(*file_iter).first);
							for(int i=0;i<20;i++)
								fprintf(index_ptr,"%02x",(unsigned char)(*file_iter).second[i]);
							fputs("\r\n",index_ptr);
							fflush(stdin);
						}*/
					break;
					}
				case 1:
					{
						out=sortIn(cache_sha1_index);

					for(out_t=out.begin();out_t!=out.end();out_t++)
					{
						vector<int> ab;
						ab=out[(*out_t).first];
						for(int i=0;i<(int)ab.size();i++)
						{
						fprintf(index_ptr,"%d=",ab[i]);
						for(int j=0;j<20;j++)
								fprintf(index_ptr,"%02x",(unsigned char)(*out_t).first[j]);
							fputs("\r\n",index_ptr);
						fflush(stdin);
						}
					}
					}
				}
			break;
			}
		case 3:
			{
			map<int,unsigned char *>::iterator file_iter;
			switch(filetype)
				{
				case 0:
					{
					for(file_iter=vector_index.begin();file_iter!=vector_index.end();file_iter++)
							{
							fprintf(index_ptr,"%d=",(*file_iter).first);
							for(int i=0;i<128;i++)
								fprintf(index_ptr,"%02x",(unsigned char)(*file_iter).second[i]);
							fputs("\r\n",index_ptr);
							fflush(stdin);
							
							}
					break;
					}
				case 1:
					{
					for(file_iter=cache_vector_index.begin();file_iter!=cache_vector_index.end();file_iter++)
							{
							fprintf(index_ptr,"%d=",(*file_iter).first);
							for(int i=0;i<128;i++)
								fprintf(index_ptr,"%02x",(unsigned char)(*file_iter).second[i]);
							fputs("\r\n",index_ptr);
							fflush(stdin);
							}
							break;
					}
				}
			break;
			}
		case 4:
			{
			map<int,string>::iterator file_iter;
			switch (filetype)
				{
				case 0:
					{
					for(file_iter=fileid_index.begin();file_iter!=fileid_index.end();file_iter++)
							{
							fprintf(index_ptr,"%d=",(*file_iter).first);
							for(int i=0;i<20;i++)
								fprintf(index_ptr,"%02x",(unsigned char)(*file_iter).second[i]);
							fputs("\r\n",index_ptr);
							fflush(stdin);
							}
					break;
					}
				
				}
			break;
			}
		case 5:
			{
			break;
			}
		}
		fclose(index_ptr);
		return true;
	}
};

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


	}
} ;
extern LRU lru;

struct getres
{
string fileid;
string sha1;
string nonce;
string filename;
int filesize;
vector<string> keyword;
};
extern int search_flag;
void displaySearchResponse(getres sr,int index);
extern pthread_cond_t search_cond;
extern pthread_mutex_t globalbufferlock;
extern void searchResponse(vector<string> Fileid,vector<string> metadata);


extern vector<struct getres> localsearchbuffer;
extern vector<struct getres> globalsearchbuffer;
extern bool fileExistsInCurr(string filename);

extern int getRequest(string fileid,string sha1);
struct FileSpec
{
string FileName;
string SHA1;
string Nonce;
string Password;
};
extern void delProc(string filename, string sha1, string nonce);
extern int  passWordExists(string filename,string sha1,string nonce);
extern bool checkFileExists(int index);
extern string generateRandomPassword();
extern string readPassword(int index);
extern void delFileRequest(string filename,string sha1, string nonce, string password);
extern void delFile(int index,int type);

extern void delIndex(int index,int type);
extern void delFileResponse(string a);
extern int indexOfFile(string filename,string sha1,string nonce);

extern  bool compareHex(string a, string b);
extern string assign(unsigned  char b[20]);
extern void reassign(char a[40], string b);
extern pthread_mutex_t pass_lock[512];
extern pthread_mutex_t meta_lock[512];
extern pthread_mutex_t data_lock[512];



extern void unLockMeta(int index);

extern void lockMeta(int index);
extern void lockPass(int index);
extern void unLockPass(int index);
extern int shortindexOfFile(string filename,string sha1,string nonce);

extern string getExtFile;
extern pthread_mutex_t filecounterlock;
