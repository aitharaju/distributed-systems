#include "header.h"
#include <sys/stat.h>

void store(string filename,int ttl,vector<string> keywords)
{
  // vector<string> inputs;
  
   string metadata = store_nonUI(filename,ttl,keywords);
   if(metadata.empty()){
	   return;
   }

  
   long metadata_length = metadata.length();
   metadata_length = metadata_length -1;
   FILE *in, *out;
   char ch;

    std::string s;
    std::stringstream out_ss;

	out_ss << file_index_counter;
	s = out_ss.str();


  if((in=fopen(filename.c_str(), "r")) == NULL) {
    cout<<"Cannot open file"<<filename<<endl;
    return;
  }


  char filename1[256];
  strcpy(filename1,HomeDir);
  strcat(filename1,"/");
  strcat(filename1,"files/");
  
  char filename2[100];

  strcpy(filename2,s.c_str());
  strcat(filename2,".data");
  strcat(filename1,filename2);


  if((out=fopen(filename1, "w")) == NULL) {
     cout<<"Cannot open file "<<filename1<<endl;
	 fclose(in);
     return;
  }

  while(!feof(in)) {
    ch = getc(in);
    if(ferror(in)) {
      printf("Read Error");
      clearerr(in);
      break;
    } else {
      if(!feof(in)) putc(ch, out);
      if(ferror(out)) {
        printf("Write Error");
        clearerr(out);
        break;
      }
    }
  }
  fclose(in);
  fclose(out);

  file_index_counter++;
  
   struct stat FileAttrib;
   long file_Size;
   if (stat(filename.c_str(), &FileAttrib) == 0){
     file_Size = FileAttrib.st_size;
   }
   else
   {
	   return;
   }

   unsigned char buf[8192];
   long total_length = 4+metadata_length+file_Size;
   messageconstruction(buf,33,0);
   memcpy(&buf[23],&total_length,4);
   memcpy(&buf[27],&metadata_length,4);
   memcpy(&buf[31],metadata.c_str(),metadata_length-1);
   memcpy(&buf[31+metadata_length],filename.c_str(),filename.length());
   uint8_t ttl_temp = (uint8_t)ttl;
   memcpy(&buf[21],&ttl_temp,1);
    
   total_length = 27+4+metadata_length+filename.length();

   string UOID_temp((const char*)&buf[1],20);    
   string msg_temp((const char*)&buf[0],total_length);


   pthread_mutex_lock(&msg_buffer_mutex);
   msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
   pthread_mutex_unlock(&msg_buffer_mutex);
   int l;
   pthread_mutex_lock(&write_buffer_mutex);
   pthread_mutex_lock(&act);

  if(ttl > 0){
   for (l=0;l<(int)activeconnections.size();l++)
   {
	  double dval=(double)drand48();
      if(dval <= NeighborStoreProb)
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
   }
   pthread_mutex_unlock(&write_buffer_mutex);
   pthread_mutex_unlock(&act);
  }

}
		   
 void InitRandom(long l_seed)
    {
        if (l_seed == 0L) {
            time_t localtime=(time_t)0;

            time(&localtime);
            srand48((long)localtime);
        } else {
            srand48(l_seed);
        }
    }

