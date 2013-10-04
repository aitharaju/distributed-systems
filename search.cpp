#include "header.h"
#include <sys/stat.h>

void search(int search_type,vector<string> inputs)
{
  unsigned char buf[8192];
  long total_length = 1;
  messageconstruction(buf,63,0);
  
  if(search_type == 0)
  {
    buf[27] = 0x00;
  }
  else if(search_type == 1)
  {
    buf[27] = 0x01;
  }
  else 
  {
   buf[27] = 0x02;
  }

  int i;
  int curr_loc = 28;
  for(i=0;i<(int)inputs.size();i++)
  {
	 memcpy(&buf[curr_loc],inputs[i].c_str(),inputs[i].length());
     total_length += inputs[i].length();
	 curr_loc += inputs[i].length();
     memcpy(&buf[curr_loc]," ",1);
	 total_length += 1;
	 curr_loc += 1;
  }
  memcpy(&buf[23],&total_length,4);

  string UOID_temp((const char*)&buf[1],20);    
  string msg_temp((const char*)&buf[0],27+total_length);

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

   pthread_mutex_lock(&msg_UOID_port_map_mutex);
   msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,atoi(Port)));
   pthread_mutex_unlock(&msg_UOID_port_map_mutex);

}


void parse_search_response(char *msg)
{
   uint32_t recordlength;
   uint32_t last_recordlength;
   vector<string> fileId_Vec;
   vector<string> sha1_Vec;
   char fileid_buf[20];
   char mdata_buf[2048];
  
   memcpy(&recordlength,&msg[23],4); 
   long recordlength1 = (long)recordlength;
   if(recordlength1 > 24)
   {
       memcpy(&last_recordlength,&msg[47],4);
	   if((long)last_recordlength == 0)
	   {
          memcpy(&fileid_buf[0],&msg[51],20);
		  memcpy(&mdata_buf[0],&msg[71],recordlength1-44);
		  string fileId(fileid_buf,20);
		  string data(mdata_buf,recordlength1-44);
		  sha1_Vec.push_back(fileId);
		  fileId_Vec.push_back(data);
	   }
	   else
	   {
         long curr_loc = 51;
		 while((long)last_recordlength != 0)
		 {
		  memcpy(&fileid_buf[0],&msg[curr_loc],20);
          curr_loc +=20;
		  memcpy(&mdata_buf[0],&msg[curr_loc],last_recordlength);
		  curr_loc +=last_recordlength;
          string fileId(fileid_buf,20);
		  string data(mdata_buf,last_recordlength);
		  sha1_Vec.push_back(fileId);
		  fileId_Vec.push_back(data);
		  memcpy(&last_recordlength,&msg[curr_loc],4);
		  curr_loc +=4;
		 }
          memcpy(&fileid_buf[0],&msg[curr_loc],20);
          curr_loc +=20;
		//  cout<<"curr_loc"<<curr_loc<<endl; 
          memcpy(&mdata_buf[0],&msg[curr_loc],recordlength1-curr_loc+27);
		  string fileId(fileid_buf,20);
		  string data(mdata_buf,recordlength1-curr_loc+27);
		  sha1_Vec.push_back(fileId);
		  fileId_Vec.push_back(data);

	   }
     
	   searchResponse(sha1_Vec,fileId_Vec);
	 }
	 else
	 {
	 }
}


bool checkKeyWordExists(int index,vector<string> keys)
{
MetaDatas m;
vector<string> k;
lockMeta(index);
m.init((char *)constructFileName(index,0).c_str());
k=m.getKeywords();
unLockMeta(index);
for(int i=0;i<(int)keys.size();i++)
	{
	for(int j=0;j<(int)k.size();j++)
		{
		if(k[j].compare(keys[i])==0)
			return true;
		}
	}
	return false;
}

