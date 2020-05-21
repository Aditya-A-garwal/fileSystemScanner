#include <string>
#include <iostream>
#include <filesystem>

#define NUMDIGITS	14

using namespace std;
using namespace filesystem;

void printFileSize(unsigned long pSize) 
{
	int    ctr = 0;
	string toRet = to_string(pSize);
	
	for(int i = toRet.length()-1; i >= 0; i--) 
	{
		if(ctr%3 == 0 && ctr>0) {
			toRet = toRet.substr(0, (i+1)) + "," + toRet.substr((i+1), toRet.length());
		}
		ctr++;		
	}
	
	cout << setfill(' ') << setw(NUMDIGITS) << toRet;
}

void printIndent(int s) 
{
	for(int i = 0; i <= min(s, 6); i++) cout << "    ";	
	if(s > 6) {		
		for(int i = 6; i <= s; i++) cout << "..";
	}		
}

unsigned long getSizeOf(directory_entry entry) 
{		
	directory_iterator iter(entry.path());
				
	unsigned long size = 0;

	while(iter != end(iter)) 
	{
		directory_entry ent = *iter;			
			
		size += ent.file_size();
		if(ent.is_directory() == 1) 
			size += getSizeOf(ent);
			
		iter++;		
	}
		
	return size;
}

void printContent(path pPath, int pLevel, bool showDir, bool showFile) 
{
	unsigned long size = 0;
	unsigned int numFiles = 0;
	
	directory_iterator 	iter(pPath);
	directory_entry 	ent;
	
	while(iter != end(iter)) 
	{
		ent = *iter;											
		
	    if(ent.is_directory() == 1) 
		{		
			printFileSize(getSizeOf(ent));
			//cout << setfill(' ') << setw(NUMDIGITS) << prntNum(getSizeOf(ent));
			printIndent(pLevel);
			cout << "<" << ent.path().stem().string() << ">" << endl;
			
			if(showDir) 
				printContent(ent.path(), pLevel+1, showDir, showFile);
		}		
		
		else 
		{
			if(showFile)
			{
				printFileSize(ent.file_size());
				//cout << setfill(' ') << setw(NUMDIGITS) << prntNum(ent.file_size());
				printIndent(pLevel);
				cout << ent.path().stem().string() << ent.path().extension().string() << endl;				
			}
			else 
			{
				size += ent.file_size();
				numFiles++;
			}
		}		
		iter++;		
	}
	
	if(numFiles != 0 && !showFile) 
	{
		printFileSize(size);
		//cout << setfill(' ') << setw(NUMDIGITS) << prntNum(size);
		printIndent(pLevel);
		cout << "<" << numFiles << " files>\t" << endl;				
	}	
}

int main (int argc, char* argv[]) 
{						
	bool	showDir = 0;	
	bool	showFile = 0;			
	
	path	p = ".";	
	
	cout << "\nFilesystem Scanner v0.9" << endl;					
	cout << "From dumblebots.com" << endl;								
	
	if (argc >= 2) 
	{
		for(int i = 1; i < argc; i++) 
		{					
			if((argv[i])[0] == '-') 
			{				
				if((argv[i])[1] == 'd') 
					showDir = 1;
				else if((argv[i])[1] == 'f') 
					showFile = 1;
				else 
				{
					cout << "\nSyntax:\n";
					cout << "fss [-flag(s)] [path]\n";
					
					cout << "\nFlags:\n";					
					cout << "-d show sub-directories\n";
					cout << "-f show individual files\n";
					cout << "-h for help\n";
					
					cout << "\nExamples:\n";					
					cout << "fss (show directories with sizes from current path)\n";
					cout << "fss -f (show directories and files with sizes from current path)\n";
					cout << "fss -d -f (show directories, sub-directories and files with sizes from current path)" << endl;					
					
					return 0;
				}
			}				
			else 
				p = argv[i];						
		}			
	}					
	
	cout << absolute(p).string() << endl;
	printContent(p, 0, showDir, showFile);
}
