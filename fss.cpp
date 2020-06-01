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
	bool		u_show_err;	
	
	unsigned long      	u_total_file;
	unsigned long      	u_total_dir;
	unsigned long      	u_inaccessible_file;
	unsigned long      	u_inaccessible_dir;
	unsigned long      	u_invalid_entry;	
	
	unsigned long		u_file;
	unsigned long		u_dir;
	unsigned long		u_file_size;
	unsigned long		u_dir_size;
	
	char*				u_filter;
};

string printFileSize(long long value) 
{
	string numWithCommas = to_string(value);
	int insertPosition = numWithCommas.length() - 3;
	while (insertPosition > 0) {
		numWithCommas.insert(insertPosition, ",");
		insertPosition-=3;
	}
	return numWithCommas;
}

void printIndent(int s) 
{
	for(int i = 0; i <= min(s, 6); i++) cout << "    ";	
	if(s > 6) {		
		for(int i = 6; i <= s; i++) cout << "..";
	}		
}

void printFinalStats(long long val1, unsigned int val2, unsigned int val3, char* id) 
{
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(val2);		
	printIndent(val3);
	cout << "<" << val1 << " " << string(id) << ">" << endl;			
}

void printErr(error_code & ec, directory_entry & entry) 
{	
	cout << "         ***ERROR***    " << entry.path().string() << "\t" << ec.message() << "\t(code: " << ec.value() << ")" << endl;
	
}

bool my_find(string c1, char * c2)
{
	size_t found = c1.find(string(c2));
	if(found != string::npos) return true;
	else return false;
	
}
long long size_of_dir(path pPath, error_code & ecode, struct FSS_Info * pFss_info) 
{	
	unsigned long	total_file = 0;
	unsigned long	total_dir = 0;
	
	unsigned long	inaccessible_file = 0;
	unsigned long	inaccessible_dir = 0;
	
	long long 		vSize = 0;
	error_code 	ec;	
	
	directory_iterator vIter(pPath, ec);			
	
	if(ec.value() != 0) 
	{					
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
			if(pFss_info) {
				pFss_info->u_invalid_entry++;						
				if(pFss_info->u_show_err)
					printErr(ec, entry);
			}
			ec.clear();
			continue;			
		}
		
		if(isDir) 
		{
			total_dir++;
			vSize += size_of_dir(entry, ec, pFss_info);
			if(ec.value() != 0) 
			{				
				if(pFss_info) {
					pFss_info->u_inaccessible_dir++;							
					if(pFss_info->u_show_err)
						printErr(ec, entry);
				}
				ec.clear();
				continue;			
			}
			continue;
		}
		
		bool isFile = entry.is_regular_file(ec); 
		if(ec.value() != 0) 
		{		
			if(pFss_info) {
				pFss_info->u_invalid_entry++;						
				if(pFss_info->u_show_err)
					printErr(ec, entry);
			}
			ec.clear();
			continue;			
		}
		
		if(isFile)		
		{	
			total_file++;
			
			long long sizeBuff = entry.file_size(ec);
			if(ec.value() != 0) 
			{
				if(pFss_info) {
					pFss_info->u_inaccessible_file++;							
					if(pFss_info->u_show_err)
						printErr(ec, entry);
				}
				ec.clear();
				continue;
			}			
			vSize += sizeBuff;									
		}
	}

	if(pFss_info)
	{
		pFss_info->u_total_file += total_file;
		pFss_info->u_total_dir += total_dir;
	}
	
	return vSize;
}

