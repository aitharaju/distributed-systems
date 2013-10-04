#include "header.h" 

int main(int argc,char *argv[])
{
int flag=0,i,j=0;
int a=0;
char filename[100];
vector<pthread_t> activeThread;
event temp;

signal(SIGINT,siginthandler);

/*------------------------------Parsing the INI File-----------------------------------------------*/
dictionary *d;
string str;
commandline(argc,argv);
d=iniparser_load(initfilename);
if(iniparser_find_entry(d,(char *)"init:LogFilename"))
strcpy(LogFilename,(char *)iniparser_getstr(d,(char *)"init:LogFilename"));
if(iniparser_find_entry(d,(char *)"init:Port"))
strcpy(Port,iniparser_getstr(d,(char *)"init:Port"));
if(iniparser_find_entry(d,(char *)"init:HomeDir"))
strcpy(HomeDir,iniparser_getstr(d,(char *)"init:HomeDir"));
if(iniparser_find_entry(d,(char *)"init:Location"))
Location=iniparser_getint(d,(char *)"init:Location",notfound);
if(iniparser_find_entry(d,(char *)"init:AutoShutdown"))
AutoShutdown=iniparser_getint(d,(char *)"init:AutoShutdown",notfound);
if(iniparser_find_entry(d,(char *)"init:TTL"))
TTL=iniparser_getint(d,(char *)"init:TTL",notfound);
if(iniparser_find_entry(d,(char *)"init:MsgLifetime"))
MsgLifetime=iniparser_getint(d,(char *)"init:MsgLifetime",notfound);
if(iniparser_find_entry(d,(char *)"init:InitNeighbors"))
InitNeighbors=iniparser_getint(d,(char *)"init:InitNeighbors",notfound);
if(iniparser_find_entry(d,(char *)"init:JoinTimeout"))
JoinTimeout=iniparser_getint(d,(char *)"init:JoinTimeout",notfound);
if(iniparser_find_entry(d,(char *)"init:KeepAliveTimeout"))
KeepAliveTimeout=iniparser_getint(d,(char *)"init:KeepAliveTimeout",notfound);
if(iniparser_find_entry(d,(char *)"init:MinNeighbours"))
MinNeighbours=iniparser_getint(d,(char *)"init:MinNeighbours",notfound);
if(iniparser_find_entry(d,(char *)"init:NoCheck"))
NoCheck=iniparser_getint(d,(char *)"init:NoCheck",notfound);
if(iniparser_find_entry(d,(char *)"init:CacheProb"))
CacheProb=iniparser_getdouble(d,(char *)"init:CacheProb",notfound);
if(iniparser_find_entry(d,(char *)"init:StoreProb"))
StoreProb=iniparser_getdouble(d,(char *)"init:StoreProb",notfound);
if(iniparser_find_entry(d,(char *)"init:NeighborStoreProb"))
NeighborStoreProb=iniparser_getdouble(d,(char *)"init:NeighborStoreProb",notfound);
if(iniparser_find_entry(d,(char *)"init:CacheSize"))
CacheSize=iniparser_getint(d,(char *)"init:CacheSize",notfound);
if(iniparser_find_entry(d,(char *)"beacons:Retry"))
Retry=iniparser_getint(d,(char *)"beacons:Retry",notfound);
for (i=0 ; i<d->size ; i++) 
	 {
       if (d->key[i]==NULL)
           continue ;
       else if (strchr(d->key[i], ':')==NULL) 
		 {
		 char *p;
		 p=strtok(d->key[i],":");
		 if(strcmp(p,"beacons")==0)
			 flag=1;	
		 }
		else if (flag==1)
			{
			if(strcmp(d->key[i],"beacons:retry")!=0)
				{	
			for(j=22;j<27;j++)
				{
				str.push_back(d->key[i][j]);
				}
			beacons.push_back(str);
			str.clear();
				}
			}
     }
/*-------------------------------------------------------------------------------------------------*/


/*-------------------------Opening the Log File----------------------------------------------------*/
strcpy(filename,HomeDir);
strcat(filename,"/");
strcat(filename,LogFilename);
fp=fopen(filename,"w");
/*-------------------------------------------------------------------------------------------------*/


/*------------------------------Starting the Auto Shut Down buffer--------------------------------------*/
timeparam tparam;
pthread_mutex_lock(&time_buffer_lock);
tparam.index=a;
tparam.value=AutoShutdown;
tparam.type=3;
time_buffer.push_back(tparam);
pthread_mutex_unlock(&time_buffer_lock);
/*---------------------------------------------------------------------------------------------*/


InitRandom(0);
/*creating the subdirectory 'files' if it does not exist*/
char filename1[100];
strcpy(filename1,HomeDir);
strcat(filename1,"/");
strcat(filename1,"files");
struct stat st;
if(stat(filename1,&st) != 0)
{
  int status_file = 0;
  status_file = mkdir(filename1,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if(status_file == -1){
    cout<<"failed to create the directory "<<filename1<<endl;
	exit(-1);
  }
}

/*---------------------------------------------------------------------------------------------*/





getnodeid();
j=0;

if( (pthread_create(&serv,NULL,server,NULL)) )
		{
				threadfailed();
		}
	    if( (pthread_create(&event_dispatcher_t,NULL,eventdispatcher,NULL)) )
		{
				threadfailed();
		}
	    if( (pthread_create(&timer_tt,NULL,timer,NULL)) )
		{
				threadfailed();
		}
        if( (pthread_create(&ui,NULL,UIHandler,NULL)) )
		{
				threadfailed();
		}

 switch(isBeacon())
 {
	case 0:  
    {    
	  non_beacon_client_connect();
	  break;
	}
	case 1:
	{    
	    for(i=0;i<(int)beacons.size();i++)
	    {
	      if(beacons[i].compare(Port)!=0)
		  {
	
 	         if((pthread_create(&cli[j],NULL,client,(void *)beacons[i].c_str()) ))
			 {
				threadfailed();
			 }	
	         j++;
		  }

	    }
		break;
	}
 }

pthread_join(timer_tt, NULL);
pthread_kill(serv,SIGUSR1);
pthread_join(serv,NULL); 
/*---------------------------To sending signal to all  Clients Waiting on a Retry-----------------------------------*/
	pthread_mutex_lock(&retry_lock);
	for ( it2=retry_co_map.begin() ; it2 != retry_co_map.end(); it2++ )
	{
			temp.disp=&retry_co[(*it2).second];
			pthread_mutex_lock(&event_dispatcher_lock);
			event_dispatcher.push(temp);
			pthread_mutex_unlock(&event_dispatcher_lock);
	}
	pthread_mutex_unlock(&retry_lock);
   fputs("out of clients retry\n",fp);
/*-------------------------------------------------------------------------------------------------------------------*/


/*-----------------------To close all the client connections----------------------------------------------------------*/
for(i=0;i<(int)beacons.size();i++)
	pthread_join(cli[i],NULL);
/*-------------------------------------------------------------------------------------------------------------------*/
 fputs("clients closed\n",fp);

event_shutdown=0; //To close the event dispatcher
pthread_join( event_dispatcher_t, NULL); 
pthread_join(ui,NULL);
fputs("event dispatcher closed\n",fp);
fclose(fp);

return 1;
}



void *server(void *ptr)
{
	signal(SIGUSR1,killsigserver);
	/*----------------Extra Variables------------------------------*/
	int i=0; 
	int j=0;
	int flag_server=1;
	event e;
	/* ------------------------------------------------------------*/
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, Port, &hints, &servinfo)) != 0) {
       // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 0;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
       // fprintf(stderr, "server: failed to bind\n");
        return 0;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while(flag_server ) 
		{  // main accept() loop
		
		sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) 
			{
			flag_server=0;
			}


        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
		if( (pthread_create(&c_serv[j], NULL,c_server, (void *)new_fd)) )
			 {
				threadfailed();
			 }
		j++;
		
		
    }

	/*---------------------Handling SIG INT--------------------*/
	pthread_mutex_lock(&auto_co_lock);
	for ( its=auto_co_map.begin() ; its != auto_co_map.end(); its++ )
	{
			 e.disp=&auto_co[(*its).second];
			 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
	
	}
	pthread_mutex_unlock(&auto_co_lock);
	/*--------------------------------------------------------*/


	/*-------------------------Waiting for all the servers to join--------------*/
	for(i=0;i<j;i++)
		pthread_join(c_serv[i],NULL);

	/*--------------------------------------------------------------------------*/

	pthread_mutex_lock(&log_lock);
	fputs("The server is shutting down\n",fp);
	pthread_mutex_unlock(&log_lock);


	close(sockfd);
	pthread_exit(NULL);
}

void *c_server(void *ptr)
{
    

	/*---------------------Thread Parameters---------------------------*/
	int sockfds=(int )ptr;
	/* ----------------------------------------------------------------*/


	/*---------------------Local Pthread Values------------------------*/
	pthread_t aread;
	pthread_t awrite;
	pthread_t self;
	/* ----------------------------------------------------------------*/


	
	/*---------------------Message Buffers-----------------------------*/
	unsigned char buf[512];
	/* ----------------------------------------------------------------*/


	uint16_t sport; // to read the port no of the incoming connection
	int numbytes;// Used to indicate the num of bytes read
	int temp; // Used for message length
	activeparam param; // Structure to store how a connection is stored in the active connections list
	struct soc sock; //Parameter to be passed on to the Child read and write threads 
	timeparam tparam; // Used for keep alive time out and keep alive retry
	string index; //Used for timer buffer. Here port number is the index

	
	/*--------------------Condition Variables and Locks------------------*/

	pthread_mutex_t global_lock=PTHREAD_MUTEX_INITIALIZER;
	/*-------------------------------------------------------------------*/


	self=pthread_self();
	/*-------------------------Reading the Hello Message--------------------------*/
	while(shutdowns)
	{
	if(sock_ready(sockfds))
	{
	 if((numbytes=read(sockfds,buf,512))== -1)
	 {
		close(sockfds);
		pthread_exit(NULL);
	 }
	 buf[numbytes]='\0';
	 if(numbytes == 42)
		 {
		 logging(buf,0);
		 break;
		 }
	}
	}
	if(shutdowns == 0)
		{
		close(sockfds);
		pthread_exit(NULL);
		}
	/*-----------------------------------------------------------------------------*/	
   
	switch(find_Msg_Type(buf))
	{
	  case 0:
	  {
		memcpy(&sport,&buf[27],2);
	   

	    /*-------Converting into string------*/
	    std::string s;
	    std::stringstream out;
	    out << sport;
	    s = out.str();
	   /*-------------------------------------*/
	   index.append(s);
  	   memset(buf,0,512);
	   temp=strlen(hostname)+29;
	   messageconstruction(buf,0,0);
	
       logging(buf,1);
      /*-------------------------Writing the Hello Message--------------------------*/	
       if (write(sockfds,buf,temp) == -1) 
		{
	  	close(sockfds);
        pthread_exit(NULL);
		}
      /*-----------------------------------------------------------------------------*/	
  
      pthread_mutex_lock(&act);
      if(tieBreak(atoi(Port),sport,1))
	  {
			

			sock.port=sport;
			sock.sockfd=sockfds;
			sock.global_variable=1;

			/*------------------Creating the Read Thread----------------------*/
			if( (pthread_create(&aread,NULL,read,(void *)&sock) ))
				 {
					close(sockfds);
					 pthread_exit(NULL);
				 }	
			/*----------------------------------------------------------------*/


			/*-----------------Creatinawriteg the Write Thread----------------------*/
			if( (pthread_create(&awrite,NULL,write,(void *)&sock) ))
				 {
					close(sockfds);
					pthread_exit(NULL);
				 }	
			/*----------------------------------------------------------------*/
			

			/*-----------------------Placing in the active connection list--------*/
			param.port=sport;
			param.self=self;
			param.read=aread;
			param.write=awrite;	
			activeconnections.push_back(param);
			pthread_mutex_unlock(&act);
			/*--------------------------------------------------------------------*/


			/*--------------Pushing Keep Alive Timer and Keep Alive Retry in the Time Buffer----------*/
			setKeepAlive(index);
			/*----------------------------------------------------------------------------------------*/
			
			
			/*---------Inserting the Thread's AutoShutDown condition variable into auto_cond buffer---*/
			setAuto(sport);
			/*----------------------------------------------------------------------------------------*/
			
			if(shutdowns)
					{
			/*----------Condition wait on the global_lock---------------------------------------------*/
			pthread_mutex_lock(&global_lock);
			pthread_cond_wait(&auto_co[getAuto(sport)],&global_lock);
			pthread_mutex_unlock(&global_lock);

					}
			sock.global_variable=0;
			/*----------------------------------------------------------------------------------------*/


			pthread_join(awrite,NULL);
			pthread_join(aread,NULL);
			removeConnection(sport);

 	     }
	      pthread_mutex_unlock(&act);
	     break;
	
	  }
      case 1:
	  {
        memcpy(&sport,&buf[31],2);
	    unsigned char UOID_frm[SHA_DIGEST_LENGTH];
	    memset(&UOID_frm,0,sizeof(UOID_frm));
        memcpy(&UOID_frm,&buf[1],20);
      
        string UOID_temp((const char*)&buf[1],20); 
	    uint16_t port_temp=atoi(Port);

	    string msg_temp((const char*)&buf[0],numbytes);
	    memcpy(&msg_temp[31],&port_temp,2); 
        memcpy(&port_temp,&msg_temp[31],2);

	    uint8_t ttl_temp;
        memcpy(&ttl_temp,&buf[21],1);
       
        pthread_mutex_lock(&msg_buffer_mutex);
	    if(msg_buffer.find(UOID_temp) == msg_buffer.end())
	    {
		  // UOID does not exist ... flood the msg and add it to the msg buffer
		   msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
		  if(ttl_temp <= 0)
		  {
            int l;
		    pthread_mutex_lock(&write_buffer_mutex);
		    pthread_mutex_lock(&act);
	        for (l=0;l<(int)activeconnections.size();l++)
		    {
             if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
		     {
			   list<string> L;
			   L.push_back(UOID_temp);
               write_buffer[activeconnections[l].port] = L;
		     }
		     else
		     { 
			  map<int,list<string> >::iterator it;
              it = write_buffer.find(activeconnections[l].port);
			  it->second.push_back(UOID_temp);
		     }     
            }
		    pthread_mutex_unlock(&msg_buffer_mutex);
		    pthread_mutex_unlock(&write_buffer_mutex);
		    pthread_mutex_unlock(&act);
	       }
		   else
		   {
             pthread_mutex_unlock(&msg_buffer_mutex);
		   }
		        	     
	       temp=strlen(hostname)+53;
		   unsigned char *msg;
	       msg=(unsigned char *)malloc(temp*sizeof(char));
		   if(msg == NULL){
		     cout<<"memory allocation error"<<endl;
		    exit(-1);
	       }
   	       messageconstruction(msg,2,0);
	 
	       uint32_t Loc_diff,temp_loc;
	       memcpy(&temp_loc,&buf[27],4);
           Loc_diff = (long)Location-(long)temp_loc;
	       uint16_t port=htons(atoi(Port));
	       memcpy(&msg[27],&UOID_temp,20);
   	       memcpy(&msg[47],&Loc_diff,4);

	       memcpy(&msg[51],&port,2);
	       memcpy(&msg[53],(unsigned char*)hostname,strlen(hostname));
           int num_bytes;
	       //send join response to the client
	       if ((num_bytes = write(sockfds,msg,temp)) == -1) 
           {
             perror("recvs");
			 close(sockfds);
             pthread_exit(NULL);
           }    
		   pthread_mutex_lock(&msg_UOID_port_map_mutex);
		     msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,sport));
		     pthread_mutex_unlock(&msg_UOID_port_map_mutex);

           
			
			sock.port=sport;
			sock.sockfd=sockfds;
			sock.global_variable=1;

			/*------------------Creating the Read Thread----------------------*/
			if( (pthread_create(&aread,NULL,read,(void *)&sock) ))
				 {
					close(sockfds);
					 pthread_exit(NULL);
				 }	
			/*----------------------------------------------------------------*/


			/*-----------------Creatinawriteg the Write Thread----------------------*/
			if( (pthread_create(&awrite,NULL,write,(void *)&sock) ))
				 {
					close(sockfds);
					pthread_exit(NULL);
				 }	
			/*----------------------------------------------------------------*/
			

			/*-----------------------Placing in the active connection list--------*/
			pthread_mutex_lock(&act);
			param.port=sport;
			param.self=self;
			param.read=aread;
			param.write=awrite;	
			activeconnections.push_back(param);
			pthread_mutex_unlock(&act);
			/*--------------------------------------------------------------------*/


			/*--------------Pushing Keep Alive Timer and Keep Alive Retry in the Time Buffer----------*/
			setKeepAlive(index);
			/*----------------------------------------------------------------------------------------*/
			
			
			/*---------Inserting the Thread's AutoShutDown condition variable into auto_cond buffer---*/
			/*pthread_mutex_lock(&auto_cond_lock);
			auto_cond.insert(pair<int,pthread_cond_t *>(sport,&auto_co));
			pthread_mutex_unlock(&auto_cond_lock);*/
			setAuto(sport);
			/*----------------------------------------------------------------------------------------*/
			if(shutdowns)
					{
			/*----------Condition wait on the global_lock---------------------------------------------*/
			pthread_mutex_lock(&global_lock);
			pthread_cond_wait(&auto_co[getAuto(sport)],&global_lock);
			pthread_mutex_unlock(&global_lock);

					}
			sock.global_variable=0;
			/*----------------------------------------------------------------------------------------*/
            
			pthread_join(awrite,NULL);
			pthread_join(aread,NULL);
			removeConnection(sport);


	      }
	      else
	      {
		    pthread_mutex_unlock(&msg_buffer_mutex);
		    //entry already exists .. 
	      }
          break;
	  }
	  default:
	  {
		  break;
	  }
	}
	close(sockfds);
  pthread_exit(NULL);
}


