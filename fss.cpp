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
	unsigned long      	u_total_symlinks;	
	
	unsigned long      	u_inaccessible_file;
	unsigned long      	u_inaccessible_dir;
	unsigned long      	u_invalid_entry;	
	
	unsigned long		u_files_in_path;
	unsigned long		u_dirs_in_path;	
	unsigned long		u_symlinks_in_path;	
	
	long long			u_total_dir_size;
	long long			u_total_file_size;
	
	char*				u_filter;
};

string format_number(long long val) 
{
	string formatted_num = to_string(val);
	int insert_pos = formatted_num.length() - 3;
	while (insert_pos > 0) 
	{
		formatted_num.insert(insert_pos, ",");
		insert_pos -= 3;
	}
	return formatted_num;
}

void print_indent(int s) 
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
	if(found != string::npos) 
		return true;
	else 
		return false;
	
}
long long size_of_dir(path pPath, error_code & ecode, struct FSS_Info * pFss_info) 
{	
	unsigned long	total_files		= 0;
	unsigned long	total_symlinks	= 0;
	unsigned long	total_dirs 		= 0;
	
	long long 		size_of_path	= 0;
	long long		entry_size		= 0;
	
	error_code 	ec;	
	
	directory_iterator path_iterator(pPath, ecode);			
	
	if(ecode.value() != 0) 				
		return 0;		

	while(path_iterator != end(path_iterator)) 
	{
		directory_entry entry = *path_iterator++;			
		
		bool is_dir = entry.is_directory(ec); 
		bool is_file = entry.is_regular_file(ec); 		
		bool is_sym = entry.is_symlink(ec); 	
		
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
			entry_size = size_of_dir(entry, ec, pFss_info);
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
			size_of_path += entry_size;
			continue;
		}			
		
		if(is_file)		
		{	
			if(is_sym)
			{
				total_symlinks++;
				continue;
			}
			
			total_files++;
			
			entry_size = entry.file_size(ec);
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
			size_of_path += entry_size;	
			continue;			
		}
	}

	if(pFss_info)
	{
		pFss_info->u_total_files += total_files;
		pFss_info->u_total_symlinks += total_symlinks;
		pFss_info->u_total_dirs += total_dirs;
	}
	
	return size_of_path;
}

