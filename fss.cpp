#include <string>
#include <iostream>
#include <filesystem>
#include <fcntl.h>
#include <io.h>

#define NUMDIGITS	20

using namespace std;
using namespace filesystem;

struct FSS_Info 
{	
	bool 		u_show_dir;
	bool 		u_show_file;
	
	unsigned long      	u_total_file;
	unsigned long      	u_total_dir;
	unsigned long      	u_inaccessible_file;
	unsigned long      	u_inaccessible_dir;
	unsigned long      	u_invalid_entry;
	
	//long long 			u_total_size;
};

string printFileSize(long long pSize) 
{
	//int    ctr = 0;
	string toRet = to_string(pSize);
	
/*	
	for(int i = toRet.length()-1; i >= 0; i--) 
	{
		if(ctr%3 == 0 && ctr>0) {
			toRet = toRet.substr(0, (i+1)) + "," + toRet.substr((i+1), toRet.length());
		}
		ctr++;		
	}	
	*/	
	return toRet; 
}

void printIndent(int s) 
{
	for(int i = 0; i <= min(s, 6); i++) cout << "    ";	
	if(s > 6) {		
		for(int i = 6; i <= s; i++) cout << "..";
	}		
}

void printFinalStats(long long val1, unsigned int val2, unsigned int val3, char* id) {
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(val2);		
	printIndent(val3);
	cout << "<" << val1 << " " << string(id) << ">" << endl;			
}

long long size_of_dir(path pPath, error_code & ecode) 
{		
	long long vSize = 0;
	
	error_code ec;	
	directory_iterator vIter(pPath, ec);			
	
	if(ec.value() != 0) 
	{				
		cout << "***** size_of_dir : creating directory iterator: " << ec.value() << "  " << ec.message() << endl;
		ecode = ec;
		return 0;
	}
	
	ecode.clear();

	while(vIter != end(vIter)) 
	{
		directory_entry entry = *vIter;	
		vIter++;		
		
		bool isDir = entry.is_directory(ec); 
		if(ec.value() != 0) 
		{				
			cout << "***** size_of_dir : checking isDir observer: " << ec.value() << "  " << ec.message() << endl;			
			ec.clear();
			continue;			
		}
		
		if(isDir) 
		{
			vSize += size_of_dir(entry, ec);
			if(ec.value() != 0) 
			{				
				cout << "***** size_of_dir : reading dir size: " << ec.value() << "  " << ec.message() << endl;
				ec.clear();
				continue;			
			}
			continue;
		}
		
		bool isFile = entry.is_regular_file(ec); 
		if(ec.value() != 0) 
		{				
			cout << "***** size_of_dir : reading file size: " << ec.value() << "  " << ec.message() << endl;
			ec.clear();
			continue;			
		}
		
		if(isFile)		
		{	
			long long sizeBuff = entry.file_size(ec);
			if(ec.value() != 0) {
				cout << "***** size_of_dir : checking isFile observer: " << ec.value() << "  " << ec.message() << endl;
				ec.clear();
				continue;
			}			
			vSize += sizeBuff;									
		}
	}	
	return vSize;
}

long long size_of_dir(path pPath, error_code & ecode, struct FSS_Info & pFss_info) 
{		
	long long vSize = 0;
	
	error_code ec;	
	directory_iterator vIter(pPath, ec);			
	
	pFss_info.u_total_dir++;
	
	if(ec.value() != 0) 
	{			
		pFss_info.u_inaccessible_dir++;
		cout << "***** size_of_dir : creating directory iterator: " << ec.value() << "  " << ec.message() << endl;
		ecode = ec;
		return 0;
	}
	
	ecode.clear();

	while(vIter != end(vIter)) 
	{
		directory_entry entry = *vIter;	
		vIter++;		
		
		bool isDir = entry.is_directory(ec); 
		if(ec.value() != 0) 
		{							
			pFss_info.u_invalid_entry++;
			cout << "***** size_of_dir : checking isDir observer: " << ec.value() << "  " << ec.message() << endl;			
			ec.clear();
			continue;			
		}
		
		if(isDir) 
		{
			vSize += size_of_dir(entry, ec, pFss_info);
			if(ec.value() != 0) 
			{				
				pFss_info.u_inaccessible_dir++;
				cout << "***** size_of_dir : reading dir size: " << ec.value() << "  " << ec.message() << endl;
				ec.clear();
				continue;			
			}
			continue;
		}
		
		bool isFile = entry.is_regular_file(ec); 
		if(ec.value() != 0) 
		{		
			pFss_info.u_invalid_entry++;
			cout << "***** size_of_dir : reading file size: " << ec.value() << "  " << ec.message() << endl;
			ec.clear();
			continue;			
		}
		
		if(isFile)		
		{	
			pFss_info.u_total_file++;
			long long sizeBuff = entry.file_size(ec);
			if(ec.value() != 0) {
				pFss_info.u_inaccessible_file++;
				cout << "***** size_of_dir : checking isFile observer: " << ec.value() << "  " << ec.message() << endl;
				ec.clear();
				continue;
			}			
			vSize += sizeBuff;									
		}
	}	
	return vSize;
}