void *client(void *ptr)
{

	/*----------------------------TCP Control Variables------------------------*/
	int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	/*-------------------------------------------------------------------------*/

	/*----------------------------Message Buffers------------------------------*/
	unsigned char buf[512];
	/*-------------------------------------------------------------------------*/
	
	/*----------------------------Locak Pthread Values-------------------------*/
	pthread_t aread;
	pthread_t awrite;
	pthread_t self;
	/*-------------------------------------------------------------------------*/
	
	int done=0; // Variable to Break out of Retry Loop once the other node is up
	int temp=0;
	int tport=atoi((char *)ptr);
	int numbytes; // Used to indicate the num of bytes read
	activeparam param; // Structure to store how a connection is stored in the active connections list
	soc sock; //Parameter to be passed on to the Child read and write threads 
	uint16_t sport; // to read the port no of the incoming connection
	timeparam tparam; // Used for keep alive time out and keep alive retry
	string index; //Used for timer buffer. Here port number is the index

	/*--------------------Condition Variables and Locks------------------*/
	pthread_mutex_t global_lock=PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t dummy=PTHREAD_MUTEX_INITIALIZER; // Dummy lock used for retry
	/*-------------------------------------------------------------------*/

	//cout<<"port is "<<(char *)ptr<<endl;
	if ((rv = getaddrinfo("nunki.usc.edu", (char *)ptr, &hints, &servinfo)) != 0) 
		{
        pthread_exit(NULL);
		}
    
	p=servinfo;
	self=pthread_self();
	index.append((char *)ptr);

	/*------Inserting the Thread's Retry condition variable into auto_cond buffer---*/
	pthread_mutex_lock(&retry_lock);
	retry_co_map.insert(pair<int,int>(tport,retry_co_index));
pthread_cond_init(&retry_co[retry_co_index],NULL);
retry_co_index++;
	pthread_mutex_unlock(&retry_lock);
	/*------------------------------------------------------------------------------*/

    while((!done))
		{
			if(!shutdowns)
			{
				pthread_mutex_lock(&retry_lock);
				retry_co_map.erase(tport);
				pthread_mutex_unlock(&retry_lock);
				pthread_exit(NULL);
			}
			if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
				{
					perror("client: socket");
				}
	        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
				{	
					done=0;
					/*--------------Pushing Retry into the Time Buffer----------*/
					pthread_mutex_lock(&time_buffer_lock);
					tparam.type=0;
					tparam.value=Retry;
					tparam.index=index;
					time_buffer.push_back(tparam);
					pthread_mutex_unlock(&time_buffer_lock);
					/*----------------------------------------------------------*/
					
					/*-----------------Condition Waiting on a Dummy Lock--------*/
					pthread_mutex_lock(&dummy);
					pthread_cond_wait(&retry_co[retry_co_map[tport]],&dummy);
					pthread_mutex_unlock(&dummy);

					/*----------------------------------------------------------*/

					close(sockfd);
				}
			 else{

				pthread_mutex_lock(&retry_lock);
				retry_co_map.erase(tport);
				pthread_mutex_unlock(&retry_lock);
				done=1;

			 }
		}
 
	if (p == NULL) 
		{
		close(sockfd);
        pthread_exit(NULL);
		}
	
	neighbour_joined++;
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
    freeaddrinfo(servinfo); // all done with this structure

	temp=strlen(hostname)+29;
	memset(buf,0,512);
	messageconstruction(buf,0,0);
	buf[temp]='\0';
	
	logging(buf,1);
	/*-------------------------Writing the Hello Message--------------------------*/	
    if (write(sockfd,buf,temp) == -1) 
		{
        close(sockfd);
		pthread_exit(NULL);
		}
	/*-----------------------------------------------------------------------------*/	
	
	while(shutdowns)
	{
	if(sock_ready(sockfd))
	{
	 /*-------------------------Reading the Hello Message--------------------------*/
	 if((numbytes=read(sockfd,buf,temp))== -1)
	 {
		close(sockfd);
		pthread_exit(NULL);
	 }
	 buf[numbytes]='\0';


	 if(numbytes == 42)
	 {
		 logging(buf,0);
		 break;
	 }
	
	
	}
	}
	/*-----------------------------------------------------------------------------*/	
	if(shutdowns == 0)
		{
		close(sockfd);
		pthread_exit(NULL);
		}
	memcpy(&sport,&buf[27],2);

	pthread_mutex_lock(&act);
	if(tieBreak(atoi(Port),sport,0))
	{	
			
			sock.port=sport;
			sock.sockfd=sockfd;
			sock.global_variable=1;

			/*------------------Creating the Read Thread----------------------*/
			if( (pthread_create(&aread,NULL,read,(void *)&sock) ))
				 {
					close(sockfd);
					pthread_exit(NULL);
				 }	
			/*----------------------------------------------------------------*/
			
			/*-----------------Creating the Write Thread----------------------*/
			if( (pthread_create(&awrite,NULL,write,(void *)&sock) ))
				 {
					close(sockfd);
					pthread_exit(NULL);
				 }
			/*----------------------------------------------------------------*/	 
			
			/*-----------------------Placing in the active connection list--------*/
			param.port=sport;
			param.self=self;
			param.read=aread;
			param.write=awrite;	
			activeconnections.push_back(param);
			pthread_mutex_unlock(&act);
			/*--------------------------------------------------------------------*/

			/*--------------Pushing Keep Alive Timer and Keep Alive Retry in the Time Buffer----------*/
			setKeepAlive(index);
			/*----------------------------------------------------------------------------------------*/

			/*---------Inserting the Thread's AutoShutDown condition variable into auto_cond buffer---*/
			setAuto(sport);
			/*----------------------------------------------------------------------------------------*/

			if(shutdowns)
			{
			/*----------Condition wait on the global_lock---------------------------------------------*/
			 pthread_mutex_lock(&global_lock);
			 pthread_cond_wait(&auto_co[getAuto(sport)],&global_lock);
			 pthread_mutex_unlock(&global_lock);
		    }
			sock.global_variable=0;
			/*----------------------------------------------------------------------------------------*/
		
			pthread_join(awrite,NULL);
			pthread_join(aread,NULL);
			removeConnection(sport);
	}
	
    pthread_mutex_unlock(&act);
	
	close(sockfd);
	pthread_exit(NULL);
}

void messageconstruction(unsigned char *msg,int type,int errorcode)
{

	unsigned char UOID[SHA_DIGEST_LENGTH];
	uint32_t size=0;
	uint16_t port=atoi(Port);
	int def=0;
	switch(type)
	{
	case 0:
		//For Hello Message
	GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	msg[0]=0xfa;
	memcpy(&msg[1],UOID,20);
	memcpy(&msg[21],&TTL,1);
	memcpy(&msg[22],&def,1);
	size=strlen(Port)+strlen("nunki.usc.edu");
	memcpy(&msg[23],&size,4);
	memcpy(&msg[27],&port,2);
	memcpy(&msg[29],(unsigned char*)hostname,strlen(hostname));
	break;
    
	case 1:
	  //for join msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xfc;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=4+strlen(Port)+strlen("nunki.usc.edu");
	  memcpy(&msg[23],&size,4);
      memcpy(&msg[27],&Location,4);
   	  memcpy(&msg[31],&port,2);
	  memcpy(&msg[33],(unsigned char*)hostname,strlen(hostname));
      break;
	

	 case 2:
	  //for join msg response
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xfb;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=20+4+strlen(Port)+strlen("nunki.usc.edu");
	  memcpy(&msg[23],&size,4);
      break;


	 case 10:
	//status msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xac;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=1;
	  memcpy(&msg[23],&size,4);
      if (errorcode == 1)
      {
         msg[27]=0x01;
      }
	  else
	  {
         msg[27]=0x02;
	  }
     
      break;

    case 11:
	//status response msg
     GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xab;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
      break;   

	case 8:
	//For Keep Alive
	GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	size=0;
	msg[0]=0xf8;
	memcpy(&msg[1],UOID,20);
	memcpy(&msg[21],&TTL,1);
	memcpy(&msg[22],&def,1);
	memcpy(&msg[23],&size,4);
	break;

	case 7:
	//For Notify
	GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	size=1;
	msg[0]=0xf7;
	memcpy(&msg[1],UOID,20);
	memcpy(&msg[21],&TTL,1);
	memcpy(&msg[22],&def,1);
	memcpy(&msg[23],&size,4);
	memcpy(&msg[27],&errorcode,1);
	break;

	case 33:
	//store msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xcc;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=1;
	  memcpy(&msg[23],&size,4);
      break;

	case 63:
	//search msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xec;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=1;
	  memcpy(&msg[23],&size,4);
      break;

   case 62:
	//search response msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xeb;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=1;
	  memcpy(&msg[23],&size,4);
      break;

   case 43:
	//get  msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xdc;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=1;
	  memcpy(&msg[23],&size,4);
      break;

	case 42:
	//get response msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xdb;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=1;
	  memcpy(&msg[23],&size,4);
      break;
	
	case 23:
	//delete msg
      GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
	  msg[0]=0xbc;
	  memcpy(&msg[1],UOID,20);
      memcpy(&msg[21],&TTL,1);
	  memcpy(&msg[22],&def,1);
	  size=1;
	  memcpy(&msg[23],&size,4);
      break;
	
	
	}


}

void getnodeid()
{
	time_t seconds;
	seconds = time (NULL);
	int size;
	//Convert into to string http://notfaq.wordpress.com/2006/08/30/c-convert-int-to-string/
	std::string s;
	std::stringstream out;
	out << seconds;
	s = out.str();
	//s contains the string

	//Getting the Nodeid
	strcat(nodeid,"nunki.usc.edu_");
	strcat(nodeid,Port);

	//Getting the Node instance ID
	size=strlen(nodeid)+s.size();
	nodeinstanceid=(char *)malloc(sizeof(char)*size);
	strcat(nodeinstanceid,nodeid);
	strcat(nodeinstanceid,"_");
	strcat(nodeinstanceid,s.c_str());
	//cout<<nodeinstanceid;
}


void GetUOID(char *node_inst_id,char *obj_type,unsigned char *uoid_buf,int uoid_buf_sz)
  {
      static unsigned long seq_no=(unsigned long)1;
      char sha1_buf[SHA_DIGEST_LENGTH], str_buf[104];
      snprintf(str_buf, sizeof(str_buf), "%s_%s_%1ld", node_inst_id, obj_type, (long)seq_no++);
	  SHA1((unsigned char*)str_buf, strlen(str_buf), (unsigned char*)sha1_buf);
      memset(uoid_buf, 0, uoid_buf_sz);
      memcpy(uoid_buf, sha1_buf, min((int)uoid_buf_sz,(int)sizeof(sha1_buf)));
  } 


void *get_in_addr(struct sockaddr *sa)
  {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }

bool isPresent(int port)
{
int i;
activeparam param;
for(i=0;i<(int)activeconnections.size();i++)
	{
	param=activeconnections[i];
	if(param.port==port)
		return true;
	}
return false;
}

/*-------------------Accept Connection from a client with a higher port---------------------------*/
bool tieBreak(int a, int b, int c)
{

/*if(c==1)
	{
	if(b>a)
		return true;
	else 
		return false;
	}
else 
	{
	if(a>b)
	return true;
	else 
	return false;*/
	
	if(!isPresent(b)){
	 return true;
	}
	else
	{
	  return false;
	}
	

}

/*-------------------------------------------------------------------------------------------------*/