void scan_path(path pPath, int u_level, struct FSS_Info & pFss_info) 
{
	unsigned long	numFiles = 0;
	unsigned long	numSymlinks = 0;
	unsigned long	numDirs = 0;

	long long		fileSize = 0;
	long long		dirSize = 0;
	
	long long		entry_size = 0;		
	
	error_code 			ec;	
	directory_iterator 	path_iterator(pPath, ec);			
	
	if(ec.value() != 0) {
		if(u_level == 0) 
			pFss_info.u_inaccessible_dir;
		return;
	}

	while(path_iterator != end(path_iterator)) 
	{
		directory_entry 	entry;
		
		entry = *path_iterator++;							
		
		bool is_dir = entry.is_directory(ec);
		bool is_file = entry.is_regular_file(ec);
		bool is_sym = entry.is_symlink(ec);
		
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
					entry_size = size_of_dir(entry.path(), ec, &pFss_info);
					pFss_info.u_total_dirs++;
				}
				else 
					entry_size = size_of_dir(entry.path(), ec, NULL);
		
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
				
				dirSize += entry_size;							
		
				cout << setfill(' ') << setw(NUMDIGITS) << format_number(entry_size);		
				print_indent(u_level);
				wcout << "<" << entry.path().filename().wstring() << L">\n";											
			}
			else if(my_find(entry.path().filename().string(), pFss_info.u_filter))
			{						
				pFss_info.u_dirs_in_path++;
				entry_size = size_of_dir(entry.path(), ec, NULL);
				if(ec.value() != 0) 
				{				
					if(u_level == 0 && pFss_info.u_show_err) 								
						printErr(ec, entry);
					
					ec.clear();
				}
				
				pFss_info.u_total_dir_size += entry_size; 
				
				cout << setfill(' ') << setw(NUMDIGITS) << format_number(entry_size);						
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
				if(is_sym)
				{
					numSymlinks++;
					if(u_level == 0) 
						pFss_info.u_total_symlinks++;
					
					if(pFss_info.u_show_file)
					{						
						cout << "             SYMLINK\t";										
						wcout << entry.path().filename().wstring() << L"[" << read_symlink(entry).wstring() << L"]\n";
					}
					continue;
				}
				pFss_info.u_total_files++;
	
				numFiles++;
				entry_size = entry.file_size(ec);			
			
				if(ec.value() != 0) 
				{
					if(u_level == 0) 
					{
						if(pFss_info.u_show_err)
							pFss_info.u_inaccessible_file++;				
						printErr(ec, entry);
					}
					ec.clear();
					entry_size = 0;				
				}
			
				fileSize += entry_size;										
			
				if(pFss_info.u_show_file)
				{						
					cout << setfill(' ') << setw(NUMDIGITS) << format_number(entry_size);				
					print_indent(u_level);												
					wcout << entry.path().filename().wstring() << L"\n";
				}
				continue;
			}						
			if(pFss_info.u_show_file && my_find(entry.path().filename().string(), pFss_info.u_filter))
			{				
				if(is_sym)
				{
					pFss_info.u_symlinks_in_path++;															
					
					cout << "             SYMLINK\t";										
					wcout << entry.path().filename().wstring() << L"[" << read_symlink(entry).wstring() << L"]\t" << entry.path().parent_path().wstring() << L"\n";
					continue;
				}
				
				pFss_info.u_files_in_path++;
				entry_size = entry.file_size(ec);
				if(ec.value() != 0) 
				{				
					if(u_level == 0 && pFss_info.u_show_err) 										
						printErr(ec, entry);										
					ec.clear();
				}
				
				pFss_info.u_total_file_size += entry_size; 
				
				cout << setfill(' ') << setw(NUMDIGITS) << format_number(entry_size);						
				wcout << L"\t" << entry.path().filename().wstring() << L"\t" << entry.path().parent_path().wstring() << L"\n";						
			}			
			continue;			
		}	
		
	}	
	
	if(!pFss_info.u_show_file)	
	{	
		if(numFiles != 0) 
		{
			cout << setfill(' ') << setw(NUMDIGITS) << format_number(fileSize);		
			print_indent(u_level);
			cout << "<" << numFiles << " files>" << endl;	
		}
		if(numSymlinks != 0) 
		{
			cout << "                    ";		
			print_indent(u_level);
			cout << "<" << numSymlinks << " symlinks>" << endl;	
		}
	}
	
	if(u_level == 0 && !pFss_info.u_apply_filter) 
	{			
		pFss_info.u_files_in_path = numFiles;
		pFss_info.u_dirs_in_path = numDirs;
		pFss_info.u_symlinks_in_path = numSymlinks;
		
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
	fss_info.u_total_symlinks		= 0;
	fss_info.u_total_dirs			= 0;
	fss_info.u_inaccessible_file	= 0;
	fss_info.u_inaccessible_dir		= 0;
	fss_info.u_invalid_entry		= 0;
	fss_info.u_dirs_in_path			= 0;
	fss_info.u_files_in_path		= 0;	
	fss_info.u_symlinks_in_path		= 0;	
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


	cout << endl;
	
	if(!fss_info.u_apply_filter)
	{	
		cout << "Statistics of " << absolute(p).string() << endl;
		
		cout << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_file_size);			
		cout << "\t<" << format_number(fss_info.u_files_in_path) << " files>" << endl;		
		
		cout << "                   -";			
		cout << "\t<" << format_number(fss_info.u_symlinks_in_path) << " symlinks>" << endl;		
		
		cout << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_dir_size);			
		cout << "\t<" << format_number(fss_info.u_dirs_in_path) << " sub-directories>" << endl;		
		
		cout << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_file_size+fss_info.u_total_dir_size);			
		cout << "\t<" << format_number(fss_info.u_files_in_path+fss_info.u_dirs_in_path+fss_info.u_symlinks_in_path) << " total entries>" << endl;		
		
		cout << "\nIncluding sub-directories" << endl;
	
		cout << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_files);		
		cout << "\tfiles";
		if(fss_info.u_inaccessible_file != 0) 
			cout << " (" << format_number(fss_info.u_inaccessible_file) << " inaccessible)";	
		
		cout << "\n" << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_symlinks);		
		cout << "\tsymlinks";
		
		cout << "\n" << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_dirs);			
		cout << "\tdirectories";
		if(fss_info.u_inaccessible_dir != 0) 
			cout << " (" << format_number(fss_info.u_inaccessible_dir) << " inaccessible)";	
	}	
	else 
	{
		if(fss_info.u_show_file)
		{
			if(fss_info.u_files_in_path != 0) 
			{			
				cout << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_file_size);			
				cout << "\t<" << fss_info.u_files_in_path << " files found>" << endl;		
			}	
			else 			
				cout << "\t\t\tNo files found with given filter" << endl;
			
			if(fss_info.u_symlinks_in_path != 0) 
			{			
				cout << "                    ";			
				cout << "\t<" << fss_info.u_symlinks_in_path << " symlinks found>" << endl;		
			}	
			else 				
				cout << "\t\t\tNo symlinks found with given filter" << endl;
		}									
		
		if(fss_info.u_dirs_in_path != 0) 
		{
			cout << setfill(' ') << setw(NUMDIGITS) << format_number(fss_info.u_total_dir_size);			
			cout << "\t<" << fss_info.u_dirs_in_path << " directories found>" << endl;				
		}
		else 		
			cout << "\t\t\tNo directories found with given filter" << endl;
	}
}