string store_nonUI(string filename,int ttl,vector<string> keywords)
{

  char prefix[256];
  char suffix[100];
  char meta[100];
  char pass[100];
  char buffer[8192];
  string metadata_nonui;
  string sha1_str;

  strcpy(prefix,HomeDir);
  strcat(prefix,"/");
  strcat(prefix,"files/");
  
  std::string s;
  std::stringstream out_ss;
  out_ss << file_index_counter;
  s = out_ss.str();
  
  strcpy(meta,prefix);	
  strcpy(suffix,s.c_str());
  strcat(suffix,".metadata");
  strcat(meta,suffix);

/*--------------------------------obtaining the file size-----------------------------------*/
struct stat st;
int size;
if(stat(filename.c_str(), &st)==-1)
	{
	cout<<"Error getting the file size"<<endl;
	return "";

	}
size = st.st_size;
/*------------------------------------------------------------------------------------------*/


/*-------------------------obtaining file SHA1------------------------------------*/
unsigned char file_sha1[20];
SHA_CTX sha1_context;
int filesize=size;
SHA1_Init(&sha1_context);
FILE *filedescriptor;
filedescriptor=fopen(filename.c_str(),"r");
if(filedescriptor==NULL)
	{

	return "";
	}

								
								while(filesize > 0)
								{
									int bytesread = fread(buffer, 1, 8192, filedescriptor);
								//	fwrite(buffer, 1, bytesread, newfiledescriptor);
									
									filesize -= bytesread;
									
									SHA1_Update(&sha1_context, buffer, bytesread); 
								}	
									
									SHA1_Final(file_sha1, &sha1_context);
									sha1_str.assign((char *)file_sha1,20);
/*--------------------------------------------------------------------------------*/

/*-------------------------obtaining a one time password---------------------------*/
	unsigned char pass_UOID[20];
	unsigned char nonce[20];
	GetUOID(nodeinstanceid,(char *)"file",pass_UOID,sizeof(pass_UOID));
	SHA1((unsigned char *)pass_UOID,20,nonce);
	/*cout<<"File one time password"<<endl;
	hex_display(20,pass_UOID);*/

/*---------------------------------------------------------------------------------*/



pthread_mutex_lock(&fileindexlock);
file_index.insert(pair<int,string>(file_index_counter,filename));
pthread_mutex_unlock(&fileindexlock);

pthread_mutex_lock(&sha1indexlock);
sha1_index.insert(pair<int,string>(file_index_counter,sha1_str));
pthread_mutex_unlock(&sha1indexlock);


/*-----------------------------------Generating Bit Vectors----------------------------------*/
unsigned char obuf[20];
MD5_CTX cd;
unsigned char md[16];

for(int i=0;i<(int)keywords.size();i++)
	{
	memset(md,0,16);
	memset(obuf,0,20);

	MD5_Init(&cd);
	MD5_Update(&cd, (const void *)keywords[i].c_str(),(int)keywords[i].size());
	MD5_Final(md,&cd);
	setBit(mods1(md),vectors[file_index_counter]);

	SHA1((unsigned char *)keywords[i].c_str(),strlen(keywords[i].c_str()),obuf);
	setBit((mods(obuf)+512),vectors[file_index_counter]);
	}

	pthread_mutex_lock(&vectorindexlock);
	vector_index.insert(pair<int,unsigned char *>(file_index_counter,vectors[file_index_counter]));
	pthread_mutex_unlock(&vectorindexlock);
	//hex_display(128,vectors[file_index_counter]);


/*---------------------------------------------------------------------------------------------*/


/*-------------------------Creating MetaData----------------------------------------------------*/
FILE *mdt;

mdt=fopen(meta,"w");
if(mdt==NULL)
	{
return "";
	}
fputs("[metadata]\r\n",mdt);
fprintf(mdt,"FileName=%s\r\n",filename.c_str());
fprintf(mdt,"FileSize=%d\r\n",size);
fputs("SHA1=",mdt);
for (int j = 0; j < 20; j++) {
        fprintf(mdt,"%02x", file_sha1[j]);
}
fputs("\r\n",mdt);
fputs("Nonce=",mdt);
for (int j = 0; j < 20; j++) {
        fprintf(mdt,"%02x", nonce[j]);
}
fputs("\r\n",mdt);
fputs("Keywords=",mdt);
for(int j=0;j<(int)keywords.size();j++)
	{
	if(j!=((int)keywords.size()-1))
fprintf(mdt,"%s ",keywords[j].c_str());
	else
fprintf(mdt,"%s",keywords[j].c_str());


	}
fputs("\r\n",mdt);

fputs("Bit-vector=",mdt);

for(int j=0;j<128;j++)
	fprintf(mdt,"%02x",vectors[file_index_counter][j]);
fputs("\r\n",mdt);
fclose(mdt);
/*----------------------------------------------------------------------------------------------*/



/*---------------------------Creating the Metadata String--------------------------------------*/
mdt=fopen(meta,"r");
char c;
if(mdt==NULL)
	{
	return "";
	}
do {
      c = getc (mdt);
		metadata_nonui.push_back(c);
      } while (c != EOF);
fclose(mdt);
/*---------------------------------------------------------------------------------------------*/
/*vector<string> sam1;
vector<string> sam2;
sam1.push_back(generateFileId(1));
sam2.push_back(metadata_nonui);
searchResponse(sam1,sam2);*/
  memset(suffix,0,100);
  strcpy(pass,prefix);
  strcpy(suffix,s.c_str());
  strcat(suffix,".password");
  strcat(pass,suffix);
  
  FILE *pw;
  pw=fopen(pass,"w");
  if(pw==NULL)
	{
	  return "";
	}
fputs("[password]\r\n",pw);
fputs("pass=",pw);
for(int j=0;j<20;j++)
	fprintf(pw,"%02x",pass_UOID[j]);
fputs("\r\n",pw);

fclose(pw);
pthread_mutex_lock(&filecounterlock);
//file_index_counter++;
pthread_mutex_unlock(&filecounterlock);

return metadata_nonui;
}