void *read(void *ptr)
{	
	struct soc* soc_ptr=(struct soc *)ptr;
	int sockfd=soc_ptr->sockfd;
	int port=soc_ptr->port;
	unsigned char buf[8192];
	uint8_t temp;
	int numbytes;
	unsigned char* msg;
	int i;
	uint16_t sport;

	while(soc_ptr->global_variable)
	{
	if(sock_ready(sockfd))
	 {
		if((numbytes=read(sockfd,buf,27))== -1)
		{
			event e;
			e.disp=&auto_co[getAuto(port)];	

			
			pthread_mutex_lock(&event_dispatcher_lock);
			event_dispatcher.push(e);
			pthread_mutex_unlock(&event_dispatcher_lock);
			
           pthread_exit(NULL);		
		 }
	 }
	 else
	 {
		 sleep(1);
		 continue;
	 }
	 
		if(numbytes == 0){
			sleep(1);
          continue;
		}
		buf[numbytes]='\0';
		setAgainKeepAlive(port);
	
          switch(find_Msg_Type(&buf[0]))
          {
		   case 1: 
	       {
			logging(buf,0);
            memcpy(&sport,&buf[31],2);
	        unsigned char UOID_frm[SHA_DIGEST_LENGTH];
	        memset(&UOID_frm,0,sizeof(UOID_frm));
            memcpy(&UOID_frm,&buf[1],20);
      
            string UOID_temp((const char*)&buf[1],20); 

	        uint16_t port_temp=atoi(Port);
	        uint8_t ttl_temp;

            memcpy(&ttl_temp,&buf[21],1);
	        if(ttl_temp > 0){
		     ttl_temp = ttl_temp-1;       
	        }

	        string msg_temp((const char*)&buf[0],numbytes);

	      memcpy(&msg_temp[31],&port_temp,2); 
          memcpy(&port_temp,&msg_temp[31],2);
     
	 
          pthread_mutex_lock(&msg_buffer_mutex);
	      if(msg_buffer.find(UOID_temp) == msg_buffer.end())
	      {
	   
	       // UOID does not exist ... flood the msg and add it to the msg buffer
	       msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
		    
            int l;
		    pthread_mutex_lock(&write_buffer_mutex);
		    pthread_mutex_lock(&act);
	        for (l=0;l<(int)activeconnections.size();l++)
		    {
             if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
		     {
			   list<string> L;
			   L.push_back(UOID_temp);
               write_buffer[activeconnections[l].port] = L;
		     }
		     else
		     {
			  map<int,list<string> >::iterator it;
              it = write_buffer.find(activeconnections[l].port);
			  it->second.push_back(UOID_temp);
		     }     
            }  
		    pthread_mutex_unlock(&msg_buffer_mutex);
		    pthread_mutex_unlock(&write_buffer_mutex);
		    pthread_mutex_unlock(&act);
		
         
	      temp=strlen(hostname)+53;
	      msg=(unsigned char *)malloc(temp*sizeof(char));
		  if(msg == NULL){
		   cout<<"memory allocation error"<<endl;
		   exit(1);
	      }
   	      messageconstruction(msg,2,0);
	 
	      uint32_t Loc_diff,temp_loc;
	      memcpy(&temp_loc,&buf[27],4);
          Loc_diff = Location-temp_loc;
	      uint16_t port=htons(atoi(Port));
     
	      memcpy(&msg[21],&ttl_temp,1);
	      memcpy(&msg[27],UOID_frm,20);
   	      memcpy(&msg[47],&Loc_diff,4);
	      memcpy(&msg[51],&port,2);
	      memcpy(&msg[53],(unsigned char*)hostname,strlen(hostname));

	      memcpy(&UOID_frm,&msg[1],20);
          string UOID_to_temp((const char*)&msg[1],20); 

          string msg_to_temp((const char*)&msg[0],66);
		  pthread_mutex_lock(&msg_buffer_mutex);
	      msg_buffer.insert(std::pair<string,string>(UOID_to_temp,msg_to_temp));
		  pthread_mutex_unlock(&msg_buffer_mutex);
		  pthread_mutex_lock(&write_buffer_mutex);
	  	  pthread_mutex_lock(&act);
	      for (l=0;l<(int)activeconnections.size();l++)
		  {
		   if(activeconnections[l].port == sport)
		   {
            if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
		    {
			   list<string> L;
			   L.push_back(UOID_to_temp);
               write_buffer[activeconnections[l].port] = L;
		    }
		    else
		    {
			  map<int,list<string> >::iterator it;
              it = write_buffer.find(activeconnections[l].port);
			  it->second.push_back(UOID_to_temp);
		    }     
		   }         
         }
		 pthread_mutex_unlock(&write_buffer_mutex);
		 pthread_mutex_unlock(&act);
		

		 pthread_mutex_lock(&msg_UOID_port_map_mutex);
		 msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,sport));
		 pthread_mutex_unlock(&msg_UOID_port_map_mutex);
	   }
       else
	   {
		   pthread_mutex_unlock(&msg_buffer_mutex);
		  //entry already exists .. 
	  }
      break;
      
	}
    case 2 :
    {
		logging(buf,0);
	    unsigned char UOID_frm[SHA_DIGEST_LENGTH];
	    memset(&UOID_frm,0,sizeof(UOID_frm));
        memcpy(&UOID_frm,&buf[1],20);

		memcpy(&sport,&buf[51],2);

		string UOID_temp((const char*)&buf[1],20); ; 

	    string msg_temp((const char*)&buf[0],numbytes);
	
          pthread_mutex_lock(&msg_buffer_mutex);
	      if(msg_buffer.find(UOID_temp) == msg_buffer.end())
		  {
            //duplicate does not exist
			 msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
			 pthread_mutex_unlock(&msg_buffer_mutex);
		    
            // memcpy(&UOID_frm,&buf[27],20);
             string UOID_to_temp((const char*)&buf[27],20);
			 map<string,int >::iterator it;
			 pthread_mutex_lock(&msg_UOID_port_map_mutex);
			 pthread_mutex_lock(&write_buffer_mutex);
			 if(msg_UOID_port_map.find(UOID_to_temp) == msg_UOID_port_map.end()){
				 //message deleted from queue .. no where to go 
				  pthread_mutex_unlock(&msg_UOID_port_map_mutex);
				  pthread_mutex_unlock(&write_buffer_mutex);
				  
				  break;
			 }
			 else
			 {
				
			   it = msg_UOID_port_map.find(UOID_to_temp);
               int port_temp = it->second;
			   if(write_buffer.find(port_temp) == write_buffer.end())
		       {
			     list<string> L;
			     L.push_back(UOID_temp);
                 write_buffer[port_temp] = L;
		       }
		       else
		       {
			     map<int,list<string> >::iterator it;
                 it = write_buffer.find(port_temp);
			     it->second.push_back(UOID_temp);
		       }    
              pthread_mutex_unlock(&msg_UOID_port_map_mutex);
			  pthread_mutex_unlock(&write_buffer_mutex);
		   
			}	    
		  }
		  else
		  {
			  pthread_mutex_unlock(&msg_buffer_mutex);
			  break;
             //duplicate exists ... ignore 
		  }
		break;
	 }
	 
		   case 7:
		   {
			 
            if((numbytes=read(sockfd,&buf[27],1))== -1)
		    {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		   }
			 logging(buf,0);
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
			 
			 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
			 break;
		  }
		  case 10: 
	      {	
			
		    if((numbytes=read(sockfd,&buf[27],1))== -1)
		    {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		   }
		  
		   logging(buf,0);
	       unsigned char UOID_frm[SHA_DIGEST_LENGTH];
	       memset(&UOID_frm,0,sizeof(UOID_frm));
           memcpy(&UOID_frm,&buf[1],20);
           string UOID_temp((const char*)&buf[1],20); 
	       
	       uint8_t ttl_temp;
		 uint8_t ttl_temp1 ;
		 uint8_t ttl_temp2 ;
		 memset(&ttl_temp1,0,1);
		 memset(&ttl_temp2,1,1);
           memcpy(&ttl_temp,&buf[21],1);
		   //ttl_temp1 = atoi((const char*)&ttl_temp);
		  // ttl_temp1 = ttl_temp1 -1;
	       if(ttl_temp >  ttl_temp1){
			   ttl_temp = ttl_temp-(uint8_t)ttl_temp2;
               memcpy(&buf[21],&ttl_temp,1);
		   }
          
          // ttl_temp1 =1;
           memcpy(&buf[21],&ttl_temp,1);
	       string msg_temp((const char*)&buf[0],28);
           pthread_mutex_lock(&msg_buffer_mutex);
	       if(msg_buffer.find(UOID_temp) == msg_buffer.end())
	       {	   
	        // UOID does not exist ... flood the msg and add it to the msg buffer
	        msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
		    pthread_mutex_unlock(&msg_buffer_mutex);
		    if(ttl_temp > ttl_temp1)
		    { 
             int l;
		     pthread_mutex_lock(&write_buffer_mutex);
		     pthread_mutex_lock(&act);
	         for (l=0;l<(int)activeconnections.size();l++)
		     {
              if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
		      {
			   list<string> L;
			   L.push_back(UOID_temp);
               write_buffer[activeconnections[l].port] = L;
		      }
		      else
		      {
			   map<int,list<string> >::iterator it;
               it = write_buffer.find(activeconnections[l].port);
			   it->second.push_back(UOID_temp);
		      }     
             }
		     pthread_mutex_unlock(&write_buffer_mutex);
		     pthread_mutex_unlock(&act);
            }
		    else
		    {
              pthread_mutex_unlock(&msg_buffer_mutex);
		    }
	        
           uint8_t message_type;		  
           memcpy(&message_type,&buf[27],1);
		   //char message_type1 = (char)message_type;
		   int fwd_write =1;
		   string UOID_to_temp1;
		   string msg_to_temp1;
	       switch(message_type){
		    case 0x01: 
			{
			  temp = 64+((int)activeconnections.size())*19;
	          msg=(unsigned char *)malloc(temp*sizeof(char));
		      if(msg == NULL){
		       cout<<"memory allocation error in case 10"<<endl;
		       exit(1);
	          }
   	          messageconstruction(msg,11,0);
		      uint32_t temper;
		      temper = 64+((int)activeconnections.size())*19;
              temper = temper-27;
	          memcpy(&msg[23],&temper,4);
	          memcpy(&msg[27],UOID_frm,20);
	          uint16_t size1=strlen(Port)+strlen("nunki.usc.edu");
	          memcpy(&msg[47],&size1,2);
		      uint16_t port_temp1 = atoi(Port);
              memcpy(&msg[49],&port_temp1,2);
	   
	          memcpy(&msg[51],(unsigned char*)hostname,strlen(hostname));
		
              int curr_loc = 64;
		      pthread_mutex_lock(&act);
		      activeparam param;
              for(i=0;i<(int)activeconnections.size();i++)
	          {
               if(i == ((int)activeconnections.size())-1){
			    temper = 0;
                memcpy(&msg[curr_loc],&temper,4); 
                curr_loc = curr_loc+4;
		       }else
		       {
			    temper = strlen(Port)+strlen("nunki.usc.edu");
			    memcpy(&msg[curr_loc],&temper,4);
			    curr_loc = curr_loc+4;
		       }
	           param=activeconnections[i];
		       uint16_t port_temp = param.port;
	           memcpy(&msg[curr_loc],&port_temp,2);
               curr_loc = curr_loc+2;
               memcpy(&msg[curr_loc],(unsigned char*)hostname,strlen(hostname));
               curr_loc = curr_loc+strlen(hostname);		 
	         }
             pthread_mutex_unlock(&act);
             string UOID_to_temp((const char*)&msg[1],20); 
             string msg_to_temp((const char*)&msg[0],curr_loc);

			 UOID_to_temp1.append(UOID_to_temp);
             msg_to_temp1.append(msg_to_temp);
			  break;
			}
			case 0x02:
			{
				msg=(unsigned char *)malloc(8192*sizeof(char));
				memcpy(&msg[27],UOID_frm,20);
	            uint16_t size1=strlen(Port)+strlen("nunki.usc.edu");
	            memcpy(&msg[47],&size1,2);
		        uint16_t port_temp1 = atoi(Port);
                memcpy(&msg[49],&port_temp1,2);
	   
	            memcpy(&msg[51],(unsigned char*)hostname,strlen(hostname));
				int curr_loc = 64;
				int i;
				char filename1[256];

				long tot_len = 37;
            
				std::string s;
                
			    struct stat FileAttrib;
				long file_Size;
	            messageconstruction(msg,11,0);
  
                char filename2[100];
				int temp_file_index_counter = file_index_counter;
	
				for(i=1;i<temp_file_index_counter;i++){
		         std::stringstream out_ss;
				 out_ss << i;
	             s = out_ss.str();

				 strcpy(filename1,HomeDir);
                 strcat(filename1,"/");
                 strcat(filename1,"files/");

				 strcpy(filename2,s.c_str());
		         strcat(filename2,".metadata");
		         strcat(filename1,filename2);

				 if (stat(filename1, &FileAttrib) == 0){
                    file_Size = FileAttrib.st_size;
					if(curr_loc+4+file_Size > 8192){
						break;
					}
					tot_len +=4+file_Size;
					if(i == temp_file_index_counter-1){
					  long len_s = 0;
                      memcpy(&msg[curr_loc],&len_s,4);
					}
					else
					{
                      memcpy(&msg[curr_loc],&file_Size,4);
					}
                    curr_loc = curr_loc+4;
                    
                    fp = fopen(filename1,"r");
				    if(fp == NULL)
				    {
					 continue;
				    }

					size_t result = fread(&msg[curr_loc],1,file_Size,fp);
				    if((long)result !=file_Size){ 
					   //cout<<"erasing 3 "<<endl;
					  continue;
				    }
                    curr_loc = curr_loc+file_Size;

                 }
                 else
                 {
	               continue;
                 }
				   
				}
				memcpy(&msg[23],&tot_len,4);
				string UOID_to_temp((const char*)&msg[1],20); 
                string msg_to_temp((const char*)&msg[0],curr_loc);

               UOID_to_temp1.append(UOID_to_temp);
               msg_to_temp1.append(msg_to_temp);

				
                 
			
		     break; 
			}
			default: 
			{
			  fwd_write =0;
			  break;
			}
		   }

	    if(fwd_write == 1){
		 pthread_mutex_lock(&msg_buffer_mutex);
	     msg_buffer.insert(std::pair<string,string>(UOID_to_temp1,msg_to_temp1));
		 pthread_mutex_unlock(&msg_buffer_mutex);
         int l;
		 pthread_mutex_lock(&write_buffer_mutex);
		 pthread_mutex_lock(&act);
	     for (l=0;l<(int)activeconnections.size();l++)
		 {
		   if(activeconnections[l].port == port)
		   {
            if(write_buffer.find(port) == write_buffer.end())
		    {
			   list<string> L;
			   L.push_back(UOID_to_temp1);
               write_buffer[port] = L;
		    }
		    else
		    {
			  map<int,list<string> >::iterator it;
              it = write_buffer.find(port);
			  it->second.push_back(UOID_to_temp1);
		    }
			break;
		   }         
         }
		 pthread_mutex_unlock(&write_buffer_mutex);
		 pthread_mutex_unlock(&act);
 
 		 pthread_mutex_lock(&msg_UOID_port_map_mutex);
		 msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,port));
		 pthread_mutex_unlock(&msg_UOID_port_map_mutex);

		 pthread_mutex_lock(&msg_UOID_status_msg_type_map_mutex);
		   switch(message_type){
		    case 0x01: 
			{
				 msg_UOID_status_msg_type_map.insert(std::pair<string,int>(UOID_temp,1));
			}
            case 0x02: 
			{
				 msg_UOID_status_msg_type_map.insert(std::pair<string,int>(UOID_temp,2));
			}
		   }
		 pthread_mutex_unlock(&msg_UOID_status_msg_type_map_mutex);
		}
		
		
	  }
      else
	  {
		  pthread_mutex_unlock(&msg_buffer_mutex);
		  //entry already exists .. 
	  }
      break;
	 }
     case 11 :
     {
        uint32_t total_length;
		memcpy(&total_length,&buf[23],4);
		int total_length1 = (int)total_length;
         if((numbytes=read(sockfd,&buf[27],total_length1))== -1)
		 {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		  }
		 logging(buf,0);

		
		string UOID_temp((const char*)&buf[1],20); ; 
	    string msg_temp((const char*)&buf[0],total_length1+27);

	
        pthread_mutex_lock(&msg_buffer_mutex);
	    if(msg_buffer.find(UOID_temp) == msg_buffer.end())
		{
           //duplicate does not exist
		   msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
		   pthread_mutex_unlock(&msg_buffer_mutex);
		    
           string UOID_to_temp((const char*)&buf[27],20);
		   map<string,int >::iterator it;
		   pthread_mutex_lock(&msg_UOID_port_map_mutex);
		   pthread_mutex_lock(&write_buffer_mutex);
			 if(msg_UOID_port_map.find(UOID_to_temp) == msg_UOID_port_map.end()){
				 //message deleted from queue .. no where to go 
				  pthread_mutex_unlock(&msg_UOID_port_map_mutex);
				  pthread_mutex_unlock(&write_buffer_mutex);
				  break;
			 }
			 else
			 {
			   it = msg_UOID_port_map.find(UOID_to_temp);
               int port_temp = it->second;
			   if(port_temp == atoi(Port)){
				   int msg_type = 0;
				   int fileNum;
                   pthread_mutex_lock(&msg_UOID_status_msg_type_map_mutex);
                   if(msg_UOID_status_msg_type_map.find(UOID_to_temp) == msg_UOID_status_msg_type_map.end())
				   {
					  // cout<<"no way to know if it is a response for files or neighbors"<<endl;
				   }
				   else
				   {
					 it = msg_UOID_status_msg_type_map.find(UOID_to_temp);
                     msg_type = it->second;    
				   }
				   pthread_mutex_unlock(&msg_UOID_status_msg_type_map_mutex);
                    
					pthread_mutex_lock(&msg_UOID_status_filename_map_mutex);
                   if(msg_UOID_status_filename_map.find(UOID_to_temp) == msg_UOID_status_filename_map.end())
				   {
					  // cout<<"no way to know to which UOID is this response"<<endl;
				   }
				   else
				   {
					 it = msg_UOID_status_filename_map.find(UOID_to_temp);
                     fileNum = it->second;    
				   }
				   pthread_mutex_unlock(&msg_UOID_status_filename_map_mutex);
                   

                   
				   if(msg_type == 1)
				   {
					 
					 name.populate((char*)msg_temp.c_str(),1,fileNum,0);	
				   }
				   else if(msg_type == 2)
				   {
					 name.populate((char*)msg_temp.c_str(),2,fileNum,0);
				   }
				    
			   }
			   else
			   { 
				 if(write_buffer.find(port_temp) == write_buffer.end())
		         {
			      list<string> L;
			      L.push_back(UOID_temp);
                  write_buffer[port_temp] = L;
		         }
		         else
		         {
			      map<int,list<string> >::iterator it;
                  it = write_buffer.find(port_temp);
			      it->second.push_back(UOID_temp);
		         }  

			   }
			}
			pthread_mutex_unlock(&msg_UOID_port_map_mutex);
		    pthread_mutex_unlock(&write_buffer_mutex);
		  }
		  else
		  {
			  pthread_mutex_unlock(&msg_buffer_mutex);
             //duplicate exists ... ignore 
		  }

		break;

	   }
	   case 8 :
       {
		 logging(buf,0);
         break;
	   }
       
	    case 33 :
       {
		logging(buf,0);
		unsigned char msg[8192];
        uint32_t total_length;
		memcpy(&total_length,&buf[23],4);

		 uint8_t ttl_temp;
		 uint8_t ttl_temp1 ;
		 uint8_t ttl_temp2 ;
		 memset(&ttl_temp1,0,1);
		 memset(&ttl_temp2,1,1);
           memcpy(&ttl_temp,&buf[21],1);
		   //ttl_temp1 = atoi((const char*)&ttl_temp);
		  // ttl_temp1 = ttl_temp1 -1;
	       if(ttl_temp >  ttl_temp1){
			   ttl_temp = ttl_temp-ttl_temp2;
               memcpy(&buf[21],&ttl_temp,1);
		   }

		unsigned char UOID_frm[SHA_DIGEST_LENGTH];
	    memset(&UOID_frm,0,sizeof(UOID_frm));
        memcpy(&UOID_frm,&buf[1],20);

		string UOID_temp((const char*)&buf[1],20); ; 
		int store_fwd = 0;

		if(msg_buffer.find(UOID_temp) == msg_buffer.end())
		{
			store_fwd = 1;
		}
		else
		{
		}

		memcpy(&msg[0],&buf[0],27);
		int total_length1 = (int)total_length;
        int pos = 0;
  
        string msg_to_temp1;
    
	    std::string s;
        std::stringstream out_ss;
        int temp_file_index_counter = get_file_index_counter();
	    out_ss << temp_file_index_counter;
	    s = out_ss.str();

         char filename1[256];
         strcpy(filename1,HomeDir);
         strcat(filename1,"/");
         strcat(filename1,"files/");
  
         char filename2[100];

		 strcpy(filename2,s.c_str());
		 strcat(filename2,".data");
		 strcat(filename1,filename2);

		 string file_temp(filename1,strlen(filename1));

		 int fwd_msg = 1;

		if(total_length1 <= 8192)
		{
		   if((numbytes=read(sockfd,&buf[0],total_length1))== -1)
		       {
			     perror("read");
			     event e;
			     e.disp=&auto_co[getAuto(port)];	
		 	     pthread_mutex_lock(&event_dispatcher_lock);
			     event_dispatcher.push(e);
			     pthread_mutex_unlock(&event_dispatcher_lock);
                 pthread_exit(NULL);		
		       }

			
	        double dval=(double)drand48();

		    if(store_fwd == 1)
			{
             if(dval <= StoreProb)
			 {
			 
			  char filename4[256];
              strcpy(filename4,HomeDir);
              strcat(filename4,"/");
              strcat(filename4,"files/");
			 
			  char filename3[100];

             strcpy(filename3,s.c_str());
             strcat(filename3,".metadata");
             strcat(filename4,filename3);

			 uint32_t metadata_leng;
			 memcpy(&metadata_leng,&buf[0],4);
             long metadata_leng1 = (long)metadata_leng;
			 char metadata_buf[8192];
			 memset(metadata_buf,0,8192);
			 memcpy(&metadata_buf[0],&buf[4],metadata_leng1);
			 memcpy(&msg[27],&buf[0],4);
			 metadata_buf[metadata_leng1] = '\0';
			 string metadata(metadata_buf,metadata_leng1);
			 memcpy(&msg[31],metadata.c_str(),metadata.length());

			 FILE *fp ;
			 if((fp=fopen(filename4, "w")) == NULL) {
              // cout<<"Cannot open file "<<filename4<<endl;
			   fwd_msg = 0;
             }
			 if(fwd_msg != 0){
			  int result = fwrite(&buf[4],1,metadata_leng1,fp);
			  if(result != metadata_leng1){
               fwd_msg = 0;
			  }
              fclose(fp); 
           
			  if((fp=fopen(filename1, "w")) == NULL) {
			   fwd_msg = 0;
              }

			  memcpy(&msg[31+metadata_leng1],file_temp.c_str(),file_temp.length());

			  int tot_length = 31+metadata_leng1+file_temp.length();
			  
			  string msg_to_temp((const char*)&msg[0],tot_length);
              msg_to_temp1.append(msg_to_temp);

              if(fp != NULL){
			   result = fwrite(&buf[4+metadata_leng1],1,total_length1-4-metadata_leng1,fp);
			   if(result != total_length1-4-metadata_leng1){
			        fwd_msg = 0;
			   }
			   fclose(fp);
			  }

			 }
             
			}
			else
			{
			  fwd_msg = 0;
			  file_index_counter--;

			}  
		  }
		  else
		  {
			  file_index_counter--;
		  }

		 }
		 else
		 {
			 FILE *fp ;
			 while(total_length1 > 0)
		     {
		       if(pos == 0)
			   {
		         if((numbytes=read(sockfd,&buf[0],8192))== -1)
		         {
				  perror("read");
			      event e;
			      e.disp=&auto_co[getAuto(port)];	
		 	      pthread_mutex_lock(&event_dispatcher_lock);
			      event_dispatcher.push(e);
			      pthread_mutex_unlock(&event_dispatcher_lock);
                  pthread_exit(NULL);		
		        }
               total_length1 = total_length1-numbytes;
			   pos++;

			   double dval=(double)drand48();
			  if(store_fwd == 1)
			  {
               if(dval <= StoreProb)
			   {
			 
			    char filename4[256];
                strcpy(filename4,HomeDir);
                strcat(filename4,"/");
                strcat(filename4,"files/");
			 
			    char filename3[100];

               strcpy(filename3,s.c_str());
               strcat(filename3,".metadata");
               strcat(filename4,filename3);

			   uint32_t metadata_leng;
			   long metadata_leng1;
			   memcpy(&metadata_leng,&buf[0],4);
               metadata_leng1 = (long)metadata_leng;
			   char metadata_buf[8192];
			   memcpy(&metadata_buf[0],&buf[4],metadata_leng1);
			  // metadata_buf[metadata_leng1] = '\0';
			   //string metadata(metadata_buf);
			   //cout<<"metadata is "<<metadata<<endl;
               memcpy(&msg[27],&buf[0],4+metadata_leng1);
			  
			  
			   if((fp=fopen(filename4, "w")) == NULL) {
			    fwd_msg = 0;
               }
			   if(fp != NULL)
			   {
                int result = fwrite(&buf[4],1,metadata_leng1,fp);
			    if(result != metadata_leng1){
                fwd_msg = 0;
			    }
                fclose(fp); 
			   }

			   if((fp=fopen(filename1, "w")) == NULL) {
			    fwd_msg = 0;
               }
			   memcpy(&msg[31+metadata_leng1],&filename1,(int)strlen(filename1));
			   string msg_to_temp((const char*)&msg[0],31+metadata_leng1+(int)strlen(filename1));

               msg_to_temp1.append(msg_to_temp);

			    if(fp != NULL)
			    {
			     int result = fwrite(&buf[4+metadata_leng1],1,numbytes-4-metadata_leng1,fp);
			     if(result != numbytes-4-metadata_leng1){
                  fwd_msg = 0;
			     }
			     fclose(fp); 
			    }
			   }
			   else
			   {
				  file_index_counter--;
                 fwd_msg = 0; 
			   }  
			  }
			  else
				   {
				     file_index_counter--;
				   }
		     }
		     else
		     {
              if(total_length1 > 8192){
			   if((numbytes=read(sockfd,&buf[0],8192))== -1)
		       {
				perror("read");
			    event e;
			    e.disp=&auto_co[getAuto(port)];	
		 	    pthread_mutex_lock(&event_dispatcher_lock);
			    event_dispatcher.push(e);
			    pthread_mutex_unlock(&event_dispatcher_lock);
                pthread_exit(NULL);		
		      }
			 

			  if(fwd_msg == 1 && (store_fwd == 1))
			  {
			   if((fp=fopen(filename1, "a")) == NULL) {
			     fwd_msg = 0;
               }
               if(fp != NULL)
			   {
                int result = fwrite(&buf[0],1,numbytes,fp);
			    if(result != numbytes){
                 fwd_msg = 0;
			    }
				fclose(fp);
			   }
			  }
             
              total_length1 = total_length1-numbytes;
			 }
			 else
			 {
			  if(total_length1 > 0){
               if((numbytes=read(sockfd,&buf[0],total_length1))== -1)
		       {
				 perror("read");
			     event e;
			     e.disp=&auto_co[getAuto(port)];	
		 	     pthread_mutex_lock(&event_dispatcher_lock);
			     event_dispatcher.push(e);
			     pthread_mutex_unlock(&event_dispatcher_lock);
                 pthread_exit(NULL);		
		       }

			   if(fwd_msg == 1 && (store_fwd == 1)){
				if((fp=fopen(filename1, "a")) == NULL) {
			     fwd_msg = 0;
                }
				if(fp != NULL)
			    {
                 int result = fwrite(&buf[0],1,numbytes,fp);
			     if(result != numbytes){
                 
                  fwd_msg = 0;
			     }
				 fclose(fp);
				}
			  }
			  total_length1 = total_length1-numbytes;
			 }
			}
			pos++;
			
		   }	     
          }
		 }
		 if(fwd_msg == 1 && (store_fwd == 1)){
		  string metaDataFileName = constructFileName(temp_file_index_counter,0);
		  readMetadata((char*)metaDataFileName.c_str());
         
          if(ttl_temp > ttl_temp1){
          if(msg_buffer.find(UOID_temp) == msg_buffer.end())
		  {
		 
            pthread_mutex_lock(&msg_buffer_mutex);
            msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_to_temp1));
            pthread_mutex_unlock(&msg_buffer_mutex);
            int l;
            pthread_mutex_lock(&write_buffer_mutex);
            pthread_mutex_lock(&act);

            for (l=0;l<(int)activeconnections.size();l++)
            {
	 
             if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
		     {
			   list<string> L;
			   L.push_back(UOID_temp);
               write_buffer[activeconnections[l].port] = L;
		     }
		     else
		     {
			   map<int,list<string> >::iterator it;
               it = write_buffer.find(activeconnections[l].port);
			   it->second.push_back(UOID_temp);
		     }
			
	           
           }
           pthread_mutex_unlock(&write_buffer_mutex);
           pthread_mutex_unlock(&act);
		 }
		 else
	     {
		 }
		  }

          

		 }
	    break;
	   }
	   case 63 :
       {
		  uint32_t tot_length;
		  memcpy(&tot_length,&buf[23],4);
		  long total_len = (long)tot_length;
		  int total_len1 =  (int)total_len;
         
		  if((numbytes=read(sockfd,&buf[27],total_len1))== -1)
		  {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		  }
		  
		  logging(buf,0);
	      unsigned char UOID_frm[SHA_DIGEST_LENGTH];
	      memset(&UOID_frm,0,sizeof(UOID_frm));
          memcpy(&UOID_frm,&buf[1],20);
          string UOID_temp((const char*)&buf[1],20); 
	       
	      uint8_t ttl_temp;
		  int ttl_temp1 ;
          memcpy(&ttl_temp,&buf[21],1);
		  ttl_temp1 = (int)ttl_temp -1;
	      if(ttl_temp1 >  0){
			   ttl_temp = ttl_temp-1;
		  }
	        
          // ttl_temp1 =1;
          memcpy(&buf[21],&ttl_temp,1);
	      string msg_temp((const char*)&buf[0],27);
          pthread_mutex_lock(&msg_buffer_mutex);
	      if(msg_buffer.find(UOID_temp) == msg_buffer.end())
	      {	   
	        // UOID does not exist ... flood the msg and add it to the msg buffer
	        msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
		    pthread_mutex_unlock(&msg_buffer_mutex);
		    if(ttl_temp1 > 0)
		    { 
             int l;
		     pthread_mutex_lock(&write_buffer_mutex);
		     pthread_mutex_lock(&act);
	         for (l=0;l<(int)activeconnections.size();l++)
		     {
              if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
		      {
			   list<string> L;
			   L.push_back(UOID_temp);
               write_buffer[activeconnections[l].port] = L;
		      }
		      else
		      {
			   map<int,list<string> >::iterator it;
               it = write_buffer.find(activeconnections[l].port);
			   it->second.push_back(UOID_temp);
		      }     
             }
		     pthread_mutex_unlock(&write_buffer_mutex);
		     pthread_mutex_unlock(&act);
            }
		    else
		    {
              pthread_mutex_unlock(&msg_buffer_mutex);
		    }
	        
           uint8_t message_type;		  
           memcpy(&message_type,&buf[27],1);
		   //char message_type1 = (char)message_type;
		   int fwd_write =1;
		   int sch_type;
	       switch(message_type){
			case 0x00:
		    {
              sch_type = 0;
              break;
			}
		    case 0x01:
			{
			  sch_type = 1;
              break;
			}
			case 0x02:
			{
			  sch_type = 2;
              break;
			}
            default:
			{
			  fwd_write =0;
			  break;
			}
		   } 
		   if(fwd_write != 0)
		   {
			 vector<string > keywords;
             char metakey[512];
             char *pch;
             string s;
			 memcpy(&metakey[0],&buf[28],total_len-1);
             pch=strtok(metakey," ");
             while(1)
		     {
		      s.clear();
		      if(pch==NULL)
			    break;
		      s.assign(pch);
		      keywords.push_back(s);
		      pch=strtok(NULL," ");
		    }
			vector<struct searchresponse > searchresponse_Vec;
            struct searchresponse str;
			str.fileid = "sdaaStytytyuioplkjhg";
            str.mdata = "sdasdaddaasdasaaS";

			struct searchresponse str1;
			str1.fileid = "sdaaStytytyuioplkjhg";
            str1.mdata = "sdasdaddaasdasaaS";
            
			searchresponse_Vec.push_back(str);
			searchresponse_Vec.push_back(str1);

			string UOID_to_temp1;
		    string msg_to_temp1;
           
			//call nikhil's function here
			searchresponse_Vec = searchProc(keywords,sch_type);
            //File_Id_Vec = get_File_Id(sch_type,keywords);
            if(((int)searchresponse_Vec.size()) == 0){
			 temp = 51;
	         msg=(unsigned char *)malloc(temp*sizeof(char));
		     if(msg == NULL){
		      cout<<"memory allocation error in case 10"<<endl;
		      exit(1);
	        }
   	        messageconstruction(msg,62,0);
		    uint32_t temper;
		    temper = 24;
	        memcpy(&msg[23],&temper,4);
	        memcpy(&msg[27],UOID_frm,20);
	        temper =0;
	        memcpy(&msg[47],&temper,4);

		    string UOID_to_temp((const char*)&msg[1],20); 
            string msg_to_temp((const char*)&msg[0],51);

			UOID_to_temp1.append(UOID_to_temp);
		    msg_to_temp1.append(msg_to_temp);

		   }
		   else
		   {
			  msg=(unsigned char *)malloc(8192*sizeof(char));
		      if(msg == NULL){
		       cout<<"memory allocation error in case 10"<<endl;
		       exit(1);
	          } 
              messageconstruction(msg,62,0);
			  memcpy(&msg[27],UOID_frm,20);
			  int curr_loc = 47;
			  int i;
			  uint32_t temper;      
              for(i=0;i<(int)searchresponse_Vec.size();i++)
			  {

                  str = searchresponse_Vec[i];                   
				  if(i == (int)searchresponse_Vec.size()-1)
				  {
					 temper = 0;                   
				  }
				  else
				  {
					 temper = str.mdata.length();
				  }
				  memcpy(&msg[curr_loc],&temper,4);
				  curr_loc+=4;
				
                  memcpy(&msg[curr_loc],str.fileid.c_str(),20);
				  curr_loc += 20;
				  
                  memcpy(&msg[curr_loc],str.mdata.c_str(),str.mdata.length());
				  curr_loc +=str.mdata.length();
			  }
			  temper = curr_loc-27;
			  memcpy(&msg[23],&temper,4);
			  string UOID_to_temp((const char*)&msg[1],20); 
              string msg_to_temp((const char*)&msg[0],curr_loc);

			  UOID_to_temp1.append(UOID_to_temp);
		      msg_to_temp1.append(msg_to_temp);

		   }
           
		  
		   pthread_mutex_lock(&msg_buffer_mutex);
	       msg_buffer.insert(std::pair<string,string>(UOID_to_temp1,msg_to_temp1));
		   pthread_mutex_unlock(&msg_buffer_mutex);
           int l;
		   pthread_mutex_lock(&write_buffer_mutex);
		   pthread_mutex_lock(&act);
	       for (l=0;l<(int)activeconnections.size();l++)
		   {
		    if(activeconnections[l].port == port)
		    {
             if(write_buffer.find(port) == write_buffer.end())
		     {
			   list<string> L;
			   L.push_back(UOID_to_temp1);
               write_buffer[port] = L;
		     }
		     else
		     {
			  map<int,list<string> >::iterator it;
              it = write_buffer.find(port);
			  it->second.push_back(UOID_to_temp1);
		     }
			 break;
		   }          
          }
		  pthread_mutex_unlock(&write_buffer_mutex);
		  pthread_mutex_unlock(&act);
 
 		  pthread_mutex_lock(&msg_UOID_port_map_mutex);
		  msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,port));
		  pthread_mutex_unlock(&msg_UOID_port_map_mutex);


	     }
		  
         
	    }
		break;
	   }

	   case 62 :
       {
         uint32_t total_length;
		 memcpy(&total_length,&buf[23],4);
		 int total_length1 = (int)total_length;
         if((numbytes=read(sockfd,&buf[27],total_length1))== -1)
		 {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		 }
		 logging(buf,0);

		
		string UOID_temp((const char*)&buf[1],20); ; 
	    string msg_temp((const char*)&buf[0],total_length1+27);
	
        pthread_mutex_lock(&msg_buffer_mutex);
	    if(msg_buffer.find(UOID_temp) == msg_buffer.end())
		{
           //duplicate does not exist
		   msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
		   pthread_mutex_unlock(&msg_buffer_mutex);
		    
           string UOID_to_temp((const char*)&buf[27],20);
		   map<string,int >::iterator it;
		   pthread_mutex_lock(&msg_UOID_port_map_mutex);
		   pthread_mutex_lock(&write_buffer_mutex);
			 if(msg_UOID_port_map.find(UOID_to_temp) == msg_UOID_port_map.end()){
				 //message deleted from queue .. no where to go 
				  pthread_mutex_unlock(&msg_UOID_port_map_mutex);
				  pthread_mutex_unlock(&write_buffer_mutex);
				  break;
			 }
			 else
			 {
			   it = msg_UOID_port_map.find(UOID_to_temp);
               int port_temp = it->second;
			   if(port_temp == atoi(Port)){
				   parse_search_response((char*)msg_temp.c_str());			    
			   }
			   else
			   { 
				 if(write_buffer.find(port_temp) == write_buffer.end())
		         {
			      list<string> L;
			      L.push_back(UOID_temp);
                  write_buffer[port_temp] = L;
		         }
		         else
		         {
			      map<int,list<string> >::iterator it;
                  it = write_buffer.find(port_temp);
			      it->second.push_back(UOID_temp);
		         }  

			   }
			}
			pthread_mutex_unlock(&msg_UOID_port_map_mutex);
		    pthread_mutex_unlock(&write_buffer_mutex);
		  }
		  else
		  {
			  pthread_mutex_unlock(&msg_buffer_mutex);
             //duplicate exists ... ignore 
		  }

		break;

	   }

	   case 43 :
       {
		  uint32_t tot_length;
		  memcpy(&tot_length,&buf[23],4);
		  long total_len = (long)tot_length;
		  int total_len1 =  (int)total_len;
         
		  if((numbytes=read(sockfd,&buf[27],total_len1))== -1)
		  {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		  }
		  
		  logging(buf,0);
	      unsigned char UOID_frm[SHA_DIGEST_LENGTH];
	      memset(&UOID_frm,0,sizeof(UOID_frm));
          memcpy(&UOID_frm,&buf[1],20);
          string UOID_temp((const char*)&buf[1],20); 
	       
	      uint8_t ttl_temp;
		  int ttl_temp1 ;
          memcpy(&ttl_temp,&buf[21],1);
		  ttl_temp1 = (int)ttl_temp -1;
	      if(ttl_temp1 >  0){
			   ttl_temp = ttl_temp-1;
		  }
	        
          // ttl_temp1 =1;
          memcpy(&buf[21],&ttl_temp,1);
	      string msg_temp((const char*)&buf[0],27);
          pthread_mutex_lock(&msg_buffer_mutex);
	      if(msg_buffer.find(UOID_temp) == msg_buffer.end())
	      {	   
	        // UOID does not exist ... flood the msg and add it to the msg buffer
	        msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
		    pthread_mutex_unlock(&msg_buffer_mutex);
		    if(ttl_temp1 > 0)
		    { 
             int l;
		     pthread_mutex_lock(&write_buffer_mutex);
		     pthread_mutex_lock(&act);
	         for (l=0;l<(int)activeconnections.size();l++)
		     {
              if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
		      {
			   list<string> L;
			   L.push_back(UOID_temp);
               write_buffer[activeconnections[l].port] = L;
		      }
		      else
		      {
			   map<int,list<string> >::iterator it;
               it = write_buffer.find(activeconnections[l].port);
			   it->second.push_back(UOID_temp);
		      }     
             }
		     pthread_mutex_unlock(&write_buffer_mutex);
		     pthread_mutex_unlock(&act);
            }
		    else
		    {
              pthread_mutex_unlock(&msg_buffer_mutex);
		    }
	        
             string fileid;
            string sha1_str;
			unsigned char abc[20],bdc[20];
			memcpy(abc,&buf[27],20);
			memcpy(bdc,&buf[47],20);
			fileid.assign((char *)abc,20);
			sha1_str.assign((char *)bdc,20);
			hex_display(20,bdc);
			hex_display(20,abc);
			//call nikhil's function to get the file_index of the file
		    int file_ind = getRequest(fileid,sha1_str);
            string metaDataFileName = constructFileName(file_ind,0);
			string dataFileName = constructFileName(file_ind,3);

			msg=(unsigned char *)malloc(8192*sizeof(char));
			messageconstruction(msg,42,0);
			memcpy(&msg[27],UOID_frm,20);
			int curr_loc = 47;
            long tot_len = 0;
            struct stat FileAttrib;
			long file_Size;

			int fwd_write = 1;

			if (stat(metaDataFileName.c_str(), &FileAttrib) == 0)
			{
              file_Size = FileAttrib.st_size;
			  tot_len +=24+file_Size;
			  memcpy(&msg[curr_loc],&file_Size,4);				
              curr_loc = curr_loc+4;                   
              fp = fopen(metaDataFileName.c_str(),"r");
			  if(fp == NULL)
			  {
				fwd_write = 0;
			  }

			  size_t result = fread(&msg[curr_loc],1,file_Size,fp);
			  if((long)result !=file_Size){
                 cout<<"Cannot read all the bytes from the metadata file"<<metaDataFileName.c_str()<<endl;
			     
			  }
              curr_loc = curr_loc+file_Size;

            }
            else
            {
			  fwd_write = 0;
            }
            
			if (stat(dataFileName.c_str(), &FileAttrib) == 0)
			{
              file_Size = FileAttrib.st_size;
			  tot_len +=file_Size;
			  memcpy(&msg[curr_loc],dataFileName.c_str(),dataFileName.length());
              curr_loc = curr_loc+dataFileName.length();
            }
            else
            {
			  fwd_write = 0;
            }	
		    
			if(fwd_write != 0){

			 memcpy(&msg[23],&tot_len,4);
		     string UOID_to_temp((const char*)&msg[1],20); 
             string msg_to_temp((const char*)&msg[0],curr_loc);
			 
		   pthread_mutex_lock(&msg_buffer_mutex);
	       msg_buffer.insert(std::pair<string,string>(UOID_to_temp,msg_to_temp));
		   pthread_mutex_unlock(&msg_buffer_mutex);
           int l;
		   pthread_mutex_lock(&write_buffer_mutex);
		   pthread_mutex_lock(&act);
	       for (l=0;l<(int)activeconnections.size();l++)
		   {
		    if(activeconnections[l].port == port)
		    {
             if(write_buffer.find(port) == write_buffer.end())
		     {
			   list<string> L;
			   L.push_back(UOID_to_temp);
               write_buffer[port] = L;
		     }
		     else
		     {
			  map<int,list<string> >::iterator it;
              it = write_buffer.find(port);
			  it->second.push_back(UOID_to_temp);
		     }
			 break;
		   }          
          }
		  pthread_mutex_unlock(&write_buffer_mutex);
		  pthread_mutex_unlock(&act);
 
 		  pthread_mutex_lock(&msg_UOID_port_map_mutex);
		  msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,port));
		  pthread_mutex_unlock(&msg_UOID_port_map_mutex);



			}				   
		 }
         break;
	     }

	   case 42 :
       {
		 
         uint32_t total_length;
		 memcpy(&total_length,&buf[23],4);
		 long total_length1 = (long)total_length;

		  if((numbytes=read(sockfd,&buf[27],20))== -1)
		  {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		  }
		  
         if(total_length1+27 <= 8192)
		 {
		  if((numbytes=read(sockfd,&buf[47],total_length1-20))== -1)
		  {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		  }
          logging(buf,0);
		  string UOID_temp((const char*)&buf[1],20); ; 
	
          pthread_mutex_lock(&msg_buffer_mutex);
	      if(msg_buffer.find(UOID_temp) == msg_buffer.end())
		  {
           //duplicate does not exist
		   pthread_mutex_unlock(&msg_buffer_mutex);
		    
           string UOID_to_temp((const char*)&buf[27],20);
		   map<string,int >::iterator it;
		   pthread_mutex_lock(&msg_UOID_port_map_mutex);
		   pthread_mutex_lock(&write_buffer_mutex);
			 if(msg_UOID_port_map.find(UOID_to_temp) == msg_UOID_port_map.end()){
				 //message deleted from queue .. no where to go 
				  pthread_mutex_unlock(&msg_UOID_port_map_mutex);
				  pthread_mutex_unlock(&write_buffer_mutex);
				  break;
			 }
			 else
			 {
			   string dataFileName;
			   string metadataFileName;
			   int fwd_write = 1;
			   int store_meta = 1;
			   int temp_file_index_counter = get_file_index_counter();
			   it = msg_UOID_port_map.find(UOID_to_temp);
               int port_temp = it->second;
			   if(port_temp != atoi(Port))
			   {
				   //got the reponse to me .. store it in a buffer
				 dataFileName.append(constructFileName(temp_file_index_counter,3));
				// file_index_counter++;
				 metadataFileName.append(constructFileName(temp_file_index_counter,0));
				 fwd_write = 0;				  		    
			   }
			   else
			   { 
                 double dval=(double)drand48();
                
                 if(dval <= CacheProb)
			     {
                   //should store it in cache ... so give file_index as the filename
			       dataFileName.append(constructFileName(temp_file_index_counter,3));				   
				   metadataFileName.append(constructFileName(temp_file_index_counter,0));
				 }
				 else
				 {
                    //should not store it .. so copy the content into the temp location
					char filename1[28];
					unsigned char UOID[SHA_DIGEST_LENGTH];
	                GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
					string temp = "private_";
                    memcpy(&filename1[0],temp.c_str(),8);
                    memcpy(&filename1[8],UOID,20);
					dataFileName.append(filename1,28);
					store_meta = 0;
				 }
			   }

              uint32_t metadata_leng;
			  long metadata_leng1;
			  memcpy(&metadata_leng,&buf[47],4);
              metadata_leng1 = (long)metadata_leng;
			  char metadata_buf[8192];
			  memset(metadata_buf,0,8192);
			  memcpy(&metadata_buf[0],&buf[51],metadata_leng1);
			  FILE *fp ;
              int result;
			  if(store_meta != 0)
			  {
				  
			   if((fp=fopen(metadataFileName.c_str(), "w")) == NULL) {
			    fwd_write = 0;
               }

			   result = fwrite(&buf[51],1,metadata_leng1,fp);
			   if(result != metadata_leng1){
                fwd_write = 0;
			   }
               fclose(fp); 

			 }

			 
			  if((fp=fopen(dataFileName.c_str(), "w")) == NULL) {
			    fwd_write = 0;
              }

              if(fp != NULL){
			    result = fwrite(&buf[51+metadata_leng1],1,total_length1-24-metadata_leng1,fp);
			    if(result != total_length1-24-metadata_leng1){
                 fwd_write = 0;
			    }
			    fclose(fp);
			  }

			   if((fp=fopen(getExtFile.c_str(), "w")) == NULL) {
			    fwd_write = 0;
              }

              if(fp != NULL){
			    result = fwrite(&buf[51+metadata_leng1],1,total_length1-24-metadata_leng1,fp);
			    if(result != total_length1-24-metadata_leng1){
                 fwd_write = 0;
			    }
			    fclose(fp);
			  }
              
              

              if(fwd_write != 0){
				//  cout<<"metadataFileName.c_str() is "<<metadataFileName.c_str()<<endl;
                 readMetadata((char*)metadataFileName.c_str());
			     memcpy(&buf[51+metadata_leng1],dataFileName.c_str(),dataFileName.length());
                 string msg_temp((const char*)&buf[0],51+metadata_leng1+dataFileName.length());
                 pthread_mutex_lock(&msg_buffer_mutex);
                 msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
                 pthread_mutex_unlock(&msg_buffer_mutex);
			
				 if(write_buffer.find(port_temp) == write_buffer.end())
		         {  
			      list<string> L;
			      L.push_back(UOID_temp);
                  write_buffer[port_temp] = L;
		         }
		         else
		         {
			      map<int,list<string> >::iterator it;
                  it = write_buffer.find(port_temp);
			      it->second.push_back(UOID_temp);
		         }
				
               }
               pthread_mutex_unlock(&msg_UOID_port_map_mutex);
		       pthread_mutex_unlock(&write_buffer_mutex);
		    }

			
		  }
		  else
		  {
			  pthread_mutex_unlock(&msg_buffer_mutex);
             //duplicate exists ... ignore 
		  }
		 
		 }
		 else
	     {
		  string dataFileName;
		  string metadataFileName;
		  unsigned char msg[8192];
		  string UOID_temp((const char*)&buf[1],20); 
		  int fwd_write = 1;
		  int port_temp;
		  int temp_file_index_counter = get_file_index_counter();
		  long metadata_leng1;
		  FILE *fp ;
		  int result;

		  if((numbytes=read(sockfd,&buf[47],8192-47))== -1)
		    {
			 event e;
			 e.disp=&auto_co[getAuto(port)];	
		 	 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(e);
			 pthread_mutex_unlock(&event_dispatcher_lock);
             pthread_exit(NULL);		
		    }
            logging(buf,0);
		   
            pthread_mutex_lock(&msg_buffer_mutex);
	        if(msg_buffer.find(UOID_temp) == msg_buffer.end())
		    {
             //duplicate does not exist
		     pthread_mutex_unlock(&msg_buffer_mutex);
		    
             string UOID_to_temp((const char*)&buf[27],20);
		     map<string,int >::iterator it;
		     pthread_mutex_lock(&msg_UOID_port_map_mutex);
			 if(msg_UOID_port_map.find(UOID_to_temp) == msg_UOID_port_map.end()){
				 //message deleted from queue .. no where to go 
				  pthread_mutex_unlock(&msg_UOID_port_map_mutex);
                  fwd_write = 0;
			 }
			 else
			 {
			   int store_meta = 1;
			   it = msg_UOID_port_map.find(UOID_to_temp);
               port_temp = it->second;
			   if(port_temp != atoi(Port)){
				   //got the reponse to me .. store it in a buffer
				 dataFileName.append(constructFileName(temp_file_index_counter,3));
				 metadataFileName.append(constructFileName(temp_file_index_counter,0));
				 fwd_write = 0;				  		    
			   }
			   else
			   { 
                 double dval=(double)drand48();                
                 if(dval <= CacheProb)
			     {
                    //should store it in cache ... so give file_index as the filename
			       dataFileName.append(constructFileName(file_index_counter,3));				   
				   metadataFileName.append(constructFileName(file_index_counter,0));
				 }
				 else
				 {
                    //should not store it .. so copy the content into the temp location
					char filename1[28];
					unsigned char UOID[SHA_DIGEST_LENGTH];
	                GetUOID(nodeinstanceid,(char *)"msg",UOID,sizeof(UOID));
					string temp = "private_";
                    memcpy(&filename1[0],temp.c_str(),8);
                    memcpy(&filename1[8],UOID,20);
					dataFileName.append(filename1,28);
					store_meta = 0;
				 }
			   }

               uint32_t metadata_leng;

			   memcpy(&metadata_leng,&buf[47],4);
               metadata_leng1 = (long)metadata_leng;
			   memcpy(&msg[0],&buf[0],47+metadata_leng1);
			   char metadata_buf[8192];
			   memset(metadata_buf,0,8192);
			   memcpy(&metadata_buf[0],&buf[51],metadata_leng1);

			   if(store_meta != 0)
			   {
				  
			    if((fp=fopen(metadataFileName.c_str(), "w")) == NULL) {
			     fwd_write = 0;
                }

			    result = fwrite(&buf[51],1,metadata_leng1,fp);
			    if(result != metadata_leng1){
                 fwd_write = 0;
			    }
                fclose(fp); 

			   }

			   if((fp=fopen(dataFileName.c_str(), "w")) == NULL) {
			    fwd_write = 0;
               }

               if(fp != NULL){
			    result = fwrite(&buf[51+metadata_leng1],1,8192-51-metadata_leng1,fp);
			    if(result != total_length1-51-metadata_leng1){
                 fwd_write = 0;
			    }
			    fclose(fp);
			   }

			    if((fp=fopen(getExtFile.c_str(), "w")) == NULL) {
			    fwd_write = 0;
              }

              if(fp != NULL){
			    result = fwrite(&buf[51+metadata_leng1],1,8192-51-metadata_leng1,fp);
			    if(result != 8192-51-metadata_leng1){
                 fwd_write = 0;
			    }
			    fclose(fp);
			  }
			  pthread_mutex_unlock(&msg_UOID_port_map_mutex);
			 }

		    }
		    else
		    {
			  pthread_mutex_unlock(&msg_buffer_mutex);
              fwd_write = 0;
             //duplicate exists ... ignore 
		    }
           
		  total_length1 = total_length1-20-numbytes;
		  while(total_length1 > 0)
		  {
		    if(total_length1 < 8192)
		    {
			 if((numbytes=read(sockfd,&buf[0],total_length1))== -1)
		     {
			  event e;
			  e.disp=&auto_co[getAuto(port)];	
		 	  pthread_mutex_lock(&event_dispatcher_lock);
			  event_dispatcher.push(e);
			  pthread_mutex_unlock(&event_dispatcher_lock);
              pthread_exit(NULL);		
		     }		 
	
             if((fp=fopen(dataFileName.c_str(), "a")) == NULL) {
			    fwd_write = 0;
             }

             if(fp != NULL){
			  result = fwrite(&buf[0],1,total_length1,fp);
			  if(result != total_length1){
               fwd_write = 0;
			  }
			  fclose(fp);
			 }

			  if((fp=fopen(getExtFile.c_str(), "a")) == NULL) {
			    fwd_write = 0;
             }

             if(fp != NULL){
			  result = fwrite(&buf[0],1,total_length1,fp);
			  if(result != total_length1){
               fwd_write = 0;
			  }
			  fclose(fp);
			 }
            total_length1 = total_length1-numbytes;
		   }
		   else
	       {
			 if((numbytes=read(sockfd,&buf[0],8192))== -1)
		     {
			  event e;
			  e.disp=&auto_co[getAuto(port)];	
		 	  pthread_mutex_lock(&event_dispatcher_lock);
			  event_dispatcher.push(e);
			  pthread_mutex_unlock(&event_dispatcher_lock);
              pthread_exit(NULL);		
		     }
			   if((fp=fopen(dataFileName.c_str(), "a")) == NULL) {
			    fwd_write = 0;
             }

             if(fp != NULL){
			  result = fwrite(&buf[0],1,8192,fp);
			  if(result != 8192){
               fwd_write = 0;
			  }
			  fclose(fp);
			 }

			  if((fp=fopen(getExtFile.c_str(), "a")) == NULL) {
			    fwd_write = 0;
             }

             if(fp != NULL){
			  result = fwrite(&buf[0],1,8192,fp);
			  if(result != 8192){
               fwd_write = 0;
			  }
			  fclose(fp);
			 }
            total_length1 = total_length1-numbytes;
		   }
		 }

         if(fwd_write != 0){
           readMetadata((char*)metadataFileName.c_str());
		   memcpy(&msg[51+metadata_leng1],dataFileName.c_str(),dataFileName.length());
           string msg_temp((const char*)&msg[0],51+metadata_leng1+dataFileName.length());
           pthread_mutex_lock(&msg_buffer_mutex);
           msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
           pthread_mutex_unlock(&msg_buffer_mutex);
		  if(write_buffer.find(port_temp) == write_buffer.end())
		  {  
			 list<string> L;
			 L.push_back(UOID_temp);
             write_buffer[port_temp] = L;
		  }
		  else
		  {
			 map<int,list<string> >::iterator it;
             it = write_buffer.find(port_temp);
			 it->second.push_back(UOID_temp);
		  }
				
		  pthread_mutex_unlock(&msg_UOID_port_map_mutex);
		  pthread_mutex_unlock(&write_buffer_mutex);
         }
		
	   }
	   break;    
	  }
		
		 
	  case 23 :
      {
		//logging(buf,0);
        uint32_t total_length;
	  	memcpy(&total_length,&buf[23],4);
		long total_length1 = (long)total_length;
		if((numbytes=read(sockfd,&buf[0],total_length1))== -1)
		{
		   event e;
		   e.disp=&auto_co[getAuto(port)];	
		   pthread_mutex_lock(&event_dispatcher_lock);
		   event_dispatcher.push(e);
		   pthread_mutex_unlock(&event_dispatcher_lock);
           pthread_exit(NULL);		
		}
        string FileSpec((const char*)&buf[0],(int)total_length1);
		delFileResponse(FileSpec);
		//call nikhil's function 
	    break;
	  }
        
       default:
		  break;

     }
	}
     
	pthread_exit(NULL);
}
void *write(void *ptr)
{
    vector<string> keywords;	
	
	signal(SIGPIPE,sigpipehandler);
	list<string>local_msg_buffer;
    map<string,string>::iterator iter2;


	struct soc* soc_ptr = (struct soc*)ptr;

	int sockfd = soc_ptr->sockfd;
	int port = soc_ptr->port;

	

	while(soc_ptr->global_variable)
	{
	   
       pthread_mutex_lock(&write_buffer_mutex);
	   if(write_buffer.find(port) == write_buffer.end()){
		  
		  pthread_mutex_unlock(&write_buffer_mutex);
		  sleep(1);
		  continue; 
	   }
	   else
	   {
         map<int,list<string> >::iterator iter;
         iter = write_buffer.find(port);
		 pthread_mutex_lock(&msg_buffer_mutex);
		 for (list<string>::iterator iter1 = iter->second.begin(); iter1!= iter->second.end(); ++iter1)
		 {
            if(msg_buffer.find(*iter1) ==  msg_buffer.end())
		    {
				continue;
			}
			else
		    {
			   iter2 = msg_buffer.find(*iter1);
               local_msg_buffer.push_back(iter2->second);
			}
         }     	  
	    }
  	  pthread_mutex_unlock(&msg_buffer_mutex);
      write_buffer.erase(write_buffer.find(soc_ptr->port));
	  pthread_mutex_unlock(&write_buffer_mutex);
	  string temp;
	  list<string>::iterator itera;
	  for ( itera=local_msg_buffer.begin() ; itera != local_msg_buffer.end(); itera++ )
      {
         temp = *itera;
         int num_bytes;
		 unsigned char *tempest = (unsigned char *)temp.c_str();
		 switch(find_Msg_Type(tempest))
		 {
			 num_bytes =0;
			 case 10:
			 {
				logging(tempest,2);
			    if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),28)) == -1) 
	              {
                    perror("recvs");		
                    event e;
		            e.disp=&auto_co[getAuto(port)];	 
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
	              }
				   local_msg_buffer.erase(itera);
				  break;
		     }
             case 11:
			 {
				 logging(tempest,1);
				 if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),temp.length())) == -1) 
	              {
                   perror("recvs");		
                  event e;
		            e.disp=&auto_co[getAuto(port)];	

		         
		           	
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
	              }
				   local_msg_buffer.erase(itera);
				  break;
		     }

			 case 1:
			 {
                  logging(tempest,2);
				  if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),46)) == -1) 
	              {
                    perror("recvs");	
				    event e;
		            e.disp=&auto_co[getAuto(port)];			            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
                   
	              }
                  local_msg_buffer.erase(itera);
				  break;
			 }
			 case 2:
			 {
				 logging(tempest,1);
			    if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),66)) == -1) 
	              {
                   perror("recvs");		
                    event e;
		            e.disp=&auto_co[getAuto(port)];			           	
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
	              }
                  local_msg_buffer.erase(itera);
				  break;
		     }
             
             case 8:
			 {
				logging(tempest,1);
			    if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),27)) == -1) 
	              {
                   perror("recvs");		
                    event e;
		            e.disp=&auto_co[getAuto(port)];		            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
	              }
				   local_msg_buffer.erase(itera);
				  break;
		     }
              case 33:
			 {
				logging(tempest,1);
				uint32_t metadata_length;
				long metadata_length1;
                memcpy(&metadata_length,&tempest[27],4);
				metadata_length1 = (long)metadata_length;
				char *file_to_send = new char[256];
				memset(file_to_send,0,256);
				int len_temp = temp.length();
				memcpy(file_to_send,&tempest[31+metadata_length1],(len_temp-31-metadata_length1));
                file_to_send[len_temp-30-metadata_length1] = '\0';
				int write_fail =0;
                
				uint32_t total_length;
				long total_length1;
			    memcpy(&total_length,&tempest[23],4);
                total_length1 = (long)total_length;

				unsigned char buf[8192];
				memcpy(&buf[0],&tempest[0],31+metadata_length1);
				FILE *fp;
				long file_size = (total_length1-4-metadata_length1);
				if((total_length1+27) <= 8192)
				{
                  fp = fopen(file_to_send,"r");
				  if(fp == NULL)
				  {
					 local_msg_buffer.erase(itera);
                     break;
				  }
                   
				  size_t result = fread(&buf[31+metadata_length1],1,file_size,fp);
				  if((long)result !=file_size){
					   //cout<<"erasing 3 "<<endl;
					  fclose(fp);
					  local_msg_buffer.erase(itera);
                      break;
				  }
                   
                  if ((num_bytes = write(soc_ptr->sockfd,&buf,total_length1+27)) == -1) 
	              {
                    perror("recvs");		
                    event e;
		            e.disp=&auto_co[getAuto(port)];		            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
	              }
				  fclose(fp);
				 // cout<<"erasing "<<endl;
				  local_msg_buffer.erase(itera);
                 
				}
				else
				{
				  int pos = 0;
				  int cur_file_pos = 0;
				  fp = fopen(file_to_send,"r");
				  if(fp == NULL)
				  {
					 write_fail =1;
                     break;
				  }
				  while(file_size >= 8192)
				  {  
				   if(pos == 0)
				   {
					   
					 //cout<<"reading from file"<<endl;
                     size_t result = fread(&buf[31+metadata_length1],1,8192-31-metadata_length1,fp);
				     if((long)result != 8192-31-metadata_length1){
					  write_fail = 1;
                      break;
				     }
					 file_size = file_size-8192+31+metadata_length1;
                     cur_file_pos=(8192-31-metadata_length1);
					 if(fseek(fp,cur_file_pos,SEEK_SET)!= 0){
                      printf("failed to move to the offset\n");
			  	      printf("File Error Message = %s for file '%s'\n", strerror(errno),buf);
					  write_fail = 1;
                      break;
		            }
				    
					 pos++;
				   }
				   else
				   {
					 memset(&buf,0,8192);
					 size_t result = fread(&buf[0],1,8192,fp);
				     if((long)result != 8192){
					  write_fail = 1;
                      break;
				     }
					  file_size = file_size-8192;
                      cur_file_pos = cur_file_pos+8192;
					 if(fseek(fp,cur_file_pos,SEEK_SET)!= 0){
                     printf("failed to move to the offset\n");
			  	     printf("File Error Message = %s for file '%s'\n", strerror(errno),buf);
					  write_fail = 1;
                      break;
		            }
					 pos++;
				   }

                  
                   if ((num_bytes = write(soc_ptr->sockfd,&buf,8192)) == -1) 
	               {
                    perror("recvs");		
                    event e;
		            e.disp=&auto_co[getAuto(port)];		            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					write_fail = 1;
					break;
	               }
				  }

				   if(write_fail == 1){
					   fclose(fp);
					   local_msg_buffer.erase(itera);
					   break;
				   }
				   else if(file_size > 0)
				   {
					   memset(buf,0,8192);
                     size_t result = fread(&buf[0],1,file_size,fp);
				     if((long)result != file_size){
					  //write_fail = 1;
					  fclose(fp);
                      local_msg_buffer.erase(itera);
                      break;
				     }
                     if ((num_bytes = write(soc_ptr->sockfd,&buf,file_size)) == -1) 
	                 {
                      perror("recvs");		
                      event e;
					  fclose(fp);
		              e.disp=&auto_co[getAuto(port)];		            
		              pthread_mutex_lock(&event_dispatcher_lock);
		              event_dispatcher.push(e);
		              pthread_mutex_unlock(&event_dispatcher_lock);
					  soc_ptr->global_variable = 0;
					  write_fail = 1;
					  break;
	                 }
					local_msg_buffer.erase(itera);
					fclose(fp);
				  }
				}				  
				  break;
			 }
             case 63 : 
		     {
				  logging(tempest,2);
				  if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),temp.length())) == -1) 
	              {
                    perror("recvs");	
				    event e;
		            e.disp=&auto_co[getAuto(port)];			            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
                   
	              }
                  local_msg_buffer.erase(itera);
				 break;
			 }

			 case 62 : 
		     {
				  logging(tempest,1);
				  if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),temp.length())) == -1) 
	              {
                    perror("recvs");	
				    event e;
		            e.disp=&auto_co[getAuto(port)];			            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
                   
	              }
                  local_msg_buffer.erase(itera);
				 break;
			 }

			 case 43 : 
		     {
				  logging(tempest,2);
				  if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),67)) == -1) 
	              {
                    perror("recvs");	
				    event e;
		            e.disp=&auto_co[getAuto(port)];			            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
                   
	              }
                  local_msg_buffer.erase(itera);

				 break;
			 }

			 case 42 : 
		     {
				logging(tempest,1);
				uint32_t metadata_length;
				long metadata_length1;
                memcpy(&metadata_length,&tempest[47],4);
				metadata_length1 = (long)metadata_length;
				char *file_to_send = new char[256];
				memset(file_to_send,0,256);
				memcpy(&file_to_send[0],&tempest[51+metadata_length1],(temp.length()-51-metadata_length1));
                file_to_send[temp.length()-50-metadata_length1] = '\0';
                string file_send(file_to_send,temp.length()-50-metadata_length1);
				int write_fail =0;
                
				uint32_t total_length;
				long total_length1;
			    memcpy(&total_length,&tempest[23],4);
                total_length1 = (long)total_length;
				unsigned char buf[8192];
				memcpy(&buf[0],&tempest[0],51+metadata_length1);
				FILE *fp;
				long file_size = (total_length1-24-metadata_length1);
				if((total_length1+27) <= 8192)
				{
                  fp = fopen(file_to_send,"r");
				  if(fp == NULL)
				  {
					 local_msg_buffer.erase(itera);
                     break;
				  }
                   
				  size_t result = fread(&buf[51+metadata_length1],1,file_size,fp);
				  if((long)result !=file_size){
					  fclose(fp);
					  local_msg_buffer.erase(itera);
                      break;
				  }
                   
                  if ((num_bytes = write(soc_ptr->sockfd,&buf,total_length1+27)) == -1) 
	              {
                    perror("recvs");		
                    event e;
		            e.disp=&auto_co[getAuto(port)];		            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
	              }
				  fclose(fp);
				  local_msg_buffer.erase(itera);
                 
				}
				else
				{
				  int pos = 0;
				  int cur_file_pos = 0;
				  fp = fopen(file_to_send,"r");
				  if(fp == NULL)
				  {
					 write_fail =1;
                     break;
				  }
				  while(file_size >= 8192)
				  {  
				   if(pos == 0)
				   {
                     size_t result = fread(&buf[51+metadata_length1],1,8192-51-metadata_length1,fp);
				     if((long)result != 8192-51-metadata_length1){
					  write_fail = 1;
                      break;
				     }
					 file_size = file_size-8192+51+metadata_length1;
                     cur_file_pos=(8192-51-metadata_length1);
					 if(fseek(fp,cur_file_pos,SEEK_SET)!= 0){
                      printf("failed to move to the offset\n");
			  	      printf("File Error Message = %s for file '%s'\n", strerror(errno),buf);
					  write_fail = 1;
                      break;
		            }
				    
					 pos++;
				   }
				   else
				   {
					 memset(&buf,0,8192);
					 size_t result = fread(&buf[0],1,8192,fp);
				     if((long)result != 8192){
					  write_fail = 1;
                      break;
				     }
					  file_size = file_size-8192;
                      cur_file_pos = cur_file_pos+8192;
					 if(fseek(fp,cur_file_pos,SEEK_SET)!= 0){
                     printf("failed to move to the offset\n");
			  	     printf("File Error Message = %s for file '%s'\n", strerror(errno),buf);
					  write_fail = 1;
                      break;
		            }
					 pos++;
				   }

                  
                   if ((num_bytes = write(soc_ptr->sockfd,&buf,8192)) == -1) 
	               {
                    perror("recvs");		
                    event e;
		            e.disp=&auto_co[getAuto(port)];		            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					write_fail = 1;
					break;
	               }
				  }

				   if(write_fail == 1){
					   fclose(fp);
					   local_msg_buffer.erase(itera);
					   break;
				   }
				   else if(file_size > 0)
				   {
					   memset(buf,0,8192);
                     size_t result = fread(&buf[0],1,file_size,fp);
				     if((long)result != file_size){
					  fclose(fp);
                      local_msg_buffer.erase(itera);
                      break;
				     }
                      
			
                     if ((num_bytes = write(soc_ptr->sockfd,&buf,file_size)) == -1) 
	                 {
                      perror("recvs");		
                      event e;
					  fclose(fp);
		              e.disp=&auto_co[getAuto(port)];		            
		              pthread_mutex_lock(&event_dispatcher_lock);
		              event_dispatcher.push(e);
		              pthread_mutex_unlock(&event_dispatcher_lock);
					  soc_ptr->global_variable = 0;
					  write_fail = 1;
					  break;
	                 }
					local_msg_buffer.erase(itera);
					fclose(fp);
				  }
				}
 				
			 break;
			 }

			 case 23 : 
		     {
				  logging(tempest,2);
				  if ((num_bytes = write(soc_ptr->sockfd,temp.c_str(),temp.length())) == -1) 
	              {
                    perror("recvs");	
				    event e;
		            e.disp=&auto_co[getAuto(port)];			            
		            pthread_mutex_lock(&event_dispatcher_lock);
		            event_dispatcher.push(e);
		            pthread_mutex_unlock(&event_dispatcher_lock);
					soc_ptr->global_variable = 0;
					break;
                   
	              }
                  local_msg_buffer.erase(itera);

				 break;
			 }
			

			 default:
			 {
				 break;
			 }
		 }
	  }

	}
	/*------------Send Notify Message because the node is closing---------------------*/
	unsigned char *buf;
	buf=(unsigned char *)malloc(sizeof(char)*28);
       
	messageconstruction(buf,7,0);
	logging(buf,1);
	if (write(sockfd,buf,28) == -1) 
		{
	    pthread_exit(NULL);
		}

	/*--------------------------------------------------------------------------------*/

	pthread_exit(NULL);

}