vector<searchresponse> searchProc(vector<string> keys,int type)
{
vector<searchresponse> lsr;
switch(type)
	{
	case 0:
		{
		string filen;
		filen=keys[0];
		map<int,string>::iterator fiter;
		pthread_mutex_lock(&cfileindexlock);
		for(fiter=cache_file_index.begin();fiter!=cache_file_index.end();fiter++)
		{
		if((*fiter).second.compare(filen)==0)
			{
			searchresponse sr;
			pthread_mutex_lock(&fileidindex);
			sr.fileid=generateFileId((*fiter).first);
			pthread_mutex_unlock(&fileidindex);
			lockMeta((*fiter).first);
			sr.mdata=generateMetaData(constructFileName((*fiter).first,0));
			unLockMeta((*fiter).first);
			lsr.push_back(sr);
			}

		}
		pthread_mutex_unlock(&cfileindexlock);
		
		pthread_mutex_lock(&fileindexlock);
		for(fiter=file_index.begin();fiter!=file_index.end();fiter++)
		{
		if((*fiter).second.compare(filen)==0)
			{
			searchresponse sr;
			pthread_mutex_lock(&fileidindex);
			sr.fileid=generateFileId((*fiter).first);
			pthread_mutex_unlock(&fileidindex);
			lockMeta((*fiter).first);
			sr.mdata=generateMetaData(constructFileName((*fiter).first,0));
			unLockMeta((*fiter).first);
			lsr.push_back(sr);
			}

		}
		pthread_mutex_unlock(&fileindexlock);

		break;
		}
	case 1:
		{
		string filen1;
		filen1=keys[0];
		map<int,string>::iterator fiter1;
		pthread_mutex_lock(&csha1indexlock);
		for(fiter1=cache_sha1_index.begin();fiter1!=cache_sha1_index.end();fiter1++)
		{
		if((*fiter1).second.compare(filen1)==0)
			{

			searchresponse sr;
			pthread_mutex_lock(&fileidindex);
			sr.fileid=generateFileId((*fiter1).first);
			pthread_mutex_unlock(&fileidindex);
			lockMeta((*fiter1).first);
			sr.mdata=generateMetaData(constructFileName((*fiter1).first,0));
			unLockMeta((*fiter1).first);
			lsr.push_back(sr);
			}
		}


		pthread_mutex_unlock(&csha1indexlock);

				pthread_mutex_lock(&sha1indexlock);

		for(fiter1=sha1_index.begin();fiter1!=sha1_index.end();fiter1++)
		{
			/*hex_display(20,(unsigned char*)(*fiter1).second.c_str());
			hex_display(20,(unsigned char*)filen1.c_str());*/

		if(compareHex((*fiter1).second,filen1))
			{
			searchresponse sr;
			pthread_mutex_lock(&fileidindex);
			sr.fileid=generateFileId((*fiter1).first);
			pthread_mutex_unlock(&fileidindex);
			lockMeta((*fiter1).first);
			sr.mdata=generateMetaData(constructFileName((*fiter1).first,0));
			unLockMeta((*fiter1).first);
			lsr.push_back(sr);
			}
		}
				pthread_mutex_unlock(&sha1indexlock);

		break;
		}
	case 2:
		{
		/*-----------------------------------Generating Bit Vectors----------------------------------*/
		unsigned char obuf[20];
		MD5_CTX cd;
		unsigned char md[16];
		unsigned char vec[128];
		memset(vec,0,128);
		for(int i=0;i<(int)keys.size();i++)
			{
			memset(md,0,16);
			memset(obuf,0,20);

			MD5_Init(&cd);
			MD5_Update(&cd, (const void *)keys[i].c_str(),(int)keys[i].size());
			MD5_Final(md,&cd);
			setBit(mods1(md),vec);

			SHA1((unsigned char *)keys[i].c_str(),strlen(keys[i].c_str()),obuf);
			setBit((mods(obuf)+512),vec);
			}
		//hex_display(128,vec);
		/*--------------------------------------------------------------------------------------------*/
		map<int,unsigned char *>::iterator fiters;
		
		pthread_mutex_lock(&cvectorindexlock);

		

		for(fiters=cache_vector_index.begin();fiters!=cache_vector_index.end();fiters++)
		{

		if(bitwiseOr(vec,(*fiters).second))
			{
			if(checkKeyWordExists((*fiters).first,keys))
				{
			searchresponse sr;
			pthread_mutex_lock(&fileidindex);
			sr.fileid=generateFileId((*fiters).first);
			pthread_mutex_unlock(&fileidindex);
			lockMeta((*fiters).first);
			sr.mdata=generateMetaData(constructFileName((*fiters).first,0));
			unLockMeta((*fiters).first);
			lsr.push_back(sr);
				}

			}
		
		}
				pthread_mutex_unlock(&cvectorindexlock);



		pthread_mutex_lock(&vectorindexlock);

		for(fiters=vector_index.begin();fiters!=vector_index.end();fiters++)
		{
		

		if(bitwiseOr(vec,(*fiters).second))
			{		

			if(checkKeyWordExists((*fiters).first,keys))
				{

			searchresponse sr;
			pthread_mutex_lock(&fileidindex);
			sr.fileid=generateFileId((*fiters).first);
			pthread_mutex_unlock(&fileidindex);
			lockMeta((*fiters).first);
			sr.mdata=generateMetaData(constructFileName((*fiters).first,0));
			//cout<<sr.mdata<<endl;
			unLockMeta((*fiters).first);
			lsr.push_back(sr);
				}

			}
		
		}
		pthread_mutex_unlock(&vectorindexlock);


		break;
	}
	}

return lsr;

}

bool isPresentFileId(int index)
{
if(fileid_index.find(index)!=fileid_index.end())
	return true;
else 
	return false;

}