int mods(unsigned char buf[20])
{
uint8_t a,b;
int value;
int val;
memcpy(&a,&buf[19],1);
memcpy(&b,&buf[18],1);
val=(b)&(00000001);
value=val*256+a;
return value;

}
int mods1(unsigned char buf[16])
{
uint8_t a,b;
int value;
int val;
memcpy(&a,&buf[15],1);
memcpy(&b,&buf[14],1);
val=(b)&(00000001);
value=val*256+a;
return value;

}

void setBit(int position,unsigned char *vectors)
{
int rel,abs;
uint8_t value;
char a;
rel=position%8;
abs=127-position/8;
value=pow(2,rel);
memcpy(&a,&value,1);
vectors[abs]=vectors[abs]| a;
}


void hex_display(int len, unsigned char *buf)
{for(int i=0;i<len;i++)
	printf("%02x",buf[i]);
cout<<endl;

}



void populate()
{

populates s;

s.init((char *)"fileindex.ini");
s.populateData(0,0);

s.init((char *)"file_index");
pthread_mutex_lock(&fileindexlock);
s.populateData(1,0);
pthread_mutex_unlock(&fileindexlock);

s.init((char *)"sha_index");
pthread_mutex_lock(&sha1indexlock);
s.populateData(1,1);
pthread_mutex_unlock(&sha1indexlock);

s.init((char *)"cache_file_index");
pthread_mutex_lock(&cfileindexlock);
s.populateData(1,2);
pthread_mutex_unlock(&cfileindexlock);

s.init((char *)"cache_sha_index");
pthread_mutex_lock(&csha1indexlock);
s.populateData(1,3);
pthread_mutex_unlock(&csha1indexlock);

s.init((char *)"vector_index");
pthread_mutex_lock(&vectorindexlock);
s.populateData(1,4);
pthread_mutex_unlock(&vectorindexlock);

s.init((char *)"cache_vector_index");
pthread_mutex_lock(&cvectorindexlock);
s.populateData(1,5);
pthread_mutex_unlock(&cvectorindexlock);

s.init((char *)"fileid_index");
pthread_mutex_lock(&fileidindex);
s.populateData(1,6);
pthread_mutex_unlock(&fileidindex);

}