void *timer(void *ptr)
{

int i;
std::queue<timeparam> q;
timeparam temp;
event l;
while(shutdowns)
	{
	
	/*----------Obtaining the Timer Buffer Lock, decrementing values -----------------------*/
	pthread_mutex_lock(&time_buffer_lock);
	for(i=0;i<(int)time_buffer.size();i++)
	{
	
	time_buffer[i].value--;
	if(time_buffer[i].value==0)
		{
		q.push(time_buffer[i]);
		time_buffer.erase(time_buffer.begin()+i);
		}

	}
	pthread_mutex_unlock(&time_buffer_lock);
	/*----------------------------------------------------------------------------------------*/
	while (!q.empty())
	{
     temp=q.front();
	 switch(temp.type)
	 {
		 /*---------------------Retry Condition--------------------------*/
		 case 0:
		 {
			 pthread_mutex_lock(&retry_lock);
		 
			 l.disp=&retry_co[retry_co_map[atoi(temp.index.c_str())]];
				  
			 pthread_mutex_unlock(&retry_lock);

			 pthread_mutex_lock(&event_dispatcher_lock);
			 event_dispatcher.push(l);
			 pthread_mutex_unlock(&event_dispatcher_lock);
			 break;
		 /*-------------------------------------------------------------*/
         }
		 /*-----------------Keep Alive TimeOut--------------------------*/
		 case 1: // Keep Alive Time out
		 {
		 /*event l;
			 pthread_mutex_lock(&log_lock);

			fflush(fp);
			pthread_mutex_unlock(&log_lock);
			
			 l.disp=&auto_co[getAuto(atoi(temp.index.c_str()))];	
			 l.port=atoi(temp.index.c_str());

			 pthread_mutex_lock(&event_dispatcher_lock);

			 event_dispatcher.push(l);
			 pthread_mutex_unlock(&event_dispatcher_lock);*/
			 
			 break;
		 }
		/*--------------------------------------------------------------*/

		/*-----------------Keep Alive Retry-----------------------------*/
		 case 2: //Keep Alive Retry
		 {
			 pthread_mutex_lock(&event_dispatcher_lock);
			 unsigned char buf[27];
			 messageconstruction(buf,8,0);
			 string UOID_temp((const char*)&buf[1],20); 
			 string msg_temp((const char*)&buf[0],27);

			 msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));

		
		    pthread_mutex_lock(&write_buffer_mutex);
	        
             if(write_buffer.find(atoi(temp.index.c_str())) == write_buffer.end())
		     {
			   list<string> L;
			   L.push_back(UOID_temp);
               write_buffer[atoi(temp.index.c_str())] = L;
		     }
		     else
		     { 
			  map<int,list<string> >::iterator it;
              it = write_buffer.find(atoi(temp.index.c_str()));
			  it->second.push_back(UOID_temp);
		     }     
            
		    pthread_mutex_unlock(&msg_buffer_mutex);
		    pthread_mutex_unlock(&write_buffer_mutex);
			 pthread_mutex_unlock(&event_dispatcher_lock);	
			 setAgainKeepAliveRetry(temp.index);
			break;
		 }
		/*--------------------------------------------------------------*/

		/*-------------AutoShutDown-------------------------------------*/
        case 3:
		{
		
			shutdowns=0;
			break;
		}
		/*--------------------------------------------------------------*/

	  }
     q.pop();
	}

    sleep(1);
	}
	
