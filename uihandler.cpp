#include "header.h"
int nextisget=0;
int bug=0;
void *UIHandler(void *ptr)
{

char a[120];
char b[120];
	int ttl;
	string filename;
	string store_filename;
	populate();
while(shutdowns)
	{
	keywords.clear();
	filename.clear();
	char *pch;

if(bug)
		{
gets(a);

bug=0;

	continue;
		}
	cout<<"servant:"<<Port<<">";

gets(a);
	if(strcmp(a,"shutdown")==0)
		{
	shutdowns=0;
		}
else
	{

	if(strlen(a)>0)
		{
		strcpy(b,a);
		pch=strtok(a," ");
		if(strcmp(pch,"status")==0)
			{
			nextisget=0;
			pch=strtok(NULL," ");
			  if(pch!=NULL)
				{
				if(strcmp(pch,"neighbors")==0)
					{
					pch=strtok(NULL," ");

						if(pch!=NULL)
							{
							 ttl=atoi(pch);	
							 if(pch!=NULL)
								{
							 pch=strtok(NULL," ");
								 if(pch!=NULL)
								{
								filename.append(pch);
                                status_msg_generate(ttl,filename,1);

								}
								}
							}
					}
					else if(strcmp(pch,"files")==0)
					{
					pch=strtok(NULL," ");

						if(pch!=NULL)
							{
							 ttl=atoi(pch);	
							 if(pch!=NULL)
								{
							 pch=strtok(NULL," ");
								 if(pch!=NULL)
								{
								filename.append(pch);
                                status_msg_generate(ttl,filename,2);

								}
								}
							}
					}
				}
			
			
			}
			else
			{
			string t;
			t.append(b);
			breakKey(t);
			}

		
		}


	}

	}
makePersistent();
//display1();
pthread_exit(NULL);
}
void breakKey(string str)
{
	int index=0;
	string command;
	string name;
	string tt;
	string s;
	name.clear();
	//Reading the command name
	for(int i=0;i<(int)str.length();i++)
	{
	if(str[i]==' ')
		{
		index=i;
		break;
		}
	command.push_back(str[i]);
	}
	
	if(command.compare("store")==0)
	{
					nextisget=0;

	for(int i=index+1;i<(int)str.length();i++)
	{
	if(str[i]==' ')
		{
		index=i;
		break;
		}
	name.push_back(str[i]);
	}

	for(int i=index+1;i<(int)str.length();i++)
	{
	if(str[i]==' ')
		{
		index=i;
		break;
		}
	tt.push_back(str[i]);
	}

	//Splitting all the strings
	for(int i=index+1;i<(int)str.length();i++)
	{
		if(str[i]=='='||str[i]==' ')
		{
		std::transform(s.begin(),s.end(),s.begin(),::tolower);
		keywords.push_back(s);
		s.clear();
		continue;
		}
		if(str[i]!='\"')
		s.push_back(str[i]);
		else
		{
		if(i==((int)str.length()-1))
		{
		std::transform(s.begin(),s.end(),s.begin(),::tolower);
		keywords.push_back(s);
		s.clear();
		}
		}

	}
		store(name,atoi(tt.c_str()),keywords);

	//display();
	}
	else 
		if(command.compare("search")==0)
	{
			localsearchbuffer.clear();

		string type;
		string content;
		vector<string> v;
		vector<searchresponse> j;
vector<string> fd;
vector<string> md;
/*---------Reading the type of the file---------------*/
		for(int i=index+1;i<(int)str.length();i++)
	{
	if(str[i]=='=')
		{
		index=i;
		break;
	
		}
	type.push_back(str[i]);
	}
/*----------------------------------------------------*/
	for(int i=index+1;i<(int)str.length();i++)
	{

	content.push_back(str[i]);
	}


if(type.compare("filename")==0)
		{
	v.push_back(content);
	j=searchProc(v,0);

	search(0,v);
		}
if(type.compare("sha1hash")==0)
		{
	 char ab[40];
	 string snd;
	 
	reassign(ab,content);
	unsigned char shg[20];
	
	stripIndex(ab,shg);
	snd=assign(shg);

	v.push_back(snd);
		j=searchProc(v,1);

	search(1,v);


		}
if(type.compare("keywords")==0)
		{
int nospace=1;
for(int i=index+1;i<(int)str.length();i++)
	{
	if(str[i]=='\"'&&i==(index+1))
		continue;
		if(str[i]==' '||str[i]=='\"')
		{
		v.push_back(s);
		s.clear();
		nospace=0;
		
		continue;

		}

	s.push_back(str[i]);
	
	}
	if(nospace)
			{
		v.push_back(s);

			}
	j=searchProc(v,2);

	search(2,v);

	/*for(int i=0;i<(int)v.size();i++)
		cout<<v[i]<<endl;*/
	
}


for(int i=0;i<(int)j.size();i++)
		{
		md.push_back(j[i].fileid);
		fd.push_back(j[i].mdata);
		}


searchResponse(md,fd);
		int sid=1;
		while(search_flag)
		{
		pthread_mutex_lock(&globalbufferlock);
		if(globalsearchbuffer.size()==0)
			{
			pthread_mutex_unlock(&globalbufferlock);
			//cout<<"sleeping"<<endl;
			sleep(1);
			continue;
			}
		for(int i=0;i<(int)globalsearchbuffer.size();i++)
			{
			
			localsearchbuffer.push_back(globalsearchbuffer[i]);
			displaySearchResponse(globalsearchbuffer[i],sid);
			sid++;
			}
		globalsearchbuffer.clear();
		pthread_mutex_unlock(&globalbufferlock);
		}
		search_flag=1;
		nextisget=1;



	}
	else if(command.compare("get")==0)
	{
		if(nextisget)
		{
		string no;
		int ino;
		int extfile_flag=0;
		getres gs;
		getExtFile.clear();

/*---------Reading the type of the file---------------*/
	for(int i=index+1;i<(int)str.length();i++)
	{
	if(str[i]==' ')
		{
		index=i;
		extfile_flag=1;
		break;
		}
	no.push_back(str[i]);
	}
/*----------------------------------------------------*/
string nameoffile;
if(extfile_flag==1)
			{
for(int i=index+1;i<(int)str.length();i++)
				{
				nameoffile.push_back(str[i]);
				}
			}
	ino=atoi(no.c_str());
	if(localsearchbuffer.size()!=0)
			{
	gs=localsearchbuffer[ino-1];
			}

	if(extfile_flag)
			{
			getExtFile.assign(nameoffile);
			//cout<<nameoffile<<"Sdsd"<<endl;
			if(fileExistsInCurr(nameoffile))
				{
				string dec;
				cout<<"THE FILE EXISTS...SHOULD WE OVERWRITE THE FILE(yes/no)\n";
				cin>>dec;
				bug=1;
				if(dec[0]=='y'||dec[0]=='Y')
					{
					//cout<<"he is asking us to rewrite the file";
					get_UI(gs.fileid,gs.sha1);
					}
				else
					{
					cout<<"FILE RETRY FAILED\n";
					}
				}
			else
				{
				//cout<<"Calling get"<<endl;
				get_UI(gs.fileid,gs.sha1);
				}
			}
	else
			{
			getExtFile.assign(gs.filename);
			if(fileExistsInCurr(gs.filename))
				{
				string dec;
				cout<<"THE FILE EXISTS...SHOULD WE OVERWRITE THE FILE(yes/no)";
				cin>>dec;
				bug=1;
				if(dec[0]=='y'||dec[0]=='Y')
					{
					get_UI(gs.fileid,gs.sha1);
					}
				else
					{
					cout<<"RETRY FAILED"<<endl;
					}
				}
			else
				{
				//cout<<"Calling get"<<endl;
				get_UI(gs.fileid,gs.sha1);
				}

			}
		}
		else
		{
		cout<<"GET SHOULD SUCCEED SEARCH"<<endl;
		}

	}

	else if(command.compare("delete")==0)
	{
	string delfilename;
	string delsha1;
	string delNonce;
	

	/*--------------------FileName------------------------*/
	int j=index+1;
	while(str[j]!='=')
		{
		j++;
		}
	for(int i=j+1;i<(int)str.length();i++)
	{
	if(str[i]==' ')
		{
		index=i;
		break;
		}
	delfilename.push_back(str[i]);
	}
	/*-----------------------------------------------------*/


	/*--------------------SHA1-----------------------------*/
	j=index+1;
	while(str[j]!='=')
		{
		j++;
		}
	//getting the delete sha1
	for(int i=j+1;i<(int)str.length();i++)
	{
	if(str[i]==' ')
		{
		index=i;
		break;
		}
	delsha1.push_back(str[i]);
	}
	/*-----------------------------------------------------*/



	/*--------------------Nonce----------------------------*/
	j=index+1;
	while(str[j]!='=')
		{
		j++;
		}
	for(int i=j+1;i<(int)str.length();i++)
	{
	delNonce.push_back(str[i]);
	}
	int ret=passWordExists(delfilename,delsha1,delNonce);
	switch(ret)
		{
		case -1:
			break;
		case 0:
			{
			string dec;
			bug=1;
			cout<<"No one-time password found.\n";
			cout<<"Okay to use a random password [yes/no]?\n";
			cin>>dec;
			if(dec[0]=='y')
			{
			string rpass;
			rpass=generateRandomPassword();
			//delFileRequest(delfilename,delsha1,delNonce,rpass);
			}
			break;
			}
		default:
			{
			string rpass;
			rpass=readPassword(indexOfFile(delfilename,delsha1,delNonce));
			
			delFileRequest(delfilename,delsha1,delNonce,rpass);
			break;
			}
		}
	}

}
void display()
{
 for(int i=0;i<(int)keywords.size();i++)
	 cout<<keywords[i]<<endl;

}
