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
	bool		u_apply_filter;	
	
	unsigned long      	u_total_files;
	unsigned long      	u_total_dirs;	
	
	unsigned long      	u_inaccessible_file;
	unsigned long      	u_inaccessible_dir;
	unsigned long      	u_invalid_entry;	
	
	unsigned long		u_files_in_path;
	unsigned long		u_dirs_in_path;	
	
	long long			u_total_dir_size;
	long long			u_total_file_size;
	
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
	if(s <= 6)
		for(int i = 0; i <= s; i++) cout << "    ";	
	else 	
	{	
		for(int i = 0; i <= 6; i++) cout << "    ";	
		for(int i = 7; i <= s; i++) cout << "..";	
	}
}

void printErr(error_code & ec, directory_entry & entry) 
{	
	wcout << L"         ***ERROR***    " << entry.path().wstring() << L"\t";
	cout << ec.message() << "\t(code: " << ec.value() << ")\n";
	
}

bool my_find(string c1, char * c2)
{
	size_t found = c1.find(string(c2));
	if(found != string::npos) return true;
	else return false;
	
}
long long size_of_dir(path pPath, error_code & ecode, struct FSS_Info * pFss_info) 
{	
	unsigned long	total_files = 0;
	unsigned long	total_dirs = 0;
	
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
		
		bool is_dir = entry.is_directory(ec); 
		bool is_file = entry.is_regular_file(ec); 		
		
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
		
		if(is_dir) 
		{
			total_dirs++;
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
		
		if(is_file)		
		{	
			total_files++;
			
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
		pFss_info->u_total_files += total_files;
		pFss_info->u_total_dirs += total_dirs;
	}
	
	return vSize;
}

void scan_path(path pPath, int u_level, struct FSS_Info & pFss_info) 
{
	unsigned long	numFiles = 0;
	unsigned long	numDirs = 0;

	long long		fileSize = 0;
	long long		dirSize = 0;
	
	long long		entrySize = 0;		
	
	error_code 			ec;	
	directory_iterator 	vIter(pPath, ec);			
	
	if(ec.value() != 0) {
		if(u_level == 0) 
			pFss_info.u_inaccessible_dir;
		return;
	}

	while(vIter != end(vIter)) 
	{
		directory_entry 	entry;
		
		entry = *vIter;					
		vIter++;		
		
		bool is_dir = entry.is_directory(ec);
		bool is_file = entry.is_regular_file(ec);
		
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
		
	    if(is_dir == true) 
		{	
			if(!pFss_info.u_apply_filter)
			{
				numDirs++;	
								
				if(u_level == 0) 
				{
					entrySize = size_of_dir(entry.path(), ec, &pFss_info);
					pFss_info.u_total_dirs++;
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
				wcout << "<" << entry.path().filename().wstring() << L">\n";											
			}
			else if(my_find(entry.path().filename().string(), pFss_info.u_filter))
			{						
				pFss_info.u_dirs_in_path++;
				entrySize = size_of_dir(entry.path(), ec, NULL);
				if(ec.value() != 0) 
				{				
					if(u_level == 0 && pFss_info.u_show_err) 								
						printErr(ec, entry);
					
					ec.clear();
				}
				
				pFss_info.u_total_dir_size += entrySize; 
				
				cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);						
				wcout << L"\t<" << entry.path().filename().wstring() << L">\t" << entry.path().parent_path().wstring() << endl;		
		
			}			
			if(pFss_info.u_show_dir) 			
				scan_path(entry.path(), u_level+1, pFss_info);

			continue;
		}									
		
		if(is_file == true)
		{		
			if(!pFss_info.u_apply_filter)
			{
				pFss_info.u_total_files++;
	
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
					wcout << entry.path().filename().wstring() << L"\n";									
				}		

				continue;
			}
			else if(my_find(entry.path().filename().string(), pFss_info.u_filter) && pFss_info.u_show_file)
			{
				pFss_info.u_files_in_path++;
				entrySize = entry.file_size(ec);
				if(ec.value() != 0) 
				{				
					if(u_level == 0 && pFss_info.u_show_err) 										
						printErr(ec, entry);					
					
					ec.clear();
				}
				
				pFss_info.u_total_file_size += entrySize; 
				
				cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(entrySize);						
				wcout << L"\t" << entry.path().filename().wstring() << L"\t" << entry.path().parent_path().wstring() << L"\n";		
			}
			continue;			
		}						
	}	
	
	if(numFiles != 0 && !pFss_info.u_show_file)	
	{		
		cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fileSize);		
		printIndent(u_level);
		cout << "<" << numFiles << " files>" << endl;	
	}
	
	if(u_level == 0 && !pFss_info.u_apply_filter) 
	{			
		pFss_info.u_files_in_path = numFiles;
		pFss_info.u_dirs_in_path = numDirs;
		pFss_info.u_total_file_size = fileSize;
		pFss_info.u_total_dir_size = dirSize;								
	}
}

int main (int argc, char* argv[]) 
{			

	bool path_set = false;	
	
	struct FSS_Info	fss_info;		
	path	p;	

	cout << "Filesystem Scanner v0.9" << endl;					
	cout << "From dumblebots.com" << endl;								
		
	fss_info.u_show_dir 			= false;
	fss_info.u_show_file 			= false;	
	fss_info.u_show_err 			= false;		
	fss_info.u_apply_filter			= false;		
	
	fss_info.u_total_file_size		= 0;
	fss_info.u_total_dir_size		= 0;
	
	fss_info.u_total_files			= 0;
	fss_info.u_total_dirs			= 0;
	fss_info.u_inaccessible_file	= 0;
	fss_info.u_inaccessible_dir		= 0;
	fss_info.u_invalid_entry		= 0;
	fss_info.u_dirs_in_path			= 0;
	fss_info.u_files_in_path		= 0;
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
					fss_info.u_apply_filter = true;					
				}
			}
		}			
	}								
	
	cout << absolute(p).string() << endl;
	scan_path(p, 0, fss_info);	

	if(!fss_info.u_apply_filter)
	{	
		cout << "\nLocal Statistics of " << absolute(p).string() << endl;
		
		cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_file_size);			
		cout << "\t<" << fss_info.u_files_in_path << " files>" << endl;		
		
		cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_dir_size);			
		cout << "\t<" << fss_info.u_dirs_in_path << " sub-directories>" << endl;		
		
		cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_file_size+fss_info.u_total_dir_size);			
		cout << "\t<" << fss_info.u_files_in_path+fss_info.u_dirs_in_path << " total entries>" << endl;		
		
		cout << "\nGlobal Statistics of " << absolute(p).string() << endl;
	
		cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_files);		
		cout << "\tfiles traversed";
		if(fss_info.u_inaccessible_file != 0) 
			cout << " (" << fss_info.u_inaccessible_file << " inaccessible)";	
		
		cout << "\n" << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_dirs);			
		cout << "\tdirectories traversed";
		if(fss_info.u_inaccessible_dir != 0) 
			cout << " (" << fss_info.u_inaccessible_dir << " inaccessible)";	
	}	
	else 
	{
		cout << endl;
		cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_file_size);			
		cout << "\t<" << fss_info.u_files_in_path << " files found>" << endl;		
		
		cout << setfill(' ') << setw(NUMDIGITS) << printFileSize(fss_info.u_total_dir_size);			
		cout << "\t<" << fss_info.u_dirs_in_path << " directories found>" << endl;				
	}
}