pthread_exit(NULL);
}

void *eventdispatcher(void *ptr)
{
while(event_shutdown)
	{
	/*----------------Obtaining the lock for event dispatcher-----------------*/
	pthread_mutex_lock(&event_dispatcher_lock);

	while(!event_dispatcher.empty())
	{
	event e=event_dispatcher.front();
		{
	pthread_cond_signal(e.disp);
	
		}
	
	event_dispatcher.pop();	
	}
	pthread_mutex_unlock(&event_dispatcher_lock);
	/*-----------------------------------------------------------------------*/
	}
	pthread_exit(NULL);

}



void removeConnection(int port)
{
/*---------------------------Removing the connection from Active Connection List-----------------------------*/
pthread_mutex_lock(&act);
for(int i=0;i<(int)activeconnections.size();i++)
	{
	if(activeconnections[i].port==port)
		{
		activeconnections.erase(i+activeconnections.begin());
		}
	}
pthread_mutex_unlock(&act);

/*-----------------------------------------------------------------------------------------------------------*/

/*---------------------------Removing the port from time buffer----------------------------------------------*/
std::string s;
std::stringstream out;
out << port;
s = out.str();
pthread_mutex_lock(&time_buffer_lock);
for(int i=0;i<(int)time_buffer.size();i++)
	{
	if(time_buffer[i].index.compare(s)==0)
		{
		time_buffer.erase(time_buffer.begin()+i);
		}

	}
pthread_mutex_unlock(&time_buffer_lock);
/*-----------------------------------------------------------------------------------------------------------*/
/*----------------Removing from auto_co & retry_cond buffers------------------------------------------------*/

	auto_co_map.erase(getAuto(port));
/*------------------------------------------------------------------------------------------------------------*/




}


