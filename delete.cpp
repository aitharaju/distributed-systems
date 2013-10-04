#include "header.h"
#include <sys/stat.h>

void delete_UI(string FileSpec)
{
   unsigned char buf[8192];
   long total_length = FileSpec.length();
   messageconstruction(buf,23,0);
   memcpy(&buf[23],&total_length,4);
   memcpy(&buf[27],FileSpec.c_str(),FileSpec.length());
    
   total_length = 27+FileSpec.length();

   string UOID_temp((const char*)&buf[1],20);    
   string msg_temp((const char*)&buf[0],total_length);


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
}

int passWordExists(string filename,string sha1,string nonce)
{
unsigned char sha[20];
char csha[40];
unsigned char nonc[20];
char cnonce[40];
map<int,string>::iterator index_iter;
string dsha1;
string nonce2;

int index=-1;

reassign(csha,sha1);
stripIndex(csha,sha);
dsha1=assign(sha);
int i;
reassign(cnonce,nonce);
stripIndex(cnonce,nonc);
nonce2=assign(nonc);

pthread_mutex_lock(&cfileindexlock);

for(index_iter=cache_file_index.begin();index_iter!=cache_file_index.end();index_iter++)
	{
	i=(*index_iter).first;
	

	if(cache_file_index[i].compare(filename)==0)
		{

		pthread_mutex_lock(&csha1indexlock);
		if(compareHex(cache_sha1_index[i],dsha1))
			{
			
			string non_meta;
			string nonce_meta;
			non_meta=constructFileName(i,0);
			MetaDatas m;
			lockMeta(i);
			m.init((char *)(non_meta.c_str()));
			nonce_meta=m.getNonce();			
			if(compareHex(nonce2,nonce_meta))
				{
				index=i;
				}
			unLockMeta(i);

			}
		pthread_mutex_unlock(&csha1indexlock);
		}
	}
pthread_mutex_unlock(&cfileindexlock);

pthread_mutex_lock(&fileindexlock);

for(index_iter=file_index.begin();index_iter!=file_index.end();index_iter++)
	{
		i=(*index_iter).first;

	if(file_index[i].compare(filename)==0)
		{
		pthread_mutex_lock(&sha1indexlock);
		if(compareHex(sha1_index[i],dsha1))
			{

			string non_meta;
			string nonce_meta;
			non_meta=constructFileName(i,0);
			MetaDatas m;
			lockMeta(i);
			m.init((char *)(non_meta.c_str()));
			nonce_meta=m.getNonce();			
			if(compareHex(nonce2,nonce_meta))
				{
				index=i;
				}
			unLockMeta(i);

			}
		pthread_mutex_unlock(&sha1indexlock);
		}
	}
pthread_mutex_unlock(&fileindexlock);
if(index!=-1)
	{
	//cout<<index<<endl;
	index=checkFileExists(index);
	}
return index;
}

bool checkFileExists(int index)
{
string password;
password=constructFileName(index,2);
FILE *passwd;
lockPass(index);
//cout<<password.c_str()<<"ff"<<endl;
passwd=fopen(password.c_str(),"r");
unLockPass(index);
if(passwd==NULL)
	{
	 fclose(passwd);
	 return false;
	}
fclose(passwd);
return true;
}


int indexOfFile(string filename,string sha1,string nonce)
{
unsigned char sha[20];
char csha[40];
unsigned char nonc[20];
char cnonce[40];
map<int,string>::iterator index_iter;
string dsha1;
string nonce2;
int index=-1;

reassign(csha,sha1);
stripIndex(csha,sha);
dsha1=assign(sha);
int i;
reassign(cnonce,nonce);
stripIndex(cnonce,nonc);
nonce2=assign(nonc);


pthread_mutex_lock(&fileindexlock);

for(index_iter=file_index.begin();index_iter!=file_index.end();index_iter++)
	{
	i=(*index_iter).first;
	if(file_index[i].compare(filename)==0)
		{
		pthread_mutex_lock(&sha1indexlock);
		if(compareHex(sha1_index[i],dsha1))
			{

			string non_meta;
			string nonce_meta;
			non_meta=constructFileName(i,0);
			MetaDatas m;
			lockMeta(i);
			m.init((char *)(non_meta.c_str()));
			nonce_meta=m.getNonce();			
			if(compareHex(nonce2,nonce_meta))
				{
				//cout<<"yeah"<<endl;
				index=i;
				}
			unLockMeta(i);

			}
		pthread_mutex_unlock(&sha1indexlock);
		}
	}
pthread_mutex_unlock(&fileindexlock);

return index;
}