void scan_path(path pPath, int u_level, struct FSS_Info & pFss_info) 
{
	bool			isFile;
	bool			isDir;
	unsigned int	numFiles = 0;
	unsigned int	numDirs = 0;
	unsigned long 	totalFiles = 0;
	unsigned long 	totalDirs = 0;
	
	unsigned int	invalidEntries = 0;

	long long		fileSize = 0;
	long long		dirSize = 0;
	
	long long		entrySize = 0;		
	
	error_code 			ec;
	directory_entry 	entry;	
	directory_iterator 	vIter(pPath, ec);			
	
	if(ec.value() != 0) {
		if(u_level == 0) 
			pFss_info.u_inaccessible_dir;
		return;
	}

	while(vIter != end(vIter)) 
	{
		entry = *vIter;					
		vIter++;		
		
		isDir = entry.is_directory(ec);
		isFile = entry.is_regular_file(ec);
		
		if(ec.value() != 0) 
		{
			if(u_level == 0) {
				pFss_info.u_invalid_entry++;			
				if(pFss_info.u_show_err)
					printErr(ec, entry);
			}
			ec.clear();
			continue;
		}
		
	    if(isDir == true) 
		{	
			numDirs++;	
			
			
			if(u_level == 0) 
			{
				entrySize = size_of_dir(entry.path(), ec, &pFss_info);
				totalDirs++;
			}
			else 
				entrySize = size_of_dir(entry.path(), ec, NULL);
	
			if(ec.value() != 0) 
			{				
				if(u_level == 0) 
				{
					pFss_info.u_inaccessible_dir++;	
					if(pFss_info.u_show_err)
						printErr(ec, entry);
				}
				ec.clear();
			}
			
			dirSize += entrySize;							
	
			cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);		
			printIndent(u_level);
			cout << "<" << entry.path().filename().string() << ">" << endl;				
			
			if(pFss_info.u_show_dir) 			
				scan_path(entry.path(), u_level+1, pFss_info);

			continue;
		}									
		
		if(isFile == true)
		{				
			totalFiles++;
	
			numFiles++;
			entrySize = entry.file_size(ec);			
			
			if(ec.value() != 0) 
			{
				if(u_level == 0) 
				{
					if(pFss_info.u_show_err)
						pFss_info.u_inaccessible_file++;				
					printErr(ec, entry);
				}
				ec.clear();
				entrySize = 0;				
			}
			
			fileSize += entrySize;										
			
			if(pFss_info.u_show_file)
			{						
				cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);				
				printIndent(u_level);												
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
		pFss_info.u_total_file += totalFiles;
		pFss_info.u_total_dir += totalDirs;
		pFss_info.u_file = numFiles;
		pFss_info.u_dir = numDirs;
		pFss_info.u_file_size = fileSize;
		pFss_info.u_dir_size = dirSize;								
	}
}

void scan_path_filter(path pPath, int u_level, struct FSS_Info & pFss_info) 
{
	bool			isFile;
	bool			isDir;
	unsigned int	numFiles = 0;
	unsigned int	numDirs = 0;
	unsigned long 	totalFiles = 0;
	unsigned long 	totalDirs = 0;
	
	unsigned int	invalidEntries = 0;

	long long		fileSize = 0;
	long long		dirSize = 0;
	
	long long		entrySize = 0;		
	
	error_code 			ec;
	directory_entry 	entry;	
	directory_iterator 	vIter(pPath, ec);			
	
	if(ec.value() != 0) {
		if(u_level == 0) 
			pFss_info.u_inaccessible_dir;
		return;
	}

	while(vIter != end(vIter)) 
	{
		entry = *vIter;					
		vIter++;		
		
		isDir = entry.is_directory(ec);
		isFile = entry.is_regular_file(ec);
		
		if(ec.value() != 0) 
		{
			if(u_level == 0) {
				pFss_info.u_invalid_entry++;			
				if(pFss_info.u_show_err)
					printErr(ec, entry);
			}
			ec.clear();
			continue;
		}
		
	    if(isDir == true) 
		{					
			totalDirs++;			
			pFss_info.u_total_dir++;		
	
			if(my_find(entry.path().filename().string(), pFss_info.u_filter)) 
			{
				numDirs++;					
				if(u_level == 0) 							
					entrySize = size_of_dir(entry.path(), ec, &pFss_info);
				else  							
					entrySize = size_of_dir(entry.path(), ec, NULL);
				
				if(ec.value() != 0) 
				{				
					if(u_level == 0) 
					{
						pFss_info.u_inaccessible_dir++;	
						if(pFss_info.u_show_err)
							printErr(ec, entry);
					}
					ec.clear();
				}
				
				dirSize += entrySize;							
				pFss_info.u_dir_size += entrySize;		
				
				cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);		
				printIndent(0);
				cout << "<" << entry.path().filename().string() << ">" << "\t" << entry.path().parent_path().string() << endl;											
			}				
			scan_path_filter(entry.path(), u_level+1, pFss_info);				
			continue;
		}			
		
		if(isFile == true)
		{						
			totalFiles++;
			pFss_info.u_total_file++;
	
			if(my_find(entry.path().filename().string(), pFss_info.u_filter)) 
			{
				numFiles++;			
				entrySize = entry.file_size(ec);			
			
				if(ec.value() != 0) 
				{
					if(u_level == 0) 
					{
						if(pFss_info.u_show_err)
							pFss_info.u_inaccessible_file++;				
						printErr(ec, entry);
					}
					ec.clear();
					entrySize = 0;				
				}
				
				fileSize += entrySize;			
				pFss_info.u_file_size += entrySize;				
								
				cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);				
				printIndent(0);												
				cout << entry.path().filename().string() << "\t" << entry.path().parent_path().string() << endl;													
			}											
		}						
	}							
		
	pFss_info.u_file += numFiles;
	pFss_info.u_dir += numDirs;					
}