/*--------------------------Checking whether Port Number is present in Active Connections or not---------------------*/
bool isActive(int port)
{
if(port==0)
	return true;
else
	{
	 pthread_mutex_lock(&act);
	 for(int i=0;i<(int)activeconnections.size();i++)
		{
	if(activeconnections[i].port==port)
		{
		 pthread_mutex_unlock(&act);
		return true;
		}
		}
     pthread_mutex_unlock(&act);
	 return false;
	}

}
/*-------------------------------------------------------------------------------------------------------------------*/




int getAuto(int port)
{
int a;
pthread_mutex_lock(&auto_co_lock);
a=auto_co_map[port];
pthread_mutex_unlock(&auto_co_lock);
return a;
}

void setAuto(int port)
{
pthread_mutex_lock(&auto_co_lock);
pthread_cond_init(&auto_co[auto_co_index],NULL);
auto_co_map.insert(pair<int,int>(port,auto_co_index));
auto_co_index++;
pthread_mutex_unlock(&auto_co_lock);
}

void siginthandler(int sig)
{
	search_flag=0;
 signal(SIGINT,siginthandler);
}


void threadfailed()
{
fprintf(stderr,"Thread Creation Failed\n");
exit(1);
}

int isBeacon()
{
	int i;
for(i=0;i<(int)beacons.size();i++)
	{
	if(beacons[i].compare(Port)==0)
		{
return 1;
		}

	}

return 0;
}