string generateFileId(int index)
{
if(isPresentFileId(index))
	{
return fileid_index[index];
	}
else
	{

unsigned char tempfileid[20];
string s;
GetUOID(nodeinstanceid,(char *)"msg",tempfileid,sizeof(tempfileid));
s.append((char *)tempfileid);
fileid_index.insert(pair<int,string>(index,s));
return s;
	}
}

string constructFileName(int index,int type)
{
	string fn;
	std::string s;
	std::stringstream out_ss;
	out_ss << index;
	s = out_ss.str();
	char prefix[100],suffix[100],fname[100],i_file[100];
	
switch(type)
	{
	case 0:
		strcpy(prefix,HomeDir);
		strcat(prefix,"/files/");
		strcpy(fname,s.c_str());
		strcat(fname,".metadata");
		strcpy(suffix,fname);
		strcpy(i_file,prefix);
		strcat(i_file,suffix);
		fn.assign(i_file);
		break;
	case 1:	
		strcpy(prefix,HomeDir);
		strcat(prefix,"/");
		strcpy(fname,"temp.metadata");
		strcpy(suffix,fname);
		strcpy(i_file,prefix);
		strcat(i_file,suffix);
		fn.assign(i_file);
		break;
	case 2:
		strcpy(prefix,HomeDir);
		strcat(prefix,"/files/");
		strcpy(fname,s.c_str());
		strcat(fname,".password");
		strcpy(suffix,fname);
		strcpy(i_file,prefix);
		strcat(i_file,suffix);
		fn.assign(i_file);
		break;
	case 3:
		strcpy(prefix,HomeDir);
		strcat(prefix,"/files/");
		strcpy(fname,s.c_str());
		strcat(fname,".data");
		strcpy(suffix,fname);
		strcpy(i_file,prefix);
		strcat(i_file,suffix);
		fn.assign(i_file);
		break;
	case 4:
		strcpy(prefix,HomeDir);
		strcat(prefix,"/");
		strcat(fname,"temp.delete");
		strcpy(suffix,fname);
		strcpy(i_file,prefix);
		strcat(i_file,suffix);
		fn.assign(i_file);
		break;

	}
return fn;

}

string generateMetaData(string filename)
{
FILE *mdt;
string metadata_nonui;
mdt=fopen(filename.c_str(),"r");
char c;
if(mdt==NULL)
	{
	return "";
	}
do {
      c = getc (mdt);
		if(c!=EOF)
		metadata_nonui.push_back(c);
      } while (c != EOF);

fclose(mdt);
return metadata_nonui;
}

bool bitwiseOr(unsigned char a[128],unsigned char b[128])
	{
	for(int i=0;i<128;i++)
		{
		if(a[i]&b[i])
			return true;
		}
	return false;

	}


void searchResponse(vector<string> Fileid,vector<string> metadata)
{
pthread_mutex_lock(&globalbufferlock);
for(int i=0;i<(int)Fileid.size();i++)
	{
getres g;
lockMeta(0);
MetaDatas m;
FILE *fs;
vector<string> ke;
fs=fopen(constructFileName(0,1).c_str(),"w");
if(fs==NULL)
	{
	pthread_mutex_unlock(&globalbufferlock);
		return;
	}

metadata[i][metadata[i].size()-1]='\0';
fflush(fs);

fprintf(fs,"%s",metadata[i].c_str());
fclose(fs);

m.init((char *)constructFileName(0,1).c_str());

g.fileid=Fileid[i];
g.filename=m.getFileName();
g.filesize=m.getFileSize();
g.sha1=m.getSHA();
g.nonce=m.getNonce();
g.keyword=m.getKeywords();
remove(constructFileName(0,1).c_str());
unLockMeta(0);

globalsearchbuffer.push_back(g);
	}
pthread_mutex_unlock(&globalbufferlock);
}


void displaySearchResponse(getres g,int index)
{
vector<string> ke;
printf("               %d              FileID=",index);
hex_display(20,(unsigned char *)g.fileid.c_str());
printf("                              FileName=%s\n",g.filename.c_str());
printf("                              FileSize=%d\n",g.filesize);
printf("                              SHA1=");
hex_display(20,(unsigned char *)g.sha1.c_str());
printf("                              Nonce=");
hex_display(20,(unsigned char *)g.nonce.c_str());
ke=g.keyword;

printf("                              Keywords=");
for(int i=0;i<(int)ke.size();i++)
	printf("%s ",ke[i].c_str());
cout<<endl;
}

bool fileExistsInCurr(string filename)
{
FILE *fe;
//cout<<"feike"<<filename.c_str()<<endl;
fe=fopen(filename.c_str(),"r");
if(fe==NULL)
	{
	fclose(fe);
	return false;

	}
fclose(fe);
return true;


}

