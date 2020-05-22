#include <string>
#include <iostream>
#include <filesystem>

#define NUMDIGITS	20

using namespace std;
using namespace filesystem;

string printFileSize(unsigned long long pSize) 
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
		
	return toRet; 
}

void printIndent(int s) 
{
	for(int i = 0; i <= min(s, 6); i++) cout << "    ";	
	if(s > 6) {		
		for(int i = 6; i <= s; i++) cout << "..";
	}		
}


void printFinalStats(unsigned long long val1, unsigned int val2, unsigned int val3, char* id) {
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(val2);		
	printIndent(val3);
	cout << "<" << val1 << " " << string(id) << ">" << endl;			
}

unsigned long long getSizeOf(directory_entry entry) 
{		

	unsigned long long size = 0;

	error_code ec;
	ec.clear();
	//directory_iterator iter(entry.path(), directory_options::skip_permission_denied);
	directory_iterator iter(entry.path(), ec);
				
	if(ec.value() != 0) {
		cout << "********* error in getSize Path: "  << ec.path1() << "  " << ec.path2() << " error value: " << ec.value() << " error msg: " << ec.message() << endl;	
		return	size;
	}												

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

void printContent(path pPath, int pLevel, bool showDir, bool showFile/*, int maxLevel, char* mark*/) 
{
	unsigned long long	fileSize = 0;
	unsigned long long	dirSize = 0;
	
	unsigned int	numFiles = 0;
	unsigned int	numDirs = 0;	
	
	error_code ec;
	ec.clear();
	directory_iterator 	iter(pPath, ec);	
	//directory_iterator 	iter(pPath, directory_options::skip_permission_denied);
	directory_entry 	ent;
	
	if(ec.value() != 0) {
		cout << "********* error in printContent Path: "  << pPath << " error value " << ec.value() << endl;	
	}
		
	while(iter != end(iter)) 
	{
		ent = *iter;																	
		
		unsigned long	entrySize;			
		
	    if(ent.is_directory() == 1) 
		{	
			numDirs++;
			entrySize = getSizeOf(ent);
			dirSize += entrySize;
	
			cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);		
			printIndent(pLevel);
			cout << "<" << ent.path().stem().string() << ">" << endl;				
			
			if(showDir) 
				printContent(ent.path(), pLevel+1, showDir, showFile);
		}		
		
		else 
		{			
			numFiles++;	
			entrySize = ent.file_size();
			fileSize += entrySize;					
			
			if(showFile)
			{				
				cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);				
				printIndent(pLevel);
				cout << ent.path().stem().string() << ent.path().extension().string() << endl;											
			}								
		}		
			
			
		iter++;		
	}
	
	if(numFiles != 0 && !showFile)	
		printFinalStats(numFiles, fileSize, pLevel, "files");	
	
	if(pLevel == 0) 
	{				
		cout << "\n";
		printFinalStats(numFiles, fileSize, pLevel, "files");
		printFinalStats(numDirs, dirSize, pLevel, "sub-directories");
		printFinalStats(numFiles + numDirs, fileSize+dirSize, pLevel, "total entries");								
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