void scan_path(path & pPath, int u_level, struct FSS_Info & pFss_info/*, int pLevel, bool showDir, bool showFile/*, int maxLevel, char* mark*/) 
{
	long long	fileSize = 0;
	long long	dirSize = 0;
	
	unsigned int	numFiles = 0;
	unsigned int	numDirs = 0;	
	
	error_code ec;
	directory_iterator 	vIter(pPath, ec);			
	directory_entry 	entry;	
	
	if(ec.value() != 0) {
		cout << "***** scan_path : creating directory iterator: " << ec.value() << "  " << ec.message() << endl;
		return;
	}

	while(vIter != end(vIter)) 
	{
		entry = *vIter;					
		vIter++;		
		
		bool isDir = entry.is_directory(ec);
		if(ec.value() != 0) {
			if(u_level == 0) 
				pFss_info.u_invalid_entry++;
			cout << "***** scan_path : checking isDirectory observer: " << ec.value() << "  " << ec.message() << endl;
			ec.clear();
			continue;
		}
		
		unsigned long	entrySize;						
				
	    if(isDir == true) 
		{	
			numDirs++;	
			if(u_level == 0) entrySize = size_of_dir(entry.path(), ec, pFss_info);
			else entrySize = size_of_dir(entry.path(), ec);
	
			if(ec.value() != 0) 
			{
				cout << "*****scan_path : reading dir size: " << ec.value() << "  " << ec.message() << endl;
				ec.clear();
				//continue;
			}
			
			dirSize += entrySize;							
	
			cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);		
			printIndent(u_level);
			cout << "<" << entry.path().filename().string() << ">" << endl;				
			
			if(pFss_info.u_show_dir) 			
				scan_path((path)(entry.path()), u_level+1, pFss_info);

			continue;
		}		
				
		bool isFile = entry.is_regular_file(ec);
		if(ec.value() != 0) {
			if(u_level == 0) 
				pFss_info.u_invalid_entry++;
			cout << "*****scan_path : checking isFile observer: " << ec.value() << "  " << ec.message() << endl;
			ec.clear();
			continue;
		}		
		
		if(isFile == true)
		{				
			if(u_level == 0) 
				pFss_info.u_total_file++;
	
			numFiles++;
			entrySize = entry.file_size(ec);			
			
			if(ec.value() != 0) 
			{
				if(u_level == 0) 
					pFss_info.u_inaccessible_file++;
				cout << "*****scan_path : reading file size: " << ec.value() << "  " << ec.message() << endl;
				ec.clear();
				entrySize = 0;
				//continue;
			}
			
			fileSize += entrySize;										
			
			if(pFss_info.u_show_file)
			{						
				cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);				
				printIndent(u_level);		
				//cout << entry.path().string() << endl;							
				wcout << (entry.path().filename().wstring());				
				cout << "XXX" << endl; 
			}		

			continue;
		}						
	}	
	
	if(numFiles != 0 && !pFss_info.u_show_file)	
		printFinalStats(numFiles, fileSize, u_level, "files");	
	
	if(u_level == 0) 
	{				
		cout << "\n";
		printFinalStats(numFiles, fileSize, u_level, "files");
		printFinalStats(numDirs, dirSize, u_level, "sub-directories");
		printFinalStats(numFiles + numDirs, fileSize+dirSize, u_level, "total entries");								
	}
}

int main (int argc, char* argv[]) 
{			
	struct FSS_Info	fss_info;		
	path	p;	
/*	
#ifdef _MSC_VER
    _setmode(_fileno(stderr), _O_WTEXT);
#else
    std::setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));
    std::cout.imbue(std::locale());
    std::wcerr.imbue(std::locale());
#endif	
*/	
	cout << "\nFilesystem Scanner v0.9" << endl;					
	cout << "From dumblebots.com" << endl;								
		
	fss_info.u_show_dir 			= 0;
	fss_info.u_show_file 			= 0;	
	fss_info.u_total_file			= 0;
	fss_info.u_total_dir			= 0;
	fss_info.u_inaccessible_file	= 0;
	fss_info.u_inaccessible_dir		= 0;
	fss_info.u_invalid_entry		= 0;
	
	p = ".";	
	
	if (argc >= 2)
	{
		for(int i = 1; i < argc; i++) 
		{					
			if((argv[i])[0] == '-') 
			{				
				if((argv[i])[1] == 'd') 
					fss_info.u_show_dir = 1;
				else if((argv[i])[1] == 'f') 
					fss_info.u_show_file = 1;
				else 
				{
					cout << "\nSyntax:\n";
					cout << "fss [-flag(s)] [path]\n";
					
					cout << "\nFlags:\n";					
					cout << "-d show sub-directories\n";
					cout << "-f show individual files\n";
					cout << "-e show errors\n";
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
	scan_path(p, 0, fss_info);
	
	cout << endl;
	
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_file);		
	printIndent(0);
	cout << "files traversed" << " (" << fss_info.u_inaccessible_file << " innaccessible)" << endl;
	
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_dir);		
	printIndent(0);
	cout << "directories traversed" << " (" << fss_info.u_inaccessible_dir << " innaccessible)" << endl;
		
	//cout << "Total inaccessible file entries: " << fss_info.u_inaccessible_file << endl;
	//cout << "Total inaccessible directory entries: " << fss_info.u_inaccessible_dir << endl;
	//cout << "Invalid entries: " << fss_info.u_invalid_entry << endl;
}