void makePersistent()
{
populates s;
s.init((char *)"fileindex.ini");
s.makePersistent(0,0);

s.init((char *)"file_index");
pthread_mutex_lock(&fileindexlock);
s.makePersistent(1,0);
pthread_mutex_unlock(&fileindexlock);


s.init((char *)"sha_index");
pthread_mutex_lock(&sha1indexlock);
s.makePersistent(2,0);
pthread_mutex_unlock(&sha1indexlock);

s.init((char *)"cache_file_index");
pthread_mutex_lock(&cfileindexlock);
s.makePersistent(1,1);
pthread_mutex_unlock(&cfileindexlock);


s.init((char *)"cache_sha_index");
pthread_mutex_lock(&csha1indexlock);
s.makePersistent(2,1);
pthread_mutex_unlock(&csha1indexlock);

pthread_mutex_lock(&vectorindexlock);
s.init((char *)"vector_index");
s.makePersistent(3,0);
pthread_mutex_unlock(&vectorindexlock);

pthread_mutex_lock(&cvectorindexlock);
s.init((char *)"cache_vector_index");
s.makePersistent(3,1);
pthread_mutex_unlock(&cvectorindexlock);

pthread_mutex_lock(&fileidindex);
s.init((char *)"fileid_index");
s.makePersistent(4,0);
pthread_mutex_unlock(&fileidindex);

}


void readMetadata(char  *filepaths)
{
int temp_index_counter;
temp_index_counter=file_index_counter;


MetaDatas m;
m.init(filepaths);
pthread_mutex_lock(&lrulock);
//lru.add(temp_index_counter-1,m.getFileSize());

pthread_mutex_unlock(&lrulock);


pthread_mutex_lock(&cfileindexlock);
cache_file_index.insert(pair<int,string>(temp_index_counter-1,m.getFileName()));
pthread_mutex_unlock(&cfileindexlock);

pthread_mutex_lock(&csha1indexlock);
cache_sha1_index.insert(pair<int,string>(temp_index_counter-1,m.getSHA()));
pthread_mutex_unlock(&csha1indexlock);

pthread_mutex_lock(&cvectorindexlock);
cache_vector_index.insert(pair<int,unsigned char*>(temp_index_counter-1,m.getVector()));
pthread_mutex_unlock(&cvectorindexlock);

//file_index_counter++;
}



void stripIndex(char *buf,unsigned char *vec)
	{
	int vec_index=0;
	uint8_t count=0;
	int i;
	
	for(i=0;i<40;i++)
		{
	
	count=0;
	if(isalpha(buf[i]))
			{
            count=buf[i]-'a'+10;
            count=count*16;
			}
			else
			{
	count=buf[i]-'0';
	count=count*16;
			}
	i++;
	if(isalpha(buf[i]))
			{
	
	count+=buf[i]-'a'+10;
			}
			else
			{
				count+=buf[i]-'0';
			}
	memcpy(&vec[vec_index],&count,1);
	vec_index++;
		}
	}

void vstripIndex(char *buf,unsigned char *vec)
	{
	int vec_index=0;
	uint8_t count=0;
	int i;
	
	for(i=0;i<256;i++)
		{
	
	count=0;
	if(isalpha(buf[i]))
			{
            count=buf[i]-'a'+10;
            count=count*16;
			}
			else
			{
	count=buf[i]-'0';
	count=count*16;
			}
	i++;
	if(isalpha(buf[i]))
			{
	
	count+=buf[i]-'a'+10;
			}
			else
			{
				count+=buf[i]-'0';
			}
	memcpy(&vec[vec_index],&count,1);
	vec_index++;
		}
	}

 bool compareHex(string a, string b)
 {

for(int i=0;i<20;i++)
	 {
	
if((unsigned char)a.c_str()[i]!=(unsigned char)b.c_str()[i])
return false;
	 }
//hex_display(20,(unsigned char*)a.c_str());
//hex_display(20,(unsigned char*)b.c_str());

return true;
 }

 