int shortindexOfFile(string filename,string sha1,string nonce)
{


string dsha1;
string nonce2;

int index=-1;

int i;
dsha1=sha1;

map<int,string>::iterator index_iter;

nonce2=nonce;


pthread_mutex_lock(&fileindexlock);

for(index_iter=file_index.begin();index_iter!=file_index.end();index_iter++)
	{
	i=(*index_iter).first;
	if(file_index[i].compare(filename)==0)
		{
		pthread_mutex_lock(&sha1indexlock);
		if(compareHex(sha1_index[i],dsha1))
			{

			string non_meta;
			string nonce_meta;
			non_meta=constructFileName(i,0);
			MetaDatas m;
			lockMeta(i);
			m.init((char *)(non_meta.c_str()));
			nonce_meta=m.getNonce();			
			if(compareHex(nonce2,nonce_meta))
				{
				//cout<<"yeah"<<endl;
				index=i;
				}
			unLockMeta(i);

			}
		pthread_mutex_unlock(&sha1indexlock);
		}
	}
pthread_mutex_unlock(&fileindexlock);
return index;
}

int indexOfFileInCache(string filename,string sha1,string nonce)
{
unsigned char sha[20];
char csha[40];
unsigned char nonc[20];
char cnonce[40];
int i;
string dsha1;
string nonce2;
map<int,string>::iterator index_iter;

int index=-1;

reassign(csha,sha1);
stripIndex(csha,sha);
dsha1=assign(sha);

reassign(cnonce,nonce);
stripIndex(cnonce,nonc);
nonce2=assign(nonc);


pthread_mutex_lock(&cfileindexlock);

for(index_iter=cache_file_index.begin();index_iter!=cache_file_index.end();index_iter++)
	{
	i=(*index_iter).first;
	if(cache_file_index[i].compare(filename)==0)
		{
		pthread_mutex_lock(&csha1indexlock);
		if(compareHex(cache_sha1_index[i],dsha1))
			{

			string non_meta;
			string nonce_meta;
			non_meta=constructFileName(i,0);
			MetaDatas m;
			lockMeta(i);
			m.init((char *)(non_meta.c_str()));
			nonce_meta=m.getNonce();			
			if(compareHex(nonce2,nonce_meta))
				{
				//cout<<"yeah"<<endl;
				index=i;
				}
			unLockMeta(i);

			}
		pthread_mutex_unlock(&csha1indexlock);
		}
	}
pthread_mutex_unlock(&cfileindexlock);
return index;
}



int shortindexOfFileInCache(string filename,string sha1,string nonce)
{

string dsha1;
string nonce2;

int index=-1;
map<int,string>:: iterator index_iter;
int i;

dsha1=sha1;


nonce2=nonce;



pthread_mutex_lock(&cfileindexlock);

for(index_iter=cache_file_index.begin();index_iter!=cache_file_index.end();index_iter++)
	{
	i=(*index_iter).first;
	if(cache_file_index[i].compare(filename)==0)
		{
		//cout<<"hell yea"<<endl;
		pthread_mutex_lock(&csha1indexlock);
		if(compareHex(cache_sha1_index[i],dsha1))
			{
			//cout<<"hell yea"<<endl;
			string non_meta;
			string nonce_meta;
			non_meta=constructFileName(i,0);
			MetaDatas m;
			lockMeta(i);
			m.init((char *)(non_meta.c_str()));
			nonce_meta=m.getNonce();			
			if(compareHex(nonce2,nonce_meta))
				{
				//cout<<"yeah"<<endl;
				index=i;
				}
			unLockMeta(i);

			}
		pthread_mutex_unlock(&csha1indexlock);
		}
	}
pthread_mutex_unlock(&cfileindexlock);
return index;
}

string generateRandomPassword()
{
string retrandpass;
unsigned char randpass[20];
GetUOID(nodeinstanceid,(char *)"msg",randpass,sizeof(randpass));
retrandpass=assign(randpass);
return retrandpass;
}


void delFileRequest(string filename,string sha1, string nonce, string password)
{
//All the values are 20 bytes unsigned characters
FileSpec a;
string delReq;

unsigned char sh[20];
char sh1[40];

unsigned char n[20];
char n1[40];

reassign(sh1,sha1);
stripIndex(sh1,sh);
string delSha;
delSha=assign(sh);


reassign(n1,nonce);
stripIndex(n1,n);
string delnonce;
delnonce=assign(n);

//cout<<filename<<"kkl"<<endl;
delReq.assign(filename);
delReq.append(" ");
delReq.append(delSha);
delReq.append(" ");
delReq.append(delnonce);
delReq.append(" ");
delReq.append(password);
delReq.append(" ");
delFileResponse(delReq);
delete_UI(delReq);

/*hex_display(20,(unsigned char*)delSha.c_str());
hex_display(20,(unsigned char*)delnonce.c_str());
hex_display(20,(unsigned char*)password.c_str());*/

}