//Checks whether a socket is ready or not
int sock_ready(int sock) 
{ 
    int             res; 
    fd_set          sready; 
    struct timeval  nowait; 

	nowait.tv_usec=200;
    FD_ZERO(&sready); 
    FD_SET((unsigned int)sock,&sready); 

    memset((char *)&nowait,0,sizeof(nowait)); 


    res = select(sock+1,&sready,NULL,NULL,&nowait); 
    if( FD_ISSET(sock,&sready) ) 
        res = 1; 
    else 
        res = 0; 

    return(res); 
}


void sigpipehandler(int sig)

{

signal(SIGPIPE,sigpipehandler);
}

int find_Msg_Type(unsigned char *buf){
   uint8_t message_type=0;
   memcpy(&message_type,buf,1);
   switch (message_type) {
    case 0xfc:
    {
      return 1;
	}
	case 0xfa:
    {
      return 0;
	}
	case 0xfb:
    {
      return 2;
	}
	case 0xac:
    {
      return 10;
	}
	case 0xab:
    {
      return 11;
	}
	case 0xf7:
	{
      return 7;
	}
	case 0xf8:
	{
      return 8;
	}
	case 0xcc:
	{
      return 33;
	}
	case 0xec:
	{
      return 63;
	}
	case 0xeb:
	{
      return 62;
	}
	case 0xdc:
	{
      return 43;
	}
	case 0xdb:
	{
      return 42;
	}
	case 0xbc:
	{
      return 23;
	}
   }
   return -1; 
}



void status_msg_generate(int ttl,string filename,int message_type){

  int fileNum = name.setFileName(filename);

  if(message_type == 1)
  {   
    name.initConnections(fileNum);
  }
  else
  {	
	 unsigned char *msg;
	 msg=(unsigned char *)malloc(8192*sizeof(char));
	 uint16_t size1=strlen(Port)+strlen("nunki.usc.edu");
	 memcpy(&msg[47],&size1,2);
     uint16_t port_temp1 = atoi(Port);
     memcpy(&msg[49],&port_temp1,2);
	   
	memcpy(&msg[51],(unsigned char*)hostname,strlen(hostname));
	int curr_loc = 64;
	int i;
	char filename1[256];

	long tot_len = 37;
            
	std::string s;
                
    struct stat FileAttrib;
	long file_Size;
	messageconstruction(msg,11,0);

	char filename2[100];

   int temp_file_index_counter = file_index_counter;
   int j = 0;
    long len_s = 0;
   int curr_current_loc = 0;
	for(i=1;i<temp_file_index_counter;i++){
      std::stringstream out_ss;
	  out_ss << i;
	  s = out_ss.str();
	 


				 strcpy(filename1,HomeDir);
                 strcat(filename1,"/");
                 strcat(filename1,"files/");

				 strcpy(filename2,s.c_str());
		         strcat(filename2,".metadata");
		         strcat(filename1,filename2);

				 if (stat(filename1, &FileAttrib) == 0){
					j++;
                    file_Size = FileAttrib.st_size;
					if(curr_loc+4+file_Size > 8192){
						break;
					}
					if(i == temp_file_index_counter-1){
					  
                      memcpy(&msg[curr_loc],&len_s,4);
					}
					else
					{
                      memcpy(&msg[curr_loc],&file_Size,4);
					}
					curr_current_loc = curr_loc;
                    curr_loc = curr_loc+4;
					tot_len +=4;
                    
                    fp = fopen(filename1,"r");
				    if(fp == NULL)
				    {
					 continue;
				    }
					
					size_t result = fread(&msg[curr_loc],1,file_Size,fp);
				    if((long)result !=file_Size){
					  continue;
				    }
                    curr_loc = curr_loc+file_Size;
					tot_len +=file_Size;

                 }
                 else
                 {
	               continue;
                 }
				   
				}
				memcpy(&msg[23],&tot_len,4);
				if(curr_current_loc != 0)
				memcpy(&msg[curr_current_loc],&len_s,4);

      name.populate((char *)msg,2,fileNum,1);


  }

  if(ttl > 0){
    unsigned char *buf;
    int temp=28;
    buf=(unsigned char *)malloc(temp*sizeof(char));
	if(message_type == 1){
	  messageconstruction(buf,10,1);

	}
	else
	{
	  messageconstruction(buf,10,2);
	}
   
	uint8_t ttl_temp = (uint8_t)ttl;
	memcpy(&buf[21],&ttl_temp,1);
    string UOID_temp((const char*)&buf[1],20); 
    string msg_temp((const char*)&buf[0],temp);

	//memcpy(&ttl_temp,&buf[27],1);
	/*switch(ttl_temp){
		case 0x01: {
			cout<<"in 1"<<endl;
			break;
		}
		case 0x02: {
			cout<<"in 2"<<endl;
			break;
		}

	}*/
    
    pthread_mutex_lock(&msg_buffer_mutex);
	msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
	pthread_mutex_unlock(&msg_buffer_mutex);
		
    int l;
    pthread_mutex_lock(&write_buffer_mutex);
    pthread_mutex_lock(&act);
	for (l=0;l<(int)activeconnections.size();l++)
    {
      if(write_buffer.find(activeconnections[l].port) == write_buffer.end())
	  {
	    list<string> L;
		L.push_back(UOID_temp);
        write_buffer[activeconnections[l].port] = L;
	  }
	  else
	  {
		 map<int,list<string> >::iterator it;
         it = write_buffer.find(activeconnections[l].port);
		 it->second.push_back(UOID_temp);
	  }     
    }
	pthread_mutex_unlock(&write_buffer_mutex);
	pthread_mutex_unlock(&act);

	pthread_mutex_lock(&msg_UOID_status_filename_map_mutex);
	msg_UOID_status_filename_map.insert(std::pair<string,int>(UOID_temp,fileNum));
    pthread_mutex_unlock(&msg_UOID_status_filename_map_mutex);
    
	pthread_mutex_lock(&msg_UOID_port_map_mutex);
	msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,atoi(Port)));
    pthread_mutex_unlock(&msg_UOID_port_map_mutex);

	pthread_mutex_lock(&msg_UOID_status_msg_type_map_mutex);
    switch(message_type){
	 case 0x01: 
	 {
	   msg_UOID_status_msg_type_map.insert(std::pair<string,int>(UOID_temp,1));
	 }
     case 0x02: 
	 {
	    msg_UOID_status_msg_type_map.insert(std::pair<string,int>(UOID_temp,2));
	 }
   }
   pthread_mutex_unlock(&msg_UOID_status_msg_type_map_mutex);
   


	

}

   
}