int main (int argc, char* argv[]) 
{			

	bool path_set = false;
	bool apply_filter = false;
	
	struct FSS_Info	fss_info;		
	path	p;	

	cout << "\nFilesystem Scanner v0.9" << endl;					
	cout << "From dumblebots.com" << endl;								
		
	fss_info.u_show_dir 			= false;
	fss_info.u_show_file 			= false;	
	fss_info.u_show_err 			= false;		
	
	fss_info.u_total_file			= 0;
	fss_info.u_total_dir			= 0;
	fss_info.u_inaccessible_file	= 0;
	fss_info.u_inaccessible_dir		= 0;
	fss_info.u_invalid_entry		= 0;
	fss_info.u_dir					= 0;
	fss_info.u_file					= 0;
	fss_info.u_filter				= NULL;
	
	p = ".";	
	
	if (argc >= 2)
	{
		for(int i = 1; i < argc; i++) 
		{					
			if((argv[i])[0] == '-') 
			{				
				if((argv[i])[1] == 'd') 
					fss_info.u_show_dir = true;
				else if((argv[i])[1] == 'f') 
					fss_info.u_show_file = true;
				else if((argv[i])[1] == 'e')
					fss_info.u_show_err = true;
				else 
				{
					cout << "\nSyntax:\n";
					cout << "fss [-flag(s)] [path] [filter]\n";
					
					cout << "\nFlags:\n";					
					cout << "-d show sub-directories\n";
					cout << "-f show individual files\n";
					cout << "-e show errors\n";
					cout << "-h for help\n";
					
					cout << "\nExamples:\n";					
					cout << "fss (show directories with sizes from current path)\n";
					cout << "fss -f (show directories and files with sizes from current path)\n";
					cout << "fss -d -f (show directories, sub-directories and files with sizes from current path)" << endl;					
					cout << "fss -e (show directories with sizes from current path along with errors)\n";
					
					return 0;
				}
			}				
			else 
			{
				if(!path_set)
				{	
					p = argv[i];						
					path_set = true;
				}
				else 
				{
					fss_info.u_filter = argv[i];		
					apply_filter = true;					
				}
			}
		}			
	}								
	
	cout << absolute(p).string() << endl;
	if(!apply_filter) scan_path(p, 0, fss_info);
	else scan_path_filter(p, 0, fss_info);
	
	cout << endl;
	
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_file_size);			
	cout << "\t<" << fss_info.u_file << " files>" << endl;		
	
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_dir_size);			
	cout << "\t<" << fss_info.u_dir << " sub-directories>" << endl;		
	
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_file_size+fss_info.u_dir_size);			
	cout << "\t<" << fss_info.u_file+fss_info.u_dir << " total entries>" << endl;		
	
	cout << endl;
	
	cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_file);		
	cout << "\tfiles traversed";
	if(fss_info.u_inaccessible_file != 0) 
		cout << " (" << fss_info.u_inaccessible_file << " inaccessible)";	
	
	cout << "\n" << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_dir);			
	cout << "\tdirectories traversed";
	if(fss_info.u_inaccessible_dir != 0) 
		cout << " (" << fss_info.u_inaccessible_dir << " inaccessible)";	
}