void delFileResponse(string a)
{
int index;
string filename;
string sha1;
string password;
string nonce;
string cnonce;
unsigned char nonce_c[20];
int i;
int count=0;

for( i=0;i<(int)a.size();i++)
	{
	if(a[i]==' ')
		{
		i+=1;
		break;
		}
	filename.push_back(a[i]);
	}
for(;i<(int)a.size();i++)
	{
	
	if(count==20)
		{
		i+=1;
		break;
		}
	sha1.push_back(a[i]);
	count++;
	}
	count=0;
for(;i<(int)a.size();i++)
	{
	if(count==20)
		{
		i+=1;
		break;
		}
	nonce.push_back(a[i]);
		count++;

	}
count=0;
for(;i<(int)a.size();i++)
	{
	if(count==20)
		{
		i+=1;
		break;
		}
	password.push_back(a[i]);
			count++;

	}



	

SHA1((unsigned char *)password.c_str(),20,nonce_c);
cnonce=assign(nonce_c);
if(compareHex(cnonce,nonce))
	{

	

index=shortindexOfFile(filename,sha1,nonce);
if(index!=-1)
	{
	delFile(index,0);

	}

	index=shortindexOfFileInCache(filename,sha1,nonce);
	if(index!=-1)
		{
	delFile(index,1);


		}	


	}
/*cout<<"printing the index"<<endl;
hex_display(20,(unsigned char *)sha1.c_str());
hex_display(20,(unsigned char *)nonce.c_str());
hex_display(20,(unsigned char *)password.c_str());
hex_display(20,(unsigned char *)cnonce.c_str());*/

}


string readPassword(int index)
{
string password;
char pass1[40];
unsigned char pass2[20];
dictionary *d;
password=constructFileName(index,2);
lockPass(index);
d=iniparser_load((char *)password.c_str());
if(iniparser_find_entry(d,(char *)"password:pass"))
strcpy(pass1,( char *)iniparser_getstr(d,(char *)"password:pass"));
unLockPass(index);
stripIndex(pass1,pass2);
password.clear();
password.assign((char *)pass2);
return password;
}


void delFile(int index,int type)
{
delIndex(index,type);
//delete_ui(string);

}


void delIndex(int index,int type)
{
switch(type)
	{
case 0:
		{
		pthread_mutex_lock(&fileindexlock);
		//cout<<index<<"Removing index from file"<<endl;
		remove(constructFileName(index,0).c_str());
		file_index.erase(index);
		pthread_mutex_unlock(&fileindexlock);

		pthread_mutex_lock(&sha1indexlock);
		remove(constructFileName(index,2).c_str());
		sha1_index.erase(index);
		pthread_mutex_unlock(&sha1indexlock);

		pthread_mutex_lock(&vectorindexlock);
		remove(constructFileName(index,3).c_str());
		vector_index.erase(index);
		pthread_mutex_unlock(&vectorindexlock);

		break;
		}

case 1:
		{
			//cout<<index<<"Removing index from file"<<endl;

		pthread_mutex_lock(&cfileindexlock);
		remove(constructFileName(index,0).c_str());
		cache_file_index.erase(index);
		pthread_mutex_unlock(&cfileindexlock);

		pthread_mutex_lock(&csha1indexlock);
		remove(constructFileName(index,2).c_str());
		cache_sha1_index.erase(index);
		pthread_mutex_unlock(&csha1indexlock);

		pthread_mutex_lock(&cvectorindexlock);
		remove(constructFileName(index,3).c_str());
		cache_vector_index.erase(index);
		pthread_mutex_unlock(&cvectorindexlock);

		break;
		}
	}
}


void delProc(string filename, string sha1, string nonce)
{
//delete_ui(string);


}


void reassign(char a[40], string b)
{
for(int i=0;i<40;i++)
	a[i]=b[i];
}

string assign( unsigned char b[20])
{

string a;
for(int i=0;i<20;i++)
	a.push_back(b[i]);
return a;


}

void lockMeta(int index)
{
pthread_mutex_lock(&meta_lock[index]);

}


void unLockMeta(int index)
{
pthread_mutex_unlock(&meta_lock[index]);

}

void lockPass(int index)
{
	//cout<<"locking";
pthread_mutex_lock(&pass_lock[index]);
}


void unLockPass(int index)
{
		//cout<<"unlocking"<<endl;

pthread_mutex_unlock(&pass_lock[index]);
}
