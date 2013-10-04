#include "header.h"
#include <sys/stat.h>

void get_UI(string FileID,string Sha1_str){

   unsigned char buf[67];
   long total_length = 40;
  // cout<<"total length is "<<total_length<<endl;
   messageconstruction(buf,43,0);
   memcpy(&buf[23],&total_length,4);
   memcpy(&buf[27],FileID.c_str(),20);
   memcpy(&buf[47],Sha1_str.c_str(),20);
  /* cout<<"Err"<<endl;
   hex_display(20,(unsigned char*)FileID.c_str());
   hex_display(20,(unsigned char*)Sha1_str.c_str());*/

    int file_ind = getRequest(FileID,Sha1_str);
	if(file_ind != 0){
	  string dataFileName = constructFileName(file_ind,3);
	  FILE *in, *out;
    char ch;

	if((in=fopen(dataFileName.c_str(), "r")) == NULL) {
     cout<<"Cannot open file"<<dataFileName<<endl;
     return;
    }
    
	 if((out=fopen(getExtFile.c_str(), "w")) == NULL) {
     cout<<"Cannot open file "<<getExtFile<<endl;
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

	}
	
    
	
   string UOID_temp((const char*)&buf[1],20);    
   string msg_temp((const char*)&buf[0],67);


   pthread_mutex_lock(&msg_buffer_mutex);
   msg_buffer.insert(std::pair<string,string>(UOID_temp,msg_temp));
   pthread_mutex_unlock(&msg_buffer_mutex);
   int l;
   pthread_mutex_lock(&write_buffer_mutex);
   pthread_mutex_lock(&act);

   //cout<<"activeconnections.size() is "<<(int)activeconnections.size()<<endl;
   for (l=0;l<(int)activeconnections.size();l++)
   {
	  double dval=(double)drand48();
	  //cout<<"prob is "<<dval<<endl;
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
  // cout<<"added into the list"<<endl;
   pthread_mutex_unlock(&write_buffer_mutex);
   pthread_mutex_unlock(&act);

    pthread_mutex_lock(&msg_UOID_port_map_mutex);
   msg_UOID_port_map.insert(std::pair<string,int>(UOID_temp,atoi(Port)));
   pthread_mutex_unlock(&msg_UOID_port_map_mutex);


}

int getRequest(string fileid,string sha1)
{
int retval=0;
map<int,string>:: iterator its;

	pthread_mutex_lock(&fileidindex);
	for(its=fileid_index.begin();its!=fileid_index.end();its++)
	{

	/*cout<<"File SHA is"<<endl;
	hex_display(20,(unsigned char *)sha1.c_str());
	cout<<"FILE ID IS"<<endl;
	hex_display(20,(unsigned char *)fileid.c_str());*/
	
	if(compareHex(fileid_index[(*its).first],fileid))
		{
			pthread_mutex_lock(&csha1indexlock);
			if(compareHex(cache_sha1_index[(*its).first],sha1))
				{
				retval=(*its).first;
				pthread_mutex_unlock(&csha1indexlock);
				break;

				}
			pthread_mutex_unlock(&csha1indexlock);

			pthread_mutex_lock(&sha1indexlock);
			if(compareHex(sha1_index[(*its).first],sha1))
				{
				retval=(*its).first;
				pthread_mutex_unlock(&sha1indexlock);
				break;
				}
			
			pthread_mutex_unlock(&sha1indexlock);
			break;
		}
	}
	pthread_mutex_unlock(&fileidindex);
	return retval;
}

map<string,vector<int> > sortIn(map<int,string> in)
{
map<string,vector<int> > out;
map<int,string>::iterator in_t;
map<string,vector<int> >::iterator out_t;
for(in_t=in.begin();in_t!=in.end();in_t++)
	{
	 vector<int> ab;
	if(out.find((*in_t).second)!=out.end())
		{
		ab=out[(*in_t).second];
		}
	 ab.push_back((*in_t).first);
	 out.erase((*in_t).second);
	 out.insert(pair<string,vector<int> >((*in_t).second,ab));
	}



return out;
}