void *NonBeacon_client(void *ptr)
{
	//TCP Control Variables
	int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	//End of TCP

	int done=0,i;
	unsigned char *msg;
	unsigned char buf1[10][512];
	int temp=0,numbytes;
	pthread_t self;
	self=pthread_self();
	struct LocDistStruct *param;
    param = (struct LocDistStruct *)malloc(sizeof(struct LocDistStruct));
	memset(param,0,sizeof(struct LocDistStruct));

    if ((rv = getaddrinfo("nunki.usc.edu", (char *)ptr, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "non beacon client : getaddrinfo: %s\n", gai_strerror(rv));
		pthread_exit(NULL);	
	}
    // loop through all the results and connect to the first we can
    p=servinfo;
    while(!done)
	{
		 if(shutdowns == 0){
           pthread_exit(NULL);	
		 }
		 if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		 {
		   perror("client: socket");
		 }
	     if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		 {
					done=0;
					sleep(Retry);
					close(sockfd);
		  }
		  else
		    done=1;
	}
    if (p == NULL) 
	{
        fprintf(stderr, "client: failed to connect\n");
		pthread_exit(NULL);	
	}
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
    freeaddrinfo(servinfo); // all done with this structure

	temp=strlen(hostname)+33;
	msg=(unsigned char *)malloc(temp*sizeof(char));
	messageconstruction(msg,1,0);

	uint16_t temper;
	memcpy(&temper,&msg[31],2);
    
    logging(msg,1);
	int num_bytes;
    if ((num_bytes = write(sockfd,msg,temp)) == -1) 
	{
        perror("recvs");		
        pthread_exit(NULL);	
	}
    
    i =0;
	uint32_t temp_dist;

    signal (SIGALRM, catch_alarm_msg_join);
	alarm (JoinTimeout);

	while(shutdowns == 1)
	{
	  if((non_beacon_join_timeout == 1) && sock_ready(sockfd))
	  {
	    if((numbytes = read(sockfd,buf1[i],66))) 
        {
        // printf ("Error reading  file unexist.ent: %s\n",strerror(errno));
		 break;
        }	
		logging(buf1[i],0);
        buf1[i][numbytes] = '\0';
    
    
	    if(numbytes > 0 && numbytes == 66){
         buf1[i][numbytes]='\0';

		 memcpy(&temp_dist,&buf1[i][47],4);
         param->distance = temp_dist;
	     memcpy(&param->port,&buf1[i][51],2);
         param->hostname = hostname;
	    // memcpy(&param->hostname,&buf1[i][53],numbytes-53);
	     
         neighbors.push_back(param);
         i++;
		 param = (struct LocDistStruct *)malloc(sizeof(struct LocDistStruct));
	     memset(param,0,sizeof(struct LocDistStruct));
	   }
	  }
	  else
	  {
	      if(non_beacon_join_timeout == 1){
			  continue;
		  }
		  else
		  break;
	  }
  
    }

  
	unsigned char *buf;
	buf=(unsigned char *)malloc(sizeof(char)*28);
       
	messageconstruction(buf,7,0);
	logging(buf,2);
	
	if (write(sockfd,buf,28) == -1) 
	{
		
	  
	   pthread_exit(NULL);
	}

	/*--------------------------------------------------------------------------------*/
	

	close(sockfd);

    if(shutdowns == 0){
	   //close the threads and then quit
       pthread_exit(NULL);	
	}

    if((int)neighbors.size() < InitNeighbors)
	{
		cout<<"Number of neighbours are less .. quitting"<<endl;		
			
	}
	else
	{
	   int i;
       sort(neighbors.begin(), neighbors.end(), sort_neighbour_list);
	   FILE *fptr1;
	   char filename1[100];
       strcpy(filename1,HomeDir);
       strcat(filename1,"/");
       strcat(filename1,"init_neighbor_list");
       fptr1= fopen(filename1,"w");

       if (fptr1 == NULL) 
	   {
        pthread_exit(NULL);
       }
	   for(i=0;i<InitNeighbors;i++)
       {
	     param = neighbors[i];  
		 fprintf(fptr1,"%s:%d\n",param->hostname,param->port);
       }
	 fclose(fptr1);  
	}
	 
	pthread_exit(NULL);	
 	
}  


void non_beacon_client_connect(){
	
       FILE *fptr1;
	   int i,j;
	   char buf[InitNeighbors][512];
	   char filename1[100];
       strcpy(filename1,HomeDir);
       strcat(filename1,"/");
       strcat(filename1,"init_neighbor_list");
       fptr1 = fopen(filename1,"r");
      // string port_temps[InitNeighbors]; 
                 
			char *hostname,*temp[InitNeighbors];
			j=0,i=0;
			neighbour_joined = 0;
			memset(buf[j],0,512);
		    while(fgets(buf[j],sizeof(buf[j]),fptr1) != NULL)
			{
               hostname = strtok(buf[j],":");
			   temp[j] = strtok(NULL,"\n");
               if(temp[j] == NULL){
                   continue;
			   }
			   else
			   {					      
	               j++;
			   }
            } 
			  for(i=0;i<j;i++){
                  if((pthread_create(&cli[i],NULL,client,(void *)(temp[i]))))
	              {
		             threadfailed();
	              }	 

			  }
		   fclose(fptr1);			
			           
	
}
void catch_alarm_msg_join (int sig)
 {
   non_beacon_join_timeout = 0;
   signal (sig, catch_alarm_msg_join);
 }

bool sort_neighbour_list(struct LocDistStruct *a ,struct LocDistStruct *b){
   return(a->distance < b->distance);

}

void getType(char msgtype[],unsigned char msg[])
{
char a,temp;
a=msg[0];
temp=0xfc;
if(a==temp)
	strcpy(msgtype,"JNRQ");
temp=0xfb;
if(a==temp)
	strcpy(msgtype,"JNRS");
temp=0xfa;
if(a==temp)
	strcpy(msgtype,"HLLO");
temp=0xf8;
if(a==temp)
	strcpy(msgtype,"KPAV");
temp=0xf6;
if(a==temp)
	strcpy(msgtype,"CKRQ");
temp=0xf5;
if(a==temp)
	strcpy(msgtype,"CKRS");
temp=0xac;
if(a==temp)
	strcpy(msgtype,"STRQ");
temp=0xab;
if(a==temp)
	strcpy(msgtype,"STRS");
temp=0xf7;
if(a==temp)
	strcpy(msgtype,"NTFY");
temp=0xcc;
if(a==temp)
	strcpy(msgtype,"STOR");
temp=0xec;
if(a==temp)
	strcpy(msgtype,"SHRQ");
temp=0xeb;
if(a==temp)
	strcpy(msgtype,"SHRS");
temp=0xdc;
if(a==temp)
	strcpy(msgtype,"GTRQ");
temp=0xdb;
if(a==temp)
	strcpy(msgtype,"GTRS");
temp=0xbc;
	if(a==temp)
	strcpy(msgtype,"DELT");
}

void logging(unsigned char msg[],int type)
{
char msgtype[4];
struct timeval timer;
uint32_t size;
int sizes;
uint8_t ttls;
uint16_t pors;
 char *data;
int ttlss;
unsigned char UOIDs[4];
gettimeofday(&timer,NULL);
memcpy(&size,&msg[23],4);
memcpy(&ttls,&msg[21],1);
memcpy(&UOIDs,&msg[17],4);
memcpy(&pors,&msg[27],2);
getType(msgtype,msg);
sizes=size;
ttlss=ttls;
data=new  char[sizes];
uint16_t portss;
char hostnames[13];
char uoid[20];
uint16_t dist;
int sa;
string ss;
stringstream outs;
pthread_mutex_lock(&log_lock);
if(strcmp(msgtype,"HLLO")==0)
	{
	memcpy(&portss,&msg[27],2);
	sa=portss;
	memcpy(&hostnames,&msg[29],13);
	hostnames[13]='\0';
	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],sa,hostname);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],sa,hostname);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],sa,hostname);
	fflush(fp);
		break;

	}

	}

if(strcmp(msgtype,"JNRQ")==0)
	{
	memcpy(&portss,&msg[31],2);
	sa=portss;
	memcpy(&hostnames,&msg[33],13);
	hostnames[13]='\0';
	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],sa,hostname);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],sa,hostname);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],sa,hostname);
	fflush(fp);
		break;

	}

	}
 if(strcmp(msgtype,"JNRS")==0)
	{
	memcpy(&uoid,&msg[27],20);
	memcpy(&dist,&msg[47],4);
	memcpy(&portss,&msg[31],2);
	memcpy(&hostnames,&msg[33],13);
	hostnames[13]='\0';
	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x ",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	for(int r=0;r<20;r++)
		fprintf(fp,"%x",uoid[r]);
	fprintf(fp," %d %d %s",dist,portss,hostname);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x ",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	for(int r=0;r<20;r++)
		fprintf(fp,"%x",uoid[r]);
	fprintf(fp," %d %d %s",dist,portss,hostname);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x ",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	for(int r=0;r<20;r++)
		fprintf(fp,"%x",uoid[r]);
	fprintf(fp," %d %d %s",dist,portss,hostname);
	fflush(fp);
		break;

	}

	}
  if(strcmp(msgtype,"STRQ")==0)
	{

	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x %x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],msg[27]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x %x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],msg[27]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x %x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],msg[27]);
	fflush(fp);
		break;

	}
	}
	  if(strcmp(msgtype,"STRS")==0)
	{
		memcpy(&uoid,&msg[27],20);

	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x ",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	for(int r=16;r<20;r++)
		fprintf(fp,"%x",(unsigned char)uoid[r]);
	fputs("\n",fp);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x ",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	for(int r=16;r<20;r++)
		fprintf(fp,"%x",(unsigned char)uoid[r]);
		fputs("\n",fp);

	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x ",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	for(int r=16;r<20;r++)
		fprintf(fp,"%x",(unsigned char)uoid[r]);
		fputs("\n",fp);

	fflush(fp);
		break;

	}



	}
  


if(strcmp(msgtype,"NTFY")==0)
	{

	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x %x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],msg[27]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x %x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],msg[27]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x %x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],msg[27]);
	fflush(fp);
		break;

	}
	}
if(strcmp(msgtype,"KPAV")==0)
	{

	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
		break;

	}
	}

	if(strcmp(msgtype,"STOR")==0)
	{

	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
		break;

	}

	}

	
	if(strcmp(msgtype,"DELT")==0)
	{

	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x \n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3]);
	fflush(fp);
		break;

	}

	}

	if(strcmp(msgtype,"GTRQ")==0)
	{
	unsigned char flid[4];
	memcpy(flid,&msg[43],4);
	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
		break;

	}

	}
	
	if(strcmp(msgtype,"GTRS")==0)
	{
	unsigned char flid[4];
	memcpy(flid,&msg[43],4);
	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
		break;

	}

	}
if(strcmp(msgtype,"SHRS")==0)
	{
	unsigned char flid[4];
	memcpy(flid,&msg[43],4);
	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d from %s %d %d %02x%02x%02x%02x %02x%02x%02x%02x\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],flid[0],flid[1],flid[2],flid[3]);
	fflush(fp);
		break;

	}

	}
if(strcmp(msgtype,"SHRQ")==0)
	{
	string ms;
	uint32_t total_length;
	uint8_t typeb;
	memcpy(&typeb,&msg[27],1);
	memcpy(&total_length,&msg[23],4);
	memcpy((char *)ms.c_str(),&msg[28],(long)total_length-1);	
	switch(type)
	{
	case 0:
	fprintf(fp,"r %10ld.%03d from %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],typeb,(char *)ms.c_str());
	fflush(fp);
	     break;
	case 1:
	fprintf(fp,"s %10ld.%03d to %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],typeb,(char *)ms.c_str());
	fflush(fp);
		break;
	case 2:
	fprintf(fp,"f %10ld.%03d to %s %d %d %x%x%x%x %d %s\n",timer.tv_sec,(int)timer.tv_usec,msgtype,size,ttls,UOIDs[0],UOIDs[1],UOIDs[2],UOIDs[3],typeb,(char *)ms.c_str());
	fflush(fp);
		break;

	}

	}


	pthread_mutex_unlock(&log_lock);

}

void commandline(int argc,char **argv)
{

for(int i=1;i<argc;i++)

	{

if(strcmp(argv[i],"-reset")==0){
	reset=1;
}
	
else
	strcpy(initfilename,argv[i]);

	}

}

void Nam::populate(char *msg,int type,int file_to_write,int mode)
{
   
   if(type == 1){
    writetofile(file_to_write,msg);    
   }
   else
   {
	 uint32_t recordlength;
     uint32_t last_recordlength;
	 if(mode == 1){
	  fps=fopen(fileName[file_to_write].c_str(),"w");    
	 }
	 else
	 {
      fps=fopen(fileName[file_to_write].c_str(),"a"); 
	 }

	  if(fps == NULL)
      {
       return;
      }
    
	 memcpy(&recordlength,&msg[23],4); 
	 uint16_t host_node1;
	 memcpy(&host_node1,&msg[49],2);
	 if((long)recordlength > 37){
       memcpy(&last_recordlength,&msg[64],4);
	   if((long)last_recordlength == 0)
	   {
          fprintf(fps,"nunki.usc.edu:%d has the following file",(int)host_node1);
		  fprintf(fps,"\r\n");
          fwrite(&msg[68],1,recordlength-41,fps);
		  fprintf(fps,"\r\n");
		  fflush(fps);
	   }
	   else
	   {
		 fprintf(fps,"nunki.usc.edu:%d has the following files",(int)host_node1);
		 fprintf(fps,"\r\n");
         long curr_loc = 68;
		 while((long)last_recordlength != 0){
		  fwrite(&msg[curr_loc],1,last_recordlength,fps);
		  fprintf(fps,"\r\n");
          curr_loc =curr_loc+last_recordlength;
          memcpy(&last_recordlength,&msg[curr_loc],4);
		  curr_loc +=4;
		 }
         fwrite(&msg[curr_loc],1,recordlength-curr_loc+27,fps);
		 fprintf(fps,"\r\n"); 
         fflush(fps);

	   }

	 }
	 else
	 {
        fprintf(fps,"nunki.usc.edu:%d has no file",(int)host_node1);
		fprintf(fps,"\r\n");
	 }
	 
    fclose(fps);
   }
   
	
			
}
void Nam::writetofile(int file_to_write,char *msg)
{
 // node.clear();
  //links.clear();
  vector<int > temp_node;
  uint16_t host_node1,host_node2;
    uint32_t recordlength;
    //int length=64;
    neighbours n;
    memcpy(&host_node1,&msg[49],2);
    if(!nodeIsPresent(host_node1,file_to_write))
	{
     node[file_to_write].push_back(host_node1);
	 temp_node.push_back(host_node1);
	}
    memcpy(&recordlength,&msg[23],4); 
	int curr_loc =68;
    do{
	n.host=host_node1;
	memcpy(&host_node2,&msg[curr_loc],2);
	n.neighbour=host_node2;
	links[file_to_write].push_back(n);
    if(!nodeIsPresent(host_node2,file_to_write))
	{
		node[file_to_write].push_back(host_node2);
		temp_node.push_back(host_node2);
	}
	    curr_loc=curr_loc+19;

	}while(curr_loc<(int)recordlength);
    

	if(recordlength != 56){
		//length = length+19;
	n.host=host_node1;
	memcpy(&host_node2,&msg[curr_loc],2);
	n.neighbour=host_node2;
	links[file_to_write].push_back(n);

	if(!nodeIsPresent(host_node2,file_to_write))
	{
	   node[file_to_write].push_back(host_node2);
	   temp_node.push_back(host_node2);
	}
	}
  
  fps=fopen(fileName[file_to_write].c_str(),"a");
  if(fps == NULL)
  {
    return;
  }
  
  //For printing nodes
  for(int i=0;i<(int)temp_node.size();i++)
  {
	fprintf(fps,"n -t * -s %d -c red -i black\n",temp_node[i]);
	fflush(fps);
  }
  //For printing links
  for(int i=0;i<(int)links[file_to_write].size();i++)
  {
	fprintf(fps,"l -t * -s %d -d %d -c blue\n",links[file_to_write][i].host,links[file_to_write][i].neighbour);
	fflush(fps);
  }
  fclose(fps);
}

bool Nam::nodeIsPresent(int port,int file_to_write)
{
int i;
for(i=0;i<(int)node[file_to_write].size();i++)
	{
	if(node[file_to_write][i]==port)
		return true;
	}
	return false;
	
}

void Nam::initConnections(int file_to_write)
{
  pthread_mutex_lock(&act);
  neighbours n;
  n.host=atoi(Port);
  node[file_to_write].push_back(atoi(Port));
  for(int i=0;i<(int)activeconnections.size();i++)
  {
    n.neighbour=activeconnections[i].port;
    node[file_to_write].push_back(activeconnections[i].port);
    links[file_to_write].push_back(n);
  }
  pthread_mutex_unlock(&act);
  fps=fopen(fileName[file_to_write].c_str(),"w");
  if(fps == NULL)
  {
    return;
  }
  fputs("V -t * -v 1.0a5\n",fps);
  //For printing nodes
  for(int i=0;i<(int)node[file_to_write].size();i++)
  {
	fprintf(fps,"n -t * -s %d -c red -i black\n",node[file_to_write][i]);
	fflush(fps);
  }
  //For printing links
  for(int i=0;i<(int)links[file_to_write].size();i++)
  {
	fprintf(fps,"l -t * -s %d -d %d -c blue\n",links[file_to_write][i].host,links[file_to_write][i].neighbour);
  }
  fclose(fps);


}
int Nam::setFileName(string filename)
{
	if(curr_file < 0){
      curr_file = 0;
	}
	fileName[curr_file]=filename;
	curr_file++;
	return (curr_file-1);

}

void killsigserver(int sig)

{

signal(SIGUSR1,killsigserver);
}

void setKeepAlive(string index)
{
timeparam tparam;
pthread_mutex_lock(&time_buffer_lock);
tparam.index=index;
tparam.value=KeepAliveTimeout;
tparam.type=1;
time_buffer.push_back(tparam);
tparam.index=index;
tparam.value=KeepAliveTimeout/2;
tparam.type=2;
time_buffer.push_back(tparam);
pthread_mutex_unlock(&time_buffer_lock);
}

void setAgainKeepAlive(int index)
{
timeparam tparam;
pthread_mutex_lock(&time_buffer_lock);
for(int i=0;i<(int)time_buffer.size();i++)
	{
	if(atoi(time_buffer[i].index.c_str())==index)
		{
		 if(time_buffer[i].type==1)
			{
			time_buffer[i].value=KeepAliveTimeout;
			}
		}
	}
pthread_mutex_unlock(&time_buffer_lock);
}

void setAgainKeepAliveRetry(string index)
{
timeparam tparam;
pthread_mutex_lock(&time_buffer_lock);
tparam.index=index;
tparam.value=KeepAliveTimeout/2;
tparam.type=2;
time_buffer.push_back(tparam);
pthread_mutex_unlock(&time_buffer_lock);
}

int get_file_index_counter()
{
  int tem;
  pthread_mutex_lock(&file_index_counter_mutex);
  tem = file_index_counter;  
  file_index_counter++;
  pthread_mutex_unlock(&file_index_counter_mutex);
  return tem;
}
